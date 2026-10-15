/* AmigaOS maps address zero; a null read must remain a memory access.  */
/* { dg-do compile } */
/* { dg-options "-O2" } */
/* { dg-final { scan-assembler-not "trap[ \t]+#7" } } */

volatile unsigned long sink;

void
read_address_zero (void)
{
  sink = *(unsigned long *) 0;
}
