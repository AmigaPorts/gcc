/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel" } */
/* -fbaserel sets flag_pic, and with it flag_shlib, which made every global
   function interposable as in a shared library: leaf was called, not
   inlined.  A4-relative code has no symbol interposition.  */

int g;

int leaf (int x)
{
  return x * 3 + g;
}

int caller (int y)
{
  return leaf (y) + 1;
}

/* { dg-final { scan-assembler-not {\m(jsr|jbsr|bsr)} } } */
