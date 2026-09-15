/* LRA is the default allocator on amigaos.  The old reload pass ICEs on
   this DImode code at -O1 -fext-dce on the 68000 (PR 127300,
   https://github.com/AmigaPorts/m68k-amigaos-gcc/issues/34): the check
   before the subtraction and the trailing call are both needed.  Run,
   not just compiled: ext-dce drops the sign extension of delta because
   only the low half of the difference survives the cast, and main checks
   that the result is still right for both signs.  The callees are noipa:
   with noinline alone the compiler still sees that pop() has no side
   effects, drops the call, and the subtraction narrows to 32 bits.  */
/* { dg-do run } */
/* { dg-additional-options "-fext-dce" } */
/* { dg-skip-if "reload ICE, PR 127300" { *-*-* } { "-O1 -mno-lra" } { "" } } */

extern long long get (void *);
extern int err (void *);
extern void pop (void *);

int
getfield (void *L, int delta)
{
  long long res = get (L);
  if (res >= 0 ? res - delta > 2147483647 : res < delta)
    return err (L);
  res -= delta;
  pop (L);
  return (int) res;
}

long long get_val;

__attribute__ ((noipa)) long long
get (void *L)
{
  return get_val;
}

__attribute__ ((noipa)) int
err (void *L)
{
  return -1;
}

__attribute__ ((noipa)) void
pop (void *L)
{
}

/* Dirty the stack so a high half taken from a stale slot would show.  */
__attribute__ ((noipa)) void
dirty (int depth)
{
  volatile unsigned junk[16];
  int i;
  for (i = 0; i < 16; i++)
    junk[i] = 0xdeadbeef;
  if (depth)
    dirty (depth - 1);
}

int
main (void)
{
  get_val = 100;
  dirty (4);
  if (getfield (0, -5) != 105)
    __builtin_abort ();
  dirty (4);
  if (getfield (0, 5) != 95)
    __builtin_abort ();
  get_val = -100;
  dirty (4);
  if (getfield (0, -200) != 100)
    __builtin_abort ();
  get_val = 2147483647LL + 10;
  dirty (4);
  if (getfield (0, 5) != -1)
    __builtin_abort ();
  return 0;
}
