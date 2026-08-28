/* This is a stripped down version of floatlib.c.  It supplies only those
   functions which exist in libgcc, but for which there is not assembly
   language versions in m68k/lb1sf68.S.

   It also includes simplistic support for extended floats (by working in
   double precision).  You must compile this file again with -DEXTFLOAT
   to get this support.  */

/*
** gnulib support for software floating point.
** Copyright (C) 1991 by Pipeline Associates, Inc.  All rights reserved.
** Permission is granted to do *anything* you want with this file,
** commercial or otherwise, provided this message remains intact.  So there!
** I would appreciate receiving any updates/patches/changes that anyone
** makes, and am willing to be the repository for said changes (am I
** making a big mistake?).
**
** Pat Wood
** Pipeline Associates, Inc.
** pipeline!phw@motown.com or
** sun!pipeline!phw or
** uunet!motown!pipeline!phw
**
** 05/01/91 -- V1.0 -- first release to gcc mailing lists
** 05/04/91 -- V1.1 -- added float and double prototypes and return values
**                  -- fixed problems with adding and subtracting zero
**                  -- fixed rounding in truncdfsf2
**                  -- fixed SWAP define and tested on 386
*/

/*
** The following are routines that replace the gnulib soft floating point
** routines that are called automatically when -msoft-float is selected.
** The support single and double precision IEEE format, with provisions
** for byte-swapped machines (tested on 386).  Some of the double-precision
** routines work at full precision, but most of the hard ones simply punt
** and call the single precision routines, producing a loss of accuracy.
** long long support is not assumed or included.
** Overall accuracy is close to IEEE (actually 68882) for single-precision
** arithmetic.  I think there may still be a 1 in 1000 chance of a bit
** being rounded the wrong way during a multiply.  I'm not fussy enough to
** bother with it, but if anyone is, knock yourself out.
**
** Efficiency has only been addressed where it was obvious that something
** would make a big difference.  Anyone who wants to do this right for
** best speed should go in and rewrite in assembler.
**
** I have tested this only on a 68030 workstation and 386/ix integrated
** in with -msoft-float.
*/

/* Prototypes for all functions that might be called by other functions.
   These need to be declared regardless of which specific function is
   being compiled.  */

/* Core single and double precision functions */
double __floatunsidf (unsigned long);
double __floatsidf (long);
float __floatsisf (long);
double __extendsfdf2 (float);
float __truncdfsf2 (double);
long __fixdfsi (double);
long __fixsfsi (float);
int __unordsf2(float, float);
int __unorddf2(double, double);
long __cmpdf2(double, double);

#ifdef EXTFLOAT
#ifndef __mcoldfire__
/* Extended precision prototypes - these override the non-EXTFLOAT versions */
double __truncxfdf2 (long double);
int __unordxf2 (long double, long double);
long double __extenddfxf2 (double);
long double __extendsfxf2 (float);
float __truncxfsf2 (long double);
long double __floatsixf (long);
long double __floatunsixf (unsigned long);
long __fixxfsi (long double);
long double __addxf3 (long double, long double);
long double __subxf3 (long double, long double);
long double __mulxf3 (long double, long double);
long double __divxf3 (long double, long double);
long double __negxf2 (long double);
long double __xf_pack(unsigned int sign, int exp, unsigned long long mant);
void __xf_unpack(long double x, unsigned int *sign, int *exp, unsigned long long *mant);
#ifdef EXTFLOATCMP
long __cmpxf2 (long double, long double);
long __eqxf2 (long double, long double);
long __nexf2 (long double, long double);
long __ltxf2 (long double, long double);
long __lexf2 (long double, long double);
long __gtxf2 (long double, long double);
long __gexf2 (long double, long double);
#endif /* EXTFLOATCMP */
#endif /* !__mcoldfire__ */
#else /* !EXTFLOAT */
/* Non-EXTFLOAT version */
double __truncxfdf2 (long double);
#endif /* EXTFLOAT */

/* the following deal with IEEE single-precision numbers */
#define EXCESS		126L
#define SIGNBIT		0x80000000L
#define HIDDEN		(1L << 23L)
#define SIGN(fp)	((fp) & SIGNBIT)
#define EXPMASK		0xFFL
#define EXP(fp)		(((fp) >> 23L) & 0xFF)
#define MANT(fp)	(((fp) & 0x7FFFFFL) | HIDDEN)
#define PACK(s,e,m)	((s) | ((e) << 23L) | (m))

/* the following deal with IEEE double-precision numbers */
#define EXCESSD		1022L
#define HIDDEND		(1L << 20L)
#define EXPDBITS	11
#define EXPDMASK	0x7FFL
#define EXPD(fp)	(((fp.l.upper) >> 20L) & 0x7FFL)
#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
#define MANTD(fp)	(((((fp.l.upper) & 0xFFFFF) | HIDDEND) << 10) | \
				(fp.l.lower >> 22))
#define MANTDMASK	0xFFFFFL /* mask of upper part */

/* the following deal with IEEE extended-precision numbers */
#define EXCESSX		16382L
#define HIDDENX		(1L << 31L)
#define EXPXBITS	15
#define EXPXMASK	0x7FFF
#define EXPX(fp)	(((fp.l.upper) >> 16) & EXPXMASK)
#define SIGNX(fp)	((fp.l.upper) & SIGNBIT)
#define MANTXMASK	0x7FFFFFFFL /* mask of upper part */

union double_long
{
  double d;
  struct {
      long upper;
      unsigned long lower;
    } l;
};

union float_long {
  float f;
  long l;
};

union long_double_long
{
  long double ld;
  struct
    {
      long upper;
      unsigned long middle;
      unsigned long lower;
    } l;
};

#ifndef EXTFLOAT

