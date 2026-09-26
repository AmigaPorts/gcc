/* The hunk linker has no GOT, so generic -fpic must be rejected instead of
   silently producing a corrupt executable (the 16-bit GOT displacement it
   emits on 68020 gets a 32-bit reloc that clobbers the next instruction).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga pic modes" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-fpic" } */
/* { dg-error "-fpic.*not supported on this target" "" { target *-*-* } 0 } */

int dummy;
