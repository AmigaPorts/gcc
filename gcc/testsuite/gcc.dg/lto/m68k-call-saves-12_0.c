/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* The callback has two calls on each path.  With the helper inlined, the
   wrapper is the only call; with the wrapper inlined too, the callback has
   a return path without a call.  */
/* { dg-final { scan-wpa-ipa-dump {Inlined note[^\n]* into tick} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {Inlined filter[^\n]* into tick} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*reject 1} "inline" } } */

extern unsigned note (unsigned);
extern unsigned filter (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  event = note (event);
  return filter (state, event);
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
