/* { dg-do compile } */
/* { dg-options "-O2" } */
/* Identical code folding merges neither an entrypoint nor a saveallregs
   function with a normal one that has the same body, in either order:
   their prologues save other registers.  */

int g[8];
long sum;

__attribute__ ((noipa)) void
use (int x)
{
  sum += x;
}

#define BODY(K)							\
  {								\
    int a = g[0], b = g[1], c = g[2], d = g[3];		\
    int e = g[4], f = g[5], h = g[6], i = g[7];		\
    use (a);							\
    use (a + b + c + d + e + f + h + i);			\
    g[0] = b * c + K;						\
    g[1] = d * e * f * h * i;					\
  }

__entrypoint void ep (void) BODY (1)
void normal_ep (void) BODY (1)

void normal_sar (void) BODY (2)
__saveallregs void sar (void) BODY (2)

/* { dg-final { scan-assembler-not {\tjra\s+_(ep|sar|normal_ep|normal_sar)\M} } } */
