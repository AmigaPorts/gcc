/* { dg-do compile } */
/* { dg-options "-Os -m68060" } */

struct block96 { unsigned short v[48]; };
void copy96 (struct block96 *d, const struct block96 *s) { *d = *s; }

/* The speed-only policy must not force this copy inline at -Os. */
/* { dg-final { scan-assembler "memcpy" } } */
