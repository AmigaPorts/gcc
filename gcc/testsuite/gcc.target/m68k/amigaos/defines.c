/* Amiga predefined macros (reserved forms, defined in every dialect).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga predefined macros" { ! { m68k-*-amigaos* } } } */

#ifndef __amigaos__
#error __amigaos__ is not defined
#endif

#ifndef __amiga__
#error __amiga__ is not defined
#endif

int dummy;
