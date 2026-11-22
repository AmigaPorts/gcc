/* { dg-do compile } */
/* { dg-options "-O2 -m68020-60 -m68881 -fexcess-precision=standard" } */
/* The long long to double libcall follows each function's flags, not the
   command line's.  */

double
standard (long long i)
{
  return (double) i;
}

__attribute__ ((optimize ("excess-precision=fast"))) double
fast (long long i)
{
  return (double) i;
}

__attribute__ ((optimize ("unsafe-math-optimizations"))) double
unsafe (unsigned long long i)
{
  return (double) i;
}

double
standard_again (unsigned long long i)
{
  return (double) i;
}

/* { dg-final { scan-assembler-times {___floatdidf_rounded} 1 } } */
/* { dg-final { scan-assembler-times {___floatundidf_rounded} 1 } } */
/* { dg-final { scan-assembler-times {___floatdidf\M} 1 } } */
/* { dg-final { scan-assembler-times {___floatundidf\M} 1 } } */
