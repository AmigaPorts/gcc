// -Wcallconv-mismatch in C++: pointers to member functions, a function
// pointer parameter type deduced by a template (the instantiation calls
// through a type without stkparm) and a lambda return are diagnosed; a
// non-type template argument, called directly by name, is not.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3" }

struct S
{
  __attribute__ ((__stkparm__)) int stk (int);
  int reg (int);
};

typedef int (S::*pmf_t) (int);

pmf_t pmf_init = &S::stk;			// { dg-warning "different calling convention" }
pmf_t pmf_plain = &S::reg;

int
pick_pmf (S *s, int c)
{
  return (s->*(c ? &S::stk : &S::reg)) (c);	// { dg-warning "different calling conventions" }
}

__attribute__ ((__stkparm__)) int stk_fn (int, int);
int reg_fn (int, int);

template <typename R, typename... A>
R
call (R (*fp) (A...), A... a)
{
  return fp (a...);
}

int
deduced (void)
{
  return call (stk_fn, 1, 2)			// { dg-warning "different calling convention" }
	 + call (reg_fn, 1, 2);
}

template <int (*FP) (int, int)>
int
nttp (void)
{
  return FP (1, 2);
}

int
use_nttp (void)
{
  return nttp<stk_fn> () + nttp<reg_fn> ();
}

auto lambda = [] () -> int (*) (int, int) { return stk_fn; }; // { dg-warning "different calling convention" }
