/* { dg-do compile } */
/* { dg-options "-Os -m68060" } */

struct block2k { unsigned int v[512]; };
void copy2k (struct block2k *d, const struct block2k *s) { *d = *s; }

/* Beyond the expander's loop limit (63 iterations of 4 moves at -Os,
   about 1 KB) the copy goes to memcpy. */
/* { dg-final { scan-assembler "memcpy" } } */
