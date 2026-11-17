/* { dg-do compile } */
/* { dg-options "-O2" } */
/* { dg-skip-if "a4-relative data" { *-*-* } { "-fbaserel*" "-resident*" } { "" } } */
/* tas takes a locked read-modify-write bus cycle, which the Amiga chipset
   does not support.  bset sets the same value, 0x80, in one instruction,
   which is atomic on a single CPU.  */

char flag;

int
set (void)
{
  return __atomic_test_and_set (&flag, __ATOMIC_SEQ_CST);
}

void
clear (void)
{
  __atomic_clear (&flag, __ATOMIC_SEQ_CST);
}

/* { dg-final { scan-assembler "bset #7,_flag" } } */
/* { dg-final { scan-assembler "sne d\[0-7\]" } } */
/* { dg-final { scan-assembler-not "\ttas" } } */
/* { dg-final { scan-assembler-not "__atomic_|__sync_" } } */
