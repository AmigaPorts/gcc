/* sub.w to an address register is suba.w, which leaves the condition codes
   alone, so an HImode compare with 0 after it must still test the
   register: tst.w An, which the 68020 has.  */

/* { dg-do compile } */
/* { dg-options "-O2 -m68020" } */

extern void g (int);

void
f (short b, short c)
{
  register short s __asm ("a0");
  __asm__ volatile ("" : "=a" (s));
  s -= b;
  if (s != 0)
    g (c);
  __asm__ volatile ("" :: "a" (s));
}

/* { dg-final { scan-assembler {sub\.w [^\n]*,a0\n\ttst\.w a0\n} } } */
