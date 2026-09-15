/* Reduced from newlib k_rem_pio2.c.  LRA checks the address of *lea
   in VOIDmode; with a displacement reach of 0 the m68k backend accepted
   128 in the 68000 (d8,An,Xn) form, and frame elimination then grew a
   stack address past the 8-bit limit, so postreload rejected the insn.  */
/* { dg-do compile } */
/* { dg-options "-O2 -mlra -m68000 -fbaserel -w" } */

extern double floor (double);
extern double scalbn (double, int);
static const int init_jk[] = { 2, 3, 4, 6 };
static const double PIo2[] = { 0 };

static const double
  zero = 0.0,
  one = 1.0,
  two24 = 1.67772160000000000000e+07, twon24 = 5.96046447753906250000e-08;
int
kernel_rem_pio2 (double *x, double *y, int e0, int nx, int prec,
		 const long *ipio2)
{
  long jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih;
  double z, fw, f[20], fq[20], q[20];
  jk = init_jk[prec];
  jx = nx - 1;
  jv = (e0 - 3) / 24;
  if (jv < 0)
    jv = 0;
  q0 = e0 - 24 * (jv + 1);
  j = jv - jx;
  m = jx + jk;
  for (i = 0; i <= m; i++, j++)
    f[i] = (j < 0) ? zero : (double) ipio2[j];
  for (i = 0; i <= jk; i++)
    {
      for (j = 0, fw = 0.0; j <= jx; j++)
	fw += x[j] * f[jx + i - j];
      q[i] = fw;
    }
recompute:
  for (i = 0, j = jz, z = q[jz]; j > 0; i++, j--)
    {
    }
  z -= 8.0 * floor (z * 0.125);
  z -= (double) n;
  ih = 0;
  if (q0 > 0)
    {
      ih = iq[jz - 1] >> (23 - q0);
    }
  if (ih > 0)
    {
      n += 1;
      carry = 0;
      for (i = 0; i < jz; i++)
	{
	  j = iq[i];
	  if (carry == 0)
	    {
	      if (j != 0)
		{
		  carry = 1;
		  iq[i] = 0x1000000 - j;
		}
	    }
	  else
	    iq[i] = 0xffffff - j;
	}
      if (ih == 2)
	{
	  if (carry != 0)
	    z -= scalbn (one, (int) q0);
	}
    }
  if (z == zero)
    {
      for (i = jz - 1; i >= jk; i--)
	j |= iq[i];
      if (j == 0)
	{
	  for (k = 1; iq[jk - k] == 0; k++);
	  for (i = jz + 1; i <= jz + k; i++)
	    {
	      f[jx + i] = (double) ipio2[jv + i];
	      for (j = 0, fw = 0.0; j <= jx; j++)
		fw += x[j] * f[jx + i - j];
	      q[i] = fw;
	    }
	  jz += k;
	  goto recompute;
	}
      if (z >= two24)
	{
	}
      else
	iq[jz] = (long) z;
    }
  fw = scalbn (one, (int) q0);
  for (i = jz; i >= 0; i--)
    {
      q[i] = fw * (double) iq[i];
      fw *= twon24;
    }
  for (i = jz; i >= 0; i--)
    {
      for (fw = 0.0, k = 0; k <= jp && k <= jz - i; k++)
	fw += PIo2[k] * q[i + k];
    }
  switch (prec)
    {
    case 3:
      for (i = jz; i > 0; i--)
	{
	  y[0] = -fq[0];
	  y[1] = -fq[1];
	  y[2] = -fw;
	}
    }
}
