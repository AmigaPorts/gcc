#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include "bench.h"

#ifndef BUILD_LABEL
#define BUILD_LABEL "unspecified"
#endif
static const char *names[BENCH_COUNT] = {
  "matrix-float", "matrix-double", "mandelbrot-float", "mandelbrot-double"
};

static clock_t
measure (unsigned kind, unsigned repeats)
{
  clock_t start = clock ();
  if (start == (clock_t) -1) return (clock_t) -1;
  for (unsigned i = 0; i < repeats; ++i) bench_run (kind);
  clock_t end = clock ();
  return end == (clock_t) -1 || end < start ? (clock_t) -1 : end - start;
}

int
main (int argc, char **argv)
{
  int verify_only = argc > 1 && argv[1][0] == 'v';
  puts (BUILD_LABEL);
  printf ("CLOCKS_PER_SEC=%lu; matrix vectors=%u; image=%ux%u; limit=%u\n",
	  (unsigned long) CLOCKS_PER_SEC, VECTOR_COUNT, IMAGE_WIDTH,
	  IMAGE_HEIGHT, ITERATION_LIMIT);
  bench_setup ();
  puts ("kernel,repeats,min_us,median_us,max_us,checksum");
  for (unsigned kind = 0; kind < BENCH_COUNT; ++kind)
    {
      unsigned hash, repeats = 1;
      bench_run (kind);
      if (bench_check (kind, &hash))
	{ fprintf (stderr, "%s: reference mismatch\n", names[kind]); return 1; }
      if (verify_only)
	{ printf ("%s: PASS %08x\n", names[kind], hash); continue; }
      /* Calibrate to at least half a second to tolerate coarse clocks.  */
      for (;;)
	{
	  clock_t ticks = measure (kind, repeats);
	  if (ticks == (clock_t) -1)
	    { fputs ("clock failed or wrapped\n", stderr); return 2; }
	  if (ticks >= (clock_t) (CLOCKS_PER_SEC / 2 + 1)) break;
	  if (repeats > UINT_MAX / 2) return 2;
	  repeats *= 2;
	}
      clock_t sample[5];
      for (unsigned i = 0; i < 5; ++i)
	{
	  sample[i] = measure (kind, repeats);
	  if (sample[i] == (clock_t) -1) return 2;
	  if (bench_check (kind, &hash)) return 1;
	}
      for (unsigned i = 1; i < 5; ++i)
	for (unsigned j = i; j > 0 && sample[j] < sample[j-1]; --j)
	  { clock_t t = sample[j]; sample[j] = sample[j-1]; sample[j-1] = t; }
      /* The driver is compiled for the conventional OS4 ABI.  All kernel
	 interfaces above use integers; no FP value crosses that boundary. */
      printf ("%s,%u,%.3f,%.3f,%.3f,%08x\n", names[kind], repeats,
	      (double) sample[0] * 1000000 / CLOCKS_PER_SEC / repeats,
	      (double) sample[2] * 1000000 / CLOCKS_PER_SEC / repeats,
	      (double) sample[4] * 1000000 / CLOCKS_PER_SEC / repeats, hash);
    }
  return 0;
}
