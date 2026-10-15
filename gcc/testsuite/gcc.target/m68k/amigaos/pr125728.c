/* { dg-do compile } */
/* { dg-options "-O2 -m68000" } */

/* A bitfield of a packed struct must not be read with a word or long
   access at an odd address, which traps on a 68000: late_combine widened
   the byte AND below to an and.l on a paradoxical subreg of the byte
   (PR target/125728).  newlib's towupper() reads its packed table so.  */

struct e { unsigned first:21; unsigned diff:8; unsigned mode:2; int delta:17; } __attribute__((packed));
extern struct e t[];

unsigned
g (void)
{
  return t[335].diff;
}

/* { dg-final { scan-assembler-not {\.[wl]\s+_t\+\d*[13579]\M} } } */
