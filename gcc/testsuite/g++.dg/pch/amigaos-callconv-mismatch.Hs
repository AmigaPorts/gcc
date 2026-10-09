// { dg-skip-if "m68k AmigaOS only" { ! m68k-*-amigaos* } }
// { dg-options "-m68000 -mregparm=3 -std=gnu++11" }

__attribute__ ((__stkparm__)) int stk_fn (int);
int reg_fn (int);
typedef int (*fnptr_t) (int);
typedef __attribute__ ((__stkparm__)) int (*stk_fnptr_t) (int);

struct stack_pointer
{
  operator stk_fnptr_t () const;
};

inline fnptr_t
choose (bool c)
{
  return c ? (fnptr_t) stk_fn : reg_fn;
}

template <typename T>
fnptr_t
choose_template (T c)
{
  return c ? static_cast<fnptr_t> (stk_fn) : reg_fn;
}
