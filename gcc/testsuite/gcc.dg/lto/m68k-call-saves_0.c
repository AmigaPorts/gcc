/* { dg-lto-do run } */
/* { dg-require-effective-target classic_m68k } */
/* { dg-require-effective-target int32 } */
/* { dg-lto-options { { -Os -flto -fdump-ipa-inline-details } } } */
/* { dg-final { scan-wpa-ipa-dump {IPA call saves tick -> update: [^\n]*reject 1} "inline" } } */
/* { dg-final { scan-wpa-ipa-dump "inlining adds register saves on a path that bypasses this call" "inline" } } */

extern unsigned update (unsigned *, unsigned);

static unsigned
tick (unsigned *state, unsigned event)
{
  if (event & 255)
    return event + 1;
  return update (state, event);
}

/* Model a callback kept in a writable dispatch slot. */
static unsigned (*volatile callback) (unsigned *, unsigned) = tick;

int
main (void)
{
  unsigned state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  unsigned sum = 0;
  for (unsigned i = 0; i < 100000; ++i)
    sum += callback (state, i);
  if (sum != 0x5bada84bU)
    __builtin_abort ();
  return 0;
}
