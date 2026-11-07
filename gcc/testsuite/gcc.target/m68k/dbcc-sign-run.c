/* { dg-do run } */
/* { dg-options "-O2" } */

/* zlib's inflate_table() checks a Huffman code for over-subscription with
   this loop.  The early exit tests the sign of LEFT, and the final peephole
   that folds it into a dbmi must then leave the loop on mi, not le: a
   complete code ends with LEFT at 0, which is not over-subscribed.  */

extern void abort (void);

#define MAXBITS 15

__attribute__ ((noipa)) int
check_lengths (const unsigned short *count)
{
  int left = 1;
  unsigned len;

  for (len = 1; len <= MAXBITS; len++)
    {
      left <<= 1;
      left -= count[len];
      if (left < 0)
	return -1;
    }
  return left;
}

int
main (void)
{
  unsigned short count[MAXBITS + 1] = { 0 };

  /* Two codes of length 1: complete, LEFT ends at 0.  */
  count[1] = 2;
  if (check_lengths (count) != 0)
    abort ();

  /* One code of length 1: incomplete, LEFT ends positive.  */
  count[1] = 1;
  if (check_lengths (count) != 1 << (MAXBITS - 1))
    abort ();

  /* Three codes of length 1: over-subscribed.  */
  count[1] = 3;
  if (check_lengths (count) != -1)
    abort ();

  return 0;
}
