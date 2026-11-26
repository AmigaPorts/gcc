/* The 68060's secondary pipeline takes no PC-relative address (MC68060
   User's Manual, 10.1.3): with -mpcrel, lea (_table+16:w,pc) goes in the
   primary pipeline, with a secondary instruction beside it.  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative data, register arguments, 16-bit int" { *-*-* } { "-fbaserel*" "-resident*" "-mregparm*" "-msasregparm" "-mshort" } { "" } } */
/* { dg-options "-O2 -m68060 -mpcrel -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

static int table[16];

int *
pcrel (int *p, int a, int b)
{
  int *q = &table[4];

  *p = a + b;
  return q;
}

/* { dg-final { scan-rtl-dump {%?a\d=const\(`table'\+0x10\) +:m68060_pOEP\n} "sched2" } } */
