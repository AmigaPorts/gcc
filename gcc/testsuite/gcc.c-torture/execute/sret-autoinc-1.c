/* A struct returned through the hidden struct-return pointer, with a
   by-value struct argument copied to the stack.  On m68k the block move
   copies the incoming argument with post_inc through a fresh pseudo.
   instantiate_virtual_regs dropped the REG_INC note on that pseudo's
   setter, so cse1 (and gcse cprop) reused the incremented pseudo as the
   struct-return pointer: app() wrote its result over B instead of A.  */

extern void abort (void);

typedef struct
{
  unsigned char kind, pad;
  unsigned short off;
  int size;
  union { void *ptr; double n; } u;
} jv;

static jv freed;

__attribute__ ((noinline, noclone)) jv
app (jv s, const char *d, unsigned len)
{
  s.kind = 7;
  s.size += len + d[0];
  return s;
}

__attribute__ ((noinline, noclone)) const char *
val (jv s)
{
  return s.u.ptr;
}

__attribute__ ((noinline, noclone)) unsigned
len_of (jv s)
{
  return s.size;
}

__attribute__ ((noinline, noclone)) void
jfree (jv s)
{
  freed = s;
}

__attribute__ ((noinline, noclone)) jv
concat (jv a, jv b)
{
  a = app (a, val (b), len_of (b));
  jfree (b);
  return a;
}

int
main (void)
{
  static const char str[] = "x";
  jv a = { 1, 0, 0, 10, { 0 } };
  jv b = { 2, 0, 0, 3, { (void *) str } };
  jv r = concat (a, b);

  if (r.kind != 7 || r.size != 10 + 3 + 'x')
    abort ();
  if (freed.kind != 2 || freed.size != 3 || freed.u.ptr != str)
    abort ();
  return 0;
}
