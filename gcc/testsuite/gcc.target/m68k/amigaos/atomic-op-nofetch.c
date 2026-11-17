/* { dg-do compile } */
/* { dg-options "-O2" } */
/* { dg-skip-if "a4-relative data" { *-*-* } { "-fbaserel*" "-resident*" } { "" } } */
/* An atomic operation whose result is unused is one instruction to
   memory, which is atomic on a single CPU: no call to libatomic.  */

signed char c;
short s;
int i;

#define OPS(T, v)							\
  void add1_##v (void) { __atomic_fetch_add (&v, 1, __ATOMIC_SEQ_CST); } \
  void sub8_##v (void) { __atomic_sub_fetch (&v, 8, __ATOMIC_RELAXED); } \
  void addm3_##v (void) { __sync_fetch_and_add (&v, -3); }		\
  void add100_##v (void) { __atomic_add_fetch (&v, 100, __ATOMIC_ACQ_REL); } \
  void addv_##v (T x) { __atomic_fetch_add (&v, x, __ATOMIC_SEQ_CST); }	\
  void subv_##v (T x) { __atomic_fetch_sub (&v, x, __ATOMIC_RELEASE); } \
  void and_##v (void) { __atomic_fetch_and (&v, 0x3c, __ATOMIC_SEQ_CST); } \
  void andv_##v (T x) { __atomic_and_fetch (&v, x, __ATOMIC_SEQ_CST); } \
  void or_##v (void) { __atomic_fetch_or (&v, 0x41, __ATOMIC_SEQ_CST); } \
  void orv_##v (T x) { __sync_or_and_fetch (&v, x); }			\
  void xor_##v (void) { __atomic_fetch_xor (&v, 0x12, __ATOMIC_ACQUIRE); } \
  void xorv_##v (T x) { __atomic_xor_fetch (&v, x, __ATOMIC_SEQ_CST); }

OPS (signed char, c)
OPS (short, s)
OPS (int, i)

/* { dg-final { scan-assembler-not "__atomic_|__sync_" } } */

/* { dg-final { scan-assembler-times "addq\\.b #1,_c" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.b #8,_c" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.b #3,_c" 1 } } */
/* { dg-final { scan-assembler-times "add\\.b #100,_c" 1 } } */
/* { dg-final { scan-assembler-times "add\\.b d\[0-7\],_c" 1 } } */
/* { dg-final { scan-assembler-times "sub\\.b d\[0-7\],_c" 1 } } */
/* { dg-final { scan-assembler-times "and\\.b #60,_c" 1 } } */
/* { dg-final { scan-assembler-times "and\\.b d\[0-7\],_c" 1 } } */
/* { dg-final { scan-assembler-times "or\\.b #65,_c" 1 } } */
/* { dg-final { scan-assembler-times "\tor\\.b d\[0-7\],_c" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.b #18,_c" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.b d\[0-7\],_c" 1 } } */

/* { dg-final { scan-assembler-times "addq\\.w #1,_s" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.w #8,_s" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.w #3,_s" 1 } } */
/* { dg-final { scan-assembler-times "add\\.w #100,_s" 1 } } */
/* { dg-final { scan-assembler-times "add\\.w d\[0-7\],_s" 1 } } */
/* { dg-final { scan-assembler-times "sub\\.w d\[0-7\],_s" 1 } } */
/* { dg-final { scan-assembler-times "and\\.w #60,_s" 1 } } */
/* { dg-final { scan-assembler-times "and\\.w d\[0-7\],_s" 1 } } */
/* { dg-final { scan-assembler-times "or\\.w #65,_s" 1 } } */
/* { dg-final { scan-assembler-times "\tor\\.w d\[0-7\],_s" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.w #18,_s" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.w d\[0-7\],_s" 1 } } */

/* { dg-final { scan-assembler-times "addq\\.l #1,_i" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.l #8,_i" 1 } } */
/* { dg-final { scan-assembler-times "subq\\.l #3,_i" 1 } } */
/* { dg-final { scan-assembler-times "add\\.l #100,_i" 1 } } */
/* { dg-final { scan-assembler-times "add\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "sub\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "and\\.l #60,_i" 1 } } */
/* { dg-final { scan-assembler-times "and\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "or\\.l #65,_i" 1 } } */
/* { dg-final { scan-assembler-times "\tor\\.l d\[0-7\],_i" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.l #18,_i" 1 } } */
/* { dg-final { scan-assembler-times "eor\\.l d\[0-7\],_i" 1 } } */
