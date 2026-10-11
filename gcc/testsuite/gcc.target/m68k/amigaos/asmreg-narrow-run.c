/* Asmreg arguments narrower than int, passed in their declared mode: the
   callee must see the low part of a wider value, whether it comes from a
   register or from memory, and a signed short must keep its sign.  */
/* { dg-do run } */

typedef unsigned short UWORD;
typedef unsigned char UBYTE;

int gi = 0x00012345;
long long gll = 0x0000000100002345LL;

__attribute__ ((noipa)) UWORD
fw (register UWORD x __asm("d1"))
{
  return x;
}

__attribute__ ((noipa)) UBYTE
fb (register UBYTE x __asm("d2"))
{
  return x;
}

__attribute__ ((noipa)) int
fwi (register UWORD x __asm("d1"), register int y __asm("d3"))
{
  return x + y;
}

__attribute__ ((noipa)) int
fsw (register short x __asm("d1"), int z)
{
  return x * z;
}

int
main (void)
{
  if (fw (gi) != 0x2345)
    __builtin_abort ();
  if (fb (gi) != 0x45)
    __builtin_abort ();
  if (fw (gll) != 0x2345)
    __builtin_abort ();
  if (fwi (gi, 1) != 0x2346)
    __builtin_abort ();
  if (fsw (-3, gi) != -3 * 0x12345)
    __builtin_abort ();
  return 0;
}
