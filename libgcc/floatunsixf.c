/* Public domain.  */
typedef int SItype __attribute__ ((mode (SI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef float XFtype __attribute__ ((mode (XF)));

XFtype
__floatunsixf (USItype u)
{
#ifdef TARGET_M68K
	extern XFtype __xf_pack(unsigned int, int, unsigned long long);

	if (u == 0)
		return __xf_pack(0, 0, 0);

	/* find highest bit */
	int exp = 31 - __builtin_clz(u);

	/* set mantisse */
	unsigned long long mant = (unsigned long long)u << (63 - exp);

	return __xf_pack(0, exp, mant);
#else
  SItype s = (SItype) u;
  XFtype r = (XFtype) s;
  if (s < 0)
    r += (XFtype)2.0 * (XFtype) ((USItype) 1
				 << (sizeof (USItype) * __CHAR_BIT__ - 1));
  return r;
#endif
}
