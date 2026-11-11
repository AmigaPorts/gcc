/* { dg-do run } */
/* { dg-shouldfail "the libraries are older than __oslibversion" } */
/* { dg-skip-if "ixemul opens no libraries for the program" { *-*-* } { "-mcrt=ixemul" } { "" } } */

/* A program that asks for newer libraries than the system has, with
   __oslibversion, must not start: the startup code names the libraries it
   could not open and exits with an error before main().  dos.library is
   one of them, so the report needs one of its own.  */

long __oslibversion = 666;

int
main (void)
{
  return 0;
}

/* { dg-output "dos.library failed to open" } */
