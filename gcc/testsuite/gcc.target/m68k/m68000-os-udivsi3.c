/* The 68000 has no 32-bit divide: at -Os doloop must not turn this
   subtraction loop into a call to __udivsi3 to count its iterations.
   AmigaPorts/m68k-amigaos-gcc#421.  */
/* { dg-do compile } */
/* { dg-options "-m68000 -Os -ffreestanding" } */

typedef unsigned long ULONG;
struct tv { ULONG secs, micro; };

void
split (struct tv *tv, ULONG t)
{
  ULONG s = 0;
  while (t >= 50)
    {
      t -= 50;
      s++;
    }
  tv->secs = s;
  tv->micro = t * 20000;
}

/* { dg-final { scan-assembler-not "__udivsi3" } } */
/* { dg-final { scan-assembler-not "__mulsi3" } } */
