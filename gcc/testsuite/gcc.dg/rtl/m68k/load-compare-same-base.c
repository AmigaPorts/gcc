/* { dg-do compile { target m68k*-*-* } } */
/* { dg-options "-O2 -m68020 -fdump-rtl-peephole2" } */

/* Both loads advance the same pointer.  Swapping them changes which
   value reaches each destination, even though the registers differ.
   Include the REG_INC notes present after auto-increment formation.  */

void __RTL (startwith ("peephole2")) same_base_load (void)
{
(function "same_base_load"
  (insn-chain
    (block 2
      (edge-from entry (flags "FALLTHRU"))
      (cnote 1 [bb 2] NOTE_INSN_BASIC_BLOCK)
      (cinsn 2 (set (reg:SI d0)
        (mem:SI (post_inc:SI (reg:SI a0)) [0 S4 A32]))
        (expr_list:REG_INC (reg:SI a0)))
      (cinsn 3 (set (reg:SI d1)
        (mem:SI (post_inc:SI (reg:SI a0)) [0 S4 A32]))
        (expr_list:REG_INC (reg:SI a0)))
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

/* { dg-final { scan-assembler {move\.l \(a0\)\+,d0[\n\r]+[\t ]+move\.l \(a0\)\+,d1} } } */
