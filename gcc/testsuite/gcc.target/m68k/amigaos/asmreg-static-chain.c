/* Amiga register-parameter ABI: the static chain of a nested function
   goes in an address register that none of its arguments takes, counting
   every register of a multi-register __asm binding and the -mregparm
   registers.  The trampoline loads the same register: movea.l #chain,a1
   is 0x227c (8828), movea.l #chain,a2 is 0x247c (9340).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -mregparm=2 -fomit-frame-pointer" } */

extern void use (void *);

/* A long long in a0 takes a1 as well: the chain goes in a2.  */
void
pair (long k)
{
  long long nested (long long x __asm ("a0")) { return x + k; }
  use (nested);
}

/* A pointer in a0 under -mregparm: the chain goes in a1.  */
void
regparm (long k)
{
  long nested (char *p) { return *p + k; }
  use (nested);
}

/* { dg-final { scan-assembler "#9340," } } */
/* { dg-final { scan-assembler "#8828," } } */
/* { dg-final { scan-assembler-not "#8316," } } */
