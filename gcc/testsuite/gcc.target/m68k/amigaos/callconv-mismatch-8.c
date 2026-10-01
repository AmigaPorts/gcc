/* Before C23 an unprototyped pointer is called with the arguments it is
   given, in the default convention's registers.  It is compared with the
   other type's argument list.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -std=gnu17" } */

__attribute__ ((__stkparm__)) int stk (int, int);
int plain (int, int);

int (*knr_from_stk) () = stk;		/* { dg-warning "different calling convention" } */
int (*knr_from_plain) () = plain;