#ifdef __UNORDSF2
int
__unordsf2(float a, float b)
{
  union float_long fl;

  fl.f = a;
  if (EXP(fl.l) == EXP(~0u) && (MANT(fl.l) & ~HIDDEN) != 0)
    return 1;
  fl.f = b;
  if (EXP(fl.l) == EXP(~0u) && (MANT(fl.l) & ~HIDDEN) != 0)
    return 1;
  return 0;
}
#endif

#ifdef __UNORDDF2
int
__unorddf2(double a, double b)
{
  union double_long dl;

  dl.d = a;
  if (EXPD(dl) == EXPDMASK
      && ((dl.l.upper & MANTDMASK) != 0 || dl.l.lower != 0))
    return 1;
  dl.d = b;
  if (EXPD(dl) == EXPDMASK
      && ((dl.l.upper & MANTDMASK) != 0 || dl.l.lower != 0))
    return 1;
  return 0;
}
#endif

#ifdef __FLOATUNSIDF
/* convert unsigned int to double */
double
__floatunsidf (unsigned long a1)
{
  long exp = 32 + EXCESSD;
  union double_long dl;

  if (!a1)
    {
      dl.l.upper = dl.l.lower = 0;
      return dl.d;
    }

  while (a1 < 0x2000000L)
    {
      a1 <<= 4;
      exp -= 4;
    }

  while (a1 < 0x80000000L)
    {
      a1 <<= 1;
      exp--;
    }

  /* pack up and go home */
  dl.l.upper = exp << 20L;
  dl.l.upper |= (a1 >> 11L) & ~HIDDEND;
  dl.l.lower = a1 << 21L;

  return dl.d;
}
#endif

#ifdef __FLOATSIDF
/* convert int to double */
double
__floatsidf (long a1)
{
  long sign = 0, exp = 31 + EXCESSD;
  union double_long dl;

  if (!a1)
    {
      dl.l.upper = dl.l.lower = 0;
      return dl.d;
    }

  if (a1 < 0)
    {
      sign = SIGNBIT;
      a1 = (long)-(unsigned long)a1;
      if (a1 < 0)
	{
	  dl.l.upper = SIGNBIT | ((32 + EXCESSD) << 20L);
	  dl.l.lower = 0;
	  return dl.d;
        }
    }

  while (a1 < 0x1000000L)
    {
      a1 <<= 4;
      exp -= 4;
    }

  while (a1 < 0x40000000L)
    {
      a1 <<= 1;
      exp--;
    }

  /* pack up and go home */
  dl.l.upper = sign;
  dl.l.upper |= exp << 20L;
  dl.l.upper |= (a1 >> 10L) & ~HIDDEND;
  dl.l.lower = a1 << 22L;

  return dl.d;
}
#endif

#ifdef __FLOATUNSISF
/* convert unsigned int to float */
float
__floatunsisf (unsigned long l)
{
  double foo = __floatunsidf (l);
  return foo;
}
#endif

#ifdef __FLOATSISF
/* convert int to float */
float
__floatsisf (long l)
{
  double foo = __floatsidf (l);
  return foo;
}
#endif

#ifdef __EXTENDSFDF2
/* convert float to double */
double
__extendsfdf2 (float a1)
{
  register union float_long fl1;
  register union double_long dl;
  register long exp;
  register long mant;

  fl1.f = a1;

  dl.l.upper = SIGN (fl1.l);
  if ((fl1.l & ~SIGNBIT) == 0)
    {
      dl.l.lower = 0;
      return dl.d;
    }

  exp = EXP(fl1.l);
  mant = MANT (fl1.l) & ~HIDDEN;
  if (exp == 0)
    {
      /* Denormal.  */
      exp = 1;
      while (!(mant & HIDDEN))
	{
	  mant <<= 1;
	  exp--;
	}
      mant &= ~HIDDEN;
    }
  exp = exp - EXCESS + EXCESSD;
  /* Handle inf and NaN */
  if (exp == EXPMASK - EXCESS + EXCESSD)
    exp = EXPDMASK;
  dl.l.upper |= exp << 20;
  dl.l.upper |= mant >> 3;
  dl.l.lower = mant << 29;

  return dl.d;
}
#endif

#ifdef __TRUNCDFSF2
/* convert double to float */
float
__truncdfsf2 (double a1)
{
  register long exp;
  register long mant;
  register union float_long fl;
  register union double_long dl1;
  int sticky;
  int shift;

  dl1.d = a1;

  if ((dl1.l.upper & ~SIGNBIT) == 0 && !dl1.l.lower)
    {
      fl.l = SIGND(dl1);
      return fl.f;
    }

  exp = EXPD (dl1) - EXCESSD + EXCESS;

  sticky = dl1.l.lower & ((1 << 22) - 1);
  mant = MANTD (dl1);
  /* shift double mantissa 6 bits so we can round */
  sticky |= mant & ((1 << 6) - 1);
  mant >>= 6;
  if (exp == EXPDMASK - EXCESSD + EXCESS)
    {
      exp = EXPMASK;
      mant = (mant >> 1) | (mant & 1) | (!!sticky);
    }
  else
    {
      /* Check for underflow and denormals.  */
      if (exp <= 0)
	{
	  if (exp < -24)
	    {
	      sticky |= mant;
	      mant = 0;
	    }
	  else
	    {
	      sticky |= mant & ((1 << (1 - exp)) - 1);
	      mant >>= 1 - exp;
	    }
	  exp = 0;
	}

      /* now round */
      shift = 1;
      if ((mant & 1) && (sticky || (mant & 2)))
	{
	  int rounding = exp ? 2 : 1;

	  mant += 1;

	  /* did the round overflow? */
	  if (mant >= (HIDDEN << rounding))
	    {
	      exp++;
	      shift = rounding;
	    }
	}
      /* shift down */
      mant >>= shift;
      if (exp >= EXPMASK)
	{
	  exp = EXPMASK;
	  mant = 0;
	}
    }

  mant &= ~HIDDEN;

  /* pack up and go home */
  fl.l = PACK (SIGND (dl1), exp, mant);
  return (fl.f);
}
#endif

