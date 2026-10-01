/* -Wcallconv-mismatch compares where each argument goes, not regparm counts.
   With -mregparm=2 an f(int,int,int) passes d0,d1,a0; regparm(3) passes
   d0,d1,d2 and stkparm passes everything on the stack.  regparm (0) means
   the default, as it does to codegen.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=2" } */

__attribute__ ((regparm (2))) int r2 (int, int, int);
__attribute__ ((regparm (3))) int r3 (int, int, int);
__attribute__ ((__stkparm__)) int stk (int, int, int);
__attribute__ ((regparm (0))) int r0 (int, int, int);

int (*default_from_r2) (int, int, int) = r2;
int (*default_from_r3) (int, int, int) = r3;	/* { dg-warning "different calling convention" } */
int (*default_from_stk) (int, int, int) = stk;	/* { dg-warning "different calling convention" } */
__attribute__ ((regparm (3))) int (*r3_from_r2) (int, int, int) = r2;	/* { dg-warning "different calling convention" } */
__attribute__ ((regparm (3))) int (*r3_from_r3) (int, int, int) = r3;
int (*default_from_r0) (int, int, int) = r0;
