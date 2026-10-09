/* A -fbaserel program on newlib must start, clear its bss and run its
   constructors.  newlib's crt0 walks __INIT_LIST__ and __CTOR_LIST__, which
   it defines in .list_* sections that the linker puts in the text hunk; the
   baserel pass read them through a4, which the linker now refuses.  The
   same crt0 also cleared bss by a size read from the wrong place and
   overwrote memory past it, so the program never reached main.  Read-only
   data lives in the text hunk too and must not go through a4.  amigaos.exp
   runs this file with -mcrt=newlib in place of the board's runtime.  */

/* { dg-do run } */
/* { dg-skip-if "amiga baserel" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -fbaserel" } */

static volatile int near_flag;
static volatile __far int far_flag;
/* volatile: a never-written static array is known to be zero, and the
   check below would otherwise be folded away.  */
static volatile int zeroed[64];
static const int ro[4] = { 0x11, 0x22, 0x33, 0x44 };
static volatile int idx = 1;

__attribute__((constructor)) static void
init (void)
{
  far_flag = 7;
  near_flag = 42;
}

int
main (void)
{
  for (int i = 0; i < 64; i++)
    if (zeroed[i])
      return 1;
  if (far_flag != 7)
    return 2;
  if (near_flag != 42)
    return 3;
  if (ro[idx] != 0x22 || ro[idx + 2] != 0x44)
    return 4;
  return 0;
}
