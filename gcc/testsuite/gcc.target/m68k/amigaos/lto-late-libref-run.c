/* A variable defined here that only a libnix member references, and that
   member is pulled in after the LTO plugin resolution because the call
   to it (memcpy for the struct copy) is synthesized at expand time.
   ld used to report SysBase as IR-only, gcc made it local, and the
   memcpy.o reference was emitted as a reloc against hunk 0: the program
   then loaded its first instruction word as the exec base and crashed.
   -nostartfiles keeps libnix's own SysBase definition out so this file
   is the one defining it, as in a library or module build.

   The second source only makes this a two-file LTO link: with a single
   input ld places the compiled LTO object after the library members
   pulled in later, and the hunk entry point is the first text byte.
   One partition keeps _start in the first object.  */

/* { dg-do run } */
/* { dg-skip-if "libnix base variables" { ! { m68k-*-amigaos* } } } */
/* { dg-additional-options "-flto -flto-partition=one -noixemul -nostartfiles" } */
/* { dg-additional-sources "lto-late-libref-run-lib.c" } */

void *SysBase;

struct big { char c[4096]; } a, b;

int __attribute__ ((used))
_start (void)
{
  SysBase = *(void **) 4;
  b.c[0] = 1;
  b.c[4095] = 2;
  __asm__ volatile ("" : : "g" (&a), "g" (&b) : "memory");
  a = b;
  __asm__ volatile ("" : : "g" (&a), "g" (&b) : "memory");
  return (a.c[0] == 1 && a.c[4095] == 2) ? 0 : 1;
}
