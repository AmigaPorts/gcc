/* { dg-do compile } */
/* { dg-options "-O2 -fomit-frame-pointer -m68000 -mregparm=2 -mmangle-regparm" } */

/* libgcc's benaphore slow paths are built once, so <benaphore.h> fixes
   their convention, whatever -mregparm and -mmangle-regparm the caller
   uses: the lock in a0, and the names -mmangle-regparm gives them.  */

#include <benaphore.h>

benaphore_t b;

void
lock (void)
{
  benaphore_lock (&b);
}

void
unlock (void)
{
  benaphore_unlock (&b);
}

/* { dg-final { scan-assembler "lea _b,a0\n\tjra \"@__benaphore_lock_slow\"" } } */
/* { dg-final { scan-assembler "lea _b,a0\n\tjra \"@__benaphore_wake\"" } } */
/* { dg-final { scan-assembler-not "___benaphore" } } */
/* { dg-final { scan-assembler-not "\[0-7\]@__benaphore" } } */
