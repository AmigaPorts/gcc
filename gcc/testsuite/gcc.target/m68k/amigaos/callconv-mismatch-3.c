/* Without -mregparm every function passes its arguments on the stack, so
   __stdargs changes nothing and is not diagnosed; an explicit regparm
   count still is.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os" } */

__attribute__ ((__stkparm__)) int stk (int, int);
__attribute__ ((regparm (2))) int r2 (int, int);
int plain (int, int);

int (*from_stk) (int, int) = stk;
int (*from_r2) (int, int) = r2;			/* { dg-warning "different calling convention" } */
__attribute__ ((__stkparm__)) int (*stk_from_plain) (int, int) = plain;
