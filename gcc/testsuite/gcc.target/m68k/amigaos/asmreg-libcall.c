/* Amiga inline library-call macros (proto/*.h, e.g. BltPattern): a library
   function is called by binding its arguments to hard registers via local
   register variables and an asm that reads/writes them through those
   registers.  The address-register arguments must actually reach a0/a1 and
   must not be clobbered ("killed address registers", bebbo/amiga-gcc#15,
   gcc PR126552).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga inline library-call ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2" } */

typedef unsigned char *PLANEPTR;
typedef short WORD;
typedef unsigned short UWORD;
struct RastPort;
extern void *GfxBase;

static inline void
BltPattern (struct RastPort *rp, const PLANEPTR mask,
	    WORD xMin, WORD yMin, WORD xMax, WORD yMax, UWORD bpr)
{
  register void *const v_base __asm ("a6") = GfxBase;
  register struct RastPort *v0 __asm ("a1") = rp;
  register PLANEPTR v1 __asm ("a0") = (PLANEPTR) mask;
  register WORD v2 __asm ("d0") = xMin;
  register WORD v3 __asm ("d1") = yMin;
  register WORD v4 __asm ("d2") = xMax;
  register WORD v5 __asm ("d3") = yMax;
  register UWORD v6 __asm ("d4") = bpr;
  __asm volatile ("jsr %%a6@(-312:W)\n"
		  : "+a" (v0), "+a" (v1), "+d" (v2), "+d" (v3)
		  : "a" (v_base), "d" (v4), "d" (v5), "d" (v6)
		  : "fp0", "fp1", "cc", "memory");
}

void
caller (struct RastPort *rp, PLANEPTR mask)
{
  BltPattern (rp, mask, 0, 0, 10, 10, 2);
}

/* rp and mask must reach the address registers a0/a1 (not be clobbered).  */
/* { dg-final { scan-assembler "\\(sp\\),a1" } } */
/* { dg-final { scan-assembler "\\(sp\\),a0" } } */
/* { dg-final { scan-assembler "jsr .*a6" } } */
