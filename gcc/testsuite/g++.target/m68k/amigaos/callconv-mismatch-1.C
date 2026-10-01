// -Wcallconv-mismatch in C++, as gcc.target/m68k/amigaos/callconv-mismatch-1.c
// for C: with -mregparm=3, a __stdargs (stkparm) function reached through a
// pointer whose type passes arguments in registers is diagnosed at every
// implicit conversion: initialization (namespace and block scope, direct,
// braced, member), assignment, argument passing, a default argument, return
// and conditional operands.  C++ treats the two function types as the same
// type, so a conditional between the functions themselves is diagnosed too.
// Direct comparisons, explicit casts and compatible conversions are not.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3" }

typedef unsigned long size_t;
__attribute__ ((__stkparm__)) void *stk_copy (void *, const void *, size_t);
__attribute__ ((__stkparm__)) void *stk_set (void *, int, size_t);
__attribute__ ((__stkparm__)) void stk_noargs (void);
__attribute__ ((__stkparm__)) int stk_printf (const char *, ...);
void *reg_copy (void *, const void *, size_t);
long asm_regs (char *p __asm ("a0"), long d __asm ("d0"));

typedef void *(*cpy_t) (void *, const void *, size_t);
typedef void *(*set_t) (void *, int, size_t);
typedef __attribute__ ((__stkparm__)) void *(*stk_cpy_t) (void *, const void *,
							   size_t);
typedef long (*asm_t) (char *p __asm ("a0"), long d __asm ("d0"));

cpy_t file_init = stk_copy;			// { dg-warning "different calling convention" }
stk_cpy_t reverse_init = reg_copy;		// { dg-warning "different calling convention" }
cpy_t braced_init { stk_copy };			// { dg-warning "different calling convention" }
cpy_t addr_init = &stk_copy;			// { dg-warning "different calling convention" }
stk_cpy_t pinned = stk_copy;
cpy_t plain = reg_copy;
void (*noargs) (void) = stk_noargs;
int (*variadic) (const char *, ...) = stk_printf;
asm_t matching_regs = asm_regs;

set_t assigned;
void take (set_t);
void take_default (cpy_t = stk_copy);

cpy_t
give (void)
{
  return stk_copy;				// { dg-warning "different calling convention" }
}

struct holder
{
  cpy_t member;
  holder () : member (stk_copy) {}		// { dg-warning "different calling convention" }
};

int
f (int c)
{
  cpy_t block_init = stk_copy;			// { dg-warning "different calling convention" }
  cpy_t direct_init (stk_copy);			// { dg-warning "different calling convention" }
  assigned = stk_set;				// { dg-warning "different calling convention" }
  take (stk_set);				// { dg-warning "different calling convention" }
  take_default ();				// { dg-warning "different calling convention" }
  void (*through) (set_t) = take;
  through (stk_set);				// { dg-warning "different calling convention" }
  cpy_t cast = (cpy_t) stk_copy;
  cpy_t scast = static_cast<cpy_t> (stk_copy);
  cpy_t rcast = reinterpret_cast<cpy_t> (stk_copy);
  cpy_t fcast = cpy_t (stk_copy);
  take ((set_t) stk_set);
  cpy_t from_var = pinned;			// { dg-warning "different calling convention" }
  int same = file_init == stk_copy;
  int pick = (c ? stk_copy : reg_copy) == file_init; // { dg-warning "different calling conventions" }
  int ptrs = (c ? &stk_copy : &reg_copy) == file_init; // { dg-warning "different calling conventions" }
  int called = (c ? stk_copy : reg_copy) (0, 0, 0) != 0; // { dg-warning "different calling conventions" }
  int fine = (c ? stk_copy : (stk_cpy_t) 0) != 0;
  return same + pick + ptrs + called + fine + (block_init != 0)
	 + (direct_init != 0) + (cast != 0) + (scast != 0) + (rcast != 0)
	 + (fcast != 0) + (from_var != 0);
}
