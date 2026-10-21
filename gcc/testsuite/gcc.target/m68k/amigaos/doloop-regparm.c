/* { dg-do compile } */
/* { dg-options "-O2 -fomit-frame-pointer -m68040 -mregparm=2" } */

/* With the arguments in d0, d1, a0 and a1, the only free scratch register
   is an address register.  The loop counter still belongs in a data
   register for dbra: saving d2 once beats a subq/cmpa/jne per iteration. */
unsigned long
checksum (unsigned long seed, const void *data, unsigned long len)
{
  const unsigned char *p = data;
  unsigned long h = seed ? seed : 2166136261UL;

  while (len--)
    h = (h ^ *p++) * 16777619UL;
  return h;
}

/* { dg-final { scan-assembler "dbra %?d\[0-7\]" } } */
/* { dg-final { scan-assembler-not "cmpa" } } */
