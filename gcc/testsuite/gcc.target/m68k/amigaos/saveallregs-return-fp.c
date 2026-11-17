/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -m68881" } */
/* With an FPU, a saveallregs function that returns a double saves fp1-fp7
   and all of d0-d7/a0-a6; one that returns an int saves fp0-fp7 and all
   but d0.  fmovem and movem number their registers the other way round
   for -(sp) and (sp)+.  */

extern double g;

__saveallregs double fd (void) { return g * 2; }
__saveallregs int fi (void) { return g; }

/* fd: fp1-fp7 */
/* { dg-final { scan-assembler-times {fmovem #254,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {fmovem \(sp\)\+,#127} 1 } } */
/* fi: fp0-fp7 */
/* { dg-final { scan-assembler-times {fmovem #255,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {fmovem \(sp\)\+,#255} 1 } } */
/* fd: d0-d7/a0-a6; fi: d1-d7/a0-a6 */
/* { dg-final { scan-assembler-times {movem\.l #65534,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {movem\.l #32766,-\(sp\)} 1 } } */
