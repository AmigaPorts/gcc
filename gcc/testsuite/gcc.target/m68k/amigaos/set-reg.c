/* { dg-do compile } */
/* { dg-options "-O2 -m68000" } */

/* For speed the fill value stays in a register, in the loop too: that is
   two bytes against six, and 12 cycles against 20 on the 68000.  */
void fill128 (void *d) { __builtin_memset (__builtin_assume_aligned (d, 4), 0xA5, 128); }

/* Which also beats clr.  */
void clear128 (void *d) { __builtin_memset (__builtin_assume_aligned (d, 4), 0, 128); }

/* Byte by byte, where the block may start at an odd address, too.  */
void fillbytes (char *d) { __builtin_memset (d, 0xA5, 128); }
void clearbytes (char *d) { __builtin_memset (d, 0, 128); }

/* { dg-final { scan-assembler-times "dbra" 4 } } */
/* { dg-final { scan-assembler-not "#-1515870811,\\(" } } */
/* { dg-final { scan-assembler-not "#-91,\\(" } } */
/* { dg-final { scan-assembler-not "clr\\.\[bl\] \\(" } } */
