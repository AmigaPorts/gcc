/* { dg-do compile { target powerpc*-*-amigaos* } } */
/* { dg-options "-O2 -mcpu=750 -fbuilding-libgcc" } */
#if __LIBGCC_DWARF_FRAME_REGISTERS__ != 119
#error unexpected unwind context size
#endif
unsigned char sizes[__LIBGCC_DWARF_FRAME_REGISTERS__ + 1];
void init (void) { __builtin_init_dwarf_reg_size_table (sizes); }
