/* Runtime check of the amiga register-parameter ABI across optimization
   levels.  The callee (below) is defined after a prototype whose return type
   is spelled differently (struct tag vs typedef), so the C front-end
   composites its type -- which used to drop the __asm register bindings, making
   the callee read its arguments from the stack.  The caller lives in a separate
   TU that sees only the prototype and therefore passes the arguments in d0/d1.
   If the bindings are dropped, the callee reads garbage and run_test() aborts.
   This is result-based, so it is valid at every -O level / allocator / CPU.

   amigaos.exp sweeps this over the -O level, allocator and CPU (via
   gcc-dg-runtest with a torture list); the companion TU (asmreg-run-lib.c) is
   pulled in by dg-additional-sources.  The xfail marks the pre-fix behaviour:
   without the m68k backend fix the run aborts, so it is an expected failure;
   the fix commit drops the xfail.  */

/* { dg-do run { xfail m68k-*-amigaos* } } */
/* { dg-additional-sources "asmreg-run-lib.c" } */

typedef struct _F { long x; } F;

struct _F *addfn (long a __asm ("d0"), long b __asm ("d1"));	/* proto: tag */

F *
addfn (long a __asm ("d0"), long b __asm ("d1"))		/* def: typedef */
{
  return (F *) (a + b);
}

extern int run_test (void);

int
main (void)
{
  return run_test ();
}
