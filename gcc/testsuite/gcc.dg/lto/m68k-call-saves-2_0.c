/* { dg-lto-do link } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -mno-ipa-callback-saves -fdump-ipa-inline-details } } } */
/* -mno-ipa-callback-saves leaves the inline decision alone.  */
/* { dg-final { scan-wpa-ipa-dump-not {IPA call saves tick -> update: [^\n]*reject 1} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump {to be inlined into tick/} "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (event & 255)
    return event + 1;
  return update (state, event);
}

static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  return callback (state, 256) == 0;
}
