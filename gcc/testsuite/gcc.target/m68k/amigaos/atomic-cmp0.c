/* { dg-do compile } */
/* { dg-options "-O2" } */
/* { dg-skip-if "a4-relative data" { *-*-* } { "-fbaserel*" "-resident*" } { "" } } */
/* An atomic operation whose new value is only compared with zero is one
   instruction to memory and an scc or a branch on its flags: no call to
   libatomic.  add and sub can overflow, so a sign test uses N alone (mi,
   pl).  */

int refs;
int i;
short s;
signed char c;

extern void destroy (void);

void
release (void)
{
  if (__atomic_sub_fetch (&refs, 1, __ATOMIC_ACQ_REL) == 0)
    destroy ();
}

int add_eq (int x) { return __atomic_add_fetch (&i, x, __ATOMIC_SEQ_CST) == 0; }
int sub_ne (void) { return __atomic_sub_fetch (&i, 2, __ATOMIC_SEQ_CST) != 0; }
int add_lt (void) { return __atomic_add_fetch (&i, 1, __ATOMIC_SEQ_CST) < 0; }
int sub_ge (void) { return __atomic_sub_fetch (&i, 7, __ATOMIC_SEQ_CST) >= 0; }
int and_eq (void) { return __atomic_and_fetch (&i, 0x100, __ATOMIC_SEQ_CST) == 0; }
int or_lt (int x) { return __atomic_or_fetch (&i, x, __ATOMIC_SEQ_CST) < 0; }
int xor_ne (void) { return __atomic_xor_fetch (&i, 5, __ATOMIC_SEQ_CST) != 0; }
int and_gt (void) { return __atomic_and_fetch (&i, 0x7f, __ATOMIC_SEQ_CST) > 0; }
int xor_le (void) { return __atomic_xor_fetch (&i, 9, __ATOMIC_SEQ_CST) <= 0; }
int sub_eq_s (void) { return __atomic_sub_fetch (&s, 1, __ATOMIC_SEQ_CST) == 0; }
int add_eq_c (void) { return __atomic_add_fetch (&c, 1, __ATOMIC_SEQ_CST) == 0; }

/* { dg-final { scan-assembler-not "__atomic_|__sync_" } } */
/* { dg-final { scan-assembler-times "subq\\.l #1,_refs" 1 } } */
/* { dg-final { scan-assembler-times "add\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.l #2,_i" 1 } } */
/* { dg-final { scan-assembler-times "addq\\.l #1,_i" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.l #7,_i" 1 } } */
/* { dg-final { scan-assembler-times "and\\.l #256,_i" 1 } } */
/* { dg-final { scan-assembler-times "\tor\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.l #5,_i" 1 } } */
/* { dg-final { scan-assembler-times "and\\.l #127,_i" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.l #9,_i" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.w #1,_s" 1 } } */
/* { dg-final { scan-assembler-times "addq\\.b #1,_c" 1 } } */
/* { dg-final { scan-assembler-times "\tsmi" 2 } } */
/* { dg-final { scan-assembler-times "\tspl" 1 } } */
/* { dg-final { scan-assembler-times "\tsgt" 1 } } */
/* { dg-final { scan-assembler-times "\tsle" 1 } } */
/* { dg-final { scan-assembler-not "\tslt|\tsge|\tblt|\tbge|\tjlt|\tjge" } } */
