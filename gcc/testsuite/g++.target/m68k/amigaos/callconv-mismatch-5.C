// Check the return type of the selected user-defined conversion, rather
// than the original class type, for initialization and argument passing.

// { dg-do compile { target c++11 } }
// { dg-options "-m68000 -Os -mregparm=3" }

typedef int (*fnptr_t) (int);
typedef __attribute__ ((__stkparm__)) int (*stk_fnptr_t) (int);

struct stack_pointer
{
  operator stk_fnptr_t () const;
};

struct register_pointer
{
  operator fnptr_t () const;
};

struct stack_reference
{
  operator stk_fnptr_t & () const;
};

struct selectable
{
  operator stk_fnptr_t () const;
  operator int () const;
};

void take (fnptr_t);
void take_stack (stk_fnptr_t);
void take_int (int);
void take_default (fnptr_t = stack_pointer {});

fnptr_t file_init = stack_pointer {}; // { dg-warning "different calling convention" }
stk_fnptr_t reverse_init = register_pointer {}; // { dg-warning "different calling convention" }

fnptr_t
give (stack_pointer p)
{
  return p; // { dg-warning "different calling convention" }
}

struct holder
{
  fnptr_t member;
  holder () : member (stack_pointer {}) {} // { dg-warning "different calling convention" }
};

void
use (stack_pointer p, register_pointer r)
{
  fnptr_t copy = p; // { dg-warning "different calling convention" }
  fnptr_t direct (p); // { dg-warning "different calling convention" }
  fnptr_t braced { p }; // { dg-warning "different calling convention" }
  copy = p; // { dg-warning "different calling convention" }
  take (p); // { dg-warning "different calling convention" }
  take_stack (r); // { dg-warning "different calling convention" }
  void (*through) (fnptr_t) = take;
  through (p); // { dg-warning "different calling convention" }
  take_default (); // { dg-warning "different calling convention" }
  take (stack_pointer {}); // { dg-warning "different calling convention" }
  fnptr_t from_reference = stack_reference {}; // { dg-warning "different calling convention" }
  take (stack_reference {}); // { dg-warning "different calling convention" }

  // Matching conventions and an unselected conversion stay quiet.
  stk_fnptr_t matching_stack = p;
  fnptr_t matching_register = r;
  take (r);
  take_stack (p);
  take_int (selectable {});

  // The explicitly requested conversion remains quiet.
  fnptr_t ccast = (fnptr_t) p;
  fnptr_t scast = static_cast<fnptr_t> (p);
  fnptr_t fcast = fnptr_t (p);
  take (static_cast<fnptr_t> (p));
}

template <typename T>
fnptr_t
template_return (T p)
{
  return p; // { dg-warning "different calling convention" }
}

fnptr_t instantiated = template_return (stack_pointer {});
