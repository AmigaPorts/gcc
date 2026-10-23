/* The calling convention keywords of the SAS/C interoperability tests,
   for SAS/C and for gcc.  gen.py's halves use them; see README.  */

#ifdef __SASC
/* The entry point the gcc driver calls, on the stack.  */
#define STDARGS __stdargs
/* Explicit argument registers.  */
#define ASMFN __asm
#define R(reg, type, name) register __##reg type name
#else
#define STDARGS __attribute__ ((stkparm))
#define ASMFN
#define R(reg, type, name) register type name __asm (#reg)
#endif

struct s8 { long x, y; };
struct s4 { long x; };

/* Mix v into h: order-sensitive, and no multiply, which SAS/C would
   call sc.lib for on a 68000.  */
#define MIX(h, v) ((((h) << 5) | ((h) >> 27)) ^ (unsigned long) (v))
