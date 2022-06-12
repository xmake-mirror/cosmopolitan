/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2022 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/assert.h"
#include "libc/bits/atomic.h"
#include "libc/calls/calls.h"
#include "libc/dce.h"
#include "libc/intrin/pthread.h"
#include "libc/nexgen32e/threaded.h"
#include "libc/sysv/consts/futex.h"

/**
 * Releases mutex.
 */
noasan noubsan int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  int owner;
  bool shouldunlock;
  assert(mutex->reent > 0);
  shouldunlock = --mutex->reent <= 0;
  if (__threaded) {
    assert(mutex->owner == gettid());
    if (shouldunlock) {
      atomic_store_explicit(&mutex->owner, 0, memory_order_relaxed);
      if (IsLinux() &&
          atomic_load_explicit(&mutex->waits, memory_order_acquire)) {
        futex((void *)&mutex->owner, FUTEX_WAKE, 1, 0, 0);
      }
    }
  }
  return 0;
}
