/* { dg-do compile } */
/* { dg-options "-O2 -m68060" } */

struct block256 { unsigned short v[128]; };
void copy256 (struct block256 *d, const struct block256 *s) { *d = *s; }

/* Larger copies retain the original policy. */
/* { dg-final { scan-assembler "memcpy" } } */
