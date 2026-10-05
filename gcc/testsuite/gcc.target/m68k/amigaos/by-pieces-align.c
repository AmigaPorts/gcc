/* { dg-do compile } */
/* { dg-options "-Os" } */
/* Reduced from c-c++-common/torture/builtin-clear-padding-2.c.  The
   by-pieces size limit scaled with the alignment in bits, so a 16384-byte
   aligned struct turned a 48 KB memset into 24577 stores and the compile
   ran for minutes.  Alignment beyond a long must not buy more pieces. */

typedef int T __attribute__((aligned (16384)));
struct S { char a; short b; T d; T e; long long f; };

void clear (struct S *s)
{
  __builtin_memset (s, 0, sizeof (*s));
}

/* { dg-final { scan-assembler "memset" } } */
