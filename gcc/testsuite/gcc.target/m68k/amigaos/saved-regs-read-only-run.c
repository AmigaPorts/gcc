/* Amiga register-parameter ABI: functions that only read their arguments
   in call-saved registers do not save them, and a forwarder that passes
   them on unchanged, long or short, tail-jumps.  The caller's own values in d2, d3 and a2
   must survive all of it, as must a value written into part of an
   argument register.  Result-based, so valid at every -O level.  */

/* { dg-do run } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */

extern void abort (void);

__attribute__ ((noipa)) long
take3 (long a __asm ("d2"), long b __asm ("d3"), long c __asm ("a2"))
{
  return a * 10000 + b * 100 + c;
}

__attribute__ ((noipa)) long
forward3 (long a __asm ("d2"), long b __asm ("d3"), long c __asm ("a2"))
{
  return take3 (a, b, c);
}

__attribute__ ((noipa)) long
take_short (short w __asm ("d2"), unsigned char m __asm ("d4"))
{
  return w * 1000L + m;
}

__attribute__ ((noipa)) long
forward_short (short w __asm ("d2"), unsigned char m __asm ("d4"))
{
  return take_short (w, m);
}

__attribute__ ((noipa)) long
ident (long a __asm ("d2"))
{
  return a;
}

/* Writes part of d2 from d2: on a 68020, a bfins.  */
__attribute__ ((noipa)) long
insert_byte (long a __asm ("d2"))
{
  a = (a & 0x00ffffffL) | (a << 24);
  return ident (a);
}

int
main (void)
{
  long a = 1, b = 2, c = 3;

  /* Live across the calls in the registers the callees must keep.  */
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (take3 (12, 34, 56) != 123456)
    abort ();
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (forward3 (12, 34, 56) != 123456)
    abort ();
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (forward_short (-12, 200) != -11800)
    abort ();
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (insert_byte (0x12345678L) != 0x78345678L)
    abort ();
  __asm__ volatile ("" : "+d" (a), "+d" (b), "+a" (c));
  if (a != 1 || b != 2 || c != 3)
    abort ();
  return 0;
}
