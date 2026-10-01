/* -msasregparm: two registers by default and no spill into the other register
   class, so a plain f(int,int,int) passes d0,d1 and the stack, as regparm (2)
   does; regparm (1) and stkparm do not.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -msasregparm" } */

int plain (int, int, int);
__attribute__ ((regparm (2))) int r2 (int, int, int);
__attribute__ ((regparm (1))) int r1 (int, int, int);
__attribute__ ((__stkparm__)) int stk (int, int, int);

int (*from_r2) (int, int, int) = r2;
int (*from_r1) (int, int, int) = r1;		/* { dg-warning "different calling convention" } */
int (*from_stk) (int, int, int) = stk;		/* { dg-warning "different calling convention" } */