#ifdef __FIXDFSI
/* convert double to int */
long
__fixdfsi (double a1)
{
  register union double_long dl1;
  register long exp;
  register long l;

  dl1.d = a1;

  if (!dl1.l.upper && !dl1.l.lower)
    return 0;

  exp = EXPD (dl1) - EXCESSD - 31;
  l = MANTD (dl1);

  if (exp > 0)
    {
      /* Return largest integer.  */
      return SIGND (dl1) ? 0x80000000L : 0x7fffffffL;
    }

  if (exp <= -32)
    return 0;

  /* shift down until exp = 0 */
  if (exp < 0)
    l >>= -exp;

  return (SIGND (dl1) ? -l : l);
}
#endif

#ifdef __FIXSFSI
/* convert float to int */
long
__fixsfsi (float a1)
{
  double foo = a1;
  return __fixdfsi (foo);
}
#endif

#else /* EXTFLOAT */

/* Extended precision (80-bit) software floating-point emulation
   for m68k/Amiga long double (12 bytes, 96-bit with 2 bytes padding)

   Format:
   - 1 sign bit
   - 15 exponent bits (bias 16383)
   - 64-bit mantissa with explicit integer bit (bit 63)
   - Total: 80 bits, stored as 12 bytes with 2 bytes padding on Amiga
*/

#include <stdint.h>

/* Extended precision (80-bit) bitfield layout for m68k/Amiga.  */
union long_double_union
{
  long double ld;
  struct
  {
    unsigned int upper;   /* Sign+exp (upper 16) + padding (lower 16).  */
    unsigned int middle;  /* Mantissa bits 63-32.  */
    unsigned int lower;   /* Mantissa bits 31-0.  */
  } w;
};

#define EXP_SENTINEL_INF_NAN 16384

#ifdef __UNPACK
/* Unpack Motorola 80-bit extended long double into sign, unbiased
   exponent, and mantissa.  Stored exponent E is biased by 16383.
   Unbiased exponent e = E - 16383.  Mantissa is stored with the
   explicit integer bit.  Padding bit (bit 15 of upper word) is
   ignored.  */
void
__xf_unpack (long double x, unsigned int *sign, int *exp,
             unsigned long long *mant)
{
  union long_double_union u;
  u.ld = x;

  unsigned int upper = u.w.upper;
  unsigned int middle = u.w.middle;
  unsigned int lower = u.w.lower;

  *sign = upper >> 31;

  int E = (upper >> 16) & 0x7FFF;
  *mant = ((unsigned long long) middle << 32) | lower;

  if (E == 0)
    {
      if (*mant == 0)
        *exp = 0;
      else
        *exp = -16382;
    }
  else if (E == 0x7FFF)
    {
      *exp = EXP_SENTINEL_INF_NAN; /* Sentinel for Inf/NaN.  */
    }
  else
    {
      *exp = E - 16383;
    }
}
#endif

#ifdef __PACK
/* Pack sign, unbiased exponent, and mantissa into Motorola 80-bit
   extended format.  Unbiased exponent e is converted to stored
   exponent E = e + 16383.  Infinity/NaN sentinel from add: exp ==
   EXP_SENTINEL_INF_NAN -> E = 0x7FFF.  Zero: mant == 0 -> E = 0.  Normal numbers:
   integer bit must be present in mantissa.  */
long double
__xf_pack (unsigned int sign, int exp, unsigned long long mant)
{
  union long_double_union u;
  unsigned int upper;
  int E;

  /* Zero.  */
  if (mant == 0)
    {
      u.w.upper = (sign ? 0x80000000u : 0x00000000u);
      u.w.middle = 0;
      u.w.lower = 0;
      return u.ld;
    }

  /* Infinity or NaN (sentinel exp == EXP_SENTINEL_INF_NAN).  */
  if (exp == EXP_SENTINEL_INF_NAN)
    {
      upper = ((unsigned int) sign << 31) | (0x7FFFu << 16);
      u.w.upper = upper;
      u.w.middle = (unsigned int) (mant >> 32);
      u.w.lower = (unsigned int) (mant & 0xFFFFFFFFu);
      return u.ld;
    }

  /* Denormal/subnormal.  If bit 63 is not set, this is a subnormal
     number (E = 0).  */
  if (exp <= -16382 && (mant & 0x8000000000000000ULL) == 0)
    {
      u.w.upper = ((unsigned int) sign << 31); /* Biased E = 0.  */
      u.w.middle = (unsigned int) (mant >> 32);
      u.w.lower = (unsigned int) (mant & 0xFFFFFFFFu);
      return u.ld;
    }

  /* Apply bias.  */
  E = exp + 16383;

  /* Overflow to infinity.  */
  if (E >= 0x7FFF)
    {
      u.w.upper = ((unsigned int) sign << 31) | (0x7FFFu << 16);
      u.w.middle = 0x80000000u; /* IEEE 754 x87 Inf has bit 63 set.  */
      u.w.lower = 0;
      return u.ld;
    }

  /* Underflow to zero if exponent remains too small after rounding.  */
  if (E <= 0)
    {
      u.w.upper = ((unsigned int) sign << 31);
      u.w.middle = 0;
      u.w.lower = 0;
      return u.ld;
    }

  /* Normal number.  */
  upper = ((unsigned int) sign << 31) | ((unsigned int) (E & 0x7FFF) << 16);
  u.w.upper = upper;
  u.w.middle = (unsigned int) (mant >> 32);
  u.w.lower = (unsigned int) (mant & 0xFFFFFFFFu);

  return u.ld;
}
#endif

/* Negate a long double - MC68881 extended format (big-endian).  */
static inline long double
__xf_neg (long double x)
{
  unsigned char *p = (unsigned char *) &x;
  /* Toggle the sign bit (bit 7 of byte 0).  */
  p[0] ^= 0x80;
  return x;
}

/* We do not need these routines for coldfire, as it has no extended
   float format. */
