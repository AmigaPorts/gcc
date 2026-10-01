// -Wno-callconv-mismatch silences the diagnostics of callconv-mismatch-1.C.

// { dg-do compile }
// { dg-options "-m68000 -Os -mregparm=3 -Wno-callconv-mismatch" }

typedef unsigned long size_t;
__attribute__ ((__stkparm__)) void *stk_copy (void *, const void *, size_t);
void *reg_copy (void *, const void *, size_t);
typedef void *(*cpy_t) (void *, const void *, size_t);
void take (cpy_t);

cpy_t file_init = stk_copy;

cpy_t
give (int c)
{
  take (stk_copy);
  file_init = stk_copy;
  return c ? stk_copy : reg_copy;
}
