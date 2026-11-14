/* { dg-do compile } */
/* { dg-options "-O2 -m68020 -fomit-frame-pointer" } */
/* Small byte-aligned copies from the 68020 on: the widest moves through
   (An)+, also below 4 bytes, and no more pointer setup than the two
   arguments need.  */

void copy1 (char *d, const char *s) { __builtin_memcpy (d, s, 1); }
void copy2 (char *d, const char *s) { __builtin_memcpy (d, s, 2); }
void copy3 (char *d, const char *s) { __builtin_memcpy (d, s, 3); }
void copy5 (char *d, const char *s) { __builtin_memcpy (d, s, 5); }
void copy6 (char *d, const char *s) { __builtin_memcpy (d, s, 6); }
void copy7 (char *d, const char *s) { __builtin_memcpy (d, s, 7); }
void copy8 (char *d, const char *s) { __builtin_memcpy (d, s, 8); }

/* 5, 6, 7 and twice for 8.  */
/* { dg-final { scan-assembler-times {move\.l \(a[0-7]\)\+,\(a[0-7]\)\+\n} 5 } } */
/* 2, 3, 6 and 7.  */
/* { dg-final { scan-assembler-times {move\.w \(a[0-7]\)\+,\(a[0-7]\)\+\n} 4 } } */
/* 1, 3, 5 and 7.  */
/* { dg-final { scan-assembler-times {move\.b \(a[0-7]\)\+?,\(a[0-7]\)\+?\n} 4 } } */
/* The two pointers of each function, and nothing else.  */
/* { dg-final { scan-assembler-times {move\.l [48]\(sp\),a[0-7]\n} 14 } } */
/* { dg-final { scan-assembler-not {\tlea } } } */
/* { dg-final { scan-assembler-not {move\.l a[0-7],} } } */
/* { dg-final { scan-assembler-not {move\.b [0-9]+\(a[0-7]} } } */
