/* Storing the address of a local array stores the stack pointer.  As an
   add into memory, (set (mem) (plus (frame) (const_int))), the frame
   pointer and the offset were reloaded apart, and the stack pointer was
   rebuilt from them: lea (N,sp),a0; move.l a0,d0; add.l #-N,d0.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68000 -fomit-frame-pointer" } */

int *ptr;
void use (void);

void
small (void)
{
  int c[1024];
  ptr = c;
  use ();
}

void
big (void)
{
  int c[1024 * 16];
  ptr = c;
  use ();
}

/* { dg-final { scan-assembler-times "move\\.l sp,_ptr" 2 } } */
/* { dg-final { scan-assembler-not "add\\.l #-4096,d" } } */
/* { dg-final { scan-assembler-not "add\\.l #-65536,d" } } */
