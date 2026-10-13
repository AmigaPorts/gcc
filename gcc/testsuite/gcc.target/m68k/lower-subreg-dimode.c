/* The cost of DImode operations must reflect their two words, or
   lower-subreg never finds a gain and leaves every long long copy and
   shift whole until reload.  DFmode lives in one FPU register and must
   not be split.  */
/* { dg-do compile } */
/* { dg-options "-O2 -fdump-rtl-subreg1-details" } */

void
copy (unsigned long long *p, unsigned long long *q)
{
  *p = *q;
}

void
shift (unsigned long long *p, unsigned long long *q)
{
  *p -= *q << 32;
}

void
scale (double *p, double *q)
{
  *p = *q * 2.0;
}

/* { dg-final { scan-rtl-dump "Splitting mode DI for copy lowering" "subreg1" } } */
/* { dg-final { scan-rtl-dump "Splitting mode DI for ashift lowering with shift amounts = 32" "subreg1" } } */
/* { dg-final { scan-rtl-dump "Splitting reg" "subreg1" } } */
/* { dg-final { scan-rtl-dump "Skipping mode DF for copy lowering" "subreg1" } } */
