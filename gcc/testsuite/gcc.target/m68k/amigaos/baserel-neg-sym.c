/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel -m68040" } */
/* Reduced from lha's read_pt_len().  The dbra count of the fill loop is
   computed from the array's end minus its start, which became the
   constant (neg (RELOC16 tab)): final had no way to print it.  */

unsigned short tab[256];

void f (unsigned short c)
{
  for (int i = 0; i < 256; i++)
    tab[i] = c;
}
