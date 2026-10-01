/* With an explicit -mregparm=0, only a regparm attribute with a nonzero
   count passes arguments in registers.  regparm (0) and attributes that do
   not concern argument passing (saveds, nonnull) keep the stack, like a
   plain prototype.  They used to select M68K_DEFAULT_REGPARM (d0/d1/a0).  */

/* { dg-do compile } */
/* { dg-skip-if "amiga regparm ABI" { ! { m68k-*-amigaos* } } } */
/* saveds is only meaningful with -fbaserel; without it the attribute warns.  */
/* { dg-options "-O1 -fbaserel -mregparm=0" } */

int plain (int, int, void *);
__attribute__ ((regparm (0))) int r0 (int, int, void *);
__attribute__ ((saveds)) int sv (int, int, void *);
__attribute__ ((nonnull)) int nn (int, int, void *);
__attribute__ ((regparm (2))) int r2 (int, int, void *);
__attribute__ ((stkparm)) int stk (int, int, void *);

int c_plain (void *p) { return plain (1111, 1112, p); }
int c_r0 (void *p) { return r0 (2221, 2222, p); }
int c_sv (void *p) { return sv (3331, 3332, p); }
int c_nn (void *p) { return nn (4441, 4442, p); }
int c_r2 (void *p) { return r2 (5551, 5552, p); }
int c_stk (void *p) { return stk (6661, 6662, p); }

/* { dg-final { scan-assembler "pea 1111\\.w" } } */
/* { dg-final { scan-assembler-not "#1111,d0" } } */
/* { dg-final { scan-assembler "pea 2221\\.w" } } */
/* { dg-final { scan-assembler-not "#2221,d0" } } */
/* { dg-final { scan-assembler "pea 3331\\.w" } } */
/* { dg-final { scan-assembler-not "#3331,d0" } } */
/* { dg-final { scan-assembler "pea 4441\\.w" } } */
/* { dg-final { scan-assembler-not "#4441,d0" } } */
/* { dg-final { scan-assembler "pea 6661\\.w" } } */
/* { dg-final { scan-assembler-not "#6661,d0" } } */
/* { dg-final { scan-assembler "#5551,d0" } } */
/* { dg-final { scan-assembler "#5552,d1" } } */
