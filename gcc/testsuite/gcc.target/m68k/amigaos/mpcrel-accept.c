/* -mpcrel makes m68k_option_override set flag_pic to 1, the same value
   as -fpic, but it addresses pc-relative without a GOT; make sure
   rejecting -fpic/-fPIC does not catch it.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-mpcrel" } */

int other (void);

int
get (void)
{
  return other () + 1;
}
