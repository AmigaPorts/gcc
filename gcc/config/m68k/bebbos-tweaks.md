;; =========================================================================
;; BEBBO*s-PATCHES for GCC 16
;; =========================================================================

;; =========================================================================
;; DOLOOP PIPELINE FOR M68K (HI/SI COMBINED)
;; =========================================================================
;; The loop end is m68k.md's dbne_hi or dbne_si. Their "+d*g" makes IRA
;; cost the counter as a data register, for dbra, and still accepts an
;; address register or a stack slot when the data registers run out.
(define_expand "doloop_end"
  [(match_operand 0 "" "")  ; The loop counter register (can be SI or HI)
   (match_operand 1 "" "")] ; The target label
  "!TARGET_COLDFIRE"
{
  /* Strictly require a register for the dbra counter. */
  if (!REG_P (operands[0]))
    FAIL;

  /* Emit the pattern of the counter's mode: a mode mismatch would ICE in
     patch_jump_insn. */
  if (GET_MODE (operands[0]) == HImode)
    emit_jump_insn (gen_dbne_hi (operands[0], operands[1]));
  else if (GET_MODE (operands[0]) == SImode)
    emit_jump_insn (gen_dbne_si (operands[0], operands[1]));
  else
    FAIL;
  DONE;
})

;; =========================================================================
;; MOVMEMSI / SETMEMSI - SUPPORT use_by_pieces_infrastructure 
;; =========================================================================

;; Argument 0 is the destination
;; Argument 1 is the source
;; Argument 2 is the length
;; Argument 3 is the alignment
(define_expand "movmemsi"
  [(parallel [(set (match_operand:BLK 0 "general_operand")
		   (match_operand:BLK 1 "general_operand"))
	      (use (match_operand:SI 2 "const_int_operand"))
	      (use (match_operand:SI 3 "const_int_operand"))])]
  ""
{
	if (m68k_emit_movmemsi(operands[0], operands[1], operands[2], operands[3],
			       true))
	  DONE;
	else
	  FAIL;
})

(define_expand "cpymemsi"
  [(parallel [(set (match_operand:BLK 0 "general_operand")
		   (match_operand:BLK 1 "general_operand"))
	      (use (match_operand:SI 2 "const_int_operand"))
	      (use (match_operand:SI 3 "const_int_operand"))])]
  ""
{
	if (m68k_emit_movmemsi(operands[0], operands[1], operands[2], operands[3],
			       false))
	  DONE;
	else
	  FAIL;
})


(define_expand "setmemsi"
  [(parallel [(set (match_operand:BLK 0 "" "")
		   (match_operand:SI 1 "const_int_operand" ""))
	      (use (match_operand:SI 2 "const_int_operand" ""))
	      (use (match_operand:SI 3 "const_int_operand" ""))])]
  ""
{
  if (m68k_emit_setmemsi (operands[0], operands[2], operands[1], operands[3]))
    DONE;
  else
    FAIL;
})
