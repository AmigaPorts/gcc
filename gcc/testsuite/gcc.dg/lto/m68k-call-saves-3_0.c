/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fdump-ipa-inline-details } } } */
/* The function splitter makes tick a test and a call to tick.part.0.
   Merging them again would give the early return the saves of the part.  */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> tick.part.0: [^\n]*reject 1} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event, unsigned a, unsigned b)
{
  if (event & 255)
    return event + 1;
  return update (state, event) + a * state[1] + b;
}

static unsigned (*volatile callback) (unsigned *, unsigned, unsigned,
				      unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256, 3, 4) == 0;
}
