/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ This program is free software; you can redistribute it and/or modify         │
│ it under the terms of the GNU General Public License as published by         │
│ the Free Software Foundation; version 2 of the License.                      │
│                                                                              │
│ This program is distributed in the hope that it will be useful, but          │
│ WITHOUT ANY WARRANTY; without even the implied warranty of                   │
│ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU             │
│ General Public License for more details.                                     │
│                                                                              │
│ You should have received a copy of the GNU General Public License            │
│ along with this program; if not, write to the Free Software                  │
│ Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA                │
│ 02110-1301 USA                                                               │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/log/check.h"
#include "libc/macros.h"
#include "libc/mem/mem.h"
#include "libc/str/str.h"
#include "libc/sysv/errfuns.h"
#include "tool/build/lib/memory.h"
#include "tool/build/lib/pml4t.h"

static int64_t MakeAddress(unsigned short a[4]) {
  uint64_t x;
  x = 0;
  x |= a[0];
  x <<= 9;
  x |= a[1];
  x <<= 9;
  x |= a[2];
  x <<= 9;
  x |= a[3];
  x <<= 12;
  return SignExtendAddr(x);
}

static uint64_t *GetPageTable(pml4t_t p, long i, void *NewPhysicalPage(void)) {
  uint64_t *res;
  DCHECK_ALIGNED(4096, p);
  DCHECK(0 <= i && i < 512);
  if (IsValidPage(p[i])) {
    res = UnmaskPageAddr(p[i]);
  } else if ((res = NewPhysicalPage())) {
    DCHECK_ALIGNED(4096, res);
    p[i] = MaskPageAddr(res) | 0b11;
  }
  return res;
}

static void PtFinder(uint64_t *a, uint64_t *b, uint64_t n, pml4t_t pd, int k) {
  unsigned i;
  uint64_t e, c;
  while (*b - *a < n) {
    i = (*b >> k) & 511;
    e = pd[i];
    c = ROUNDUP(*b + 1, 1 << k);
    if (!IsValidPage(e)) {
      *b = c;
    } else if (k && *b - *a + (c - *b) > n) {
      PtFinder(a, b, n, UnmaskPageAddr(e), k - 9);
    } else {
      *a = *b = c;
    }
    if (((*b >> k) & 511) < i) {
      break;
    }
  }
}

/**
 * Locates free memory range.
 *
 * @param h specifies signedness and around where to start searching
 * @return virtual page address with size bytes free, or -1 w/ errno
 */
int64_t FindPml4t(pml4t_t pml4t, uint64_t h, uint64_t n) {
  uint64_t a, b;
  n = ROUNDUP(n, 4096) >> 12;
  a = b = (h & 0x0000fffffffff000) >> 12;
  if (!n || n > 0x10000000) return einval();
  PtFinder(&a, &b, n, pml4t, 9 * 3);
  if (b > 0x0000001000000000) return eoverflow();
  if (h < 0x0000800000000000 && b > 0x0000000800000000) return eoverflow();
  if (b - a < n) return enomem();
  return a << 12;
}

/**
 * Maps virtual page region to system memory region.
 *
 * @param pml4t is root of 48-bit page tables
 * @param v is fixed page-aligned virtual address, rounded down
 * @param r is real memory address, rounded down
 * @param n is number of bytes needed, rounded up
 * @return 0 on success, or -1 w/ errno
 * @note existing pages are overwritten
 */
int RegisterPml4t(pml4t_t pml4t, int64_t v, int64_t r, size_t n,
                  void *NewPhysicalPage(void)) {
  unsigned i, j, k, l;
  uint64_t *pdpt, *pdt, *pd, u;
  if (!n) return 0;
  u = ROUNDDOWN(r, 4096);
  n = ROUNDUP(n, 4096) >> 12;
  i = (v >> 39) & 511;
  j = (v >> 30) & 511;
  k = (v >> 21) & 511;
  l = (v >> 12) & 511;
  if (u + n > 0x800000000000) return eoverflow();
  if (r + n > 0x800000000000) return eoverflow();
  for (; i < 512; ++i) {
    if (!(pdpt = GetPageTable(pml4t, i, NewPhysicalPage))) return -1;
    for (; j < 512; ++j) {
      if (!(pdt = GetPageTable(pdpt, j, NewPhysicalPage))) return -1;
      for (; k < 512; ++k) {
        if (!(pd = GetPageTable(pdt, k, NewPhysicalPage))) return -1;
        for (; l < 512; ++l) {
          pd[l] = MaskPageAddr(u) | 0b11;
          if (!--n) return 0;
          u += 4096;
        }
        l = 0;
      }
      k = 0;
    }
    j = 0;
  }
  return enomem();
}

/**
 * Unmaps pages and frees page tables.
 */
int FreePml4t(pml4t_t pml4t, int64_t addr, uint64_t size,
              void FreePhysicalPageTable(void *),
              int FreePhysicalPages(void *, size_t)) {
  int rc;
  char *pages;
  uint64_t i, *pdpt, *pdt, *pd;
  unsigned short r, s[4], a[4], R[2][2] = {{256, 512}, {0, 256}};
  a[0] = addr >> 39;
  a[1] = addr >> 30;
  a[2] = addr >> 21;
  a[3] = addr >> 12;
  size = ROUNDUP(size, 4096) >> 12;
  for (rc = r = 0; r < ARRAYLEN(R); ++r) {
    for (a[0] &= 511; size && R[r][0] <= a[0] && a[0] < R[r][1]; ++a[0]) {
      if (!IsValidPage(pml4t[a[0]])) continue;
      pdpt = UnmaskPageAddr(pml4t[a[0]]);
      for (s[1] = (a[1] &= 511); size && a[1] < 512; ++a[1]) {
        if (!IsValidPage(pdpt[a[1]])) continue;
        pdt = UnmaskPageAddr(pdpt[a[1]]);
        for (s[2] = (a[2] &= 511); size && a[2] < 512; ++a[2]) {
          if (!IsValidPage(pdt[a[2]])) continue;
          pd = UnmaskPageAddr(pdt[a[2]]);
          for (s[3] = (a[3] &= 511); size && a[3] < 512; ++a[3]) {
            if (IsValidPage(pd[a[3]])) {
              pages = UnmaskPageAddr(pd[a[3]]);
              pd[a[3]] = 0;
              for (i = 1; i + 1 < size && a[3] + i < 512; ++i) {
                if (!IsValidPage(pd[a[3] + i])) break;
                if (UnmaskPageAddr(pd[a[3] + i]) != pages + i * 4096) break;
                pd[a[3] + i] = 0;
              }
              FreePhysicalPages(pages, i * 4096);
              a[3] += i - 1;
              size -= i;
            }
          }
          if (s[3] == 0 && a[3] == 512) {
            FreePhysicalPageTable(pd);
            pdt[a[2]] = 0;
          }
        }
        if (s[2] == 0 && a[2] == 512) {
          FreePhysicalPageTable(pdt);
          pdpt[a[1]] = 0;
        }
      }
      if (s[1] == 0 && a[1] == 512) {
        FreePhysicalPageTable(pdpt);
        pml4t[a[0]] = 0;
      }
    }
  }
  return 0;
}
