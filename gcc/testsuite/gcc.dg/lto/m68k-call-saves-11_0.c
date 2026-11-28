/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* The wrapper has the call in a loop and one value live across it; the
   callback has one more value live across the wrapper, and reaches the
   wrapper on half of its entries.  */
/* { dg-final { scan-wpa-ipa-dump {Inlined filter[^\n]* into tick} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*live 2 [^\n]*freq 1\.[^\n]*reach 0\.5[^\n]*reject 1} "inline" } } */

extern unsigned filter (unsigned *, unsigned);
extern unsigned other (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (__builtin_expect_with_probability ((event & 255) != 0, 1, 0.5))
    return event + 1;
  return filter (state, event) + event;
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;
static unsigned (*volatile callback2) (unsigned *, unsigned) = other;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) + callback2 (state, 256) == 0;
}
