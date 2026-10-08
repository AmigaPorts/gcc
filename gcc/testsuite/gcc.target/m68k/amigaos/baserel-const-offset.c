/* { dg-do compile } */
/* { dg-options "-Os -fbaserel -m68020 -fomit-frame-pointer" } */
/* Reduced from kicksmash32's ROM switcher.  A read-only object lives in
   the text segment and must be addressed absolutely under -fbaserel.  The
   baserel pass got that right for a bare symbol but turned "symbol + offset"
   into an a4-relative reference, which the hunk linker then resolved as if
   the symbol were in the data hunk.  */

extern void serial_puts (const char *);

__attribute__ ((section (".romver")))
const char rom_id[] = "$VER: romswitcher 2.0";

const char plain_id[] = "$VER: plain";

const char *const names[] = { "one", "two" };

int counter;

void f (int i)
{
  serial_puts (rom_id + 6);
  serial_puts (plain_id + 6);
  serial_puts (names[i]);
  counter++;
}

/* libnix's __initcpp(): the constructor list is a far object in a
   .list_* section that the script keeps in text.  The pointer arithmetic
   became "add.l #___CTOR_LIST__+4:W,d0", and every baserel C++ program
   silently skipped its static constructors.  */
typedef void (*func_ptr) (void);
extern __far func_ptr __CTOR_LIST__[];

void initcpp (void)
{
  func_ptr *p0 = __CTOR_LIST__ + 1;
  func_ptr *p;
  for (p = p0; *p; p++);
  while (p > p0)
    (*--p) ();
}

/* { dg-final { scan-assembler-not {_rom_id\+6:W} } } */
/* { dg-final { scan-assembler-not {_plain_id\+6:W} } } */
/* { dg-final { scan-assembler-not {_names:W} } } */
/* { dg-final { scan-assembler-not {CTOR_LIST__(\+[0-9]+)?:W} } } */
/* { dg-final { scan-assembler {_counter:W\(a4\)} } } */
