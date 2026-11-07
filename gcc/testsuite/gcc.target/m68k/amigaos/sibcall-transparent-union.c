/* Amiga register-parameter ABI: a transparent union is passed as its
   first field, here a pointer in a0, so the -mregparm arguments after it
   start at d0 and the last one goes in d2.  The sibcall check must place
   them the same way: d2 holds a, not the 999 the callee reads.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fomit-frame-pointer -mregparm=3" } */

typedef union __attribute__ ((transparent_union)) { void *p; long l; } U;
long callee (U u0, long x1, long x2, long x3);

long
caller (void *p __asm ("a0"), long a __asm ("d2"))
{
  return callee (p, 11, a, 999);
}

/* { dg-final { scan-assembler "#999,d2" } } */
/* { dg-final { scan-assembler-not "jra _callee" } } */
