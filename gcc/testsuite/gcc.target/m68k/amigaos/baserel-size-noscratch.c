/* { dg-do compile } */
/* { dg-options "-Os -m68000 -msoft-float -fbaserel -fomit-frame-pointer -ffixed-a0 -ffixed-a1" } */
/* With a0 and a1 fixed, the only free address registers are call-saved
   ones the function doesn't use.  The LEA+MOVE peephole must not take one
   as its scratch, which would need a save and restore: the address stays
   MOVE+ADD in d0, and no register is saved.  */
extern int values[16];
int *address (void) { return values; }
/* { dg-final { scan-assembler {move\.l a4,d0\n\tadd\.l #_values:W,d0} } } */
/* { dg-final { scan-assembler-not {lea } } } */
/* { dg-final { scan-assembler-not {movem|,-\(sp\)} } } */
