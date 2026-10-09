/* The linker scripts put every .list_* section (constructor, init and exit
   lists) in the text hunk, so -fbaserel must not address them through a4
   even though the section is writable.  .dlist_* sections go to the data
   hunk and stay a4-relative.  newlib's crt0 defines its lists this way;
   with a4-relative references its constructors never ran.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga baserel" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -fomit-frame-pointer -fbaserel" } */

typedef void (*func_ptr) (void);

__attribute__((section(".list___CTOR_LIST__")))
func_ptr __CTOR_LIST__[] = {0};

__attribute__((section(".list___INIT_LIST__,\"aw\"")))
int __INIT_LIST__[1] = {0};

__attribute__((section(".dlist___LIB_LIST__")))
long __LIB_LIST__ = 0;

extern void use (void *);

void
f (void)
{
  use (__CTOR_LIST__ + 1);
  use (&__INIT_LIST__[0] + 1);
  use (&__LIB_LIST__ + 1);
  if (__CTOR_LIST__[0])
    use (0);
}

/* { dg-final { scan-assembler-not "CTOR_LIST__(\\+\[0-9\]+)?:W" } } */
/* { dg-final { scan-assembler-not "INIT_LIST__(\\+\[0-9\]+)?:W" } } */
/* { dg-final { scan-assembler "LIB_LIST__\\+4:W" } } */
