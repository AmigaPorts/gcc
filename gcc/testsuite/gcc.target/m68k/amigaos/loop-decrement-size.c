/* { dg-do compile } */
/* { dg-options "-Os -m68030 -mregparm=4 -msoft-float -fbaserel -msmall-code" } */
/* Keep the independent pointer update before the counter decrement so
   the equality branch can reuse its flags.  */
extern void consume (int *);
extern int values[10];

void
visit (void)
{
  for (int i = 0; i < 10; ++i)
    consume (&values[i]);
}

/* { dg-final { scan-assembler {subq\.l #1,d[0-7]\n\tjne } } } */
