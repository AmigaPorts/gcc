/* An indirect sibcall loads its target into a scratch address register:
   a0 (STATIC_CHAIN_REGNUM) when no argument lives there, else a1.  Only a
   call with arguments in both a0 and a1 has no register left and stays a
   normal call.  A direct call may pass an argument in a1 and still branch.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68020-60 -O2 -fomit-frame-pointer -mregparm=4" } */

typedef void (*fn_t) (void *bi);
typedef void (*fn2_t) (void *bi, void *ops);
struct Ops { fn_t f; fn2_t g; };
extern void ext_two (void *p, void *q);

/* bi stays in a0, the target goes through a1: move.l (a1),a1; jmp (a1).  */
void
set_clock (void *bi __asm ("a0"), struct Ops *ops __asm ("a1"))
{
  ops->f (bi);
}

/* bi in a0 and ops in a1 leave no scratch register: a normal call.  */
void
set_both (void *bi __asm ("a0"), struct Ops *ops __asm ("a1"))
{
  ops->g (bi, ops);
}

/* A direct call with a1 as its second pointer argument still branches.  */
void
dir_two (void *p, void *q)
{
  ext_two (p, q);
}

/* { dg-final { scan-assembler-times "jmp \\(a1\\)" 1 } } */
/* { dg-final { scan-assembler-not "jmp \\(a0\\)" } } */
/* { dg-final { scan-assembler-times "jsr \\(a\[0-9\]\\)" 1 } } */
/* { dg-final { scan-assembler-times "jra _ext_two" 1 } } */
