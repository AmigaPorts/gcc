/* A sibcall's target is loaded into a0 (STATIC_CHAIN_REGNUM) unless it is a
   direct call that m68k_symbolic_jump can branch to.
   Under -mregparm=3 a pointer argument is passed in a0,
   so such a call must not become a sibcall: the target would replace the
   argument.  A call with nothing in a0 stays a sibcall.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -fomit-frame-pointer -mregparm=3" } */

typedef unsigned long ulong;

typedef ulong (*pfn) (void *arg, ulong wait);
typedef ulong (*ifn) (ulong x);
extern ulong ext_ptr (void *p);

ulong
ind_ptr (void *base, ulong wait, pfn call, void *arg)
{
  (void) base;
  return call (arg, wait);
}

ulong
ind_int (ulong x, ifn f)
{
  return f (x);
}

ulong
dir_ptr (void *p)
{
  return ext_ptr (p);
}

/* ind_ptr passes ARG in a0 and calls; ind_int still jumps through a0, and
   the direct call still branches.  */
/* { dg-final { scan-assembler-times "move\\.l a2,a0" 1 } } */
/* { dg-final { scan-assembler-times "jmp \\(a0\\)" 1 } } */
/* { dg-final { scan-assembler-times "jra _ext_ptr" 1 } } */
