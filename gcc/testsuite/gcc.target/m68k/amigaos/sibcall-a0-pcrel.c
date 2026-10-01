/* A sibcall's target is loaded into a0 (STATIC_CHAIN_REGNUM) unless it is a
   direct call that m68k_symbolic_jump can branch to.
   With -mpcrel a direct target goes in a register too; under
   -mregparm=3 a pointer argument is in a0,
   so such a call must not become a sibcall: the target would replace the
   argument.  A call with nothing in a0 stays a sibcall.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -fomit-frame-pointer -mregparm=3 -mpcrel" } */

typedef unsigned long ulong;

extern ulong ext_ptr (void *p);
extern ulong ext_int (ulong x);

ulong
dir_ptr (void *p)
{
  return ext_ptr (p);
}

ulong
dir_int (ulong x)
{
  return ext_int (x);
}

/* dir_ptr loads the target into a1 and calls; dir_int still jumps.  */
/* { dg-final { scan-assembler-times "lea \\(_ext_ptr:w,pc\\),a1" 1 } } */
/* { dg-final { scan-assembler-not "lea \\(_ext_ptr:w,pc\\),a0" } } */
/* { dg-final { scan-assembler-times "jmp \\(a0\\)" 1 } } */
