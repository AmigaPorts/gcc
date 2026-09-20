/* { dg-do run } */
/* Exercise small constant copies, unknown alignment, and surrounding guards.
   The AmigaOS test driver supplies the optimization/allocator/CPU matrix. */
#include <stdio.h>
#include <stdlib.h>

#define FN(N) \
__attribute__((noinline,noclone)) void aligned##N(void *d, const void *s) { \
  __builtin_memcpy(__builtin_assume_aligned(d, 2), \
                   __builtin_assume_aligned(s, 2), N); } \
__attribute__((noinline,noclone)) void unknown##N(void *d, const void *s) { \
  __builtin_memcpy(d, s, N); }
FN(0) FN(1) FN(2) FN(3) FN(8) FN(55) FN(56) FN(59) FN(60)
FN(63) FN(64) FN(95) FN(96) FN(97) FN(127) FN(128) FN(129) FN(256)
#define ROW(N) {N, aligned##N, unknown##N}
static const struct { unsigned n; void (*aligned)(void*,const void*);
                     void (*unknown)(void*,const void*); } cases[] = {
 ROW(0),ROW(1),ROW(2),ROW(3),ROW(8),ROW(55),ROW(56),ROW(59),ROW(60),
 ROW(63),ROW(64),ROW(95),ROW(96),ROW(97),ROW(127),ROW(128),ROW(129),ROW(256)
};
static unsigned char src[320] __attribute__((aligned(8)));
static unsigned char dst[320] __attribute__((aligned(8)));
int main(void) {
 unsigned tests = 0;
 for (unsigned c=0; c<sizeof(cases)/sizeof(cases[0]); ++c)
  for (unsigned s=0; s<8; ++s)
   for (unsigned d=0; d<8; ++d)
    for (unsigned mode=0; mode<2; ++mode) {
     if (mode && ((s|d)&1)) continue;
     for (unsigned i=0;i<320;++i) { src[i]=(i*37+19)&255; dst[i]=0xa5; }
     unsigned si=16+s, di=16+d, n=cases[c].n;
     (mode ? cases[c].aligned : cases[c].unknown)(dst+di,src+si);
     for (unsigned i=0;i<320;++i) {
      unsigned want=(i>=di && i<di+n) ? src[si+i-di] : 0xa5;
      if (dst[i]!=want || src[i]!=((i*37+19)&255)) {
       printf("FAIL n=%u src=%u dst=%u mode=%u byte=%u\n",n,s,d,mode,i);
       return 1;
      }
     }
     ++tests;
    }
 printf("PASS: %u copy/alignment/guard cases\n",tests);
 return 0;
}
