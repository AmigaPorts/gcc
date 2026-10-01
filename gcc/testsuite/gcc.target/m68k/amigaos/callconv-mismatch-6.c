/* Types whose arguments go to the same places are not diagnosed even when
   their regparm counts differ: with -mregparm=3, f(int) passes d0 under
   regparm (1), (2) and the default alike, and f(int,int) d0,d1 under
   regparm (2) and the default.  A third argument separates them.  Every
   argument is compared: structures take no register, so after sixteen of
   them an int still goes to d0, where __stdargs passes it on the stack.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3" } */

__attribute__ ((regparm (1))) int one1 (int);
__attribute__ ((regparm (2))) int one2 (int);
__attribute__ ((regparm (2))) int two2 (int, int);
__attribute__ ((regparm (2))) int three2 (int, int, int);
struct s { long a, b; };
#define S4 struct s, struct s, struct s, struct s
__attribute__ ((__stkparm__)) int stk16 (S4, S4, S4, S4, int);

__attribute__ ((regparm (2))) int (*one2_from_one1) (int) = one1;
int (*one_from_one2) (int) = one2;
int (*two_from_two2) (int, int) = two2;
int (*three_from_three2) (int, int, int) = three2;	/* { dg-warning "different calling convention" } */
int (*from_stk16) (S4, S4, S4, S4, int) = stk16;	/* { dg-warning "different calling convention" } */
