/* Configuration for GNU C-compiler for m68k Amiga, running AmigaOS.
 Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 2003
 Free Software Foundation, Inc.
 Contributed by Markus M. Wild (wild@amiga.physik.unizh.ch).
 Heavily modified by Kamil Iskra (iskra@student.uci.agh.edu.pl).

 This file is part of GCC.

 GCC is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 GCC is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GCC; see the file COPYING.  If not, write to
 the Free Software Foundation, 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.  */

//work without flag_writable_strings which is not in GCC4
#define REGPARMS_68K 1
#define IN_TARGET_CODE 1

#include "config.h"
#define INCLUDE_STRING
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "cfghooks.h"
#include "tree.h"
#include "stringpool.h"
#include "attribs.h"
#include "rtl.h"
#include "df.h"
#include "alias.h"
#include "fold-const.h"
#include "calls.h"
#include "stor-layout.h"
#include "varasm.h"
#include "regs.h"
#include "insn-config.h"
#include "conditions.h"
#include "output.h"
#include "insn-attr.h"
#include "recog.h"
#include "diagnostic-core.h"
#include "flags.h"
#include "expmed.h"
#include "dojump.h"
#include "explow.h"
#include "memmodel.h"
#include "emit-rtl.h"
#include "stmt.h"
#include "expr.h"
#include "reload.h"
#include "tm_p.h"
#include "target.h"
#include "debug.h"
#include "cfgrtl.h"
#include "cfganal.h"
#include "lcm.h"
#include "cfgbuild.h"
#include "cfgcleanup.h"
#include "insn-codes.h"
#include "opts.h"
#include "optabs.h"
#include "builtins.h"
#include "rtl-iter.h"
#include "toplev.h"
#include "df.h"


/* This file should be included last.  */
#include "target-def.h"

//#define MYDEBUG 1
#ifdef MYDEBUG
#define DPRINTF(x) fprintf x;
#else
#define DPRINTF(x)
#endif

/*
 * begin-GG-local: explicit register specification for parameters.
 *
 * Reworked and ported to gcc-6.2.0 by Stefan "Bebbo" Franke.
 */

extern rtx
m68k_static_chain_rtx(const_tree fntype,
			       bool incoming ATTRIBUTE_UNUSED);

/**
 * Define this here and add it to tm_p -> all know the custom type and allocate/use the correct size.
 */
struct m68k_args
{
  int num_of_regs;
  long regs_already_used;
  int last_arg_reg;
  int last_arg_len;
  tree current_param_type; /* New field: formal type of the current argument.  */
  tree current_parm_decl;  /* Callee PARM_DECL cursor: its type keeps the asmreg
			      attribute that composite_type strips from fntype's
			      arg types on a prototype/definition spelling
			      mismatch.  */
  tree fntype; /* initial function type */
};

static struct m68k_args mycum, othercum;

bool m68k_is_ok_for_sibcall(tree decl, tree exp);
/**
 * Sibcall is only ok, if max regs d0/d1/a0 are used;
 * others might be trashed due to stack pop.
 * A target that is not a direct call m68k_symbolic_jump can branch to (an
 * indirect call, or a direct one with -m68000 -fbaserel, -resident or
 * -mpcrel) is loaded into STATIC_CHAIN_REGNUM (a0 on AmigaOS) by
 * m68k_legitimize_sibcall_address, so then that register must not carry
 * an argument.
 */
bool m68k_is_ok_for_sibcall(tree decl, tree exp)
{
  /* othercum describes the call expand_call just set up with the function
     type of the call expression.  Compare with that type, not with the
     decl's: after a prototype/definition spelling mismatch the decl's
     composited type is a different node and the sibcall was refused.  */
  tree fntype = TREE_TYPE (TREE_TYPE (CALL_EXPR_FN (exp)));
  /* A call to the current function is set up in mycum, not othercum
     (see m68k_init_cumulative_args), so a self tail call was always
     refused and [[gnu::musttail]] on a recursive call errored out.  */
  struct m68k_args *cum = decl == current_function_decl ? &mycum : &othercum;
  if (cum->fntype == fntype)
    {
      long allowed = 0x010103;
      if (!decl || m68k_symbolic_jump == NULL)
	allowed &= ~(1L << STATIC_CHAIN_REGNUM);
      return (cum->regs_already_used & ~allowed) == 0;
    }
  return false;
}

/* Argument-passing support functions.  */

/* Set CUM up for the arguments of a call through FNTYPE, null for a
   libcall: how many registers they may take, and the structure-return
   register as already used.  */

