/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fdump-ipa-inline-details } } } */
/* A function whose address is not taken keeps the ordinary decision.  */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tick} "inline" } } */

extern unsigned update (unsigned *, unsigned);

__attribute__ ((noinline)) static unsigned
tick (unsigned *state, unsigned event)
{
  if (event & 255)
    return event + 1;
  return update (state, event);
}

int
main (int argc, char **argv)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return tick (state, argc << 8) == 0;
}
