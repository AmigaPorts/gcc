/* Amiga register-parameter ABI: prototype and definition spell the return
   type differently (struct tag vs typedef), so the C front-end composites
   the function type.  A caller in the same translation unit and the callee
   must still agree on the __asm("reg") registers.  Swept over the torture
   options by amigaos.exp like asmreg-run.c.  */

/* { dg-do run } */

typedef struct _Handle { long value; } Handle;

volatile long seen_mode, seen_form;

/* prototype: struct tag */
struct _Handle *open_handle (char *name __asm ("a0"),
			     unsigned short mode __asm ("d0"),
			     long form __asm ("d1"));

/* definition: typedef */
__attribute__ ((noinline))
Handle *open_handle (char *name __asm ("a0"),
		     unsigned short mode __asm ("d0"),
		     long form __asm ("d1"))
{
  seen_mode = mode;
  seen_form = form;
  return (Handle *) name;
}

__attribute__ ((noinline))
Handle *caller (char *name, unsigned short mode, long form)
{
  return open_handle (name, mode, form);
}

int main (void)
{
  static Handle handle;

  if (caller ((char *) &handle, 0x1234, 0x56789abc) != &handle)
    __builtin_abort ();
  if (seen_mode != 0x1234 || seen_form != 0x56789abc)
    __builtin_abort ();
  return 0;
}