static void
m68k_init_arg_regs (struct m68k_args *cum, const_tree fntype)
{
  int regparm = sas_regparm ? 2 : m68k_regparm;

  cum->num_of_regs = 0;
  cum->last_arg_reg = -1;
  cum->regs_already_used = 0;
  if (!fntype)
    return;

  cum->num_of_regs = regparm > 0 ? regparm : 0;
  tree attrs = TYPE_ATTRIBUTES (fntype);
  if (attrs)
    {
      if (lookup_attribute ("stkparm", attrs)
	  || lookup_attribute ("fn spec", attrs))
	cum->num_of_regs = 0;
      else
	{
	  /* Only regparm (N) with N > 0 overrides the -mregparm count set
	     above.  regparm (0) and attributes unrelated to argument
	     passing (saveds, nonnull, ...) keep it, so with -mregparm=0
	     they use the stack like a plain prototype.  */
	  tree ratree = lookup_attribute ("regparm", attrs);
	  if (ratree)
	    {
	      int no = TREE_INT_CST_LOW (TREE_VALUE (TREE_VALUE (ratree)));
	      if (no > 0)
		cum->num_of_regs = no < M68K_MAX_REGPARM ? no : M68K_MAX_REGPARM;
	    }
	}
    }

  /* If this is a vararg call, put all arguments on stack.  */
  if (cum->num_of_regs)
    for (const_tree param = TYPE_ARG_TYPES (fntype); param;
	 param = TREE_CHAIN (param))
      if (!TREE_CHAIN (param) && TREE_VALUE (param) != void_type_node)
	cum->num_of_regs = 0;

#if ! defined (PCC_STATIC_STRUCT_RETURN) && defined (M68K_STRUCT_VALUE_REGNUM)
  /* If return value is a structure, and we pass the buffer address in a
   register, we cannot use this register for our own purposes.
   FIXME: Something similar would be useful for static chain.  */
  if (aggregate_value_p (TREE_TYPE (fntype), fntype))
    cum->regs_already_used |= (1 << M68K_STRUCT_VALUE_REGNUM);
#endif
}

/* Initialize a variable CUM of type CUMULATIVE_ARGS
 for a call to a function whose data type is FNTYPE.
 For a library call, FNTYPE is 0.  */

void
m68k_init_cumulative_args (CUMULATIVE_ARGS *cump, tree fntype, tree decl)
{
  struct m68k_args * cum = decl == current_function_decl ? &mycum : &othercum;
  *cump = decl == current_function_decl;
  if (sas_regparm)
    m68k_regparm = 2;

  if (!fntype && decl)
    fntype = TREE_TYPE(decl);

  /* SBF: see expr.c:init_block_clear_fn
     memset uses the stack!
        DECL_EXTERNAL (fn) = 1;
        TREE_PUBLIC (fn) = 1;
  */
    if (decl && DECL_EXTERNAL(decl) && TREE_PUBLIC(decl)
        && DECL_NAME(decl) && IDENTIFIER_POINTER (DECL_NAME (decl))
        && 0 == strcmp("memset", IDENTIFIER_POINTER (DECL_NAME (decl))))
      fntype = 0;

  if (decl && fndecl_built_in_p(decl))
    fntype = NULL;

  m68k_init_arg_regs (cum, fntype);
  DPRINTF((stderr, "m68k_init_cumulative_args %s %d -> %d\r\n", decl ? lang_hooks.decl_printable_name (decl, 2) : "?", *cump, cum->num_of_regs));

  if (fntype && fntype->base.code == FUNCTION_DECL && DECL_STATIC_CHAIN(fntype))
    {
      rtx reg = m68k_static_chain_rtx (decl, 0);
      if (reg)
	cum->regs_already_used |= (1 << REGNO(reg));
    }

  if (fntype)
    cum->current_param_type = TYPE_ARG_TYPES(cum->fntype = fntype);
  else
    /* Call to compiler-support function. */
    cum->current_param_type = cum->fntype = 0;

  /* When compiling the callee, the register-parameter bindings survive on the
     PARM_DECL types even when composite_type stripped them from fntype's arg
     types (prototype/definition spelling mismatch).  Walk the PARM_DECLs in
     parallel and prefer them for the asmreg lookup.  */
  cum->current_parm_decl = (decl && decl == current_function_decl
			    && TREE_CODE (decl) == FUNCTION_DECL)
			   ? DECL_ARGUMENTS (decl) : 0;
  DPRINTF((stderr, "9m68k_init_cumulative_args %p -> %d\r\n", cum, cum->num_of_regs));
}

int
m68k_function_arg_reg (unsigned regno)
{
  return (mycum.regs_already_used & (1 << regno)) != 0;
}

