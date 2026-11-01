/* { dg-do compile } */
/* { dg-options "-Os -m68060" } */

struct block96 { unsigned short v[48]; };
void copy96 (struct block96 *d, const struct block96 *s) { *d = *s; }

/* A dbra loop needs its pointer and counter setup as well, so -Os
   calls memcpy.  */
/* { dg-final { scan-assembler "memcpy" } } */
/* { dg-final { scan-assembler-not "dbra" } } */
