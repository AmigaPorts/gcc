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

/* { dg-final { scan-assembler-not {_rom_id\+6:W} } } */
/* { dg-final { scan-assembler-not {_plain_id\+6:W} } } */
/* { dg-final { scan-assembler-not {_names:W} } } */
/* { dg-final { scan-assembler {_counter:W\(a4\)} } } */