rtx
m68k_function_value(const_tree type, const_tree fn_decl_or_type, bool outgoing)
{
  machine_mode mode = TYPE_MODE(type);
  if (!fn_decl_or_type)
    fn_decl_or_type = outgoing ? mycum.fntype : othercum.fntype;
  if (fn_decl_or_type && TARGET_68881 && (mode == DFmode || mode == SFmode))
    return gen_rtx_REG (mode, FP0_REG);
  return gen_rtx_REG (mode, D0_REG);
}

bool
m68k_function_value_regno_p(unsigned regno) {
  if (TARGET_68881 && mycum.fntype)
	return regno == FP0_REG;
  return regno == D0_REG;
}


/* Update the data in CUM to advance over an argument.  */

void m68k_function_arg_advance (cumulative_args_t cum_v,
				       const function_arg_info & )
{
  struct m68k_args *cum = *get_cumulative_args (cum_v) ? &mycum : &othercum;
  /* Update the data in CUM to advance over an argument.  */

  DPRINTF((stderr, "m68k_function_arg_advance1 %p\r\n", cum));

  if (cum->last_arg_reg != -1)
    {
      int count;
      for (count = 0; count < cum->last_arg_len; count++)
	cum->regs_already_used |= (1 << (cum->last_arg_reg + count));
      cum->last_arg_reg = -1;
    }

  if (cum->current_param_type)
    cum->current_param_type = TREE_CHAIN(cum->current_param_type);
  if (cum->current_parm_decl)
    cum->current_parm_decl = DECL_CHAIN(cum->current_parm_decl);
}

/* Define where to put the arguments to a function.
 Value is zero to push the argument on the stack,
 or a hard register in which to store the argument.

 MODE is the argument's machine mode.
 TYPE is the data type of the argument (as a tree).
 This is null for libcalls where that information may
 not be available.
 CUM is a variable of type CUMULATIVE_ARGS which gives info about
 the preceding args and about the function being called.  */

/* The register _m68k_function_arg picks for an argument of MODE and TYPE,
   or -1 for the stack.  Writes only CUM's last_arg_reg and last_arg_len, so
   it can run on a local m68k_args as well (m68k_place_next_arg).  */

static int
m68k_arg_regno (struct m68k_args * cum, machine_mode mode, const_tree type)
{
  if (cum->num_of_regs)
    {
      int regbegin = -1, altregbegin = -1, len;

      /* FIXME: The last condition below is a workaround for a bug.  */
      if (TARGET_68881 && FLOAT_MODE_P(mode) &&
      GET_MODE_UNIT_SIZE (mode) <= 12 && (GET_MODE_CLASS (mode) != MODE_COMPLEX_FLOAT || mode == SCmode))
	{
	  regbegin = 16; /* FPx */
	  len = GET_MODE_NUNITS(mode);
	}
      /* FIXME: Two last conditions below are workarounds for bugs.  */
      else if (INTEGRAL_MODE_P (mode) && mode != CQImode && mode != CHImode)
	{
	  if (!type || POINTER_TYPE_P(type))
	    regbegin = 8; /* Ax */
	  else
	    regbegin = 0; /* Dx */

	  if (!sas_regparm)
	    altregbegin = 8 - regbegin;
	  len = (GET_MODE_SIZE (mode) + (UNITS_PER_WORD - 1)) / UNITS_PER_WORD;
	}

      if (regbegin != -1)
	{
	  int reg;
	  long mask;

	  look_for_reg: mask = 1 << regbegin;
	  for (reg = 0; reg < cum->num_of_regs; reg++, mask <<= 1)
	    if (!(cum->regs_already_used & mask))
	      {
		int end;
		for (end = reg; end < cum->num_of_regs && end < reg + len; end++, mask <<= 1)
		  if (cum->regs_already_used & mask)
		    break;
		if (end == reg + len)
		  {
		    cum->last_arg_reg = reg + regbegin;
		    cum->last_arg_len = len;
		    break;
		  }
	      }

	  if (reg == cum->num_of_regs && altregbegin != -1)
	    {
	      DPRINTF((stderr, "look for alt reg\n"));
	      regbegin = altregbegin;
	      altregbegin = -1;
	      goto look_for_reg;
	    }
	}

      if (cum->last_arg_reg != -1)
	return cum->last_arg_reg;
    }
  return -1;
}

static struct rtx_def *
_m68k_function_arg (struct m68k_args * cum, machine_mode mode, const_tree type)
{
  DPRINTF((stderr, "m68k_function_arg numOfRegs=%d\r\n", cum ? cum->num_of_regs : 0));

  int regno = m68k_arg_regno (cum, mode, type);
  if (regno != -1)
    {
      DPRINTF((stderr, "-> gen_rtx_REG %d\r\n", regno));
      return gen_rtx_REG (mode, regno);
    }
  return 0;
}

