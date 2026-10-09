/* With a frame pointer and a frame of 32 KB or more the epilogue restores
   the saved registers through an a1 index.  Before a sibcall a1 may hold
   the call target (p is in a0 here), so the sibcall epilogue must walk the
   stack pointer down to the save area instead.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -O2 -fno-omit-frame-pointer -mregparm=4" } */

typedef long (*fn_t) (void *p);
extern void fill (char *buf, long n);

long
f (void *p, fn_t fn, long n)
{
  volatile char buf[40000];
  buf[n] = 1;
  fill (0, buf[n + 1]);
  return fn (p);
}

/* { dg-final { scan-assembler-times "jmp \\(a1\\)" 1 } } */
/* { dg-final { scan-assembler-not "\\(a1,a5\\.l\\)" } } */
/* { dg-final { scan-assembler-not "jsr \\(a" } } */
