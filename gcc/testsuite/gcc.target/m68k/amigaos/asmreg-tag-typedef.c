/* Amiga register-parameter ABI: each __asm("reg") binding is stored as a type
   attribute.  A prototype/definition spelling mismatch (here the return type:
   struct tag `struct _F *` vs the typedef `F *`) makes the C front-end
   composite the function type, which must not drop those bindings.  The callee
   has to read its arguments from d0/d1, not the stack.  Regression from the
   C23 composite_type rework (remove_qualifiers stripping the attribute).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1" } */

typedef struct _F { int x; } F;

struct _F *f (long a __asm ("d0"), long b __asm ("d1"));	/* prototype: tag */
F *f (long a __asm ("d0"), long b __asm ("d1"))			/* definition: typedef */
{
  return (F *) (a + b);
}

/* Without the m68k backend fix the composited type drops the bindings and the
   argument comes off the stack; xfail marks that pre-fix state (the fix commit
   drops the xfail).  */
/* { dg-final { scan-assembler "add.l d1,d0" { xfail m68k-*-amigaos* } } } */