/* A C expression that controls whether a function argument is passed
 in a register, and which register. */

rtx m68k_function_arg (cumulative_args_t cum_v, const function_arg_info & ai)
{
  DPRINTF((stderr, "m68k_function_arg %p\r\n", cum_v.p));

  struct m68k_args *cum = *get_cumulative_args (cum_v) ? &mycum : &othercum;

  tree ptype = cum->current_parm_decl ? TREE_TYPE (cum->current_parm_decl)
	       : (cum->current_param_type ? TREE_VALUE (cum->current_param_type)
					  : NULL_TREE);
  tree asmtree = ai.type && ptype ? lookup_attribute("asmreg", TYPE_ATTRIBUTES(ptype)) : NULL_TREE;

  if (asmtree)
    {
      int i;
      cum->last_arg_reg = TREE_INT_CST_LOW(TREE_VALUE(TREE_VALUE(asmtree)));
      cum->last_arg_len = ai.mode == DImode ? 2 : 1;

      for (i = 0; i < cum->last_arg_len; i++)
	{
	  if (cum->regs_already_used & (1 << (cum->last_arg_reg + i)))
	    {
	      error ("two parameters allocated for one register");
	      break;
	    }
	  cum->regs_already_used |= (1 << (cum->last_arg_reg + i));
	}
      return gen_rtx_REG (ai.mode, cum->last_arg_reg);
    }
  return _m68k_function_arg (cum, ai.mode, ai.type);
}

/* Where the next argument, of type TYPE, goes in the call CUM describes:
   its register, or -1 for the stack.  Advances CUM past it.  */

static int
m68k_place_next_arg (struct m68k_args *cum, const_tree type)
{
  tree asmtree = lookup_attribute ("asmreg", TYPE_ATTRIBUTES (type));
  int regno, len;

  if (asmtree)
    {
      regno = TREE_INT_CST_LOW (TREE_VALUE (TREE_VALUE (asmtree)));
      len = TYPE_MODE (type) == DImode ? 2 : 1;
    }
  else
    {
      regno = m68k_arg_regno (cum, TYPE_MODE (type), type);
      len = cum->last_arg_len;
    }
  if (regno != -1)
    for (int i = 0; i < len; i++)
      cum->regs_already_used |= (1 << (regno + i));
  cum->last_arg_reg = -1;
  return regno;
}

/* Whether calls through FNTYPE1 with arguments ARGS1 and through FNTYPE2
   with ARGS2 pass each argument in the same place.  Per-declaration
   choices (builtins, memset) and the static chain are not seen here.  */

bool
m68k_fntypes_place_args_alike (const_tree fntype1, const_tree args1,
			       const_tree fntype2, const_tree args2)
{
  struct m68k_args cum1, cum2;

  m68k_init_arg_regs (&cum1, fntype1);
  m68k_init_arg_regs (&cum2, fntype2);
  for (; args1 && args2
	 && TREE_VALUE (args1) != void_type_node
	 && TREE_VALUE (args2) != void_type_node;
       args1 = TREE_CHAIN (args1), args2 = TREE_CHAIN (args2))
    if (m68k_place_next_arg (&cum1, TREE_VALUE (args1))
	!= m68k_place_next_arg (&cum2, TREE_VALUE (args2)))
      return false;
  return true;
}

void
m68k_emit_regparm_clobbers (void)
{
  for (int i = 0; i < FIRST_PSEUDO_REGISTER; ++i)
    if (mycum.regs_already_used & (1 << i))
      {
	rtx reg = gen_raw_REG (Pmode, i);
	emit_insn (gen_rtx_CLOBBER(Pmode, gen_rtx_SET(reg, gen_rtx_MEM(Pmode, reg))));
      }
}

/* Return zero if the attributes on TYPE1 and TYPE2 are incompatible,
 one if they are compatible, and two if they are nearly compatible
 (which causes a warning to be generated). */

/* Not registered as TARGET_COMP_TYPE_ATTRIBUTES, and not to be: it returns
   0, which C makes an incompatible-pointer-types error.  AmigaOS uses
   amigaos_callconv_comp_type_attributes.  */
