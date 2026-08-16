/* { dg-lto-do link } */
/* { dg-skip-if "" { ! { m68k-*-amigaos* } } } */
/* { dg-lto-options { { -flto -g -noixemul } } } */

int
main (void)
{
  return 0;
}
