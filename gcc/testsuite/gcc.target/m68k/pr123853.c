/* PR rtl-optimization/123853, PR target/123076: late-combine folded the
   destination address into a move whose source pre-decrements the same
   register, "move.l -(%a0),(%a0,%d0.l)".  The 68k evaluates the source
   side effect first, so every store went one element off.  The 16-bit
   loop produced the same shape with move.w.  The destination base must
   be an integer global for the address to be folded, as in the glibc
   obstack code this was reduced from.  */
/* { dg-do run } */
/* { dg-options "-O2 -fomit-frame-pointer" } */
/* The run is the real check; this asserts the shape is gone even if a run
   passes by luck.  It only covers the path while the loops still fold a
   pre-decrement source into an indexed destination on the same register --
   if a later gcc stops doing that, re-derive a case that does.  */
/* { dg-final { scan-assembler-not {move\.[lw]\s+-\(a(\d)\),\d*\(a\1} } } */

struct hdr { char n; };

int base_l;
struct hdr *hdr_l;
long i_l;

__attribute__ ((noinline)) void
copy_l (void)
{
  int *dst = (int *) base_l;
  i_l = hdr_l->n;
  for (; i_l; i_l--)
    dst[i_l] = ((int *) hdr_l)[i_l];
}

int base_w;
struct hdr *hdr_w;
long i_w;

__attribute__ ((noinline)) void
copy_w (void)
{
  short *dst = (short *) base_w;
  i_w = hdr_w->n;
  for (; i_w; i_w--)
    dst[i_w] = ((short *) hdr_w)[i_w];
}

/* The SFmode move insn emits the same move.l and needs the same guard.  */
int base_f;
struct hdr *hdr_f;
long i_f;

__attribute__ ((noinline)) void
copy_f (void)
{
  float *dst = (float *) base_f;
  i_f = hdr_f->n;
  for (; i_f; i_f--)
    dst[i_f] = ((float *) hdr_f)[i_f];
}

/* The DFmode and DImode move insns emit two move.l, the first to 4(a0,d1.l),
   and need the same guard.  */
int base_d;
struct hdr *hdr_d;
long i_d;

__attribute__ ((noinline)) void
copy_d (void)
{
  double *dst = (double *) base_d;
  i_d = hdr_d->n;
  for (; i_d; i_d--)
    dst[i_d] = ((double *) hdr_d)[i_d];
}

int base_q;
struct hdr *hdr_q;
long i_q;

__attribute__ ((noinline)) void
copy_q (void)
{
  long long *dst = (long long *) base_q;
  i_q = hdr_q->n;
  for (; i_q; i_q--)
    dst[i_q] = ((long long *) hdr_q)[i_q];
}

int
main (void)
{
  static int src_l[16], out_l[16];
  static short src_w[16], out_w[16];
  static float src_f[16], out_f[16];
  static double src_d[16], out_d[16];
  static long long src_q[16], out_q[16];
  int i;

  for (i = 1; i < 16; i++)
    {
      src_l[i] = 100 + i;
      src_w[i] = 200 + i;
      src_f[i] = 300 + i;
      src_d[i] = 400 + i;
      src_q[i] = 0x100000000LL * i + 500 + i;
      out_l[i] = -1;
      out_w[i] = -1;
      out_f[i] = -1;
      out_d[i] = -1;
      out_q[i] = -1;
    }
  ((char *) src_l)[0] = 9;
  ((char *) src_w)[0] = 9;
  ((char *) src_f)[0] = 9;
  ((char *) src_d)[0] = 9;
  ((char *) src_q)[0] = 9;

  hdr_l = (struct hdr *) src_l;
  base_l = (int) out_l;
  copy_l ();
  hdr_w = (struct hdr *) src_w;
  base_w = (int) out_w;
  copy_w ();
  hdr_f = (struct hdr *) src_f;
  base_f = (int) out_f;
  copy_f ();
  hdr_d = (struct hdr *) src_d;
  base_d = (int) out_d;
  copy_d ();
  hdr_q = (struct hdr *) src_q;
  base_q = (int) out_q;
  copy_q ();

  for (i = 1; i <= 9; i++)
    if (out_l[i] != src_l[i] || out_w[i] != src_w[i] || out_f[i] != src_f[i]
	|| out_d[i] != src_d[i] || out_q[i] != src_q[i])
      __builtin_abort ();
  return 0;
}
