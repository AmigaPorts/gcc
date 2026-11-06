/* { dg-do compile { target m68k*-*-* } } */
/* { dg-options "-O2 -m68020 -fdump-rtl-peephole2" } */

/* memory_operand accepts SUBREG (MEM) after reload.  Do not read
   MEM flags from a SUBREG or reorder a hidden volatile load.  */

void __RTL (startwith ("peephole2")) subreg_load (void)
{
(function "subreg_load"
  (insn-chain
    (block 2
      (edge-from entry (flags "FALLTHRU"))
      (cnote 1 [bb 2] NOTE_INSN_BASIC_BLOCK)
      (cinsn 2 (set (reg:SI d0)
        (subreg:SI (mem/v:DI (reg:SI a0) [0 S8 A32]) 0)))
      (cinsn 3 (set (reg:SI d1)
        (mem:SI (reg:SI a1) [0 S4 A32])))
      (cjump_insn 4 (set (pc)
        (if_then_else (eq (reg:SI d0) (const_int 0))
          (label_ref 8) (pc))))
      (edge-to 3 (flags "FALLTHRU"))
      (edge-to 4)
    )
    (block 3
      (edge-from 2 (flags "FALLTHRU"))
      (cnote 5 [bb 3] NOTE_INSN_BASIC_BLOCK)
      (cinsn 6 (set (mem:SI (reg:SI a2) [0 S4 A32]) (reg:SI d1)))
      (edge-to 4 (flags "FALLTHRU"))
    )
    (block 4
      (edge-from 2)
      (edge-from 3 (flags "FALLTHRU"))
      (clabel 8 1)
      (cnote 9 [bb 4] NOTE_INSN_BASIC_BLOCK)
      (cinsn 10 (use (reg:SI d0)))
      (edge-to exit (flags "FALLTHRU"))
    )
  )
)
}

/* { dg-final { scan-assembler-times {tst\.l} 1 } } */
