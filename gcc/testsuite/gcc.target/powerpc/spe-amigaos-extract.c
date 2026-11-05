/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548" } */
/* { dg-final { scan-assembler-times "\\tevmergehi " 3 } } */
/* { dg-final { scan-assembler-times "\\tefsadd " 2 } } */
/* { dg-final { scan-assembler-times "\\tevfsmul " 1 } } */
/* { dg-final { scan-assembler-not "\\t(stw|lwz|evldd|evstdd)" } } */

typedef float v2sf __attribute__((vector_size(8)));
float lane0 (v2sf x) { return x[0]; }
float lane1 (v2sf x) { return x[1]; }
float sum (v2sf x) { return x[0] + x[1]; }
float dot (v2sf a, v2sf b) { v2sf x = a * b; return x[0] + x[1]; }
