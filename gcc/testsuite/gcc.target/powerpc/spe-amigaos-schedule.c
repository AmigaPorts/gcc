/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=8548 -fdump-rtl-sched2-details" } */
/* { dg-final { scan-rtl-dump "ppc8548_mu_stage5" "sched2" } } */
/* { dg-final { scan-rtl-dump "ppc8540_mu_div\\*31" "sched2" } } */

double add (double a, double b) { return a + b; }
double multiply (double a, double b) { return a * b; }
double divide (double a, double b) { return a / b; }
