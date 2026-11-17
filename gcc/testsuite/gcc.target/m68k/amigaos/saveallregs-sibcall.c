/* { dg-do compile } */
/* { dg-options "-O2" } */
/* A saveallregs function calls with jsr and returns with rts, never with
   a tail jump.  */

__saveallregs void touch (void);
__saveallregs void check (long x __asm ("d1"));
void clobber (void);

__saveallregs void call_touch (void) { touch (); }
__saveallregs void call_check (void) { check (42); }
__saveallregs void call_clobber (void) { clobber (); }

/* { dg-final { scan-assembler-times {\tjsr\s} 3 } } */
/* { dg-final { scan-assembler-not {\tj(ra|mp)\s} } } */
