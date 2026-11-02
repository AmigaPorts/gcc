/* { dg-do compile } */
/* { dg-options "-Os -m68000 -fomit-frame-pointer" } */
/* A memset loop right after a call used to start a block with the call's
   argument still on the stack, which forces a frame pointer.  With a5
   reserved, that was an error.  */

register void *base __asm ("a5");

void use (void *);

void
clear_after_call (void *p)
{
  char buf[36];

  use (p);
  __builtin_memset (buf, 0, sizeof buf);
  use (buf);
}

/* { dg-final { scan-assembler "dbra" } } */
/* { dg-final { scan-assembler-not "link" } } */