#if !defined (__mcoldfire__)

/* Primitive extended precision floating point support.

   We assume all numbers are normalized, don't do any rounding, etc.  */

#if !defined(EXTFLOATCMP)

#ifdef __UNORDXF2
int
__unordxf2(long double a, long double b)
{
  union long_double_long ldl;

  ldl.ld = a;
  if (EXPX(ldl) == EXPXMASK
      && ((ldl.l.middle & MANTXMASK) != 0 || ldl.l.lower != 0))
    return 1;
  ldl.ld = b;
  if (EXPX(ldl) == EXPXMASK
      && ((ldl.l.middle & MANTXMASK) != 0 || ldl.l.lower != 0))
    return 1;
  return 0;
}
#endif

#ifdef __EXTENDDFXF2
/* convert double to long double */
long double
__extenddfxf2 (double d)
{
  register union double_long dl;
  register union long_double_long ldl;
  register long exp;

  dl.d = d;

  ldl.l.upper = SIGND (dl);
  if ((dl.l.upper & ~SIGNBIT) == 0 && !dl.l.lower)
    {
      ldl.l.middle = 0;
      ldl.l.lower = 0;
      return ldl.ld;
    }

  exp = EXPD (dl) - EXCESSD + EXCESSX;

  dl.l.upper &= MANTDMASK;

  /* Recover from a denorm. */
  if (exp == -EXCESSD + EXCESSX)
    {
      exp++;
      while ((dl.l.upper & HIDDEND) == 0)
	{
	  exp--;
	  dl.l.upper = (dl.l.upper << 1) | (dl.l.lower >> 31);
	  dl.l.lower = dl.l.lower << 1;
	}
    }

  /* Handle inf and NaN */
  else if (exp == EXPDMASK - EXCESSD + EXCESSX)
    {
      exp = EXPXMASK;
      /* Add hidden one bit for NaN */
      if (dl.l.upper != 0 || dl.l.lower != 0)
        dl.l.upper |= HIDDEND;
    }
  else
    {
      dl.l.upper |= HIDDEND;
    }

  ldl.l.upper |= exp << 16;
  /* 31-20: # mantissa bits in ldl.l.middle - # mantissa bits in dl.l.upper */
  ldl.l.middle = dl.l.upper << (31 - 20);
  /* 1+20: explicit-integer-bit + # mantissa bits in dl.l.upper */
  ldl.l.middle |= dl.l.lower >> (1 + 20);
  /* 32 - 21: # bits of dl.l.lower in ldl.l.middle */
  ldl.l.lower = dl.l.lower << (32 - 21);

  return ldl.ld;
}
#endif

#ifdef __TRUNCXFDF2
/* convert long double to double */
double
__truncxfdf2 (long double ld)
{
  register long exp;
  register union double_long dl;
  register union long_double_long ldl;

  ldl.ld = ld;

  dl.l.upper = SIGNX (ldl);
  if ((ldl.l.upper & ~SIGNBIT) == 0 && !ldl.l.middle && !ldl.l.lower)
    {
      dl.l.lower = 0;
      return dl.d;
    }

  exp = EXPX (ldl) - EXCESSX + EXCESSD;
  /* Check for underflow and denormals. */
  if (exp <= 0)
    {
      long shift = 1 - exp;
      if (shift > 52)
	{
	  ldl.l.middle = 0;
	  ldl.l.lower = 0;
	}
      else if (shift >= 32)
	{
	  ldl.l.lower = (ldl.l.middle) >> (shift - 32);
          ldl.l.middle = 0;
	}
      else
	{
	  ldl.l.lower = (ldl.l.middle << (32 - shift)) | (ldl.l.lower >> shift);
          ldl.l.middle = ldl.l.middle >> shift;
	}
      exp = 0;
    }
  else if (exp == EXPXMASK - EXCESSX + EXCESSD)
    {
      exp = EXPDMASK;
      ldl.l.middle |= ldl.l.lower;
    }
  else if (exp >= EXPDMASK)
    {
      exp = EXPDMASK;
      ldl.l.middle = 0;
      ldl.l.lower = 0;
    }
  dl.l.upper |= exp << (32 - (EXPDBITS + 1));
  /* +1-1: add one for sign bit, but take one off for explicit-integer-bit */
  dl.l.upper |= (ldl.l.middle & MANTXMASK) >> (EXPDBITS + 1 - 1);
  dl.l.lower = (ldl.l.middle & MANTXMASK) << (32 - (EXPDBITS + 1 - 1));
  dl.l.lower |= ldl.l.lower >> (EXPDBITS + 1 - 1);

  return dl.d;
}
#endif

#ifdef __EXTENDSFXF2
/* convert a float to a long double */
long double
__extendsfxf2 (float f)
{
  long double foo = __extenddfxf2 (__extendsfdf2 (f));
  return foo;
}
#endif

#ifdef __TRUNCXFSF2
/* convert a long double to a float */
float
__truncxfsf2 (long double ld)
{
  float foo = __truncdfsf2 (__truncxfdf2 (ld));
  return foo;
}
#endif

#ifdef __FLOATSIXF
long double
__floatsixf (long s)
{
    if (s == 0) return 0.0L;
    unsigned long u = (s < 0) ? -s : s;
    long double result = __floatunsixf (u);
    return (s < 0) ? __xf_neg (result) : result;
}
#endif

#ifdef __FLOATDIYF
long double
__floatdixf (long long s)
{
    if (s == 0) return 0.0L;
    unsigned long long u = (s < 0) ? -s : s;
    long double result = __floatundixf (u);
    return (s < 0) ? __xf_neg (result) : result;
}
#endif

#ifdef __FLOATUNDIXF
long double
__floatundixf (unsigned long long u)
{
    if (u == 0) return 0.0L;

    long double result = 0.0L;
    long double mult = 1.0L;
    unsigned long long temp = u;

    while (temp) {
        if (temp & 1) result += mult;
        temp >>= 1;
        mult += mult;  /* mult *= 2.0L */
    }
    return result;
}
#endif

