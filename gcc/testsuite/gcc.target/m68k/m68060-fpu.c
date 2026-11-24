/* The 68060's FPU instructions (MC68060 User's Manual, Table 10-4): fddiv
   takes 37 cycles, so the add of its result issues 37 cycles after it, and
   an FPU instruction from a data register is pOEP-only, alone.  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative or pc-relative addresses" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" } { "" } } */
/* { dg-options "-O2 -m68060 -m68881 -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

double
fdiv_use (double x, double y, double z)
{
  return x / y + z;
}

double
from_reg (int i, double z)
{
  return (double) (i * 3) * z;
}

/* { dg-final { scan-rtl-dump {;;\s+1--> b\s+0: i\s+\d+ fp0=fp0/\[sp\+0xc\] +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {;;\s+38--> b\s+0: i\s+\d+ fp0=fp0\+\[sp\+0x14\] } "sched2" } } */
/* { dg-final { scan-rtl-dump {fp\d=flt\(d\d\) +:m68060_single\n} "sched2" } } */
