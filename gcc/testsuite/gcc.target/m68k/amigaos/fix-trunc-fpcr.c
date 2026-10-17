/* Derived from OpenAmigaGCC's FPCR reproducer.  */
/* Copyright (C) 2026 Dalsin Limited.  */
/* Saving FPCR must not overwrite the register indexing the destination.  */
/* Volamos runs a 68020 without an FPU.  Run this on FPU hardware separately.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881" } */
/* { dg-final { scan-assembler "fmovem\\.l fpcr" } } */
/* { dg-final { scan-assembler "fmove\\.l fp0," } } */

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
