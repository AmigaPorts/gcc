/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fno-partial-inlining -fdump-ipa-inline-details } } } */
/* The callee alone stays below the MOVEM threshold.  With the values of
   the caller that are live across the call it does not.  The numbers come
   through the LTO stream of two objects.  */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> step: [^\n]* -> [12] live [1-9][^\n]*reject 1} "inline" } } */

extern unsigned step (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event, unsigned a, unsigned b)
{
  if (event & 255)
    return event + 1;
  return step (state, event) * a;
}

static unsigned (*volatile callback) (unsigned *, unsigned, unsigned,
				      unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256, 3, 4) == 0;
}
