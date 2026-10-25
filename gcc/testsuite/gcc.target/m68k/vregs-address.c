/* The inline strcat below loaded its destination with one lea of
   virtual-stack-vars + strlen (want) - 600.  vregs instantiated only
   register and memory operands, not *lea's address operand, so the
   virtual register stayed, combine dropped it, and the copy went to an
   absolute address.  Reduced from GFOffice's OWFLibTest.  */
/* { dg-do compile } */
/* { dg-options "-O1 -m68020 -fdump-rtl-vregs" } */
/* { dg-final { scan-rtl-dump-not "virtual-" "vregs" } } */

#define PHRASE "owf.library 2 inflates this with libowf's own inflate. "

int
texts_differ (const char *expect)
{
  static const char phrase[] = PHRASE;
  char want[600];
  int i, n;

  want[0] = 0;
  for (i = 0; i < 6; i++)
    __builtin_strcat (want, phrase);
  for (n = 0; expect[n] && want[n] == expect[n]; n++)
    ;
  return want[n] != expect[n];
}
