static int m68k_costs_speed[cost_max] = {
	    7,   // cost_reg

	    1,   // cost_mem_reg
	    2,   // cost_mem_plus_reg_disp
	    3,   // cost_mem_plus_reg_reg
	    0,   // cost_mem_other

	    9,   // cost_const_int_q
	    1,   // cost_const_int_w
	    5,   // cost_const_int_l
	    0,   // cost_const_double
	    7,   // cost_symbol

	    3,   // cost_plus_reg_reg
	    1,   // cost_plus_reg_constq
	    0,   // cost_plus_reg_constw
	    0,   // cost_plus_reg_constl
	    6,   // cost_plus_other

	    12,  // cost_logic_reg_const

	    10,  // cost_shift_const
	    8,   // cost_shift_const_w

	    3,   // cost_neg_not_l
	    1,   // cost_neg_not_w

	    0,   // cost_branch

	    1,   // cost_set_reg_reg
	    12,  // cost_set_clr_mem_l

	    15,  // cost_call_reg
	    16   // cost_call_other
	};

