/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
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
#include "libc/calls/calls.h"
#include "libc/calls/syscall-sysv.internal.h"
#include "libc/calls/syscall_support-sysv.internal.h"
#include "libc/calls/termios.h"
#include "libc/dce.h"
#include "libc/intrin/strace.internal.h"
#include "libc/sysv/consts/pty.h"
#include "libc/sysv/errfuns.h"

#define TIOCSPTLCK  0x40045431  // linux
#define TIOCPTYUNLK 0x20007452  // xnu

/**
 * Unlocks pseudoteletypewriter pair.
 *
 * @return 0 on success, or -1 w/ errno
 * @raise EBADF if fd isn't open
 * @raise EINVAL if fd is valid but not associated with pty
 */
int unlockpt(int fd) {
  int rc, unlock = 0;
  if (IsFreebsd() || IsOpenbsd() || IsNetbsd()) {
    rc = _isptmaster(fd);
  } else if (IsXnu()) {
    rc = sys_ioctl(fd, TIOCPTYUNLK);
  } else if (IsLinux()) {
    rc = sys_ioctl(fd, TIOCSPTLCK, &unlock);
  } else {
    rc = enosys();
  }
  STRACE("unlockpt(%d) → %d% m", fd, rc);
  return rc;
}
