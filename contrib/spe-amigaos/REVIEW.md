# SPE review follow-up

This records the review of original PRs #167 and #168 and the consolidation
of the later transfer/benchmark work from #172 into #167.

## Runtime selection

The old generated list had nine entries, including
`clib2/soft-float/spe`. It did not have ten: an explicit conventional
newlib entry aliases the default directory. Removing the unintended
soft-float SPE combination leaves eight entries:

```text
.
clib2
clib4
spe
newlib/spe
clib2/soft-float
clib2/spe
clib4/spe
```

AmigaOS4 provides only the e500v2 hard-float SPE runtime in this port.
The driver rejects plain `-mcpu=8540`, incomplete explicit SPE selections,
and conflicting soft-float/non-SPE ABI options, including when linking
existing objects. The backend also checks the final effective options.
The historical `-mcpu=8540 -mspe -mabi=spe -mfloat-gprs=double` spelling
remains supported because it explicitly requests the e500v2 double ABI.

## Conventional unwind layout

`FIRST_PSEUDO_REGISTER` necessarily includes the restored SPE registers.
`DWARF_FRAME_REGISTERS` now retains the pre-port value 119 for conventional
builds and uses 153 for SPE. This preserves the conventional libgcc unwind
context and table sizes instead of growing them for an unused feature.
Compile checks verify both libgcc predefines. Runtime tests surround the
size table with guard bytes and verify its initialization, including the
32 SPE high-word entries.

This does not establish exception compatibility with every older SDK
`libgcc.so`. An actual OS4 throw/catch test across a shared-library boundary
remains an integration check. Conventional assembly equality and a
freestanding C++ test were never evidence for that boundary.

## Alignment audit

The e500 reference manual describes naturally misaligned SPE accesses as
alignment-interrupt conditions (Rev. 1, section 5.7.6). Such an interrupt
would incur software handling overhead on a system that emulates it;
QEMU passing a misaligned test alone cannot prove the absence of that
problem on hardware.

At `-O3 -mcpu=8548`, the float-add, integer-add and float-absolute loops in
`tests/optimize-kernels.c` have vectorizer dumps reporting both alignment
versioning and peeling. In the add loops, assembly checks that all pointers
have the same offset modulo eight, falls back to scalar code if not, and
peels one four-byte element when their common offset is four. The vector
path therefore starts with eight-byte-aligned addresses. Odd tails use
scalar code. The autovectorization test also checks for versioning/peeling
in the dump, rather than relying solely on emulator behavior.

SPE has no `movmisalign` pattern. The generic pattern in `vector.md` is
gated on VSX, and the target misalignment hook does not grant SPE
unaligned-vector support. Scalar `float *` accesses keep their four-byte
alignment contract; the new vector transfer patterns do not weaken it.

For performance, align buffers to eight bytes when possible and preserve
that alignment through offsets. If all pointers have a common four-byte
offset, scalar peeling already recovers a vector loop. Incompatible
relative offsets use the safe scalar fallback. A supplied and valid
`__builtin_assume_aligned(p, 8)` can avoid checks, but the compiler must not
invent that promise for arbitrary pointers. Hardware timing is needed to
quantify the check/fallback cost; there is no measured penalty to report
from QEMU.

## Scheduler and reload assumptions

E500CORERM Rev. 1 Table 4-8 explicitly lists `efdadd`, `efdmul`, `efdsub`
and double comparisons as 6:1 latency:throughput. Section 4.6 defines the
colon notation. The pipelined arithmetic reservation therefore stays;
division remains 32 cycles. `efdcfs`/`efscfd` are absent from the table,
so the source now labels their six-cycle reservation as an assumption.
The new stage names follow the existing `ppc8540_mu_stage*` naming even
when the double operation is only available on e500v2.

The post-reload double-word extraction path now asserts that its output
is a GPR. The public README describes the automatic ABI change selected
by `-mcpu=8548` and identifies the double scheduler reservation as new
modeling rather than a restoration of GCC6 code.

Reference: https://www.nxp.com/docs/en/reference-manual/E500CORERM.pdf
