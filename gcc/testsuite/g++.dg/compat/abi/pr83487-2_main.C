// { dg-skip-if "small alignment" { m68k-*-amigaos* } }
extern void do_test (void);

int
main ()
{
  do_test ();
  return 0;
}
