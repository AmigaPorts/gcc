/* fold_mem_offsets folded the frame offset into two of the three stores of
   a compound literal and left the third at its old displacement, below the
   stack pointer.  That store is a memory-to-memory move, which the pass
   takes as a root for its source operand only, so the use of the address
   register in its destination was never a candidate.  Reduced from libnix's
   printf; with the fix the pass folds nothing here.  */

/* { dg-do compile } */
/* { dg-options "-O2 -fomit-frame-pointer -fdump-rtl-fold_mem_offsets" } */

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
int __attribute__((noipa)) f() {
  long outcount = 0;
  while (*format) {
    long buffer[1];
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
        pieces[npieces++] = (struct piece){outp - outbuffer, preci};
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
  }
  return outcount;
}

/* { dg-final { scan-rtl-dump-not "Memory offset changed" "fold_mem_offsets" } } */
