/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* The wrapper is called in a loop that runs about three times, on half
   of the entries; its call is on about half of its runs.  The call is
   reached when one of the runs reaches it: more than half of 0.5, the
   product of the two probabilities.  */
/* { dg-final { scan-wpa-ipa-dump {Inlined filter[^\n]* into tick} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*reach 0\.4[^\n]*reject 1} "inline" } } */

extern unsigned filter (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (__builtin_expect_with_probability ((event & 255) != 0, 1, 0.5))
    return event + 1;
  do
    event = filter (state, event);
  while (__builtin_expect_with_probability ((event & 3) != 0, 1, 0.7));
  return event;
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
