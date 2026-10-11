/* The hunk linker has no GOT, and the 16-bit GOT displacement -fpic emits
   on 68020 gets a 32-bit reloc that clobbers the next instruction.  Rejecting
   -fpic broke too many configure scripts, so it is warned about and ignored,
   and the code addresses globals directly.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fpic -m68020" } */
/* { dg-warning "-fpic.*not supported on this target and are ignored" "" { target *-*-* } 0 } */
/* { dg-final { scan-assembler-not "@GOT" } } */

extern int g;
int f (void) { return g; }
