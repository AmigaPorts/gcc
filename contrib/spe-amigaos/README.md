# Experimental GCC 16.2 SPE port for A1222 / AmigaOS4

This restores GCC6's e500 SPE code generation inside GCC16's rs6000
backend. It includes scalar single/double precision, the SPE vector
intrinsics and header, register allocation and calling conventions,
prologues/epilogues, unwind register descriptions, and libgcc save/restore
routines. The new builtin definitions use GCC16's generator.

The port is based on AmigaPorts/gcc commit
`4a30559c9f3baf1b3e68a9b3a9185700fd726951` on `amiga16.2`.
It is experimental: the validation below is cross-compilation and
freestanding QEMU execution, not an AmigaOS4 or physical A1222 validation.
See [STATUS.md](STATUS.md) and [validation.json](validation.json) for results.

## Build

Build this GCC source tree with the ordinary AmigaOS4 configuration and
SDK. The default target retains the conventional PowerPC ABI; the new SPE
multilibs are selected by CPU options. Use a separate build directory and
installation prefix while testing the port.

The existing container-amiga-gcc native/container build wrapper can select
this source branch without maintaining a patch in that repository:

```sh
./build.sh --target ppc-amigaos \
  --version 16.2:stefan/ppc-amigaos-spe \
  --workdir /tmp/amiga-spe-build --prefix /tmp/amiga-spe-install
```

For the A1222's e500v2, compile SPE translation units with:

```sh
ppc-amigaos-gcc -mcpu=8548 -O2 -c kernel.c
```

`-mcpu=8548` selects SPE, its calling convention, double-precision GPR
floating point, and the SPE library directory. The GCC6 spelling
`-mcpu=8540 -mspe -mabi=spe -mfloat-gprs=double` also selects the double
precision SPE library variant through `-mfloat-gprs=double`.

This changes the meaning of an existing `-mcpu=8548` command line: it now
selects native SPE floating point and the GPR ABI automatically. Rebuild
matching objects together and keep conventional SDK boundaries separate.
Plain `-mcpu=8540`/SPEv1 and soft-float SPE combinations are rejected,
including link-only invocations, because no matching runtime is provided.
The supported set contains eight multilib directories; the conventional
clib2 soft-float directory remains available.

## AmigaOS4 interface boundaries

SPE code passes floating-point values in GPRs. Conventional AmigaOS4 code
passes them in FPRs. Recompiling a translation unit does not change the ABI
of an existing SDK library, shared object, callback, or C++ library.

For initial integration, put numerical kernels in SPE translation units
and use integer/pointer interfaces to ordinary AmigaOS4 code. Keep code
that calls conventional floating-point APIs in conventional translation
units; exchange values through memory. Keep formatted I/O with floating
arguments and other variadic library calls on that side too. An SPE
libgcc is necessary for generated helper calls but does not provide an
SPE implementation of the SDK's libc or libm. Floating-point libgcc helper
symbols also have the same names across the two ABIs: linking both
archives does not automatically give each caller the right implementation.
An isolated kernel should avoid those helper calls (inspect its undefined
symbols), or use a module with explicitly private SPE runtime symbols.
The `bridge-kernel.c` / `bridge-run.c` tests demonstrate a memory-based
interface with a conventional PPC caller. Building an SPE libstdc++
alone does not make its libc/libm boundaries compatible either.

Native SPE avoids emulation of conventional FPU instructions in converted
code. Its speedup depends on the workload and the remaining library calls;
this port has no A1222 performance measurements. NaNs, infinities, and
subnormals require the operating system's e500 floating-point assistance.
The freestanding tests deliberately exercise finite normal arithmetic.

## Reproduce the compiler tests

After building GCC and its SPE libgcc, run:

```sh
python3 contrib/spe-amigaos/check.py \
  --gcc-source /tmp/amiga-spe-build/ppc-gcc-16.2 \
  --gcc-build /tmp/amiga-spe-build/ppc-gcc-16.2-build \
  --binutils /tmp/amiga-spe-install/bin \
  --libgcc /tmp/amiga-spe-build/ppc-gcc-16.2-build/gcc/spe/libgcc.a \
  --output /tmp/spe-check --run
```

`--run` needs `qemu-system-ppc` with `ppce500` / `e500v2` support and
permission to create local debugger sockets. It stops at an explicit
completion symbol, verifies the program counter, and checks the test
return value. Omit `--run` for compilation and linking only. `results.json`
records completed cases. `QEMU_PPC` can select a different QEMU executable.

The tests cover scalar arithmetic and comparisons, conversions, calls
checked against assembly, varargs, aggregates, packed data, large stack
frames, vector construction/arithmetic/load/store/select operations,
C++ templates, intrinsic aliasing, and generated libgcc calls. They run at
`-O0`, `-O2`, `-O3`, and `-Os`. Every restored SPE builtin gets an individual
compile check. This is focused regression coverage, not a full GCC
DejaGNU run or an OS4 exception/unwinding test.

The port also fixes issues found while adapting the old implementation:

- LRA folding a memory subreg while retaining a register-merge pattern;
- missing SPE vector type and constant handling in the modern backend;
- floating comparison and FMA optabs claiming unsupported operations;
- integer selects testing the conventional EQ bit after an SPE comparison;
- scaled intrinsic offsets and access widths incorrectly described to the
  optimizer, allowing wrong loads and dead-store removal.

## Optimization follow-up

The compiler exposes standard two-float vector arithmetic and selects
8-byte SPE vectors for float and integer loops. Unaligned pointers use
alignment checks, scalar peeling and scalar fallback where required.
Conventional `-msoft-float` remains available; combining it with SPE is
rejected because the port has no soft-float SPE runtime.

Double argument/return packing uses explicit register operations, and the
scheduler models the e500v2 double-precision pipeline. This `vecdouble`
reservation is new modeling: the GCC6 scheduler had no such reservation.
Table 4-8 of E500CORERM Rev. 1 gives double arithmetic a 6:1
latency:throughput and division 32 cycles. The table omits `efdcfs` and
`efscfd`; their six-cycle reservation is an explicit modeling assumption. In small functions
compiled with `-mcpu=8548 -O3`, two-float addition falls from 14 instructions
to 2 and double division from 22 to 6, including return. Both lose their
32-byte stack frames. These are code-generation results, not measured
A1222 speedups.

The `optimize` runtime suite checks vector arithmetic, double bitcasts,
empty loops, odd-length tails, and every combination of four-byte/eight-byte
pointer alignment. It adds four images to the original 24-image suite.
See [optimization-validation.json](optimization-validation.json) for the
follow-up results; `validation.json` retains the initial port's results.

## References

- [SPE integration and AmigaOS4 ABI boundaries](https://keasigmadelta.com/blog/taborizing-drivers-libraries-a-k-a-optimizing-code-for-the-e500-spe/)
- [NXP e500 core reference manual](https://www.nxp.com/docs/en/reference-manual/E500CORERM.pdf)
- [GCC6 rs6000 source](https://github.com/AmigaPorts/gcc/tree/amiga6/gcc/config/rs6000)

## Review checks

See [REVIEW.md](REVIEW.md) for the multilib corrections, conventional
unwinder layout preservation, static alignment audit, and scheduler
references. Run `check-options.py --gcc-build BUILD --output OUTPUT` to
check supported driver options and all eight multilib selections.