/* ============================================================================
   Convert unsigned long to long double
   ============================================================================ */

#ifdef __FLOATUNSIXF
long double
__floatunsixf (unsigned long u)
{
    if (u == 0) return 0.0L;

    long double result = 0.0L;
    long double mult = 1.0L;
    unsigned long temp = u;

    while (temp) {
        if (temp & 1) result += mult;
        temp >>= 1;
        mult += mult;  /* mult *= 2.0L */
    }
    return result;
}
#endif

#ifdef __FIXXFSI
/* convert a long double to an int */
long
__fixxfsi (long double a)
{
  union long_double_long ldl;
  long exp;

  ldl.ld = a;

  exp = EXPX (ldl);
  if (exp == 0 && ldl.l.middle == 0 && ldl.l.lower == 0)
    return 0;

  exp = exp - EXCESSX - 32;

  if (exp >= 0)
    {
      /* Return largest integer.  */
      return SIGNX (ldl) ? 0x80000000L : 0x7fffffffL;
    }

  if (exp <= -32)
    return 0;

  ldl.l.middle >>= -exp;

  return SIGNX (ldl) ? -ldl.l.middle : ldl.l.middle;
}
#endif

#ifdef __ADDXF3
/* Add two long doubles - Motorola 80-bit extended format, unbiased
   exponent.  */
