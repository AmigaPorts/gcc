/* -mbaserel addresses a global relative to r2.  */
/* { dg-do compile } */
/* { dg-options "-O2 -mbaserel" } */

int g;
int f (void) { return g; }

/* { dg-final { scan-assembler "g@brel@" } } */