int
m68k_comp_type_attributes (const_tree type1, const_tree type2)
{
  DPRINTF((stderr, "m68k_comp_type_attributes\n"));
  /* Functions or methods are incompatible if they specify mutually exclusive
   ways of passing arguments. */
  if (TREE_CODE(type1) == FUNCTION_TYPE || TREE_CODE(type1) == METHOD_TYPE)
    {
      tree attrs1 = TYPE_ATTRIBUTES(type1);

      tree asm1 = lookup_attribute("asmregs", attrs1);
      tree stack1 = lookup_attribute("stkparm", attrs1);
      tree reg1 = lookup_attribute("regparm", attrs1);

      tree attrs2 = TYPE_ATTRIBUTES(type2);

      tree asm2 = lookup_attribute("asmregs", attrs2);
      tree stack2 = lookup_attribute("stkparm", attrs2);
      tree reg2 = lookup_attribute("regparm", attrs2);

      if ((asm1 && !asm2) || (!asm1 && asm2))
	return 0;

      if (reg1)
	{
	  if (stack2)
	    return 0;

	  int no1 = TREE_INT_CST_LOW(TREE_VALUE(TREE_VALUE(reg1)));
	  int no2 = reg2 ? TREE_INT_CST_LOW(TREE_VALUE(TREE_VALUE(reg2))) : m68k_regparm;
	  if (no1 != no2)
	    return 0;
	}
      else if (reg2)
	{
	  if (stack1)
	    return 0;

	  int no2 = TREE_INT_CST_LOW(TREE_VALUE(TREE_VALUE(reg2)));
	  if (m68k_regparm != no2)
	    return 0;
	}

      if (stack1) {
	  if (stack2)
	    return 1;
	  return m68k_regparm  <= 0;
      }

      if (stack2)
	  return m68k_regparm  <= 0;

      if (asm1)
	return 0 == strcmp(IDENTIFIER_POINTER(TREE_VALUE(asm1)), IDENTIFIER_POINTER(TREE_VALUE(asm2)));

    }
  return 1;
}
/* end-GG-local */

/* Handle a regparm, stkparm, saveds attribute;
 arguments as in struct attribute_spec.handler.  */
tree
m68k_handle_type_attribute (tree *node, tree name, tree args, int flags ATTRIBUTE_UNUSED, bool *no_add_attrs)
{
  tree nnn = *node;
  do
    { // while (0);
      DPRINTF((stderr, "%p with treecode %d\n", node, TREE_CODE(nnn)));
      if (TREE_CODE (nnn) == FUNCTION_DECL || TREE_CODE (nnn) == FUNCTION_TYPE || TREE_CODE (nnn) == METHOD_TYPE)
	{
	  /* 'regparm' accepts one optional argument - number of registers in
	   single class that should be used to pass arguments.  */
	  if (is_attribute_p ("regparm", name))
	    {
	      DPRINTF((stderr, "regparm found\n"));

	      if (lookup_attribute ("stkparm", TYPE_ATTRIBUTES(nnn)))
		{
		  error ("%'regparm%' and %'stkparm%' aka %'__stdargs%' are mutually exclusive");
		  break;
		}
	      if (args && TREE_CODE (args) == TREE_LIST)
		{
		  tree val = TREE_VALUE(args);
		  DPRINTF((stderr, "regparm with val: %d\n", TREE_CODE(val)));
		  if (TREE_CODE (val) == INTEGER_CST)
		    {
		      unsigned no = TREE_INT_CST_LOW(val);
		      if (no > M68K_MAX_REGPARM)
			{
			  error ("%'regparm%' attribute: value %d not in [0 - %d]", no,
			  M68K_MAX_REGPARM);
			  break;
			}
		    }
		  else
		    {
		      error ("invalid argument(s) to %'regparm%' attribute");
		      break;
		    }
		}
	    }
	  else if (is_attribute_p ("stkparm", name))
	    {
	      if (lookup_attribute ("regparm", TYPE_ATTRIBUTES(nnn)))
		{
		  error ("%'regparm%' and %'stkparm%' aka %'__stdargs%' are mutually exclusive");
		  break;
		}
	    }
	  else
	    {
	      warning (OPT_Wattributes, "%'%s%' attribute only applies to data", IDENTIFIER_POINTER(name));
	    }
	}
      else
	{
	  if (is_attribute_p ("asmreg", name))
	    {
	      if (args && TREE_CODE (args) == TREE_LIST)
		{
		  tree val = TREE_VALUE(args);
		  if (TREE_CODE (val) == INTEGER_CST)
		    {
		      unsigned no = TREE_INT_CST_LOW(val);
		      if (no >= 23)
			{
			  error ("%'asmreg%' attribute: value %d not in [0 - 23]", no);
			  break;
			}
		    }
		  else
		    {
		      error ("invalid argument(s) to %'asmreg%' attribute");
		      break;
		    }
		}
	    }
	  else
	    {
	      warning (OPT_Wattributes, "%'%s%' attribute only applies to functions", IDENTIFIER_POINTER(name));
	    }
	}
      return NULL_TREE ;
    }
  while (0);
  // error case
  *no_add_attrs = true;
  return NULL_TREE ;
}

