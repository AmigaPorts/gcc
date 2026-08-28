/* Companion TU for asmreg-run.c: the caller sees only the prototype,
   so it passes the arguments in d0/d1 (the register ABI).  */

typedef struct _F { long x; } F;

struct _F *addfn (long a __asm ("d0"), long b __asm ("d1"));

extern void abort (void);

int
run_test (void)
{
  if ((long) addfn (3, 4) != 7)
    abort ();
  return 0;
}
