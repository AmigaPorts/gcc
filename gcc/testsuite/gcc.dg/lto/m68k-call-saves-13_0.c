/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details -m68000 -msoft-float } } } */
/* The 68000 divides 32-bit values with a library call, and so does
   arithmetic without an FPU: the early returns are not free of calls.  */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tick -> update} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tock -> update} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (event & 255)
    return event / state[0];
  return update (state, event);
}

volatile float scale = 1.5f;

static unsigned
tock (unsigned *state, unsigned event)
{
  if (event & 255)
    return scale > 1.0f;
  return update (state, event);
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;
static unsigned (*volatile callback2) (unsigned *, unsigned) = tock;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) + callback2 (state, 256) == 0;
}
