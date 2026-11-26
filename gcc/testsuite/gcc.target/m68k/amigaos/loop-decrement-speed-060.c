/* { dg-do compile } */
/* { dg-options "-O2 -m68060 -mregparm=4 -msoft-float -fbaserel -msmall-code" } */
/* Keep this check when 68060 sched2 support is enabled after peephole2.  */
extern void consume (int *);
extern int values[10];
void visit (void)
{
  for (int i = 0; i < 10; ++i)
    consume (&values[i]);
}
/* { dg-final { scan-assembler {subq\.l #1,d[0-7]\n\tjne } } } */
