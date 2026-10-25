/* Subroutines used for code generation on Amiga OS 4
   Copyright (C) 2003 Free Software Foundation, Inc.
   Contributed by Thomas Frieden (ThomasF@hyperion-entertainment.com)

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the
   Free Software Foundation, 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "diagnostic-core.h"
#include "tm.h"
#include "memmodel.h"
#include "profile-count.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-attr.h"
#include "flags.h"
#include "recog.h"
#include "obstack.h"
#include "symtab.h"
#include "tree.h"
#include "expr.h"
#include "optabs.h"
#include "except.h"
#include "function.h"
#include "output.h"
#include "tm_p.h"
#include "fold-const.h"
#include "langhooks.h"
#include "explow.h"
#include "emit-rtl.h"
#include "stringpool.h"
#include "attribs.h"

#ifdef __amigaos4__
static const char * __attribute__((used)) amigaos_stack_cookie = "$STACK:768000";
#endif /* __amigaos4__ */

/* Define subtarget builtins */

void
amigaos_init_builtins (void)
{
	tree ftype;
  ftype = build_function_type (ptr_type_node,
			 tree_cons (NULL_TREE,
				    build_pointer_type (TREE_TYPE (va_list_type_node)),
				    void_list_node));

  add_builtin_function ("__builtin_getlinearva",
		    ftype,
		    AMIGAOS_BUILTIN_GETLINEARVA,
		    BUILT_IN_MD, NULL, NULL_TREE);
}


/* Expand builtins */
rtx
amigaos_expand_builtin (tree exp, rtx target, rtx subtarget ATTRIBUTE_UNUSED,
			machine_mode mode ATTRIBUTE_UNUSED,
			int ignore ATTRIBUTE_UNUSED, bool *success)
{
  tree fndecl = TREE_OPERAND (CALL_EXPR_FN (exp), 0);
  tree f_gpr, f_fpr, f_res, f_ovf;
  tree arg0, valist, ovf;

  unsigned int fcode = DECL_MD_FUNCTION_CODE (fndecl);

  if (fcode != AMIGAOS_BUILTIN_GETLINEARVA)
  {
	  *success = 0;
	  return NULL_RTX;
  }

  if (! (crtl->args.info.call_cookie & CALL_LINEARVARARGS))
  {
      /* TODO: This probably should be a fatal, but then the generated executable can't be linked. */
      error ("__builtin_getlinearva can only be used in a linearvarargs function");
      *success = 1;
      return 0;
  }

  arg0 = CALL_EXPR_ARG (exp, 0);
  if (arg0 == error_mark_node)
    {
      *success = 1;
      return const0_rtx;
    }

  f_gpr = TYPE_FIELDS (TREE_TYPE (va_list_type_node));
  f_fpr = TREE_CHAIN (f_gpr);
  f_res = TREE_CHAIN (f_fpr);
  f_ovf = TREE_CHAIN (f_res);
  target = const0_rtx;
  valist = build_simple_mem_ref(arg0);
  ovf = build3 (COMPONENT_REF, TREE_TYPE (f_ovf), valist, f_ovf, NULL_TREE);
  target = copy_to_reg (expand_expr (ovf, NULL_RTX, Pmode, EXPAND_NORMAL));
  *success = 1;
  return target;
}

/* Handle a "linearvarargs" attribute. */
tree
amigaos_handle_linearvarargs_attribute (tree *node, tree name,
	tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED,
	bool *no_add_attrs)
{
  if (TREE_CODE (*node) != FUNCTION_TYPE
      && TREE_CODE (*node) != METHOD_TYPE
      && TREE_CODE (*node) != FIELD_DECL
      && TREE_CODE (*node) != TYPE_DECL)
    {
      warning (0, "%s attribute only applies to functions",
	       IDENTIFIER_POINTER (name));
      *no_add_attrs = true;
    }

  return NULL_TREE;
}


/* Generate code for base relative access */

rtx
amigaos_legitimize_baserel_address (rtx addr)
{
  rtx dest = gen_reg_rtx (Pmode);

  emit_insn (gen_elf_base_high (dest, gen_rtx_REG (Pmode, 2), addr));
  emit_insn (gen_elf_base_low (dest, dest, addr));

  return dest;
}

