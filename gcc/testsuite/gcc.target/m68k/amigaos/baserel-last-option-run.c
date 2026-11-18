/* { dg-do run } */
/* { dg-options "-O2 -fbaserel32 -fbaserel -save-temps" } */
/* The last of -fbaserel, -fbaserel32, -resident and -resident32 decides.
   The driver used to keep both options here: no multilib matched, so the
   16-bit base-relative code was linked with the default libraries and
   crashed at startup.  */

int x = 1;
int y[4] = { 2, 3, 4, 5 };

int main (void)
{
  return x + y[3] != 6;
}

/* { dg-final { scan-assembler {_x:W\(a4\)} } } */
