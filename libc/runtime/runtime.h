#ifndef COSMOPOLITAN_LIBC_RUNTIME_RUNTIME_H_
#define COSMOPOLITAN_LIBC_RUNTIME_RUNTIME_H_
#if !(__ASSEMBLER__ + __LINKER__ + 0)
COSMOPOLITAN_C_START_
/*───────────────────────────────────────────────────────────────────────────│─╗
│ cosmopolitan § runtime                                                   ─╬─│┼
╚────────────────────────────────────────────────────────────────────────────│*/

typedef long jmp_buf[8] aligned(CACHELINE);

extern int g_argc;                               /* CRT */
extern char **g_argv;                            /* CRT */
extern char **environ;                           /* CRT */
extern unsigned long *g_auxv;                    /* CRT */
extern jmp_buf g_winmain;                        /* CRT */
extern char *program_invocation_name;            /* RII */
extern char *program_invocation_short_name;      /* RII */
extern uint64_t g_syscount;                      /* RII */
extern const uint64_t kStartTsc;                 /* RII */
extern const char kTmpPath[];                    /* RII */
extern const char kNtSystemDirectory[];          /* RII */
extern const char kNtWindowsDirectory[];         /* RII */
extern unsigned char _base[] aligned(PAGESIZE);  /* αpε */
extern unsigned char _ehead[] aligned(PAGESIZE); /* αpε */
extern unsigned char _etext[] aligned(PAGESIZE); /* αpε */
extern unsigned char _edata[] aligned(PAGESIZE); /* αpε */
extern unsigned char _end[] aligned(PAGESIZE);   /* αpε */
extern unsigned char _ereal;                     /* αpε */
extern unsigned char __privileged_start;         /* αpε */
extern unsigned char __test_start;               /* αpε */
extern unsigned char __ro;                       /* αpε */
extern unsigned char *__relo_start[];            /* αpε */
extern unsigned char *__relo_end[];              /* αpε */
extern uint8_t __zip_start[];                    /* αpε */
extern uint8_t __zip_end[];                      /* αpε */

long missingno();
void mcount(void);
unsigned long getauxval(unsigned long);
void *mapanon(size_t) vallocesque attributeallocsize((1));
int setjmp(jmp_buf) libcesque returnstwice paramsnonnull();
void longjmp(jmp_buf, int) libcesque noreturn paramsnonnull();
void exit(int) noreturn;
void quick_exit(int) noreturn;
void _exit(int) libcesque noreturn;
void _Exit(int) libcesque noreturn;
long _setstack(void *, void *, ...);
void abort(void) noreturn noinstrument;
void panic(void) noreturn noinstrument privileged;
void triplf(void) noreturn noinstrument privileged;
int __cxa_atexit(void *, void *, void *) libcesque;
int atfork(void *, void *) libcesque;
int atexit(void (*)(void)) libcesque;
void free_s(void *) paramsnonnull() libcesque;
int close_s(int *) paramsnonnull() libcesque;
char *getenv(const char *) paramsnonnull() nosideeffect libcesque;
int putenv(char *) paramsnonnull();
int setenv(const char *, const char *, int) paramsnonnull();
int unsetenv(const char *);
int clearenv(void);
void __fast_math(void);
void fpreset(void);
void savexmm(void *);
void loadxmm(void *);
void peekall(void);
int issetugid(void);
void weakfree(void *) libcesque;
bool isheap(void *);
void *mmap(void *, uint64_t, int32_t, int32_t, int32_t, int64_t);
void *mremap(void *, uint64_t, uint64_t, int32_t, void *);
int munmap(void *, uint64_t);
int mprotect(void *, uint64_t, int) privileged;
int msync(void *, size_t, int);

/*───────────────────────────────────────────────────────────────────────────│─╗
│ cosmopolitan § runtime » optimizations                                   ─╬─│┼
╚────────────────────────────────────────────────────────────────────────────│*/

#define _exit(RC) _Exit(RC)

COSMOPOLITAN_C_END_
#endif /* !(__ASSEMBLER__ + __LINKER__ + 0) */
#endif /* COSMOPOLITAN_LIBC_RUNTIME_RUNTIME_H_ */
