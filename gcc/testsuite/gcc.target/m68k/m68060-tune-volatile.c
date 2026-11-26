/* With -m68000 -mtune=68060, a zero stored to a volatile MEM is a MOVE #0,
   because the 68000's CLR would read it first.  Like any MOVE of an
   immediate to memory, it is pOEP-until-last on the 68060 (MC68060 User's
   Manual, Table 10-2).  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative or pc-relative addresses, register arguments, another CPU" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" "-mregparm*" "-msasregparm" "-m680[2-6]0*" "-mcpu=*" } { "" } } */
/* { dg-options "-O2 -m68000 -mtune=68060 -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

void
store_zero (volatile int *p, int *q, int a, int b)
{
  *p = 0;
  *q = a + b;
}

/* { dg-final { scan-rtl-dump {\[%?a\d\]=0 +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
