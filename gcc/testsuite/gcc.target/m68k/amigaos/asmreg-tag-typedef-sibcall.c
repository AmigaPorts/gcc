/* Amiga register-parameter ABI: after a prototype/definition spelling
   mismatch (struct tag vs typedef return type) the callee's decl carries
   the composited function type, a different node from the one the call
   expression uses.  m68k_is_ok_for_sibcall must compare against the
   call's type, or every tail call to such a function is refused and
   compiled as jsr/rts instead of jra.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -fno-inline" } */

typedef struct _Handle { long value; } Handle;

/* prototype: struct tag */
struct _Handle *open_handle (char *name __asm ("a0"),
			     unsigned short mode __asm ("d0"),
			     long form __asm ("d1"));

/* definition: typedef */
Handle *open_handle (char *name __asm ("a0"),
		     unsigned short mode __asm ("d0"),
		     long form __asm ("d1"))
{
  return (Handle *) (name + mode + form);
}

Handle *caller (char *name, unsigned short mode, long form)
{
  return open_handle (name, mode, form);
}

/* { dg-final { scan-assembler "jra _open_handle" } } */
/* { dg-final { scan-assembler-not "jsr _open_handle" } } */
