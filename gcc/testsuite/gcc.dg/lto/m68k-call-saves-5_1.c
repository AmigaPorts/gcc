extern unsigned other (unsigned *);

/* Two values are live across the call.  */
unsigned
step (unsigned *p, unsigned x)
{
  return other (p) + x + p[1];
}

__attribute__ ((noipa)) unsigned
other (unsigned *p)
{
  return p[0] * 3;
}
