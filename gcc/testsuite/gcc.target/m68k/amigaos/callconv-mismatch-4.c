/* -Wno-callconv-mismatch disables the diagnostic; the conversion is
   accepted either way.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-m68000 -Os -mregparm=3 -Wno-callconv-mismatch" } */

typedef unsigned long size_t;
__attribute__ ((__stkparm__)) void *stk_copy (void *, const void *, size_t);
void *reg_copy (void *, const void *, size_t);
void *(*quiet) (void *, const void *, size_t) = stk_copy;

void *quiet_conditional (int choose_stack)
{
  return (choose_stack ? stk_copy : reg_copy) (0, 0, 0);
}
