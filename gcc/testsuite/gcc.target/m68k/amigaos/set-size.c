/* { dg-do compile } */
/* { dg-options "-Os -m68000" } */

/* At -Os a block is one store in a dbra loop, up to 4 KB.  */
struct block96 { long v[24]; };
void clear96 (struct block96 *d) { __builtin_memset (d, 0, sizeof *d); }

/* Above that it is a call.  */
struct block8k { long v[2048]; };
void clear8k (struct block8k *d) { __builtin_memset (d, 0, sizeof *d); }

/* { dg-final { scan-assembler-times "clr\\.l" 1 } } */
/* { dg-final { scan-assembler-times "dbra" 1 } } */
/* { dg-final { scan-assembler-times "_memset" 1 } } */