rtx
m68k_static_chain_rtx (const_tree decl, bool incoming ATTRIBUTE_UNUSED)
{
  if (!decl || !DECL_STATIC_CHAIN(decl))
    return 0;

  unsigned used = 0;
  tree fntype = TREE_TYPE(decl);
  if (fntype)
    for (tree current_param_type = TYPE_ARG_TYPES(fntype); current_param_type; current_param_type = TREE_CHAIN(current_param_type))
      {
	tree asmtree = TYPE_ATTRIBUTES(TREE_VALUE(current_param_type));
	if (!asmtree || strcmp ("asmreg", IDENTIFIER_POINTER(TREE_PURPOSE(asmtree))))
	  continue;

	unsigned regno = TREE_INT_CST_LOW(TREE_VALUE(TREE_VALUE(asmtree)));
	used |= 1 << regno;
      }

  if (!(used & (1 << 9)))
    return gen_rtx_REG (Pmode, 9);
  if (!(used & (1 << 10)))
    return gen_rtx_REG (Pmode, 10);
  if (!(used & (1 << 11)))
    return gen_rtx_REG (Pmode, 11);
  if (!(used & (1 << 14)))
    return gen_rtx_REG (Pmode, 14);

  return 0;
}

static unsigned HOST_WIDE_INT
get_env_uint (char const * name, unsigned HOST_WIDE_INT dflt)
{
#if M68K_SWITCHES_MODE
  unsigned HOST_WIDE_INT r;
  const char *env = getenv (name);
  if (env && *env)
    r = strtoul (env, NULL, 10);
  else
    r = dflt;   /* default */
//fprintf(stderr, "%s = %d\n", name, r);
  return r;
#else
  return dflt;
#endif
}

/* Implement TARGET_USE_MOVE_BY_PIECES_INFRASTRUCTURE_P.
 */
bool
m68k_use_by_pieces_infrastructure_p (unsigned HOST_WIDE_INT size,
                                     unsigned int align ATTRIBUTE_UNUSED,
                                     enum by_pieces_operation op ATTRIBUTE_UNUSED,
                                     bool speed_p ATTRIBUTE_UNUSED)
{
  /* ALIGN is in bits.  Word alignment is as good as long alignment for
     the pieces, and nothing beyond a long helps: a 16384-byte aligned
     struct must not turn a 48 KB memset into thousands of stores
     (c-c++-common/torture/builtin-clear-padding-2.c at -Os).  */
  if (align >= 16) align = 32;
  unsigned HOST_WIDE_INT max = get_env_uint ("M68K_BY_PIECES_MAX", 48);
  return size * 32 <= max * align;
}

int
m68k_emit_setmemsi(rtx blkdest, rtx val, rtx length, rtx alignment)
{
  int align = INTVAL(alignment);
  int size = INTVAL(length);
  int n = optimize_size ? 4 : 16;
  rtx regdst = XEXP(blkdest, 0);
  rtx src, dst;
  int rest = 0;

  int value = INTVAL(val) & 0xff;
  /* Below the 68020 an odd word or long access faults, so byte-aligned
     blocks go byte by byte there.  This depends on the CPU selected, not
     on -mtune: -m68000 -mtune=68020-60 still runs on a 68000.  */
  const bool bytewise = align == 1 && !TARGET_68020;

  if (value != 0)
    {
      if (bytewise)
        {
	  src = gen_reg_rtx(QImode);
	  emit_move_insn (src, GEN_INT((signed char )value));
        }
      else
	{
	  src = gen_reg_rtx(SImode);
	  HOST_WIDE_INT v = (unsigned char)value;
	  v |= v << 8;
	  v |= v << 16;

	  emit_move_insn(src, gen_int_mode(v, SImode));
	}
    }
  else
    src = val;

  /* SBF: allocate tmp reg.
   * auto-inc-dec may benefit - maybe not.
   */
  dst = gen_reg_rtx(SImode);
  rtx_insn * dinsn = emit_move_insn(dst, regdst);
  add_reg_note (dinsn, REG_INC, dst);

  regdst = dst;

  /* move bytes. */
  if (bytewise)
    {
      dst = gen_rtx_MEM(QImode, gen_rtx_POST_INC(SImode, regdst));
    }
  else
    {
      rest = size % 4;
      size /= 4;
      dst = gen_rtx_MEM(SImode, gen_rtx_POST_INC(SImode, regdst));
    }

  int nloops = size / n - 1;

  /* Above this size, the generic implementation using MOVEM is faster.
     nloops is -1 below one unrolled iteration; keep the comparison signed.  */
  if (nloops > (HOST_WIDE_INT) get_env_uint ("M68K_SETMEMSI_MAX_NLOOPS", 63))
    return false;

  int single = size % n;

  if (nloops == 0)
    single += n;
  else if (nloops > 0)
    {
      rtx counter = gen_reg_rtx(HImode);
      rtx looplabel = gen_label_rtx();

      emit_move_insn(counter, GEN_INT(nloops));
      emit_label(looplabel);

      int count = n;
      while (count-- > 0)
        {
          rtx_insn *insn = emit_move_insn(dst, src);
          add_reg_note(insn, REG_INC, regdst);
        }

      emit_jump_insn(gen_dbne_hi(counter, looplabel));
    }

  while (single-- > 0)
    {
      rtx_insn *insn = emit_move_insn (dst, src);
      add_reg_note (insn, REG_INC, regdst);
    }

  // move trailing data
  if (rest & 2)
    {
      dst = gen_rtx_MEM (HImode, gen_rtx_POST_INC(SImode, regdst));
      rtx_insn *insn = emit_move_insn (dst,
				       GEN_INT(value + (signed char )value * 0x100));
      add_reg_note (insn, REG_INC, regdst);
    }
  if (rest & 1)
    {
      dst = gen_rtx_MEM (QImode, gen_rtx_POST_INC(SImode, regdst));
      rtx_insn *insn = emit_move_insn (dst, GEN_INT((signed char )value));
      add_reg_note (insn, REG_INC, regdst);
    }

  return true;
}

