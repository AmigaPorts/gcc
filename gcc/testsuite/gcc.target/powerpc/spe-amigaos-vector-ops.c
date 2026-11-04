/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548" } */
/* { dg-final { scan-assembler-times "\\tevfsadd " 1 } } */
/* { dg-final { scan-assembler-times "\\tevfssub " 1 } } */
/* { dg-final { scan-assembler-times "\\tevfsmul " 1 } } */
/* { dg-final { scan-assembler-times "\\tevfsdiv " 1 } } */
/* { dg-final { scan-assembler-times "\\tevfsneg " 1 } } */
/* { dg-final { scan-assembler-not "\\t(stw|lwz)" } } */

typedef float v2sf __attribute__((vector_size(8)));
v2sf add (v2sf a, v2sf b) { return a + b; }
v2sf sub (v2sf a, v2sf b) { return a - b; }
v2sf mul (v2sf a, v2sf b) { return a * b; }
v2sf div (v2sf a, v2sf b) { return a / b; }
v2sf neg (v2sf a) { return -a; }
