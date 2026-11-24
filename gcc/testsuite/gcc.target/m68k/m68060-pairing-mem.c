/* The 68060 makes at most one data memory reference per instruction pair
   (MC68060 User's Manual, 10.1.4).  An FPU instruction with a memory operand
   takes the memory unit, so sched2 can't put an independent integer load or
   store in its cycle, while a register-only FPU instruction still pairs
   with one.  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative or pc-relative addresses" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" } { "" } } */
/* { dg-options "-O2 -m68060 -m68881 -fsched-verbose=1 -fdump-rtl-sched2" } */

double
fpu_mem (const double *p, const int *q, int *r, int n)
{
  double acc = 0.0;
  for (int i = 0; i < n; i++)
    {
      acc += p[i];
      r[i] = q[i] + 1;
    }
  return acc;
}

double
fpu_reg (double a, const int *q, int *r, int n)
{
  double acc = 0.0;
  for (int i = 0; i < n; i++)
    {
      acc += a;
      r[i] = q[i] + 1;
    }
  return acc;
}

/* { dg-final { scan-rtl-dump {fp\d=fp\d\+\[a\d\+\+\] +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump-not {;;\s+(\d+)--> [^\n]* fp\d=fp\d\+\[[^\n]*\n;;\s+\1--> [^\n]*m68060_mem} "sched2" } } */
/* { dg-final { scan-rtl-dump {;;\s+(\d+)--> [^\n]* fp\d=fp\d\+fp\d +:m68060_pOEP\n;;\s+\1--> [^\n]*m68060_mem} "sched2" } } */