/* MAY_OVERLAP is set by movmemsi (memmove): the blocks may overlap, so
   the copy direction must be provable from the operands, otherwise the
   libcall is the only safe choice.  cpymemsi passes false.  */
int
m68k_emit_movmemsi(rtx blkdest, rtx blksrc, rtx length, rtx alignment,
		   bool may_overlap)
{
  int align = INTVAL(alignment);
  int size = INTVAL(length);
  int n = optimize_size ? 4 : 16;

  rtx regsrc = XEXP(blksrc, 0);
  rtx regdst = XEXP(blkdest, 0);
  rtx src, dst;
  int rest = 0;

  /* Default direction is forward (POST_INC) starting at the beginning. */
  bool backward = false;
  bool direction_known = false;

  /* 1. Overlap & Direction Check using get_inner_reference.  Only a
     common base with constant offsets on both sides proves anything. */
  tree dest_expr = MEM_EXPR (blkdest);
  tree src_expr = MEM_EXPR (blksrc);

  if (dest_expr && src_expr)
    {
      poly_int64 d_bitsize, d_bitpos, s_bitsize, s_bitpos;
      tree d_offset_tree, s_offset_tree;
      machine_mode d_mode, s_mode;
      int d_unsignedp, d_reversep, d_volatilep;
      int s_unsignedp, s_reversep, s_volatilep;

      tree dest_base = get_inner_reference (dest_expr, &d_bitsize, &d_bitpos, &d_offset_tree,
					    &d_mode, &d_unsignedp, &d_reversep, &d_volatilep);
      tree src_base = get_inner_reference (src_expr, &s_bitsize, &s_bitpos, &s_offset_tree,
					   &s_mode, &s_unsignedp, &s_reversep, &s_volatilep);

      /* get_memory_rtx keeps the base object but clears MEM_OFFSET when
	 it strips an address expression, so an unknown offset proves
	 nothing.  */
      if (dest_base && src_base && dest_base == src_base
	  && (!d_offset_tree || TREE_CODE (d_offset_tree) == INTEGER_CST)
	  && (!s_offset_tree || TREE_CODE (s_offset_tree) == INTEGER_CST)
	  && d_bitpos.is_constant () && s_bitpos.is_constant ()
	  && MEM_OFFSET_KNOWN_P (blkdest) && MEM_OFFSET_KNOWN_P (blksrc))
	{
	  HOST_WIDE_INT d_off = d_bitpos.to_constant() / BITS_PER_UNIT;
	  HOST_WIDE_INT s_off = s_bitpos.to_constant() / BITS_PER_UNIT;

	  if (d_offset_tree)
	    d_off += TREE_INT_CST_LOW (d_offset_tree);
	  if (s_offset_tree)
	    s_off += TREE_INT_CST_LOW (s_offset_tree);

	  d_off += MEM_OFFSET (blkdest).to_constant();
	  s_off += MEM_OFFSET (blksrc).to_constant();

	  direction_known = true;
	  /* If destination is ahead of source and they overlap, switch to backward. */
	  if (d_off > s_off && d_off < s_off + size)
	    backward = true;
	}
    }

  if (may_overlap && !direction_known)
    return false;

  /* See m68k_emit_setmemsi: byte by byte below the 68020 when the
     block is byte aligned.  A backward copy of odd length starts its
     wide accesses at an odd address, which faults there too.  */
  const bool bytewise = align == 1 && !TARGET_68020;
  if (backward && (size & 1) && !bytewise && !TARGET_68020)
    return false;

  /* 2. Adjust starting pointers if copying backward.
     Two additional instructions are emitted here to calculate the end of the blocks. */
  if (backward)
    {
      regsrc = plus_constant(Pmode, regsrc, size);
      regdst = plus_constant(Pmode, regdst, size);
    }

  /* Temporary registers for auto-increment/decrement tracking. */
  src = gen_reg_rtx(SImode);
  rtx_insn *sinsn = emit_move_insn(src, regsrc);
  add_reg_note(sinsn, REG_INC, src);
  regsrc = src;

  dst = gen_reg_rtx(SImode);
  rtx_insn *dinsn = emit_move_insn(dst, regdst);
  add_reg_note(dinsn, REG_INC, dst);
  regdst = dst;

  /* 3. Dynamically generate either POST_INC or PRE_DEC MEM expressions. */
  if (bytewise)
    {
      src = gen_rtx_MEM(QImode, backward ? gen_rtx_PRE_DEC(SImode, regsrc)
                                         : gen_rtx_POST_INC(SImode, regsrc));
      dst = gen_rtx_MEM(QImode, backward ? gen_rtx_PRE_DEC(SImode, regdst)
                                         : gen_rtx_POST_INC(SImode, regdst));
    }
  else
    {
      rest = size % 4;
      size /= 4;

      src = gen_rtx_MEM(SImode, backward ? gen_rtx_PRE_DEC(SImode, regsrc)
                                         : gen_rtx_POST_INC(SImode, regsrc));
      dst = gen_rtx_MEM(SImode, backward ? gen_rtx_PRE_DEC(SImode, regdst)
                                         : gen_rtx_POST_INC(SImode, regdst));
    }

  /* 4. The entire loop unrolling infrastructure remains unchanged! */
  int nloops = size / n - 1;

  /* nloops is -1 below one unrolled iteration; keep the comparison signed.  */
  if (nloops > (HOST_WIDE_INT) get_env_uint ("M68K_MOVMEMSI_MAX_NLOOPS", 63))
    return false;

  int single = size % n;

  if (nloops == 0)
    single += n;
  else if (nloops > 0)
    {
      rtx counter = gen_reg_rtx(HImode);
      rtx looplabel = gen_label_rtx();

      emit_move_insn(counter, GEN_INT(nloops));
      emit_label(looplabel);

      int count = n;
      while (count-- > 0)
        {
          rtx_insn *insn = emit_move_insn(dst, src);
          add_reg_note(insn, REG_INC, regsrc);
          add_reg_note(insn, REG_INC, regdst);
        }

      emit_jump_insn(gen_dbne_hi(counter, looplabel));
    }

  while (single-- > 0)
    {
      rtx_insn *insn = emit_move_insn(dst, src);
      add_reg_note(insn, REG_INC, regsrc);
      add_reg_note(insn, REG_INC, regdst);
    }

  /* Move trailing data (dynamically uses updated MEM expressions from above). */
  if (rest & 2)
    {
      src = gen_rtx_MEM(HImode, backward ? gen_rtx_PRE_DEC(SImode, regsrc)
                                         : gen_rtx_POST_INC(SImode, regsrc));
      dst = gen_rtx_MEM(HImode, backward ? gen_rtx_PRE_DEC(SImode, regdst)
                                         : gen_rtx_POST_INC(SImode, regdst));

      rtx_insn *insn = emit_move_insn(dst, src);
      add_reg_note(insn, REG_INC, regsrc);
      add_reg_note(insn, REG_INC, regdst);
    }

  if (rest & 1)
    {
      src = gen_rtx_MEM(QImode, backward ? gen_rtx_PRE_DEC(SImode, regsrc)
                                         : gen_rtx_POST_INC(SImode, regsrc));
      dst = gen_rtx_MEM(QImode, backward ? gen_rtx_PRE_DEC(SImode, regdst)
                                         : gen_rtx_POST_INC(SImode, regdst));

      rtx_insn *insn = emit_move_insn(dst, src);
      add_reg_note(insn, REG_INC, regsrc);
      add_reg_note(insn, REG_INC, regdst);
    }

  return true;
}
