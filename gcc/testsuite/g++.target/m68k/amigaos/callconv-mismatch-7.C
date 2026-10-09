// A default argument is copied to the call site.  Explicit casts must
// remain quiet without marking that shared location as suppressed.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3" }

__attribute__ ((__stkparm__)) int stk_fn (int);
int reg_fn (int);
typedef int (*fnptr_t) (int);
typedef __attribute__ ((__stkparm__)) int (*stk_fnptr_t) (int);
extern stk_fnptr_t stack_value;
extern bool condition;

struct stack_pointer
{
  operator stk_fnptr_t () const;
};

void ccast_default (fnptr_t = (fnptr_t) stk_fn);
void scast_default (fnptr_t = static_cast<fnptr_t> (stk_fn));
void rcast_default (fnptr_t = reinterpret_cast<fnptr_t> (stk_fn));
void fcast_default (fnptr_t = fnptr_t (stk_fn));
void variable_default (fnptr_t = (fnptr_t) stack_value);
void reverse_default (stk_fnptr_t = (stk_fnptr_t) reg_fn);
void class_default (fnptr_t = static_cast<fnptr_t> (stack_pointer {}));
void conditional_default (fnptr_t = condition ? (fnptr_t) stk_fn : reg_fn);
void uncast_default (fnptr_t = stack_pointer {});
void mixed_default (fnptr_t = static_cast<fnptr_t> (stack_pointer {}),
                    fnptr_t = stack_pointer {});
void mixed_argument (fnptr_t, fnptr_t = (fnptr_t) stack_value);
void reverse_mixed_default (fnptr_t = stack_pointer {},
                            fnptr_t = static_cast<fnptr_t> (stack_pointer {}));

template <typename T>
void template_default (T, fnptr_t = static_cast<fnptr_t> (stk_fn));

template <typename T>
void dependent_default (fnptr_t = static_cast<fnptr_t> (T {}));

struct S
{
  __attribute__ ((__stkparm__)) int stk (int);
};
typedef int (S::*pmf_t) (int);
void member_default (pmf_t = (pmf_t) &S::stk);

void
use_defaults ()
{
  ccast_default ();
  ccast_default ();
  scast_default ();
  rcast_default ();
  fcast_default ();
  variable_default ();
  variable_default ();
  reverse_default ();
  class_default ();
  conditional_default ();
  template_default (1);
  template_default (2);
  dependent_default<stack_pointer> ();
  member_default ();

  uncast_default (); // { dg-warning "different calling convention" }
  uncast_default (); // { dg-warning "different calling convention" }
  mixed_default (); // { dg-warning "different calling convention" }
  mixed_default (); // { dg-warning "different calling convention" }
  reverse_mixed_default (); // { dg-warning "different calling convention" }
  mixed_argument (stack_pointer {}); // { dg-warning "different calling convention" }
  // A following explicit argument must not inherit default suppression.
  ccast_default (); mixed_argument (stack_pointer {}); // { dg-warning "different calling convention" }
  // Reusing a declaration with an explicit argument must still warn.
  ccast_default (stack_pointer {}); // { dg-warning "different calling convention" }
}

template <typename T>
void
use_template (T value)
{
  ccast_default ();
  variable_default ();
  class_default ();
  template_default (value);
  dependent_default<T> ();
  mixed_default (); // { dg-warning "different calling convention" }
}

template void use_template<stack_pointer> (stack_pointer);
