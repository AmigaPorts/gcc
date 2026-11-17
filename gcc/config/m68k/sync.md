;; GCC machine description for m68k synchronization instructions.
;; Copyright (C) 2011-2026 Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.
;;
;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.


(define_expand "atomic_compare_and_swap<mode>"
  [(match_operand:QI 0 "register_operand" "")		;; bool success output
   (match_operand:I 1 "register_operand" "")		;; oldval output
   (match_operand:I 2 "memory_operand" "")		;; memory
   (match_operand:I 3 "register_operand" "")		;; expected input
   (match_operand:I 4 "register_operand" "")		;; newval input
   (match_operand:SI 5 "const_int_operand" "")		;; is_weak
   (match_operand:SI 6 "const_int_operand" "")		;; success model
   (match_operand:SI 7 "const_int_operand" "")]		;; failure model
  "TARGET_CAS"
{
  emit_insn (gen_atomic_compare_and_swap<mode>_1
	     (operands[0], operands[1], operands[2],
	      operands[3], operands[4]));
  emit_insn (gen_negqi2 (operands[0], operands[0]));
  DONE;
})

(define_insn "atomic_compare_and_swap<mode>_1"
  [(set (match_operand:I 1 "register_operand" "=d")
	(unspec_volatile:I
	  [(match_operand:I 2 "memory_operand" "+m")
	   (match_operand:I 3 "register_operand" "1")
	   (match_operand:I 4 "register_operand" "d")]
	  UNSPECV_CAS_1))
   (set (match_dup 2)
	(unspec_volatile:I
	  [(match_dup 2) (match_dup 3) (match_dup 4)]
	  UNSPECV_CAS_2))
   (set (match_operand:QI 0 "register_operand" "=d")
	(unspec_volatile:QI
	  [(match_dup 2) (match_dup 3) (match_dup 4)]
	  UNSPECV_CAS_2))]
  "TARGET_CAS"
  ;; Elide the seq if operands[0] is dead.
  "cas<sz> %1,%4,%2\;seq %0")

(define_expand "atomic_test_and_set"
  [(match_operand:QI 0 "register_operand" "")		;; bool success output
   (match_operand:QI 1 "memory_operand" "")		;; memory
   (match_operand:SI 2 "const_int_operand" "")]		;; model
  "ISA_HAS_TAS"
{
  rtx t = gen_reg_rtx (QImode);
  emit_insn (gen_atomic_test_and_set_1 (t, operands[1]));
  t = expand_simple_unop (QImode, NEG, t, operands[0], 0);
  if (t != operands[0])
    emit_move_insn (operands[0], t);
  DONE;
})

(define_insn "atomic_test_and_set_1"
  [(set (match_operand:QI 0 "register_operand" "=d")
	(unspec_volatile:QI
	  [(match_operand:QI 1 "memory_operand" "+m")]
	  UNSPECV_TAS_1))
   (set (match_dup 1)
	(unspec_volatile:QI [(match_dup 1)] UNSPECV_TAS_2))]
  "ISA_HAS_TAS"
{
  /* tas uses a locked read-modify-write cycle, which the Amiga chipset
     does not support.  bset sets the same value, 0x80.  ColdFire keeps
     tas: its bset #imm takes fewer addressing modes than "m".  */
  if (TARGET_AMIGA && !TARGET_COLDFIRE)
    return "bset #7,%1\;sne %0";
  return "tas %1\;sne %0";
})

;; AmigaOS runs on a single CPU, where an instruction that reads and
;; writes memory cannot be interrupted halfway: it is atomic without a
;; locked bus cycle.  The volatile insn is a compiler barrier, which is
;; all that any memory model needs there.

(define_code_iterator atomic_op [plus minus and ior xor])
(define_code_attr atomic_optab
  [(plus "add") (minus "sub") (and "and") (ior "or") (xor "xor")])

(define_insn "atomic_<atomic_optab><mode>"
  [(set (match_operand:I 0 "memory_operand" "+m")
	(unspec_volatile:I
	  [(atomic_op:I (match_dup 0)
			(match_operand:I 1 "nonmemory_operand" "di"))
	   (match_operand:SI 2 "const_int_operand")]		;; model
	  UNSPECV_ATOMIC_OP))]
  "TARGET_AMIGA && !TARGET_COLDFIRE"
{
  return m68k_output_atomic_op (<CODE>, <MODE>mode, operands);
})

(define_expand "atomic_<atomic_optab>_fetch_cmp_0<mode>"
  [(match_operand:QI 0 "register_operand")
   (atomic_op:I (match_operand:I 1 "memory_operand")
		(match_operand:I 2 "nonmemory_operand"))
   (match_operand:SI 3 "const_int_operand")			;; model
   (match_operand:SI 4 "const_int_operand")]			;; comparison
  "TARGET_AMIGA && !TARGET_COLDFIRE"
{
  enum rtx_code cmp = (enum rtx_code) INTVAL (operands[4]);
  /* add and sub can set V, so only N and Z give the sign of the
     result.  */
  if ((cmp == GT || cmp == LE) && (<CODE> == PLUS || <CODE> == MINUS))
    FAIL;
  rtx t = gen_reg_rtx (QImode);
  emit_insn (gen_atomic_<atomic_optab>_fetch_cmp_0<mode>_1
	     (t, operands[1], operands[2], operands[4]));
  t = expand_simple_unop (QImode, NEG, t, operands[0], 0);
  if (t != operands[0])
    emit_move_insn (operands[0], t);
  DONE;
})

(define_insn "atomic_<atomic_optab>_fetch_cmp_0<mode>_1"
  [(set (match_operand:QI 0 "register_operand" "=d")
	(unspec_volatile:QI
	  [(match_operand:I 1 "memory_operand" "+m")
	   (match_operand:I 2 "nonmemory_operand" "di")
	   (match_operand:SI 3 "const_int_operand")]		;; comparison
	  UNSPECV_ATOMIC_OP_CMP_0))
   (set (match_dup 1)
	(unspec_volatile:I
	  [(atomic_op:I (match_dup 1) (match_dup 2))]
	  UNSPECV_ATOMIC_OP))]
  "TARGET_AMIGA && !TARGET_COLDFIRE"
{
  output_asm_insn (m68k_output_atomic_op (<CODE>, <MODE>mode,
					  &operands[1]), &operands[1]);
  switch ((enum rtx_code) INTVAL (operands[3]))
    {
    case EQ: return "seq %0";
    case NE: return "sne %0";
    case LT: return "smi %0";
    case GE: return "spl %0";
    case GT: return "sgt %0";
    case LE: return "sle %0";
    default: gcc_unreachable ();
    }
})
