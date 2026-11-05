/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548" } */
/* { dg-final { scan-assembler-times "\\tevmergehi " 1 } } */
/* { dg-final { scan-assembler-times "\\tevmergehilo " 1 } } */
/* { dg-final { scan-assembler-times "\\tevmergelohi " 1 } } */
/* { dg-final { scan-assembler-times "\\tevmergelo " 3 } } */
/* { dg-final { scan-assembler-not "\\t(stw|lwz|evldd|evstdd)" } } */

typedef float v2sf __attribute__((vector_size(8)));
v2sf hi (v2sf a, v2sf b) { return __builtin_shufflevector (a, b, 0, 2); }
v2sf hilo (v2sf a, v2sf b) { return __builtin_shufflevector (a, b, 0, 3); }
v2sf lohi (v2sf a, v2sf b) { return __builtin_shufflevector (a, b, 1, 2); }
v2sf lo (v2sf a, v2sf b) { return __builtin_shufflevector (a, b, 1, 3); }
v2sf pair (float a, float b) { return (v2sf) { a, b }; }
v2sf splat (float a) { return (v2sf) { a, a }; }
