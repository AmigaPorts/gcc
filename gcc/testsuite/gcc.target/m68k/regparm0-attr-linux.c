/* The AmigaOS correction must not change the established Linux ABI:
   with -mregparm=0, attributed function types still select the default
   register count, while a plain prototype uses the stack.  */

/* { dg-do compile { target m68k*-*-linux* } } */
/* { dg-options "-O1 -m68000 -mregparm=0 -ffreestanding" } */

int plain (int, int, void *);
__attribute__ ((regparm (0))) int r0 (int, int, void *);
__attribute__ ((nonnull)) int nn (int, int, void *);
__attribute__ ((regparm (2))) int r2 (int, int, void *);

int c_plain (void *p) { return plain (1111, 1112, p); }
int c_r0 (void *p) { return r0 (2221, 2222, p); }
int c_nn (void *p) { return nn (3331, 3332, p); }
int c_r2 (void *p) { return r2 (4441, 4442, p); }

/* { dg-final { scan-assembler "pea 1111" } } */
/* { dg-final { scan-assembler-not "#1111,%d0" } } */
/* { dg-final { scan-assembler "#2221,%d0" } } */
/* { dg-final { scan-assembler "#2222,%d1" } } */
/* { dg-final { scan-assembler-not "pea 2221" } } */
/* { dg-final { scan-assembler "#3331,%d0" } } */
/* { dg-final { scan-assembler "#3332,%d1" } } */
/* { dg-final { scan-assembler-not "pea 3331" } } */
/* { dg-final { scan-assembler "#4441,%d0" } } */
/* { dg-final { scan-assembler "#4442,%d1" } } */
