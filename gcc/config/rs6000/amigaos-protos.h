/* Prototypes.
   Copyright (C) 2003 Free Software Foundation, Inc.

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

extern void amigaos_init_builtins (void);
extern rtx amigaos_expand_builtin (tree, rtx, rtx, machine_mode, int, bool *);
extern tree amigaos_handle_linearvarargs_attribute (tree *, tree, tree, int, bool *);
extern tree amigaos_handle_checktags_attribute (tree *, tree, tree, int, bool *);
extern tree amigaos_handle_baserel_restore_attribute (tree *, tree, tree, int, bool *);
extern tree amigaos_handle_force_no_baserel_attribute (tree *, tree, tree, int, bool *);
extern tree amigaos_handle_check68kfuncptr_attribute (tree *, tree, tree, int, bool *);
extern rtx amigaos_legitimize_baserel_address (rtx addr);
extern int amigaos_baserel_operand (rtx x);
extern int amigaos_not_baserel_tree_p (tree decl);
extern bool amigaos_use_anchors_for_symbol_p (const_rtx x);
extern void amigaos_function_end_prologue (FILE *file);
