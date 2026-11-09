/* fold_mem_offsets folded the frame offset into two of the three stores of
   a compound literal and left the third at its old displacement, below the
   stack pointer.  That store was a memory-to-memory move, which the pass
   takes as a root for its source operand only, so the use of the address
   register in its destination was never a candidate.  Reduced from libnix's
   printf.  The first field comes from memory, *startp, so the store is a
   memory-to-memory move whatever the register allocation.

   The stack below main is zeroed first, so a store left behind leaves the
   piece's startPos 0, and the piece is not counted.  */

/* { dg-do run } */
/* { dg-options "-O2 -fomit-frame-pointer" } */

extern void abort (void);

struct piece {
  int startPos;
  int count;
  long c;
};
short preci, exponent;
long type;
long outbuffer[64];
long *outp, *format;
int startPos, padcount;
int *startp;

int __attribute__((noipa))
f (void)
{
  long outcount = 0;
  while (*format) {
    long ptr = *(format + 1);
    type = ptr;
    switch (ptr)
    case L'a':
    case L'A':
    case L'f': {
      struct piece pieces[8];
      int npieces = 0, vlen = 0, pi = 0;
      if (exponent < 0 && ptr == L'f')
	pieces[npieces++] = (struct piece){};
      preci -= startPos;
      if (preci)
	pieces[npieces++] = (struct piece){*startp, preci};
      if (!(ptr == L'f'))
	if (exponent > 9)
	  outp++;
      for (; pi < npieces; ++pi)
	vlen += pieces[0].count;
      padcount = vlen;
      int len = outp - outbuffer;
      pi = 0;
      for (int k = 0; k < len; ++k)
	while (pi < npieces && pieces[pi].startPos) {
	  outcount++;
	  ++pi;
	}
    }
    format += 2;
  }
  return outcount;
}

/* Zero the stack that f's frame will use.  */
void __attribute__((noipa))
clear_stack (void)
{
  volatile char buf[1024];
  for (int i = 0; i < (int) sizeof buf; i++)
    buf[i] = 0;
}

long fmt[] = { 1, L'f', 1, L'a', 1, L'A', 0 };
int start_val = 5;

int
main (void)
{
  format = fmt;
  startp = &start_val;
  outp = outbuffer + 5;
  exponent = 12;
  preci = 7;
  startPos = 2;
  clear_stack ();
  if (f () != 3 || padcount != 1 || outp != outbuffer + 7 || preci != 1
      || type != L'A')
    abort ();
  return 0;
}