long double
__addxf3 (long double a, long double b)
{
  unsigned int sign_a, sign_b, sign_r;
  int exp_a, exp_b, exp_r;
  unsigned long long mant_a, mant_b, mant_r;

  __xf_unpack (a, &sign_a, &exp_a, &mant_a);
  __xf_unpack (b, &sign_b, &exp_b, &mant_b);

  /* NaN handling.  */
  if (exp_a == EXP_SENTINEL_INF_NAN
      && (mant_a & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return a;
  if (exp_b == EXP_SENTINEL_INF_NAN
      && (mant_b & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return b;

  /* Infinity handling.  */
  if (exp_a == EXP_SENTINEL_INF_NAN)
    {
      if (exp_b == EXP_SENTINEL_INF_NAN && sign_a != sign_b)
        return __xf_pack (0, EXP_SENTINEL_INF_NAN,
                          0x8000000000000000ULL); /* NaN.  */
      return a;
    }
  if (exp_b == EXP_SENTINEL_INF_NAN)
    return b;

  /* Zero handling.  */
  if (mant_a == 0)
    return b;
  if (mant_b == 0)
    return a;

  /* Normalize subnormal inputs.  */
  if (exp_a == -16382)
    {
      while ((mant_a & 0x8000000000000000ULL) == 0)
        {
          mant_a <<= 1;
          exp_a--;
        }
    }
  if (exp_b == -16382)
    {
      while ((mant_b & 0x8000000000000000ULL) == 0)
        {
          mant_b <<= 1;
          exp_b--;
        }
    }

  /* Align exponents.  */
  unsigned int guard = 0;
  unsigned int sticky = 0;

  if (exp_a > exp_b)
    {
      int shift = exp_a - exp_b;
      exp_r = exp_a;

      if (shift >= 66)
        {
          guard = 0;
          sticky = 1;
          mant_b = 0;
        }
      else if (shift == 65)
        {
          guard = 0;
          sticky = (mant_b != 0);
          mant_b = 0;
        }
      else if (shift == 64)
        {
          guard = (unsigned int) (mant_b >> 63);
          sticky = ((mant_b & 0x7FFFFFFFFFFFFFFFULL) != 0);
          mant_b = 0;
        }
      else
        {
          unsigned long long g_mask = 1ULL << (shift - 1);
          guard = (mant_b & g_mask) != 0;
          sticky = (mant_b & (g_mask - 1ULL)) != 0;
          mant_b >>= shift;
        }
    }
  else if (exp_b > exp_a)
    {
      int shift = exp_b - exp_a;
      exp_r = exp_b;

      if (shift >= 66)
        {
          guard = 0;
          sticky = 1;
          mant_a = 0;
        }
      else if (shift == 65)
        {
          guard = 0;
          sticky = (mant_a != 0);
          mant_a = 0;
        }
      else if (shift == 64)
        {
          guard = (unsigned int) (mant_a >> 63);
          sticky = ((mant_a & 0x7FFFFFFFFFFFFFFFULL) != 0);
          mant_a = 0;
        }
      else
        {
          unsigned long long g_mask = 1ULL << (shift - 1);
          guard = (mant_a & g_mask) != 0;
          sticky = (mant_a & (g_mask - 1ULL)) != 0;
          mant_a >>= shift;
        }
    }
  else
    {
      exp_r = exp_a;
    }

  /* Add or subtract mantissas.  */
  if (sign_a == sign_b)
    {
      /* Same sign: addition.  */
      sign_r = sign_a;
      mant_r = mant_a + mant_b;

      if (mant_r < mant_a)
        {
          /* Overflow beyond bit 63.  */
          sticky |= guard;
          guard = (unsigned int) (mant_r & 1ULL);
          mant_r = (mant_r >> 1) | 0x8000000000000000ULL;
          exp_r++;
        }
    }
  else
    {
      /* Different signs: subtraction.  */
      if (mant_a > mant_b || (mant_a == mant_b && (guard || sticky)))
        {
          sign_r = sign_a;
          /* 128-bit subtraction including guard and sticky.  */
          if (sticky || guard)
            {
              mant_r = mant_a - mant_b - 1ULL;
              if (!guard)
                sticky = 1;
              guard = !guard;
            }
          else
            {
              mant_r = mant_a - mant_b;
            }
        }
      else if (mant_b > mant_a)
        {
          sign_r = sign_b;
          mant_r = mant_b - mant_a;
          guard = 0;
          sticky = 0;
        }
      else
        {
          /* Exact zero.  */
          return __xf_pack (0, 0, 0);
        }

      /* Normalize after subtraction.  */
      while ((mant_r & 0x8000000000000000ULL) == 0 && mant_r != 0)
        {
          mant_r = (mant_r << 1) | guard;
          guard = 0; /* Guard shifts into mantissa.  */
          exp_r--;
        }
    }

  /* Underflow handling for subnormals.  */
  if (exp_r < -16382)
    {
      int shift = -16382 - exp_r;

      if (shift > 65)
        return __xf_pack (sign_r, 0, 0);

      if (shift == 65)
        {
          guard = 0;
          sticky = (mant_r != 0 || guard != 0 || sticky != 0);
          mant_r = 0;
        }
      else if (shift == 64)
        {
          sticky = ((mant_r & 0x7FFFFFFFFFFFFFFFULL) != 0
                    || guard != 0 || sticky != 0);
          guard = (unsigned int) (mant_r >> 63);
          mant_r = 0;
        }
      else
        {
          unsigned long long g_mask = 1ULL << (shift - 1);
          unsigned int new_guard = (mant_r & g_mask) != 0;
          unsigned int new_sticky = ((mant_r & (g_mask - 1ULL)) != 0
                                     || guard != 0 || sticky != 0);
          guard = new_guard;
          sticky = new_sticky;
          mant_r >>= shift;
        }
      exp_r = -16382;
    }

  /* IEEE 754 round-to-nearest-even.  */
  if (guard && (sticky || (mant_r & 1ULL)))
    {
      mant_r++;
      if (mant_r == 0)
        {
          mant_r = 0x8000000000000000ULL;
          exp_r++;
        }
    }

  /* Overflow.  */
  if (exp_r > 16383)
    return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                      0x8000000000000000ULL);

  return __xf_pack (sign_r, exp_r, mant_r);
}
#endif

#ifdef __SUBXF3
/* Subtract two long doubles: a - b = a + (-b).  */
long double
__subxf3 (long double a, long double b)
{
  /* Negate b and add.  */
  return __addxf3 (a, __xf_neg (b));
}
#endif

#ifdef __MULXF3
/* Multiply two long doubles - Motorola 80-bit extended format.  */
long double
__mulxf3 (long double a, long double b)
{
  unsigned int sign_a, sign_b, sign_r;
  int exp_a, exp_b, exp_r;
  unsigned long long mant_a, mant_b;

  unsigned long long hi, lo;
  unsigned long long mant_r;
  unsigned int guard = 0, sticky = 0;

  __xf_unpack (a, &sign_a, &exp_a, &mant_a);
  __xf_unpack (b, &sign_b, &exp_b, &mant_b);

  sign_r = sign_a ^ sign_b;

  /* NaN handling.  */
  if (exp_a == EXP_SENTINEL_INF_NAN
      && (mant_a & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return a;
  if (exp_b == EXP_SENTINEL_INF_NAN
      && (mant_b & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return b;

  /* Infinity handling.  */
  if (exp_a == EXP_SENTINEL_INF_NAN || exp_b == EXP_SENTINEL_INF_NAN)
    return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                      0x8000000000000000ULL);

  /* Zero handling.  */
  if (mant_a == 0 || mant_b == 0)
    return __xf_pack (sign_r, 0, 0);

  /* Normalize subnormal inputs (ensure bit 63 is set).  */
  if (exp_a == -16382)
    {
      while ((mant_a & 0x8000000000000000ULL) == 0)
        {
          mant_a <<= 1;
          exp_a--;
        }
    }

  if (exp_b == -16382)
    {
      while ((mant_b & 0x8000000000000000ULL) == 0)
        {
          mant_b <<= 1;
          exp_b--;
        }
    }

  /* Add exponents (unbiased, range -16382 to +16383).  */
  exp_r = exp_a + exp_b;

  /* Exact 64x64 -> 128-bit multiplication (hi:lo).  */
  unsigned int a_lo = (unsigned int) mant_a;
  unsigned int a_hi = (unsigned int) (mant_a >> 32);
  unsigned int b_lo = (unsigned int) mant_b;
  unsigned int b_hi = (unsigned int) (mant_b >> 32);

  unsigned long long p0 = (unsigned long long) a_lo * b_lo;
  unsigned long long p1 = (unsigned long long) a_lo * b_hi;
  unsigned long long p2 = (unsigned long long) a_hi * b_lo;
  unsigned long long p3 = (unsigned long long) a_hi * b_hi;

  unsigned long long mid = (p0 >> 32) + (p1 & 0xFFFFFFFFULL)
                           + (p2 & 0xFFFFFFFFULL);

  hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
  lo = (mid << 32) | (p0 & 0xFFFFFFFFULL);

  /* Normalize 128-bit product.  */
  if (hi & 0x8000000000000000ULL)
    {
      /* Product in [2.0, 4.0): bit 63 of 'hi' is already 1.
         No right shift needed.  Exponent increases by 1, 'hi' is the
         upper mantissa.  */
      mant_r = hi;
      guard = (unsigned int) (lo >> 63);
      sticky = (lo & 0x7FFFFFFFFFFFFFFFULL) != 0;
      exp_r++;
    }
  else
    {
      /* Product in [1.0, 2.0): bit 62 is 1.  Shift left by 1 to set
         bit 63.  */
      mant_r = (hi << 1) | (lo >> 63);
      unsigned long long lo_shifted = lo << 1;
      guard = (unsigned int) (lo_shifted >> 63);
      sticky = (lo_shifted & 0x7FFFFFFFFFFFFFFFULL) != 0;
    }

  /* Underflow and subnormal shifting including guard/sticky
     accumulation.  */
  if (exp_r < -16382)
    {
      int shift = -16382 - exp_r;

      if (shift > 65)
        return __xf_pack (sign_r, 0, 0);

      if (shift == 65)
        {
          guard = 0;
          sticky = (mant_r != 0 || guard != 0 || sticky != 0);
          mant_r = 0;
        }
      else if (shift == 64)
        {
          sticky = ((mant_r & 0x7FFFFFFFFFFFFFFFULL) != 0
                    || guard != 0 || sticky != 0);
          guard = (unsigned int) (mant_r >> 63);
          mant_r = 0;
        }
      else
        {
          unsigned long long guard_mask = 1ULL << (shift - 1);
          unsigned int new_guard = (mant_r & guard_mask) != 0;
          unsigned int new_sticky = ((mant_r & (guard_mask - 1ULL)) != 0
                                     || guard != 0 || sticky != 0);

          guard = new_guard;
          sticky = new_sticky;
          mant_r >>= shift;
        }

      exp_r = -16382;
    }

  /* IEEE 754 round-to-nearest-even.  */
  if (guard && (sticky || (mant_r & 1ULL)))
    {
      mant_r++;

      if (mant_r == 0)
        {
          /* Mantissa overflow on rounding up.  */
          mant_r = 0x8000000000000000ULL;
          exp_r++;
        }
    }

  /* Overflow.  */
  if (exp_r > 16383)
    return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                      0x8000000000000000ULL);

  return __xf_pack (sign_r, exp_r, mant_r);
}
#endif

#ifdef __DIVXF3
/* Divide two long doubles - Motorola 80-bit extended format.  */
long double
__divxf3 (long double a, long double b)
{
  unsigned int sign_a, sign_b, sign_r;
  int exp_a, exp_b, exp_r, s;
  unsigned long long mant_a, mant_b, mant_r;
  unsigned long long rem, half, lo;
  unsigned int rem_hi;
  int i;

  __xf_unpack (a, &sign_a, &exp_a, &mant_a);
  __xf_unpack (b, &sign_b, &exp_b, &mant_b);

  sign_r = sign_a ^ sign_b;

  /* NaN handling: exponent sentinel with at least one fraction bit
     set.  The canonical infinity has mantissa 0x8000000000000000 -
     the integer bit alone does not count as a NaN fraction and must
     not be recognized as NaN.  */
  if (exp_a == EXP_SENTINEL_INF_NAN
      && (mant_a & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return a;

  if (exp_b == EXP_SENTINEL_INF_NAN
      && (mant_b & 0x7FFFFFFFFFFFFFFFULL) != 0)
    return b;

  /* Infinity handling.  */
  if (exp_a == EXP_SENTINEL_INF_NAN)
    {
      /* Inf / Inf = NaN.  */
      if (exp_b == EXP_SENTINEL_INF_NAN)
        return __xf_pack (0, EXP_SENTINEL_INF_NAN,
                          0x8000000000000000ULL);

      /* Inf / finite = Inf.  */
      return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                        0x8000000000000000ULL);
    }

  if (exp_b == EXP_SENTINEL_INF_NAN)
    {
      /* finite / Inf = 0 (also true for 0 / Inf).  */
      return __xf_pack (sign_r, 0, 0);
    }

  /* Zero handling.  */
  if (mant_a == 0)
    {
      /* 0 / 0 = NaN.  */
      if (mant_b == 0)
        return __xf_pack (0, EXP_SENTINEL_INF_NAN,
                          0x8000000000000000ULL);

      /* 0 / finite = +/-0.  */
      return __xf_pack (sign_r, 0, 0);
    }

  if (mant_b == 0)
    {
      /* finite / 0 = Inf.  */
      return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                        0x8000000000000000ULL);
    }

  /* Normalize subnormal (and unnormal) operands exactly: shift
     mantissa left until integer bit is set, decrement exponent
     accordingly.  This is value-preserving.  */
  while ((mant_a & 0x8000000000000000ULL) == 0)
    {
      mant_a <<= 1;
      exp_a--;
    }
  while ((mant_b & 0x8000000000000000ULL) == 0)
    {
      mant_b <<= 1;
      exp_b--;
    }

  exp_r = exp_a - exp_b;

  /* Calculate:
     mant_r = floor((mant_a / mant_b) * 2^63)
     rem    = mant_a * 2^63 - mant_r * mant_b

     rem is the exact remainder, 0 <= rem < mant_b.

     Start with the integer quotient bit, then generate the remaining
     63 bits by long division.  rem needs 65 bits while shifting,
     hence the rem_hi test.  If the remainder becomes zero, all
     further quotient bits are zero.  */
  mant_r = 0;
  rem_hi = 0;

  if (mant_a >= mant_b)
    {
      mant_r = 1;
      rem = mant_a - mant_b;
    }
  else
    {
      rem = mant_a;
    }

  for (i = 0; i < 63; i++)
    {
      if (rem == 0)
        {
          /* Quotient exact.  */
          mant_r <<= 63 - i;
          break;
        }

      /* Shift remainder left by one (65 bits: rem_hi:rem).  */
      rem_hi = (unsigned int) (rem >> 63);
      rem <<= 1;

      mant_r <<= 1;

      /* If bit 64 is set, the 65-bit remainder is certainly >=
         mant_b; the subtraction then cannot underflow 64 bits.  */
      if (rem_hi || rem >= mant_b)
        {
          rem -= mant_b;
          mant_r |= 1;
        }
    }

  /* Normalize the quotient to [2^63, 2^64):

     If mant_a < mant_b, the quotient is in [0.5, 1) and so far only
     63 significant bits exist.  Perform one more long division step,
     so that afterwards - uniformly in both cases:

     mant_r = floor(Q * 2^64),   rem = exact remainder
     value = (mant_r + rem/mant_b) * 2^(exp_r - 63)

     with mant_r in [2^63, 2^64) and exp_r already adjusted.  */
  if (mant_r < 0x8000000000000000ULL)
    {
      rem_hi = (unsigned int) (rem >> 63);
      rem <<= 1;
      mant_r <<= 1;

      if (rem_hi || rem >= mant_b)
        {
          rem -= mant_b;
          mant_r |= 1;
        }

      exp_r--;
    }

  /* Round-to-nearest-even, exact and without double rounding.  */
  if (exp_r >= -16382)
    {
      /* Normal range: round mant_r to 64 bits.  Round up if
         rem/mant_b > 1/2, or exactly 1/2 and mant_r is odd.

         (The exact tie case 2*rem == mant_b cannot occur with exact
         remainder; the test remains as a safety check.)  */
      if (rem > (mant_b >> 1)
          || (rem == (mant_b >> 1) && (mant_b & 1) == 0
              && (mant_r & 1)))
        {
          mant_r++;

          /* 1.111... -> 10.000... (cannot occur here, defensive).  */
          if (mant_r == 0)
            {
              mant_r = 0x8000000000000000ULL;
              exp_r++;
            }
        }

      /* Overflow.  */
      if (exp_r > 16383)
        return __xf_pack (sign_r, EXP_SENTINEL_INF_NAN,
                          0x8000000000000000ULL);
    }
  else
    {
      /* Subnormal result.  The exact value is:

         (mant_r + rem/mant_b) * 2^(exp_r - 63)

         and is rounded in a SINGLE step directly to the subnormal
         grid - not first to 64 bits and then again (which would be a
         double rounding with up to 1 ulp error).

         s = right shift from the 64-bit normal case to the subnormal
         grid (1 .. 16447).  */
      s = -16382 - exp_r;

      if (s > 64)
        {
          /* Magnitude < half of smallest subnormal -> signed zero.  */
          return __xf_pack (sign_r, 0, 0);
        }

      if (s == 64)
        {
          /* Value in [0.5, 1) ulp of the smallest subnormal.  */
          if (mant_r > 0x8000000000000000ULL
              || (mant_r == 0x8000000000000000ULL && rem != 0))
            return __xf_pack (sign_r, -16382, 1);

          return __xf_pack (sign_r, 0, 0);
        }

      /* 1 <= s <= 63: split mant_r on the subnormal grid.  Fraction
         of the exact value: (lo + rem/mant_b) / 2^s.  */
      half = 1ULL << (s - 1);
      lo = mant_r & ((half << 1) - 1);
      mant_r >>= s;

      if (lo > half
          || (lo == half && (rem != 0 || (mant_r & 1))))
        mant_r++;       /* Can become 2^63 = smallest normal.  */

      exp_r = -16382;
    }

  return __xf_pack (sign_r, exp_r, mant_r);
}
#endif

#ifdef __NEGXF2
long double
__negxf2 (long double x1)
{
  return __xf_neg(x1);
}
#endif

#else /* EXTFLOATCMP */

/* Compare two long doubles.  Returns:
   0 if a == b
   1 if a > b
  -1 if a < b
   2 if either operand is NaN.  */
static int
__xfcmp (long double a, long double b)
{
  union long_double_long x, y;
  long xexp, yexp;
  int xsign, ysign, mag;

  x.ld = a;
  y.ld = b;
  xexp = EXPX (x);
  yexp = EXPX (y);

  /* NaN handling.  */
  if ((xexp == EXPXMASK && ((x.l.middle & MANTXMASK) != 0 || x.l.lower != 0))
      || (yexp == EXPXMASK && ((y.l.middle & MANTXMASK) != 0 || y.l.lower != 0)))
    return 2;

  /* +0 and -0 are equal.  */
  if (xexp == 0 && x.l.middle == 0 && x.l.lower == 0
      && yexp == 0 && y.l.middle == 0 && y.l.lower == 0)
    return 0;

  xsign = SIGNX (x) != 0;
  ysign = SIGNX (y) != 0;

  /* Different signs.  */
  if (xsign != ysign)
    return xsign ? -1 : 1;

  /* Compare magnitude.  */
  if (xexp != yexp)
    mag = xexp < yexp ? -1 : 1;
  else if (x.l.middle != y.l.middle)
    mag = x.l.middle < y.l.middle ? -1 : 1;
  else if (x.l.lower != y.l.lower)
    mag = x.l.lower < y.l.lower ? -1 : 1;
  else
    mag = 0;

  /* Apply sign.  */
  return xsign ? -mag : mag;
}

#ifdef __CMPXF2
long
__cmpxf2 (long double x1, long double x2)
{
  int r = __xfcmp (x1, x2);
  return r == 2 ? 1 : r;
}
#endif

#ifdef __EQXF2
long
__eqxf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __NEXF2
long
__nexf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __LTXF2
long
__ltxf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __LEXF2
long
__lexf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __GTXF2
long
__gtxf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __GEXF2
long
__gexf2 (long double x1, long double x2)
{
  return __cmpxf2(x1, x2);
}
#endif

#ifdef __TRUNCXFDF2
/* convert long double to double */
double
__truncxfdf2 (long double ld)
{
  register long exp;
  register union double_long dl;
  register union long_double_long ldl;

  ldl.ld = ld;

  dl.l.upper = SIGNX (ldl);
  if ((ldl.l.upper & ~SIGNBIT) == 0 && !ldl.l.middle && !ldl.l.lower)
    {
      dl.l.lower = 0;
      return dl.d;
    }

  exp = EXPX (ldl) - EXCESSX + EXCESSD;
  /* ??? quick and dirty: keep `exp' sane */
  if (exp >= EXPDMASK)
    exp = EXPDMASK - 1;
  dl.l.upper |= exp << (32 - (EXPDBITS + 1));
  /* +1-1: add one for sign bit, but take one off for explicit-integer-bit */
  dl.l.upper |= (ldl.l.middle & MANTXMASK) >> (EXPDBITS + 1 - 1);
  dl.l.lower = (ldl.l.middle & MANTXMASK) << (32 - (EXPDBITS + 1 - 1));
  dl.l.lower |= ldl.l.lower >> (EXPDBITS + 1 - 1);

  return dl.d;
}
#endif

#endif /* EXTFLOATCMP */

#endif /* !__mcoldfire__ */
#endif /* EXTFLOAT */
