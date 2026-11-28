/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* A saveallregs function saves all registers in any case.  The inlined
   wrapper must not make its remaining call a candidate.  */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tick -> update} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {Inlined filter[^\n]* into tick} "inline" } } */

extern unsigned filter (unsigned *, unsigned);

static unsigned __attribute__ ((saveallregs))
tick (unsigned *state, unsigned event)
{
  return filter (state, event);
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
