/* libgcc has the 68000's 32-bit multiply and divide helpers, so a program
   without a C library gets them from -lgcc.  AmigaPorts/m68k-amigaos-gcc#421.

   No startup code: the jump goes first, and start's return value is the
   program's exit status.  */

/* { dg-do run } */
/* { dg-skip-if "a4 needs the startup code, and the 68000 has no -fbaserel32" { *-*-* } { "-fbaserel*" "-resident*" } { "" } } */
/* { dg-options "-m68000 -O2 -nostdlib -lgcc" } */

__asm__ ("\t.text\n\tjra _start");

static volatile unsigned long ua = 3000000007UL, ub = 12345;
static volatile long sa = -2000011L, sb = 777;

__attribute__ ((used)) long
start (void)
{
  if (ua * ub != 3000000007UL * 12345UL)
    return 1;
  if (ua / ub != 3000000007UL / 12345UL)
    return 2;
  if (ua % ub != 3000000007UL % 12345UL)
    return 3;
  if (sa * sb != -2000011L * 777L)
    return 4;
  if (sa / sb != -2000011L / 777L)
    return 5;
  if (sa % sb != -2000011L % 777L)
    return 6;
  if (sa / -sb != -2000011L / -777L)
    return 7;
  return 0;
}
