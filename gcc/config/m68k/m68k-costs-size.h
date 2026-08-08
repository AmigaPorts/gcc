static int m68k_costs_size[cost_max] = {
	    6,   // cost_reg

	    0,   // cost_mem_reg
	    1,   // cost_mem_plus_reg_disp
	    0,   // cost_mem_plus_reg_reg
	    1,   // cost_mem_other

	    20,  // cost_const_int_q
	    1,   // cost_const_int_w
	    0,   // cost_const_int_l
	    9,   // cost_const_double
	    12,  // cost_symbol

	    5,   // cost_plus_reg_reg
	    20,  // cost_plus_reg_constq
	    0,   // cost_plus_reg_constw
	    1,   // cost_plus_reg_constl
	    12,  // cost_plus_other

	    0,   // cost_logic_reg_const

	    2,   // cost_shift_const
	    0,   // cost_shift_const_w

	    4,   // cost_neg_not_l
	    2,   // cost_neg_not_w

	    7,   // cost_branch

	    1,   // cost_set_reg_reg
	    0,   // cost_set_clr_mem_l

	    0,   // cost_call_reg
	    11   // cost_call_other
	};
