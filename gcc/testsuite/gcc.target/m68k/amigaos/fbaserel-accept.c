/* -fbaserel shares flag_pic with the generic pic modes (it sets it to 3);
   make sure rejecting -fpic/-fPIC does not catch it.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fbaserel" } */

int data = 1;

int
get (void)
{
  return data;
}
