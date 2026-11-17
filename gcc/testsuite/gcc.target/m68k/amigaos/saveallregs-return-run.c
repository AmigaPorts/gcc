/* { dg-do run } */
/* A saveallregs function gives back every register but sp and those that
   hold its return value: d0 for up to 32 bits, d0/d1 for 64.  A value
   returned in memory takes no register.  Each function overwrites
   d0-d7/a0-a3, and call_kept() checks every register it gets back.  */

extern void abort (void);

/* before[] and after[] are d0-d7, a0-a6 */
__attribute__ ((used, externally_visible)) unsigned long before[15] = {
  0x10000001, 0x11000011, 0x12000021, 0x13000031,
  0x14000041, 0x15000051, 0x16000061, 0x17000071,
  0x20000102, 0x21000112, 0x22000122, 0x23000132,
  0x24000142, 0x25000152, 0x26000162
};
__attribute__ ((used, externally_visible)) unsigned long after[15];
__attribute__ ((used, externally_visible)) unsigned long junk[13] = {
  ~0UL, ~1UL, ~2UL, ~3UL, ~4UL, ~5UL, ~6UL, ~7UL, ~8UL, ~9UL, ~10UL, ~11UL, ~12UL
};
__attribute__ ((used, externally_visible)) void *fnptr;

/* Calls fnptr with d0-d7/a0-a6 loaded from before[], and stores them in
   after[] when it returns.  It jumps with rts, so that no register holds
   the address.  */
extern void call_kept (void);
__asm__ ("	.text\n"
	 "	.even\n"
	 "	.globl	_call_kept\n"
	 "_call_kept:\n"
	 "	movem.l	d2-d7/a2-a6,-(sp)\n"
	 "	pea	1f\n"
	 "	move.l	_fnptr,-(sp)\n"
	 "	movem.l	_before,d0-d7/a0-a6\n"
	 "	rts\n"
	 "1:	movem.l	d0-d7/a0-a6,_after\n"
	 "	movem.l	(sp)+,d2-d7/a2-a6\n"
	 "	rts\n");

/* Overwrites d0-d7 and a0-a3: not a4, which -fbaserel reserves.  */
#define CLOBBER							\
  __asm__ volatile ("movem.l _junk,%%d0-%%d7/%%a0-%%a3"		\
		    ::: "d0", "d1", "d2", "d3", "d4", "d5", "d6",	\
		      "d7", "a0", "a1", "a2", "a3", "memory")

#define D0 1
#define D1 2
#define A0 0x100

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
}

#define EXPECT(name, cond)					\
  do {								\
    if (!(cond))						\
      {								\
	__builtin_printf ("%s: wrong value\n", name);		\
	failures++;						\
      }								\
  } while (0)

volatile int g = 41;

/* noipa: the direct calls below must be calls */
#define SAR __attribute__ ((noipa)) __saveallregs

struct s1 { char a; };
struct s2 { short a; };
struct s4 { short a, b; };
struct s8 { long a, b; };
struct s12 { long a, b, c; };

SAR void fv (void) { CLOBBER; g++; }
SAR signed char fc (void) { CLOBBER; return -g; }
SAR unsigned char fuc (void) { CLOBBER; return g + 200; }
SAR short fh (void) { CLOBBER; return -g * 100; }
SAR int fi (void) { CLOBBER; return g + 1; }
SAR unsigned long fl (void) { CLOBBER; return 0x80000000UL + g; }
SAR volatile int *fp (void) { CLOBBER; return &g; }
SAR long long fll (void) { CLOBBER; return ((long long) g << 32) | 2; }
SAR float ff (void) { CLOBBER; return g + 0.5f; }
SAR double fd (void) { CLOBBER; return g * 2.0; }
SAR long double fld (void) { CLOBBER; return g / 4.0L; }
SAR struct s1 fs1 (void) { struct s1 r = { g }; CLOBBER; return r; }
SAR struct s2 fs2 (void) { struct s2 r = { g }; CLOBBER; return r; }
SAR struct s4 fs4 (void) { struct s4 r = { g, -g }; CLOBBER; return r; }
SAR struct s8 fs8 (void) { struct s8 r = { g, g + 1 }; CLOBBER; return r; }
SAR struct s12 fs12 (void) { struct s12 r = { g, g + 1, g + 2 }; CLOBBER; return r; }

