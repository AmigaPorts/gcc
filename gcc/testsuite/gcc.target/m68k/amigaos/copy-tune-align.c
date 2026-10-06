/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -mtune=68020-60" } */
/* The byte path of the block move and set expanders used to depend on
   -mtune: with a 68020 tune the code below got longword moves through
   byte pointers, which fault on the 68000 it is compiled for. */

void copy (char *d, const char *s) { __builtin_memcpy (d, s, 64); }
void clear (char *d) { __builtin_memset (d, 0, 64); }

/* { dg-final { scan-assembler-not {move\.l \(a[0-9]\)\+} } } */
/* { dg-final { scan-assembler-not {clr\.l \(a[0-9]\)\+} } } */
/* { dg-final { scan-assembler {move\.b \(a[0-9]\)\+} } } */
