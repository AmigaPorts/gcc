# Validation status

Validated against AmigaPorts/gcc base
`4a30559c9f3baf1b3e68a9b3a9185700fd726951`, using GCC 16.2.1b,
the AmigaOS4 SDK/binutils from the 2026-11-03 PPC toolchain, and QEMU
11.1.50 (`v11.1.0-3642-gb6ad0b0305`).

Completed:

- C and C++ cross compiler build.
- All 228 SPE builtin declarations compile.
- Standard vector arithmetic, autovectorization, double ABI code generation,
  and scheduler reservation checks pass.
- Scalar/ABI, vector, intrinsic-memory, C++, conventional-caller bridge,
  linked libgcc, and optimization runtime tests at `-O0`, `-O2`, `-O3`, `-Os`:
  all 28 freestanding QEMU images passed.
- All nine conventional and SPE libgcc and libstdc++ multilib variants
  build successfully.
- Conventional scalar test assembly matches the installed compiler for
  default, 750, 7400/AltiVec, and soft-float options.
- SDK `<stdint.h>` plus `<spe.h>` compiles for newlib/clib2/clib4 in C/C++.
- Historical SPEv1 compile checks are superseded: unsupported SPEv1 and
  soft-float SPE selections now produce diagnostics.
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

The optimization follow-up is based on restoration commit
`97a3e6bfb04d838f630175a275630ebf5610507e`. Clean rebuilds of all nine
libgcc and libstdc++ multilib variants passed. The compile-only loop
corpus also passed at `-O2`, `-O3`, and `-Os`, with and without
`-ffast-math`.
`optimization-validation.json` records its runtime and code-generation
results. `validation.json` preserves the original restoration results.

## Consolidated review update

The current branch combines restoration and both optimization follow-ups
on AmigaPorts `amiga16.2` at
`5f61c4b95c724e04ff8d49c7afd9ae9b4b761e36`. The nine-multilib build results
above are historical: that set included an unintended clib2 soft-float SPE
variant. The current supported set has eight directories. Earlier JSON
records retain their original results; see `review-validation.json` for
the review update and `REVIEW.md` for scope and remaining OS4 checks.

The consolidated branch passes 43 focused DejaGNU checks, 40 freestanding
QEMU images, 228 builtin compile checks, six loop-corpus compilations,
and the compile/link driver-option matrix. All eight libgcc and libstdc++
multilib builds pass. The new runtime images check conventional and SPE
unwind-table initialization and bounds at all four optimization levels.