int
amigaos_not_baserel_tree_p(tree decl)
{
  return(TREE_READONLY(decl)
         || TREE_CONSTANT(decl)
         || (DECL_ATTRIBUTES(decl)
             && lookup_attribute("force_no_baserel", DECL_ATTRIBUTES(decl))));
}

int
amigaos_baserel_operand(rtx x)
{
  tree decl;
  int ret = 0;

  if (GET_CODE(x) == SYMBOL_REF)
  {
    decl = SYMBOL_REF_DECL(x);

    if (decl)
    {
      if (SYMBOL_REF_FUNCTION_P(x) || amigaos_not_baserel_tree_p(decl))
        ret = 0;
      else
        ret = 1;
    }
  }
  else if (GET_CODE(x) == CONST
           && GET_CODE(XEXP(x, 0)) == PLUS
           && GET_CODE(XEXP(XEXP(x, 0), 0)) == SYMBOL_REF
           && GET_CODE(XEXP(XEXP(x, 0), 1)) == CONST_INT)
    ret = amigaos_baserel_operand(XEXP(XEXP(x, 0), 0));
  else if (GET_CODE(x) == LO_SUM)
    ret = amigaos_baserel_operand(XEXP(x, 1));

  return ret;
}

void amigaos_function_end_prologue(FILE *file);
void amigaos_function_end_prologue(FILE *file)
{
  if (TARGET_BASEREL
      && current_function_decl
      && lookup_attribute ("baserel_restore", TYPE_ATTRIBUTES (TREE_TYPE (current_function_decl))))
    {
       fprintf( file, "\tbl __baserel_get_addr\n");
    }
}


tree
amigaos_handle_baserel_restore_attribute (tree *node, tree name,
	tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED,
	bool *no_add_attrs)
{
  if (TREE_CODE (*node) != FUNCTION_TYPE
      && TREE_CODE (*node) != METHOD_TYPE
      && TREE_CODE (*node) != FIELD_DECL
      && TREE_CODE (*node) != TYPE_DECL)
    {
      warning (0, "%s attribute only applies to functions",
	       IDENTIFIER_POINTER (name));
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

tree
amigaos_handle_force_no_baserel_attribute (tree *node, tree name,
	tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED,
	bool *no_add_attrs)
{
  if (TREE_CODE (*node) != VAR_DECL)
    {
      warning (0, "%s attribute only applies to variables",
	       IDENTIFIER_POINTER (name));
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

/* Handle a "check68kfuncptr" attribute. */

tree
amigaos_handle_check68kfuncptr_attribute (tree *node, tree name,
	tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED,
	bool *no_add_attrs)
{
  if (TREE_CODE (*node) != FUNCTION_TYPE
      && TREE_CODE (*node) != METHOD_TYPE
      && TREE_CODE (*node) != FIELD_DECL
      && TREE_CODE (*node) != TYPE_DECL)
    {
      warning (0, "%s attribute only applies to functions",
	       IDENTIFIER_POINTER (name));
      *no_add_attrs = true;
    }

  return NULL_TREE;
}

/* Disable section anchors in baserel mode to avoid r2-relative
   addressing collisions with the base register.  */
bool
amigaos_use_anchors_for_symbol_p (const_rtx x ATTRIBUTE_UNUSED)
{
  if (TARGET_BASEREL)
    return false;
  return default_use_anchors_for_symbol_p (x);
}

/* Handle a checktags attribute. This enables tag verifications */
tree
amigaos_handle_checktags_attribute (tree *node, tree name, tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED, bool *no_add_attrs)
{
  if (TREE_CODE (*node) != FUNCTION_TYPE)
    {
      warning (0, "%s attribute only applies to functions",
	       IDENTIFIER_POINTER (name));
      *no_add_attrs = true;
    }
  tree fn = *node;
  tree arglist = TYPE_ARG_TYPES(fn);
  int arg = 0;
  bool is_varargs = true;

  while (arglist != NULL_TREE)
    {
      /* The void_type marks the end of a non-varargs function */
      if (TREE_VALUE(arglist) == void_type_node)
        is_varargs = false;

      arglist = TREE_CHAIN (arglist);
      arg++;
    }

  if (!is_varargs && arg == 2)
    {
      warning (0, "%s attribute only applies to varargs functions or functions with at least two arguments",
	       IDENTIFIER_POINTER (name));
    }
  return NULL_TREE;
}
