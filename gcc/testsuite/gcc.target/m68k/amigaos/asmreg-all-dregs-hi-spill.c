/* As asmreg-all-dregs-hi.c, but the short is compared with 7, which an
   address register cannot be: cmp.w #7,An compares all 32 bits.  The
   compare's reload needs a data register and none is free.  LRA used to
   split a hard reg for it over and over, each split succeeding without
   getting the reload assigned, and never returned.  It must now stop
   after LRA_MAX_SPLITS tries and report the spill failure
   (AmigaPorts/m68k-amigaos-gcc#309).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga register-parameter ABI" { ! { m68k-*-amigaos* } } } */
/* { dg-options "-O2 -m68020 -mlra" } */

typedef short SHORT;
typedef char UBYTE;

struct BoardInfo {
  void (*Default) (struct BoardInfo *, int *, int *,
		   SHORT __asm ("d0"), SHORT __asm ("d1"),
		   SHORT __asm ("d2"), SHORT __asm ("d3"),
		   SHORT __asm ("d4"), SHORT __asm ("d5"),
		   UBYTE __asm ("d6"), UBYTE __asm ("d7"));
};

struct BoardInfo b;
int bm, r;

void
f (SHORT x __asm ("d0"), SHORT y __asm ("d1"),
   SHORT dx __asm ("d2"), SHORT dy __asm ("d3"),
   SHORT w __asm ("d4"), SHORT h __asm ("d5"),
   UBYTE minterm __asm ("d6"), UBYTE mask __asm ("d7"))
{
  if (w == 5 || h < 7)
    return;
  b.Default (&b, &bm, &r, x, y, dx, dy, w, h, minterm, mask);
} /* { dg-error "unable to find a register to spill" } */
/* { dg-error "this is the insn" "" { target *-*-* } .-1 } */
/* { dg-prune-output "confused by earlier errors" } */
/* The insn's RTL, which fatal_insn prints after the errors.  */
/* { dg-prune-output "\\(\[^\n\]*" } */
/* { dg-prune-output "-> \[0-9\]+\\)" } */
