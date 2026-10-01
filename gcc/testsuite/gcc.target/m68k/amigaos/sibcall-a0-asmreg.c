/* A sibcall's target is loaded into a0 (STATIC_CHAIN_REGNUM) unless it is a
   direct call that m68k_symbolic_jump can branch to.
   An __asm ("a0") parameter is passed in a0 without -mregparm,
   so such a call must not become a sibcall: the target would replace the
   argument.  A call with nothing in a0 stays a sibcall.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -fomit-frame-pointer" } */

typedef unsigned long ulong;

typedef ulong (*afn) (char *p __asm ("a0"), ulong d __asm ("d0"));
extern ulong ext_asm (char *p __asm ("a0"), ulong d __asm ("d0"));

ulong
ind_asm (afn f, char *p, ulong d)
{
  return f (p, d);
}

ulong
dir_asm (char *p, ulong d)
{
  return ext_asm (p, d);
}

/* { dg-final { scan-assembler-not "jmp \\(a0\\)" } } */
/* { dg-final { scan-assembler-times "jsr \\(a1\\)" 1 } } */
/* { dg-final { scan-assembler-times "jra _ext_asm" 1 } } */
