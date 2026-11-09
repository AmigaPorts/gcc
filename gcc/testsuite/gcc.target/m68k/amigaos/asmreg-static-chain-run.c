/* Amiga register-parameter ABI: a nested function whose argument is a
   long long in a0 and a1, or a -mregparm pointer in a0, gets its static
   chain in another register and still reads both.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -mregparm=2 -fno-ipa-sra" } */

extern void abort (void);

__attribute__ ((noipa)) long long
pair (long long v, long k)
{
  __attribute__ ((noinline)) long long nested (long long x __asm ("a0"))
  {
    return x + k;
  }
  return nested (v);
}

__attribute__ ((noipa)) long
regparm (char *v, long k)
{
  __attribute__ ((noinline)) long nested (char *p) { return *p + k; }
  return nested (v);
}

int
main (void)
{
  char c = 5;

  if (pair (0x100000000LL, 7) != 0x100000007LL)
    abort ();
  if (regparm (&c, 30) != 35)
    abort ();
  return 0;
}
