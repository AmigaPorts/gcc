# A1222 matrix and Mandelbrot comparison

These are independent diagnostic kernels, not the unidentified suite in
Bill Borsari's physical A1222 results table. Use them to test suspected
code-generation differences while the original source is unavailable.
There are no claims here about physical A1222 speedups.

The four cases are float/double row-major 4x4 matrix transforms of 1,024
vectors and float/double Mandelbrot images of 160x120 pixels, with a
256-iteration limit. All builds use identical data and strict floating-point
semantics. Matrix inputs are dyadic fractions with exact results; the
Mandelbrot grid also has exactly representable coordinates. Float and
double Mandelbrot intentionally have different output hashes.

Every output is checked against a separately compiled `-O0` reference,
outside the measured interval. A deterministic, endian-independent hash
provides a cross-build check as well. The timer calibrates each case to
at least half a second per sample and reports minimum, median and maximum
microseconds per kernel invocation from five samples. One matrix invocation
processes 1,024 vectors; one Mandelbrot invocation renders a whole image.

## Build and run

Host smoke test:

```sh
python3 build.py --cc cc --output /tmp/spe-bench-host --label host
/tmp/spe-bench-host/spe-bench verify
/tmp/spe-bench-host/spe-bench
```

AmigaOS4, using an installed SDK/toolchain:

```sh
python3 build.py --amigaos --cc /opt/gcc6/bin/ppc-amigaos-gcc \
  --driver-cc /opt/gcc16/bin/ppc-amigaos-gcc \
  --output /tmp/bench-gcc6 --label gcc6-O3
python3 build.py --amigaos --cc /opt/gcc16/bin/ppc-amigaos-gcc \
  --driver-cc /opt/gcc16/bin/ppc-amigaos-gcc \
  --output /tmp/bench-gcc16 --label gcc16-O3
```

Adjust installation paths. GCC16 must include the SPE port. Run the same
commands against a compiler with the candidate changes to obtain a third
binary. The output directory contains `spe-bench`, `kernels.s`, object
files, and `build.json` with exact commands, compiler versions and source
hashes. Retain these alongside the timing results.

The kernel and reference use `-mcpu=8548`; the timer/I/O driver uses
`-mcpu=750` and the conventional AmigaOS4 C runtime. Only integer arguments
and pointers cross this boundary. The numerical objects require no FP
library helpers. Do not add SPE calls to conventional FP libc/libm APIs.
GCC6 also needs `-Wa,-me500`, and marks its objects as soft-float. The build
script uses `--no-warn-mismatch` for GCC6's link across this deliberately
isolated ABI boundary; it is not a general fix for mixing the two FP ABIs.

On the A1222, copy the executables to distinct names and run:

```text
spe-bench-gcc6 verify
spe-bench-gcc16 verify
spe-bench-candidate verify
spe-bench-gcc6 > gcc6.csv
spe-bench-gcc16 > gcc16.csv
spe-bench-candidate > candidate.csv
```

Run several rounds, alternating their order, under the same CPU frequency
and background load. Record the AmigaOS4 version and runtime/library
versions. `clock()` is the C runtime's process clock; the printed
`CLOCKS_PER_SEC` states its units, not a guarantee of timer resolution.
The long samples reduce coarse-timer effects. Host and QEMU timings are
not A1222 performance results.

Expected hashes with the default strict flags:

| Kernel | Hash |
| --- | --- |
| matrix-float | d720b565 |
| matrix-double | d720b565 |
| mandelbrot-float | 98665400 |
| mandelbrot-double | 8d592c90 |

For isolation, rebuild with `--opt O2`, or add
`--kernel-flag=-fno-tree-vectorize` or
`--kernel-flag=-fno-schedule-insns2`. Change one option at a time. The normal
build disables fast-math and FP contraction. Do not use relaxed FP flags
to claim an equivalent result without checking the outputs again.

`freestanding.c` provides `test_main` for the existing e500v2 QEMU harness.
It checks every result and the hashes above, without using an OS or timer.
