/* { dg-do compile } */
/* { dg-options "-Os -m68060" } */

struct block96 { unsigned short v[48]; };
void copy96 (struct block96 *d, const struct block96 *s) { *d = *s; }

/* At -Os the expander emits a four-move dbra loop (18 bytes), which is
   smaller than pushing three arguments and calling memcpy (22 bytes). */
/* { dg-final { scan-assembler-not "memcpy" } } */
/* { dg-final { scan-assembler "dbra" } } */
