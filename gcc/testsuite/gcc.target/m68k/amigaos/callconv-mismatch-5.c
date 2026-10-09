/* With explicit -mregparm=0 on AmigaOS, regparm (0) and attributes
   unrelated to argument passing keep the stack count, like a plain
   prototype.  Only regparm (N) with N > 0 overrides it.  saveds itself
   warns without -fbaserel, hence -Wno-attributes.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=0 -Wno-attributes" } */

int plain (int);
__attribute__ ((regparm (0))) int r0 (int);
__attribute__ ((regparm (2))) int r2 (int);
__attribute__ ((__stkparm__)) int stk (int);
__attribute__ ((saveds)) int sv (int);

int (*plain_from_r0) (int) = r0;
__attribute__ ((regparm (0))) int (*r0_from_plain) (int) = plain;
__attribute__ ((regparm (0))) int (*r0_from_r2) (int) = r2; /* { dg-warning "different calling convention" } */
int (*plain_from_stk) (int) = stk;
int (*plain_from_sv) (int) = sv;
__attribute__ ((regparm (0))) int (*r0_from_sv) (int) = sv;