int
main (void)
{
  /* The registers, through call_kept().  Those that hold a value returned
     in memory may change: the caller passes the address in a1, and gets
     it back in d0, and also in a0 with a pointer.  */
  check ("void", fv, 0);
  check ("signed char", fc, D0);
  EXPECT ("signed char", (signed char) after[0] == -42);
  check ("unsigned char", fuc, D0);
  EXPECT ("unsigned char", (unsigned char) after[0] == 242);
  check ("short", fh, D0);
  EXPECT ("short", (short) after[0] == -4200);
  check ("int", fi, D0);
  EXPECT ("int", after[0] == 43);
  check ("unsigned long", fl, D0);
  EXPECT ("unsigned long", after[0] == 0x8000002aUL);
  check ("pointer", fp, D0 | A0);
  EXPECT ("pointer", after[0] == (unsigned long) &g);
  check ("long long", fll, D0 | D1);
  EXPECT ("long long", after[0] == 42 && after[1] == 2);
#ifndef __HAVE_68881__
  /* soft float: in d0 and d0/d1; with an FPU in fp0, see
     saveallregs-return-fp-run.c */
  {
    union { float f; unsigned long l; } uf;
    union { double d; unsigned long l[2]; } ud;

    check ("float", ff, D0);
    uf.l = after[0];
    EXPECT ("float", uf.f == 42.5f);
    check ("double", fd, D0 | D1);
    ud.l[0] = after[0];
    ud.l[1] = after[1];
    EXPECT ("double", ud.d == 84.0);
  }
#endif
  check ("struct s1", fs1, D0);
  check ("struct s2", fs2, D0);
  check ("struct s4", fs4, D0);
  check ("struct s8", fs8, D0 | D1);
  EXPECT ("struct s8", after[0] == 42 && after[1] == 43);
  {
    /* returned in memory, at the address in a1 */
    static struct s12 r12;
    static long double rld;

    before[9] = (unsigned long) &r12;
    check ("struct s12", fs12, D0 | A0);
    EXPECT ("struct s12", r12.a == 42 && r12.b == 43 && r12.c == 44);
#ifndef __HAVE_68881__
    before[9] = (unsigned long) &rld;
    check ("long double", fld, D0 | A0);
    EXPECT ("long double", rld == 10.5L);
#endif
    before[9] = 0x21000112;
  }

  /* The values, through ordinary calls.  */
  g = 41;
  EXPECT ("void call", (fv (), g == 42));
  EXPECT ("signed char call", fc () == -42);
  EXPECT ("unsigned char call", fuc () == 242);
  EXPECT ("short call", fh () == -4200);
  EXPECT ("int call", fi () == 43);
  EXPECT ("unsigned long call", fl () == 0x8000002aUL);
  EXPECT ("pointer call", fp () == &g);
  EXPECT ("long long call", fll () == ((42LL << 32) | 2));
  EXPECT ("float call", ff () == 42.5f);
  EXPECT ("double call", fd () == 84.0);
  EXPECT ("long double call", fld () == 10.5L);
  EXPECT ("struct s1 call", fs1 ().a == 42);
  EXPECT ("struct s2 call", fs2 ().a == 42);
  EXPECT ("struct s4 call", fs4 ().a == 42 && fs4 ().b == -42);
  EXPECT ("struct s8 call", fs8 ().a == 42 && fs8 ().b == 43);
  {
    struct s12 r = fs12 ();
    EXPECT ("struct s12 call", r.a == 42 && r.b == 43 && r.c == 44);
  }

  if (failures)
    abort ();
  return 0;
}
