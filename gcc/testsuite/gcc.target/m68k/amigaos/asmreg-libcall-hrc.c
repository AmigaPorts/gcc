/* Amiga inline library-call macros, gcc-16 hard-register-constraint form:
   the arguments are bound directly to hard registers via "{reg}" constraints
   (https://gcc.gnu.org/onlinedocs/gcc/Hard-Register-Constraints.html) instead
   of local register variables.  This requires -mlra, and -- unlike the
   local-register-variable form -- it correctly passes a const-qualified
   argument in an address register (the "a0 = mask vanishes" case of PR126552 /
   bebbo/amiga-gcc#15).  gcc 16+ only.  */

/* { dg-do compile } */
/* { dg-skip-if "amiga inline library-call ABI, gcc 16 + LRA" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O1 -mlra" } */

typedef unsigned char *PLANEPTR;
struct RastPort;
extern void *GfxBase;

void
call_hrc (struct RastPort *rp, const PLANEPTR mask)
{
  int _d0, _d1;
  __asm volatile ("jsr %%a6@(-0x138:W)"
		  : "={d0}" (_d0), "={d1}" (_d1)
		  : "{a6}" (GfxBase), "{a1}" (rp), "{a0}" (mask)
		  : "fp0", "fp1", "cc", "memory");
}

/* The const mask argument must reach the address register a0.  */
/* { dg-final { scan-assembler "\\(sp\\),a0" } } */
