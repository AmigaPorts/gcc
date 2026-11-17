/* { dg-do compile } */
/* { dg-options "-O2" } */
/* The atomic operations that are more than one instruction to memory still
   call libatomic: a fetch whose value is used, an exchange, a
   compare-exchange, and a sign test after add or sub that needs V.  */

int i;

int fetch_add (void) { return __atomic_fetch_add (&i, 1, __ATOMIC_SEQ_CST); }
int or_fetch (int x) { return __atomic_or_fetch (&i, x, __ATOMIC_SEQ_CST); }
int exchange (int x) { return __atomic_exchange_n (&i, x, __ATOMIC_SEQ_CST); }

int
cas (int *expected, int x)
{
  return __atomic_compare_exchange_n (&i, expected, x, 0, __ATOMIC_SEQ_CST,
				      __ATOMIC_SEQ_CST);
}

int add_gt (void) { return __atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) > 0; }

/* The op-fetch forms call the fetch-op function and redo the operation.  */
/* { dg-final { scan-assembler-times "__atomic_fetch_add_4" 2 } } */
/* { dg-final { scan-assembler-times "__atomic_fetch_or_4" 1 } } */
/* { dg-final { scan-assembler-times "__atomic_exchange_4" 1 } } */
/* { dg-final { scan-assembler-times "__atomic_compare_exchange_4" 1 } } */
