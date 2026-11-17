/* { dg-do run } */
/* { dg-require-effective-target m68881_runtime } */
/* { dg-options "-m68020 -m68881" } */
/* With an FPU, a saveallregs function returns float, double and long
   double in fp0, and gives back every other register, fp1-fp7 and d0-d7
   included.  An integer result leaves all of fp0-fp7 as they were.  Each
   function overwrites d0-d7/a0-a3 and fp0-fp7, and call_kept() checks
   every register it gets back.  */

extern void abort (void);

/* before[] and after[] are d0-d7, a0-a6; fbefore[] and fafter[] fp0-fp7 */
__attribute__ ((used, externally_visible)) unsigned long before[15] = {
  0x10000001, 0x11000011, 0x12000021, 0x13000031,
  0x14000041, 0x15000051, 0x16000061, 0x17000071,
  0x20000102, 0x21000112, 0x22000122, 0x23000132,
  0x24000142, 0x25000152, 0x26000162
};
__attribute__ ((used, externally_visible)) unsigned long after[15];
__attribute__ ((used, externally_visible)) long double fbefore[8] = {
  1.25L, -2.5L, 3.75L, -5.0L, 6.25L, -7.5L, 8.75L, -10.0L
};
__attribute__ ((used, externally_visible)) long double fafter[8];
__attribute__ ((used, externally_visible)) unsigned long junk[13] = {
  ~0UL, ~1UL, ~2UL, ~3UL, ~4UL, ~5UL, ~6UL, ~7UL, ~8UL, ~9UL, ~10UL, ~11UL, ~12UL
};
__attribute__ ((used, externally_visible)) long double fjunk[8] = {
  -1.0L, -2.0L, -3.0L, -4.0L, -5.0L, -6.0L, -7.0L, -8.0L
};
__attribute__ ((used, externally_visible)) void *fnptr;

/* Calls fnptr with d0-d7/a0-a6 and fp0-fp7 loaded from before[] and
   fbefore[], and stores them in after[] and fafter[] when it returns.  */
extern void call_kept (void);
__asm__ ("	.text\n"
	 "	.even\n"
	 "	.globl	_call_kept\n"
	 "_call_kept:\n"
	 "	movem.l	d2-d7/a2-a6,-(sp)\n"
	 "	fmovem.x fp2-fp7,-(sp)\n"
	 "	pea	1f\n"
	 "	move.l	_fnptr,-(sp)\n"
	 "	fmovem.x _fbefore,fp0-fp7\n"
	 "	movem.l	_before,d0-d7/a0-a6\n"
	 "	rts\n"
	 "1:	movem.l	d0-d7/a0-a6,_after\n"
	 "	fmovem.x fp0-fp7,_fafter\n"
	 "	fmovem.x (sp)+,fp2-fp7\n"
	 "	movem.l	(sp)+,d2-d7/a2-a6\n"
	 "	rts\n");

/* Overwrites d0-d7, a0-a3 and fp0-fp7: not a4, which -fbaserel
   reserves.  */
#define CLOBBER							\
  __asm__ volatile ("movem.l _junk,%%d0-%%d7/%%a0-%%a3\n\t"	\
		    "fmovem.x _fjunk,%%fp0-%%fp7"			\
		    ::: "d0", "d1", "d2", "d3", "d4", "d5", "d6",	\
		      "d7", "a0", "a1", "a2", "a3",			\
		      "fp0", "fp1", "fp2", "fp3", "fp4", "fp5", "fp6",	\
		      "fp7", "memory")

#define D0 1
#define D1 2
#define FP0 0x10000

static int failures;

/* Calls F through call_kept().  Every register not in RET must come back
   as it went in.  */
static void
check (const char *name, void *f, unsigned ret)
{
  int i;

  /* a4 as it is, which -fbaserel needs to reach the data */
  __asm__ ("move.l %%a4,%0" : "=m" (before[12]));
  fnptr = f;
  call_kept ();
  for (i = 0; i < 15; i++)
    if (!(ret & (1u << i)) && after[i] != before[i])
      {
	__builtin_printf ("%s: %c%d changed\n", name, i < 8 ? 'd' : 'a', i & 7);
	failures++;
      }
  for (i = 0; i < 8; i++)
    if (!(ret & (FP0 << i)) && fafter[i] != fbefore[i])
      {
	__builtin_printf ("%s: fp%d changed\n", name, i);
	failures++;
      }
}

#define EXPECT(name, cond)					\
  do {								\
    if (!(cond))						\
      {								\
	__builtin_printf ("%s: wrong value\n", name);		\
	failures++;						\
      }								\
  } while (0)

volatile double g = 21.0;

/* noipa: the direct calls below must be calls */
#define SAR __attribute__ ((noipa)) __saveallregs

SAR void fv (void) { CLOBBER; g++; }
SAR int fi (void) { CLOBBER; return g * 2; }
SAR long long fll (void) { CLOBBER; return (long long) g << 32; }
SAR float ff (void) { CLOBBER; return g + 1; }
SAR double fd (void) { CLOBBER; return g * 2; }
SAR long double fl (void) { CLOBBER; return g - 1; }

int
main (void)
{
  check ("void", fv, 0);
  check ("int", fi, D0);
  EXPECT ("int", after[0] == 44);
  check ("long long", fll, D0 | D1);
  EXPECT ("long long", after[0] == 22 && after[1] == 0);
  check ("float", ff, FP0);
  EXPECT ("float", fafter[0] == 23.0L);
  check ("double", fd, FP0);
  EXPECT ("double", fafter[0] == 44.0L);
  check ("long double", fl, FP0);
  EXPECT ("long double", fafter[0] == 21.0L);

  g = 21.0;
  EXPECT ("void call", (fv (), g == 22.0));
  EXPECT ("int call", fi () == 44);
  EXPECT ("long long call", fll () == 22LL << 32);
  EXPECT ("float call", ff () == 23.0f);
  EXPECT ("double call", fd () == 44.0);
  EXPECT ("long double call", fl () == 21.0L);

  if (failures)
    abort ();
  return 0;
}
