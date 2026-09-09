/* Amiga register-parameter ABI: a prototype/definition spelling mismatch
   (return type `struct _Handle *` vs the typedef `Handle *`) makes the C
   front-end composite the function type.  The composited type is what a
   caller in the same translation unit uses, so it must keep the
   __asm("reg") bindings too: the caller has to load a0/d0/d1, not push on
   the stack.  Companion of asmreg-tag-typedef.c, which checks the callee
   side.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1 -fno-inline" } */

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

/* { dg-final { scan-assembler "move.l 4\\(sp\\),a0" } } */
/* { dg-final { scan-assembler "move.w 10\\(sp\\),d0" } } */
/* { dg-final { scan-assembler "move.l 12\\(sp\\),d1" } } */
/* { dg-final { scan-assembler-not "-\\(sp\\)" } } */
