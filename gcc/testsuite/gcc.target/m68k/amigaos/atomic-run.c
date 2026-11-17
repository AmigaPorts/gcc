/* { dg-do run } */
/* The values of the atomic operations, in one task: test-and-set and clear,
   the operations whose result is unused, those whose new value is compared
   with zero, and some that call libatomic, which the driver links without
   -latomic.  */
#include <limits.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) abort (); } while (0)

signed char c;
short s;
int i;
unsigned char flag;

/* Variable operands, which the compiler cannot fold into the insn.  */
__attribute__ ((noipa)) int val (int x) { return x; }

static void
test_and_set (void)
{
  CHECK (!__atomic_test_and_set (&flag, __ATOMIC_SEQ_CST));
  CHECK (flag == 0x80);
  CHECK (__atomic_test_and_set (&flag, __ATOMIC_SEQ_CST));
  CHECK (flag == 0x80);
  __atomic_clear (&flag, __ATOMIC_SEQ_CST);
  CHECK (flag == 0);
  CHECK (!__atomic_test_and_set (&flag, __ATOMIC_ACQUIRE));
  __atomic_clear (&flag, __ATOMIC_RELEASE);
}

#define NOFETCH(T, v)							\
  static void								\
  nofetch_##v (void)							\
  {									\
    v = 10;								\
    __atomic_fetch_add (&v, 1, __ATOMIC_SEQ_CST);			\
    CHECK (v == 11);							\
    __atomic_add_fetch (&v, val (100), __ATOMIC_RELAXED);		\
    CHECK (v == 111);							\
    __sync_fetch_and_sub (&v, 8);					\
    CHECK (v == 103);							\
    __atomic_sub_fetch (&v, val (110), __ATOMIC_SEQ_CST);		\
    CHECK (v == -7);							\
    __atomic_fetch_add (&v, -3, __ATOMIC_SEQ_CST);			\
    CHECK (v == -10);							\
    v = 0x5a;								\
    __atomic_fetch_and (&v, 0x3c, __ATOMIC_SEQ_CST);			\
    CHECK (v == 0x18);							\
    __atomic_and_fetch (&v, (T) val (0x10), __ATOMIC_SEQ_CST);		\
    CHECK (v == 0x10);							\
    __atomic_fetch_or (&v, 0x41, __ATOMIC_SEQ_CST);			\
    CHECK (v == 0x51);							\
    __sync_or_and_fetch (&v, (T) val (0x22));				\
    CHECK (v == 0x73);							\
    __atomic_fetch_xor (&v, 0x12, __ATOMIC_SEQ_CST);			\
    CHECK (v == 0x61);							\
    __atomic_xor_fetch (&v, (T) val (0x61), __ATOMIC_SEQ_CST);		\
    CHECK (v == 0);							\
  }

NOFETCH (signed char, c)
NOFETCH (short, s)
NOFETCH (int, i)

static int refs;
static int destroyed;

static void
release (void)
{
  if (__atomic_sub_fetch (&refs, 1, __ATOMIC_ACQ_REL) == 0)
    destroyed++;
}

static void
cmp0 (void)
{
  refs = 2;
  release ();
  CHECK (refs == 1 && destroyed == 0);
  release ();
  CHECK (refs == 0 && destroyed == 1);

  i = -2;
  CHECK (!(__atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) == 0));
  CHECK (__atomic_add_fetch (&i, val (1), __ATOMIC_SEQ_CST) == 0);
  CHECK (__atomic_sub_fetch (&i, 2, __ATOMIC_SEQ_CST) != 0);
  CHECK (i == -2);
  CHECK (__atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) < 0);
  CHECK (!(__atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) < 0));
  CHECK (__atomic_sub_fetch (&i, 0, __ATOMIC_SEQ_CST) >= 0);
  CHECK (!(__atomic_sub_fetch (&i, val (1), __ATOMIC_SEQ_CST) >= 0));

  /* Overflow sets V: the sign of the result is N alone.  */
  i = INT_MAX;
  CHECK (__atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) < 0);
  CHECK (i == INT_MIN);
  CHECK (__atomic_sub_fetch (&i, 1, __ATOMIC_SEQ_CST) >= 0);
  CHECK (i == INT_MAX);
  s = SHRT_MAX;
  CHECK (__atomic_add_fetch (&s, 1, __ATOMIC_SEQ_CST) < 0);
  CHECK (s == SHRT_MIN);
  c = SCHAR_MIN;
  CHECK (__atomic_sub_fetch (&c, 1, __ATOMIC_SEQ_CST) >= 0);
  CHECK (c == SCHAR_MAX);

  i = 0x1ff;
  CHECK (!(__atomic_and_fetch (&i, 0x100, __ATOMIC_SEQ_CST) == 0));
  CHECK (__atomic_and_fetch (&i, 0x80, __ATOMIC_SEQ_CST) == 0);
  CHECK (__atomic_or_fetch (&i, val (INT_MIN), __ATOMIC_SEQ_CST) < 0);
  CHECK (__atomic_xor_fetch (&i, INT_MIN, __ATOMIC_SEQ_CST) == 0);
  CHECK (!(__atomic_xor_fetch (&i, 5, __ATOMIC_SEQ_CST) <= 0));
  CHECK (__atomic_and_fetch (&i, 0x7f, __ATOMIC_SEQ_CST) > 0);
  CHECK (!(__atomic_and_fetch (&i, 0x2, __ATOMIC_SEQ_CST) > 0));
  CHECK (__atomic_xor_fetch (&i, 0, __ATOMIC_SEQ_CST) <= 0);
  CHECK (__atomic_or_fetch (&i, INT_MIN, __ATOMIC_SEQ_CST) <= 0);

  s = 1;
  CHECK (__atomic_sub_fetch (&s, 1, __ATOMIC_SEQ_CST) == 0);
  c = -1;
  CHECK (__atomic_add_fetch (&c, 1, __ATOMIC_SEQ_CST) == 0);
}

/* These call libatomic.  */
static void
libcalls (void)
{
  int expected;

  i = 5;
  CHECK (__atomic_fetch_add (&i, 3, __ATOMIC_SEQ_CST) == 5);
  CHECK (__atomic_exchange_n (&i, 20, __ATOMIC_SEQ_CST) == 8);
  expected = 20;
  CHECK (__atomic_compare_exchange_n (&i, &expected, 30, 0, __ATOMIC_SEQ_CST,
				      __ATOMIC_SEQ_CST));
  CHECK (i == 30);
  expected = 1;
  CHECK (!__atomic_compare_exchange_n (&i, &expected, 40, 0,
				       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
  CHECK (expected == 30 && i == 30);
  CHECK (__atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) > 0);
  CHECK (i == 31);
}

int
main (void)
{
  test_and_set ();
  nofetch_c ();
  nofetch_s ();
  nofetch_i ();
  cmp0 ();
  libcalls ();
  return 0;
}
