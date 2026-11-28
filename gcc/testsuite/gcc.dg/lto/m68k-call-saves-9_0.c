/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* Each entry runs the loop, but an iteration seldom has the call: the
   probability to reach the call is far below one.  */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*reach 0\.[0-4][^\n]*reject 1} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  do
    {
      if (__builtin_expect_with_probability ((event & 255) == 0, 1, 0.1))
	event = update (state, event);
      event >>= 1;
    }
  while (__builtin_expect_with_probability (event != 0, 1, 0.5));
  return event;
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
