/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* The call is in a loop that runs about three times, so its frequency is
   above one, but half of the entries do not reach the loop.  */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*freq [1-9][^\n]*reach 0\.[^\n]*reject 1} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (__builtin_expect_with_probability ((event & 255) != 0, 1, 0.5))
    return event + 1;
  do
    event = update (state, event);
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
