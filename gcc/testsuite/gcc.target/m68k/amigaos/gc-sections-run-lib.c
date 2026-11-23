/* The other file of gc-sections-run.c.  */

extern int flag;

__attribute__ ((constructor)) static void
init (void)
{
  flag = 42;
}
