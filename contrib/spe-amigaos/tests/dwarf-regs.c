/* Compile with -fbuilding-libgcc, for conventional PPC and SPE separately.  */
#ifdef __SPE__
#define EXPECTED_REGISTERS 153
#else
#define EXPECTED_REGISTERS 119
#endif
#if __LIBGCC_DWARF_FRAME_REGISTERS__ != EXPECTED_REGISTERS
#error unexpected unwind context size
#endif

static struct {
  unsigned char before[8];
  unsigned char sizes[EXPECTED_REGISTERS + 1];
  unsigned char after[8];
} table;

int
test_main (void)
{
  for (unsigned i = 0; i < 8; ++i)
    table.before[i] = table.after[i] = 0xa5;
  __builtin_init_dwarf_reg_size_table (table.sizes);
  for (unsigned i = 0; i < 8; ++i)
    if (table.before[i] != 0xa5 || table.after[i] != 0xa5)
      return 1;
#ifdef __SPE__
  for (unsigned i = 121; i <= 152; ++i)
    if (table.sizes[i] != 4)
      return 2;
#endif
  return 0;
}
