# SPE code-generation audit

`codegen.c` covers 31 small functions: scalar/vector arithmetic, FP/integer
conversions, lane reads and updates, construction, reductions, selects,
and aligned/unaligned load/store interfaces. Compile with
`-mcpu=8548 -O3 -S`; no fast-math is used. Inspect instruction dependencies
as well as counts. A single function's static instruction count is not a
cycle estimate, especially for branches or FP conversions.

The GCC6 comparison uses Walkero's adtools GCC 6.4.0 from
`os4-gcc6-base-1.8.0` (ARM64 image digest
`sha256:524d4806f65edc76da0dc111cb5fd474bf48695367519f1f46cc57b397e4f5af`).
The GCC16 candidate includes the earlier SPE restoration and optimization
work plus this follow-up. Counts include return and all prologue/epilogue
instructions. They are assembly observations, not physical timings.

| Function | GCC6 instructions | GCC16 candidate | Observation |
| --- | ---: | ---: | --- |
| high float lane | 5 | 2 | One merge, no stack frame |
| low float lane | 1 | 1 | Return register already contains the lane |
| two-lane sum | 8 | 4 | No stack extraction |
| two-lane dot product | 8 | 4 | Paired multiply, merge, scalar add, return |
| two-float absolute value | 11 | 2 | Native paired absolute-value instruction |
| float pair construction | 6 | 2 | One merge |
| integer pair/splat construction | 8 | 6 | Still uses stack traffic |
| scalar-pointer pair load | 8 | 4 | Two scalar loads and merge, safe at 4-byte alignment |
| scalar-pointer pair store | 9 | 4 | Merge and two scalar stores |
| aligned vector load/store | 2 | 2 | Already one memory instruction plus return |
| int32/uint32 to float | 2 | 2 | Already direct conversion |
| int32/uint32 to double | 12 | 4 | Direct conversion plus return packing |
| double to int32/uint32 | 7 | 3 | Argument packing plus direct conversion |
| float division | 2 | 2 | Already direct division |
| double division | 16 | 6 | ABI packing dominates the wrapper |
| two-float division | 2 | 2 | Already direct paired division |
| float multiply then add | 3 | 3 | Separate multiply/add, strict semantics |
| double multiply then add | 18 | 8 | Arithmetic plus ABI packing |
| float conditional minimum | 6 | 8 | Remaining CR-to-GPR-to-CR round trip |
| double conditional minimum | 20 | 12 | Same select issue, better ABI packing |

The original GCC16 port used five instructions for either float lane read,
eight for the dot product and eight for scalar-pointer pair stores. The
new transfer patterns remove those stack round trips. Float-vector
permutations and construction also let the matrix example use paired
arithmetic without enabling fast-math. The generic transform retains an
alignment-checked scalar path; code size can increase while its vector
path performs fewer arithmetic instructions.

For double Mandelbrot, the old zero-cost single-to-double conversion model
caused the escape constant to be loaded as a float and converted by
`efdcfs` inside the iteration loop. Accounting for the extra instruction
keeps the constant in double format. This reproduces a concrete GCC16
code-generation disadvantage; it does not establish the cause or size of
Bill's reported timing regression without the original benchmark.

## Remaining priorities

1. **Floating-point selects.** The candidate emits `efscmplt`, `mfcr`, two
   rotates/masks, `cmpwi`, and a branch for a scalar conditional minimum.
   A direct branch on the SPE comparison result should avoid the GPR
   round trip. Preserve SPE condition-bit semantics and NaN behavior.
   GCC6's shorter float sequence converts both inputs to double first,
   so fewer instructions alone do not establish that GCC6 is faster.
2. **Integer vector construction and lane insertion.** Integer pair/splat
   construction still takes a stack round trip, as do float lane updates.
   SPE merges can potentially implement these in registers. Cover overlap,
   bit preservation and register pressure before enabling the patterns.
3. **Vector cost and register-pressure models.** Some generic costs still
   describe VSX operations, and SPE vector class register counts deserve
   review. Tune these only against representative assembly and A1222
   measurements; an optimistic cost can choose an unprofitable shuffle.
4. **Alignment-sensitive vectorization.** Keep the current alignment tests
   and scalar fallbacks. SPE vector accesses require eight-byte alignment;
   a scalar `float *` alone does not provide it. Measure both aligned and
   offset buffers instead of removing checks based on a QEMU run.

The full-output matrix/Mandelbrot comparison passes with GCC6, the original
GCC16 SPE image and the candidate under QEMU e500v2, matching the hashes in
README.md. The candidate also passes 32 SPE runtime images at O0/O2/O3/Os,
228 builtin compile checks, six loop-corpus compilations and 40 focused
DejaGNU checks. The new transfer tests include all 16 two-vector shuffle
selections and bit preservation for signed zeros and NaN payloads. These
are big-endian correctness checks, not A1222 performance measurements.
