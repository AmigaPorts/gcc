/* Amiga register-parameter ABI: each __asm("reg") binding is stored as a type
   attribute.  A prototype/definition spelling mismatch (here the return type:
   struct tag `struct _Handle *` vs the typedef `Handle *`) makes the C
   front-end composite the function type, which must not drop those bindings.
   The callee has to read its arguments from d0/d1, not the stack.  Regression
   from the C23 composite_type rework (remove_qualifiers stripping the
   attribute).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

typedef struct _Handle { int value; } Handle;

/* prototype: struct tag */
struct _Handle *open_handle (long mode __asm ("d0"), long form __asm ("d1"));

/* definition: typedef */
Handle *open_handle (long mode __asm ("d0"), long form __asm ("d1"))
{
  return (Handle *) (mode + form);
}

/* { dg-final { scan-assembler "add.l d1,d0" } } */
