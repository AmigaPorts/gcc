/* { dg-do compile } */
/* { dg-options "-O2 -m68030 -mregparm=4 -msoft-float -fbaserel -msmall-code" } */
/* The redundant counter test also occurs when optimizing for speed.  */
extern void consume (int *);
extern int values[10];
void visit (void)
{
  for (int i = 0; i < 10; ++i)
    consume (&values[i]);
}
/* { dg-final { scan-assembler {subq\.l #1,d[0-7]\n\tjne } } } */
