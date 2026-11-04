/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O3 -mcpu=8548 -fdump-tree-vect-details" } */
/* { dg-final { scan-tree-dump-times "LOOP VECTORIZED" 2 "vect" } } */
/* { dg-final { scan-assembler "\\tevfsadd " } } */
/* { dg-final { scan-assembler "\\tevaddw " } } */

void
add_float (float *restrict out, const float *restrict a,
	   const float *restrict b, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    out[i] = a[i] + b[i];
}

void
add_int (int *restrict out, const int *restrict a,
	 const int *restrict b, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    out[i] = a[i] + b[i];
}
