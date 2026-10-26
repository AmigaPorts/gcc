/* { dg-do compile } */
/* { dg-options "-O2 -fbaserel" } */
/* A __saveds function is entered with the caller's a4, such as a hook or an
   interrupt handler: it saves a4, loads its own with ___restore_a4 before
   its first a4-relative access, and gives the caller's back.  A function
   without __saveds does neither.  */

int g;

__saveds int hook (void)
{
  return g;
}

int plain (void)
{
  return g + 1;
}

/* { dg-final { scan-assembler-times {jsr ___restore_a4} 1 } } */
/* { dg-final { scan-assembler-times {move\.l a4,-\(sp\)} 1 } } */
/* { dg-final { scan-assembler-times {move\.l \(sp\)\+,a4} 1 } } */
