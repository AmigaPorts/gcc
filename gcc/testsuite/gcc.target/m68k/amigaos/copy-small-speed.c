/* { dg-do compile } */
/* { dg-options "-O2 -m68060" } */

struct block60 { unsigned short v[30]; };
struct block96 { unsigned short v[48]; };
struct block128 { unsigned short v[64]; };

void copy60 (struct block60 *d, const struct block60 *s) { *d = *s; }
void copy96 (struct block96 *d, const struct block96 *s) { *d = *s; }
void copy128 (struct block128 *d, const struct block128 *s) { *d = *s; }

/* { dg-final { scan-assembler-not "memcpy" } } */
