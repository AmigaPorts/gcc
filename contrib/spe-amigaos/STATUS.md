# Validation status

Validated against AmigaPorts/gcc base
`4a30559c9f3baf1b3e68a9b3a9185700fd726951`, using GCC 16.2.1b,
the AmigaOS4 SDK/binutils from the 2026-11-03 PPC toolchain, and QEMU
11.1.50 (`v11.1.0-3642-gb6ad0b0305`).

Completed:

- C and C++ cross compiler build.
- All 228 SPE builtin declarations compile.
- Scalar/ABI, vector, intrinsic-memory, C++, conventional-caller bridge,
  and linked libgcc runtime tests at `-O0`, `-O2`, `-O3`, `-Os`:
  all 24 freestanding QEMU images passed.
- All nine conventional and SPE libgcc and libstdc++ multilib variants
  build successfully.
- Conventional scalar test assembly matches the installed compiler for
  default, 750, 7400/AltiVec, and soft-float options.
- SDK `<stdint.h>` plus `<spe.h>` compiles for newlib/clib2/clib4 in C/C++.
- SPEv1 scalar source compiles, using software for double precision.
- Invalid intrinsic immediates and ABI-changing function attributes emit
  diagnostics without an internal compiler error.

The linked helper tests exercise 64-bit conversions, integer powers and
complex multiplication. The bridge test calls an SPE kernel through a
pointer interface from conventional `-mcpu=750` code. The harness verifies
an explicit completion breakpoint and the return code for each image.

Not validated: AmigaOS4 runtime, physical A1222, IEEE assist handlers,
exceptions/unwinding under OS4, performance, or SDK floating-point ABI
boundaries. The ordinary SDK runtimes have not been ported to SPE.
This is focused regression coverage, not a full GCC DejaGNU run.
