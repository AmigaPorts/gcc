/* Frame registers cannot hold explicit incoming arguments. */
/* { dg-do compile } */
/* { dg-skip-if "Amiga register parameters" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -fomit-frame-pointer" } */
void bad_a5 (unsigned long *p __asm("a5")) /* { dg-error "reserved for the frame or stack pointer" } */
{ p[0] = 1; }
