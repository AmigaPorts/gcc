/* -mschulz does not apply to interrupt handlers: rounding the interrupted
   code's FP registers to double would corrupt them, so a handler still
   saves them whole with fmovem.x.  */
/* { dg-do compile } */
/* { dg-options "-O2 -m68040 -m68881 -fomit-frame-pointer -mschulz" } */
/* { dg-final { scan-assembler {fmovem #[0-9]+,-\(sp\)} } } */
/* { dg-final { scan-assembler-not {fmove\.d fp[0-7],-\(sp\)} } } */

extern volatile double sample;

void __attribute__ ((interrupt_handler))
tick (void)
{
  sample = sample * 0.5 + 1.0;
}
