/* { dg-do run } */
/* A test of an address register against zero, signed and unsigned, for
   every condition, as a branch and as an scc.  On 68000/68010 the test can
   go through a copy to a free data register; check the result for pointer
   values whose low word alone would give the wrong answer, with every data
   register busy, and when the scratch is the scc result.  The AmigaOS test
   driver supplies the optimization/allocator/CPU matrix.  */

#define NI __attribute__ ((noinline, noclone))

volatile long sink;
struct n { struct n *nx; long v; };

#define FN(NAME, T, OP)							\
/* Branch.  */								\
NI int br_##NAME (void *p, int a, int b)				\
{									\
  __asm__ volatile ("" : "+a" (p));					\
  if ((T) p OP 0)							\
    return a;								\
  return b;								\
}									\
/* scc; the scratch can be the result register.  */			\
NI int sc_##NAME (void *p)						\
{									\
  __asm__ volatile ("" : "+a" (p));					\
  return (T) p OP 0;							\
}									\
/* Every data register live across the branch.  */			\
NI long hp_##NAME (void *p, long x)					\
{									\
  long a = x * 3, b = x ^ 5, c = x + 7, d = x - 9;			\
  long e = x * 11, g = x | 13, h = x & 0x55;				\
  __asm__ volatile ("" : "+a" (p), "+d" (a), "+d" (b), "+d" (c),	\
		    "+d" (d), "+d" (e), "+d" (g), "+d" (h));		\
  if ((T) p OP 0)							\
    return a + b + c + d;						\
  return e + g + h + a;							\
}									\
/* A data register live into one successor only.  */			\
NI long lt_##NAME (void *p, long x, long y)				\
{									\
  __asm__ volatile ("" : "+a" (p), "+d" (x), "+d" (y));		\
  if ((T) p OP 0)							\
    {									\
      __asm__ volatile ("" :: "d" (x));					\
      return x;								\
    }									\
  return y + 1;								\
}									\
/* A pointer from a data register that was compared before.  */	\
NI long mv_##NAME (long v, long k)					\
{									\
  __asm__ volatile ("" : "+d" (v));					\
  if (v == k)								\
    sink = 1;								\
  void *p = (void *) v;							\
  __asm__ volatile ("" : "+a" (p));					\
  if ((T) p OP 0)							\
    return 5;								\
  return 6;								\
}									\
/* A compared data register copied to a pointer.  */			\
NI long cp_##NAME (long v, long k)					\
{									\
  if (v > k)								\
    sink = v;								\
  struct n *p = (struct n *) v;						\
  if ((T) p OP 0)							\
    return 7;								\
  return 8;								\
}									\
static void								\
check_##NAME (long sv, long x, long y)					\
{									\
  T v = (T) sv;								\
  if (br_##NAME ((void *) sv, 11, 22) != ((v OP 0) ? 11 : 22))		\
    __builtin_abort ();							\
  if (sc_##NAME ((void *) sv) != (v OP 0))				\
    __builtin_abort ();							\
  if (hp_##NAME ((void *) sv, x)					\
      != ((v OP 0) ? (x * 3) + (x ^ 5) + (x + 7) + (x - 9)		\
	  : (x * 11) + (x | 13) + (x & 0x55) + (x * 3)))		\
    __builtin_abort ();							\
  if (lt_##NAME ((void *) sv, x, y) != ((v OP 0) ? x : y + 1))		\
    __builtin_abort ();							\
  if (mv_##NAME (sv, 3) != ((v OP 0) ? 5 : 6))				\
    __builtin_abort ();							\
  if (cp_##NAME (sv, 3) != ((v OP 0) ? 7 : 8))				\
    __builtin_abort ();							\
}

FN (seq, long, ==)
FN (sne, long, !=)
FN (slt, long, <)
FN (sle, long, <=)
FN (sgt, long, >)
FN (sge, long, >=)
FN (ueq, unsigned long, ==)
FN (une, unsigned long, !=)
FN (ult, unsigned long, <)
FN (ule, unsigned long, <=)
FN (ugt, unsigned long, >)
FN (uge, unsigned long, >=)

static const long vals[] = {
  0, 1, -1L, (long) 0x80000000UL, 0x7fffffffL, 0x10000L,
  (long) 0xffff0000UL, 0x8000L, (long) 0xffff8000UL
};

int
main (void)
{
  for (unsigned i = 0; i < sizeof vals / sizeof vals[0]; i++)
    {
      long sv = vals[i], x = 1234 + i, y = 99 - i;
      check_seq (sv, x, y);
      check_sne (sv, x, y);
      check_slt (sv, x, y);
      check_sle (sv, x, y);
      check_sgt (sv, x, y);
      check_sge (sv, x, y);
      check_ueq (sv, x, y);
      check_une (sv, x, y);
      check_ult (sv, x, y);
      check_ule (sv, x, y);
      check_ugt (sv, x, y);
      check_uge (sv, x, y);
    }
  return 0;
}
