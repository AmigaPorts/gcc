;; =========================================================================
;; BEBBO*s-PATCHES for GCC 16
;; =========================================================================

;; =========================================================================
;; DOLOOP PIPELINE FOR M68K (HI/SI COMBINED)
;; =========================================================================
(define_expand "doloop_end"
  [(match_operand 0 "" "")  ; The loop counter register (can be SI or HI)
   (match_operand 1 "" "")] ; The target label
  ""
{
  /* Strictly require a register for the dbra counter. */
  if (!REG_P (operands[0]))
    FAIL;

  /* Detect the mode of the counter and emit the correct, fully-typed RTL sequence.
     This completely prevents the mode-mismatch ICE in patch_jump_insn. */
  if (GET_MODE (operands[0]) == HImode)
    {
      emit_jump_insn (gen_rtx_PARALLEL (VOIDmode, gen_rtvec (3,
	gen_rtx_SET (pc_rtx,
		     gen_rtx_IF_THEN_ELSE (VOIDmode,
					   gen_rtx_NE (VOIDmode, operands[0], const0_rtx),
					   gen_rtx_LABEL_REF (VOIDmode, operands[1]),
					   pc_rtx)),
	gen_rtx_SET (operands[0],
		     gen_rtx_PLUS (HImode, operands[0], constm1_rtx)),
	gen_rtx_CLOBBER (VOIDmode, gen_rtx_SCRATCH (HImode)))));
      DONE;
    }
  else if (GET_MODE (operands[0]) == SImode)
    {
      emit_jump_insn (gen_rtx_PARALLEL (VOIDmode, gen_rtvec (3,
	gen_rtx_SET (pc_rtx,
		     gen_rtx_IF_THEN_ELSE (VOIDmode,
					   gen_rtx_NE (VOIDmode, operands[0], const0_rtx),
					   gen_rtx_LABEL_REF (VOIDmode, operands[1]),
					   pc_rtx)),
	gen_rtx_SET (operands[0],
		     gen_rtx_PLUS (SImode, operands[0], constm1_rtx)),
	gen_rtx_CLOBBER (VOIDmode, gen_rtx_SCRATCH (SImode)))));
      DONE;
    }
  else
    FAIL;
})

;; =========================================================================
;; MATCHING INSN PATTERNS (LRA SAFE, CLOBBER ASSIGNS DATA REGISTER)
;; =========================================================================

;; 1. The native 16-Bit Loop
(define_insn "*m68k_doloop_hi16"
  [(set (pc)
        (if_then_else
          (ne (match_operand:HI 0 "register_operand" "+d")
              (const_int 0))
          (label_ref (match_operand 1 "" ""))
          (pc)))
   (set (match_dup 0)
        (plus:HI (match_dup 0)
                 (const_int -1)))
   (clobber (match_scratch:HI 2 "=&d"))]
  ""
  "dbra %0,%l1"
  [(set_attr "type" "bcc")])

;; 2. The 32-Bit Nested Loop
(define_insn "*m68k_doloop_si32"
  [(set (pc)
        (if_then_else
          (ne (match_operand:SI 0 "register_operand" "+d")
              (const_int 0))
          (label_ref (match_operand 1 "" ""))
          (pc)))
   (set (match_dup 0)
        (plus:SI (match_dup 0)
                 (const_int -1)))
   (clobber (match_scratch:SI 2 "=&d"))]
  ""
  "dbra %0,%l1\;clr%.w %0\;subq%.l #1,%0\;jcc %l1"
  [(set_attr "type" "bcc")])
