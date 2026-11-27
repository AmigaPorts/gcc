/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* __builtin_popcount is a library call on m68k, so the callback has no
   return path without a call, and inlining adds saves to no such path.  */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tick -> update} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (event & 255)
    return __builtin_popcount (event);
  return update (state, event);
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
