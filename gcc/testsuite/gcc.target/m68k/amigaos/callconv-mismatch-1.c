/* -Wcallconv-mismatch with -mregparm=3: a __stdargs (stkparm) function
   reached through a pointer whose type passes arguments in registers is
   diagnosed at every conversion: initialization (file and block scope),
   assignment, argument passing, return and conditional operands.  A
   conditional merges the pointer types and a call through its result can
   use the wrong convention for one branch.  Direct comparisons, explicit
   casts and compatible conversions are not diagnosed.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3" } */

typedef unsigned long size_t;
__attribute__ ((__stkparm__)) void *stk_copy (void *, const void *, size_t);
__attribute__ ((__stkparm__)) void *stk_set (void *, int, size_t);
__attribute__ ((__stkparm__)) void stk_noargs (void);
__attribute__ ((__stkparm__)) int stk_printf (const char *, ...);
void *reg_copy (void *, const void *, size_t);
void *reg_move (void *, const void *, size_t);
long asm_regs (char *p __asm ("a0"), long d __asm ("d0"));

typedef void *(*cpy_t) (void *, const void *, size_t);
typedef void *(*set_t) (void *, int, size_t);
typedef __attribute__ ((__stkparm__)) void *(*stk_cpy_t) (void *, const void *,
							   size_t);
typedef long (*asm_t) (char *p __asm ("a0"), long d __asm ("d0"));

cpy_t file_init = stk_copy;			/* { dg-warning "different calling convention" } */
stk_cpy_t reverse_init = reg_copy;		/* { dg-warning "different calling convention" } */
stk_cpy_t pinned = stk_copy;
cpy_t plain = reg_copy;
void (*noargs) (void) = stk_noargs;
int (*variadic) (const char *, ...) = stk_printf;
asm_t matching_regs = asm_regs;

set_t assigned;
void take (set_t);

cpy_t
give (void)
{
  return stk_copy;				/* { dg-warning "different calling convention" } */
}

int
f (int c)
{
  cpy_t block_init = stk_copy;			/* { dg-warning "different calling convention" } */
  assigned = stk_set;				/* { dg-warning "different calling convention" } */
  take (stk_set);				/* { dg-warning "different calling convention" } */
  cpy_t cast = (cpy_t) stk_copy;
  int same = file_init == stk_copy;
  int pick = (c ? stk_copy : reg_copy) == file_init; /* { dg-warning "different calling conventions" } */
  int called = (c ? stk_copy : reg_copy) (0, 0, 0) != 0; /* { dg-warning "different calling conventions" } */
  int alike = (c ? reg_copy : reg_move) (0, 0, 0) != 0;
  return same + pick + called + alike + (block_init != 0) + (cast != 0);
}
