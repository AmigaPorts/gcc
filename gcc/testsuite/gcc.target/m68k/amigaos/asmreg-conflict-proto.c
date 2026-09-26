/* Amiga register-parameter ABI: the __asm("reg") binding of a parameter is
   part of the function type.  A prototype and a definition that bind
   different registers, or where only one binds a register, are
   conflicting declarations, not a silent ABI split.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */

long f (long a __asm ("d0"), long b __asm ("d1"));	/* { dg-message "previous declaration" } */
long f (long a __asm ("d0"), long b __asm ("d2"))	/* { dg-error "conflicting types" } */
{
  return a + b;
}

long g (long a __asm ("d0"));				/* { dg-message "previous declaration" } */
long g (long a)						/* { dg-error "conflicting types" } */
{
  return a;
}

long h (long a);					/* { dg-message "previous declaration" } */
long h (long a __asm ("d0"))				/* { dg-error "conflicting types" } */
{
  return a;
}

/* Same bindings on both sides stay compatible.  */
long k (long a __asm ("d0"), long b __asm ("d1"));
long k (long a __asm ("d0"), long b __asm ("d1"))
{
  return a - b;
}
