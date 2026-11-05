#include "bench.h"
/* QEMU verifies correctness only; use driver.c for physical timings.  */
int
test_main (void)
{
  static const unsigned expected[BENCH_COUNT] = {
    0xd720b565, 0xd720b565, 0x98665400, 0x8d592c90
  };
  unsigned hash;
  bench_setup ();
  for (unsigned kind = 0; kind < BENCH_COUNT; ++kind)
    {
      bench_run (kind);
      if (bench_check (kind, &hash) || hash != expected[kind]) return kind + 1;
    }
  return 0;
}
