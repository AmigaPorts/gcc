/* clib4 has no small-data libraries, and the driver says so rather than
   leave it to the linker.  */
/* { dg-do link } */
/* { dg-require-effective-target amigaos_clib4 } */
/* { dg-options "-msdata=sysv" } */
/* { dg-error "msdata is not supported with -mcrt=clib4" "" { target *-*-* } 0 } */

int main (void) { return 0; }
