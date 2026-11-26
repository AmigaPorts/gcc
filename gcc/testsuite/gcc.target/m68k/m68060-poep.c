/* Instructions the 68060's secondary pipeline doesn't take (MC68060 User's
   Manual, Table 10-2).  MOVE #<imm>,<mem> and BSET #<imm> are
   pOEP-until-last, and ST is pOEP-but-allows-sOEP, so they go in the
   primary pipeline with a secondary instruction beside them; SWAP and PEA
   are pOEP-only, alone.  A symbol
   displacement from an address register is the full format (bd.L,An),
   which only the primary pipeline takes (10.1.3).  */
/* { dg-do compile } */
/* { dg-skip-if "a4-relative, pc-relative or GOT addresses, register arguments, 16-bit int" { *-*-* } { "-fbaserel*" "-resident*" "-mpcrel" "-fpic" "-fPIC" "-msep-data" "-mid-shared-library" "-mregparm*" "-msasregparm" "-mshort" } { "" } } */
/* { dg-options "-O2 -m68060 -fomit-frame-pointer -fsched-verbose=1 -fdump-rtl-sched2" } */

void
store_imm (int *p, int *q, int a, int b)
{
  *p = 0x12345678;
  *q = a + b;
}

unsigned
swap16 (unsigned x, unsigned y)
{
  return ((x << 16) | (x >> 16)) + (y << 1);
}

unsigned
bset18 (unsigned x, unsigned y)
{
  return (x | 0x40000u) + (y << 1);
}

char
ones_reg (int *s, int a, int b)
{
  char c = -1;

  __asm__ ("" : "+d" (c));
  *s = a + b;
  return c;
}

extern char table[256];
void g (int, int, char *);

void
push_symbol (void)
{
  g (10, 1000, table);
}

char
full_symbol (long i)
{
  return table[i];
}

/* { dg-final { scan-rtl-dump {\[%?a\d\]=0x12345678 +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {%?d\d=%?d\d<-<0x10 +:m68060_single\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {%?d\d=%?d\d\|0x40000 +:m68060_pOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {%?d\d=0xffffffffffffffff +:m68060_pOEP\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {\[--%?sp\]=`table' +:\(m68060_single\+m68060_mem\)\n} "sched2" } } */
/* { dg-final { scan-rtl-dump {%?d\d=\[%?a\d\+`table'\] +:\(m68060_pOEP\+m68060_mem\)\n} "sched2" } } */
