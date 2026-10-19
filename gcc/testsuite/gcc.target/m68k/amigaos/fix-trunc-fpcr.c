/* Derived from OpenAmigaGCC's FPCR reproducer.  */
/* Copyright (C) 2026 Dalsin Limited.  */
/* Saving FPCR must not overwrite the register indexing the destination.
   The 68040 sequence saves FPCR to a scratch register, switches the
   rounding mode, stores the result and restores FPCR; the scratch
   registers must not share a register with the destination address.
   The simulator board has no FPU, so this is a compile test: the first
   scan checks the 68040 sequence is emitted, the second that the
   register FPCR was saved to is not an index of the store that follows.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881" } */
/* { dg-final { scan-assembler "fmovem\\.l fpcr" } } */
/* { dg-final { scan-assembler-not {fmovem\.l fpcr,(d[0-7])\n(?:[^\n]*\n){4}[^\n]*fmove\.[lwb] fp[0-7],\([^)]*\1\M} } } */

static int out[256];
static float val[64];
static unsigned char index[64];

__attribute__((noinline, noclone)) static void
scatter (int *dest, const unsigned char *where, const float *src, int count)
{
  for (int i = 0; i < count; i++)
    dest[*where++] = (int) src[i];
}

int
main (void)
{
  for (int i = 0; i < 64; i++)
    {
      index[i] = i * 3;
      val[i] = i + 0.75f;
    }
  scatter (out, index, val, 64);
  for (int i = 0; i < 64; i++)
    if (out[i * 3] != i)
      __builtin_abort ();
  return 0;
}
