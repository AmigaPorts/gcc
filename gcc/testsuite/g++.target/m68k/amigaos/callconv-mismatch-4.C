// Explicit casts in either conditional operand are quiet, including when
// the folded cast keeps the original function type.  Uncast conditionals
// and independent conversions must still be diagnosed.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3" }

__attribute__ ((__stkparm__)) int stk_fn (int);
int reg_fn (int);
typedef int (*fnptr_t) (int);
typedef __attribute__ ((__stkparm__)) int (*stk_fnptr_t) (int);

fnptr_t
casts (bool c)
{
  fnptr_t a = c ? (fnptr_t) stk_fn : reg_fn;
  fnptr_t b = c ? reg_fn : (fnptr_t) stk_fn;
  fnptr_t d = c ? static_cast<fnptr_t> (stk_fn) : reg_fn;
  fnptr_t e = c ? reg_fn : static_cast<fnptr_t> (stk_fn);
  fnptr_t f = c ? reinterpret_cast<fnptr_t> (stk_fn) : reg_fn;
  fnptr_t g = c ? reg_fn : reinterpret_cast<fnptr_t> (stk_fn);
  fnptr_t h = c ? fnptr_t (stk_fn) : reg_fn;
  fnptr_t i = c ? reg_fn : fnptr_t (stk_fn);
  fnptr_t j = c ? (fnptr_t) &stk_fn : &reg_fn;
  fnptr_t k = c ? &reg_fn : (fnptr_t) &stk_fn;
  stk_fnptr_t reverse = c ? (stk_fnptr_t) reg_fn : stk_fn;
  int called = (c ? (fnptr_t) stk_fn : reg_fn) (1);
  fnptr_t uncast = c ? stk_fn : reg_fn; // { dg-warning "different calling conventions" }
  fnptr_t separate = stk_fn; // { dg-warning "different calling convention" }
  // A cast that does not change convention must not hide the other arm.
  fnptr_t compatible = c ? static_cast<fnptr_t> (reg_fn) : stk_fn; // { dg-warning "different calling conventions" }
  return called ? a : b;
}

template <typename T>
fnptr_t
cast_template (T c)
{
  return c ? static_cast<fnptr_t> (stk_fn) : reg_fn;
}

fnptr_t instantiated = cast_template (true);

fnptr_t
nested (bool c)
{
  // An already-diagnosed operand is not an explicit cast.
  return c ? // { dg-warning "different calling conventions" }
    (c ? reg_fn : stk_fn) : stk_fn; // { dg-warning "different calling conventions" }
}

struct S
{
  __attribute__ ((__stkparm__)) int stk (int);
  int reg (int);
};
typedef int (S::*pmf_t) (int);

pmf_t
member_casts (bool c)
{
  pmf_t a = c ? (pmf_t) &S::stk : &S::reg;
  pmf_t b = c ? &S::reg : static_cast<pmf_t> (&S::stk);
  return c ? a : b;
}
