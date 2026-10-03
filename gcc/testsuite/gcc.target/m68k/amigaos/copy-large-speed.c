/* { dg-do compile } */
/* { dg-options "-O2 -m68060" } */

struct block8k { unsigned int v[2048]; };
void copy8k (struct block8k *d, const struct block8k *s) { *d = *s; }

/* Beyond the expander's loop limit (63 iterations of 16 moves at -O2,
   4 KB) the copy goes to memcpy. */
/* { dg-final { scan-assembler "memcpy" } } */
