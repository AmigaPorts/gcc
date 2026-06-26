#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "cfghooks.h"
#include "tree.h"
#include "rtl.h"

#define USE_MOVQ(i)	((unsigned) ((i) + 128) <= 255)

enum m68k_cost_kind {
    cost_reg,

    cost_mem_reg,
    cost_mem_plus_reg_disp,
    cost_mem_plus_reg_reg,
    cost_mem_other,

    cost_const_int_q,
    cost_const_int_w,
    cost_const_int_l,
    cost_const_double,
    cost_symbol,

    cost_plus_reg_reg,
    cost_plus_reg_constq,
    cost_plus_reg_constw,
    cost_plus_reg_constl,
    cost_plus_other,

    cost_logic_reg_const,

    cost_shift_const,
    cost_shift_const_w,

    cost_neg_not_l,
    cost_neg_not_w,

    cost_branch,

    cost_set_reg_reg,
    cost_set_clr_mem_l,

    cost_call_reg,
    cost_call_other,

    cost_max
};

#include "m68k-costs-speed.h"
#include "m68k-costs-size.h"

static bool
m68k_68000_10_costs_table (rtx x,
                     machine_mode mode,
                     int outer_code ATTRIBUTE_UNUSED,
                     int opno ATTRIBUTE_UNUSED,
                     int *total,
                     bool speed)
{
  const int *thecosts = speed ? m68k_costs_speed : m68k_costs_size;
  int code = GET_CODE (x);
  int total2;

  switch (code)
    {

  case PRE_DEC:
  case POST_INC:
  case IF_THEN_ELSE:
  case ZERO_EXTEND:
  case SIGN_EXTEND:
  case TRUNCATE:
  case ROTATE:
  case ROTATERT:
  case LABEL_REF:
  case SUBREG:
  case STRICT_LOW_PART:
	  *total = 0;
    return true;

    case CONST_INT:
      {
        HOST_WIDE_INT v = INTVAL (x);

        if (USE_MOVQ (v))
          *total = thecosts[cost_const_int_q];
        else if (v >= -32768 && v <= 32767)
          *total = thecosts[cost_const_int_w];
        else
          *total = thecosts[cost_const_int_l];

        return true;
      }

    case CONST_DOUBLE:
      *total = thecosts[cost_const_double];
      return true;

    case SYMBOL_REF:
      *total = thecosts[cost_symbol];
      return true;

    case CONST:
      {
        rtx inner = XEXP (x, 0);
        /* symbol+offset: wie SYMBOL_REF */
        if (GET_CODE (inner) == PLUS
            && SYMBOL_REF_P (XEXP (inner, 0))
            && CONST_INT_P (XEXP (inner, 1)))
          *total = thecosts[cost_symbol];
        else
          *total = 0;
        return true;
      }

    case REG:
      *total = thecosts[cost_reg];
      return true;

    case MEM:
      {
        rtx addr = XEXP (x, 0);

        switch (GET_CODE (addr))
          {
          case REG:
          case POST_INC:
            *total = thecosts[cost_mem_reg];
            break;

          case PRE_DEC:
            *total = 0; // must be free for push
            break;

          case PLUS:
            {
              rtx a = XEXP (addr, 0);
              rtx b = XEXP (addr, 1);

              if ((REG_P (a) && CONST_INT_P (b)))
                {
                  HOST_WIDE_INT off = INTVAL (b);
                  if (off >= -32768 && off <= 32767)
                    *total = thecosts[cost_mem_plus_reg_disp];
                  else
                    *total = thecosts[cost_mem_plus_reg_reg];
                }
              else if (REG_P (a) && REG_P (b))
                *total = thecosts[cost_mem_plus_reg_reg];
              else
                *total = thecosts[cost_mem_other];

              break;
            }

          case SYMBOL_REF:
          case LABEL_REF:
          case CONST:
            *total = thecosts[cost_mem_other];
            break;

          default:
            *total = thecosts[cost_mem_other];
            break;
          }

        if (GET_MODE_SIZE (mode) > 2)
          *total += 2;

        return true;
      }

    case PLUS:
    case MINUS:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        if (REG_P (a) && REG_P (b))
          {
            *total = thecosts[cost_plus_reg_reg];
            return true;
          }

        if (REG_P (a) && CONST_INT_P (b))
          {
            HOST_WIDE_INT v = INTVAL (b);

            if (v >= -8 && v <= 8)
              {
                *total = thecosts[cost_plus_reg_constq];
                return true;
              }

            if (REGNO (a) == SP_REG)
              {
                *total = 8;
                return true;
              }

            *total = GET_MODE_SIZE (mode) > 2
                     ? thecosts[cost_plus_reg_constl]
                     : thecosts[cost_plus_reg_constw];
            return true;
          }

        *total = (GET_CODE (a) == PLUS
                  ? thecosts[cost_plus_other]
                  : thecosts[cost_plus_other] / 2);
        return true;
      }
    case AND:
    case IOR:
    case XOR:
    case COMPARE:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        if (REG_P (a) && REG_P (b))
          {
            if (code == XOR)
              *total = 0;
            else
              *total = 1;

            return true;
          }

        if ((REG_P (a) && CONST_INT_P (b))
            || (REG_P (b) && CONST_INT_P (a)))
          {
            *total = GET_MODE_SIZE (mode) > 2
                     ? thecosts[cost_logic_reg_const] + 2
                     : thecosts[cost_logic_reg_const];
            return true;
          }

        *total = 0;
        return true;
      }

    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        if (REG_P (a) && CONST_INT_P (b))
          {
            *total = (GET_MODE_SIZE (mode) > 2 ? thecosts[cost_shift_const] : thecosts[cost_shift_const_w]);
            return true;
          }

        *total = 0;
        return true;
      }

    case NEG:
    case NOT:
    	*total = GET_MODE_SIZE(mode) > 2
    	         ? thecosts[cost_neg_not_l]
    	         : thecosts[cost_neg_not_w];
      return true;

    case MULT:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        if (CONST_INT_P (b))
          {
            HOST_WIDE_INT n = INTVAL (b);
	    unsigned HOST_WIDE_INT abs_n
	      = n < 0 ? -(unsigned HOST_WIDE_INT) n
		      : (unsigned HOST_WIDE_INT) n;
	    int shift = exact_log2 (abs_n);

	    if (shift >= 0)
              {
                *total = 8 + (shift * 2);
                return true;
              }
          }

        if (speed)
          {
            int f = GET_MODE_SIZE (mode) > 2 ? 180 : 0;

            if (GET_CODE (b) == CONST_INT)
              {
                HOST_WIDE_INT i = INTVAL (b);

                if (i > 0)
                  {
                    int bits = 0;
                    int transitions = 0;
                    int last = 0;
                    HOST_WIDE_INT t = i;

                    if (GET_CODE (a) == ZERO_EXTEND)
                      {
                        while (t)
                          {
                            bits += (t & 1);
                            t >>= 1;
                          }

                        f = 24 + bits * 6;
                      }
                    else
                      {
                        while (t || last)
                          {
                            int bit = t & 1;

                            if (bit != last)
                              {
                                transitions++;
                                last = bit;
                              }

                            t >>= 1;
                          }

                        f = 28 + transitions * 7;
                      }

                    if (GET_MODE_SIZE (mode) > 2)
                      f = (f * 3) / 2;

                    if ((i & (i - 1)) == 0)
                      f >>= 1;
                  }
              }

            *total += f;
          }
        else
          {
            *total += GET_MODE_SIZE (mode) > 2
                      ? 48
                      : 0;
          }

        return true;
      }

    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
      *total = GET_MODE_SIZE (mode) > 2 ? 260 : 0;
      return true;

    case EQ:
    case NE:
    case LT:
    case LE:
    case GT:
    case GE:
    case LTU:
    case LEU:
    case GTU:
    case GEU:
      *total = thecosts[cost_branch];
      return true;


    case SET:
          {
            rtx dest = XEXP (x, 0);
            rtx src  = XEXP (x, 1);

            /* reg -> reg: MOVE - 1 verhindert Dn->An Spills (nbody)
               ohne SHA1's Register-Rotationen zu bestrafen */
            if (REG_P (dest) && REG_P (src))
              {
                *total = thecosts[cost_set_reg_reg];
                return true;
              }

            if (!m68k_68000_10_costs_table (dest, mode, code, 0, total, speed))
              return false;
            if (!m68k_68000_10_costs_table (src, mode, code, 1, &total2, speed))
              return false;
            *total += total2;

            /* CLR - überschreibt Summe */
            if (CONST_INT_P (src) && INTVAL (src) == 0)
              {
                if (REG_P (dest))
                  *total = 0;
                else if (MEM_P (dest))
                  *total = GET_MODE_SIZE (mode) > 2
                           ? thecosts[cost_set_clr_mem_l]
                           : 0;
              }

            return true;
          }

    case CALL:
      {
        rtx mem = XEXP (x, 0); // always MEM
		rtx b = XEXP (mem, 0);

		if (REG_P (b))
		  *total = thecosts[cost_call_reg];
		else if (GET_CODE (b) == PLUS)
		  *total = 0;
		else
		  *total = thecosts[cost_call_other];

        return true;
      }

    default:
      *total = 4;
      return true;
    }
}

struct cost_entry {
    const char *name;
    int index;
};

static const struct cost_entry cost_map[] = {
    { "cost_reg", cost_reg },
    { "cost_mem_reg", cost_mem_reg },
    { "cost_mem_plus_reg_disp", cost_mem_plus_reg_disp },
    { "cost_mem_plus_reg_reg", cost_mem_plus_reg_reg },
    { "cost_mem_other", cost_mem_other },
    { "cost_const_int_q", cost_const_int_q },
    { "cost_const_int_w", cost_const_int_w },
    { "cost_const_int_l", cost_const_int_l },
    { "cost_const_double", cost_const_double },
    { "cost_symbol", cost_symbol },
    { "cost_plus_reg_reg", cost_plus_reg_reg },
    { "cost_plus_reg_constq", cost_plus_reg_constq },
    { "cost_plus_reg_constw", cost_plus_reg_constw },
    { "cost_plus_reg_constl", cost_plus_reg_constl },
    { "cost_plus_other", cost_plus_other },
    { "cost_logic_reg_const", cost_logic_reg_const },
    { "cost_shift_const", cost_shift_const },
    { "cost_shift_const_w", cost_shift_const_w },
    { "cost_branch", cost_branch },
    { "cost_call_reg", cost_call_reg },
    { "cost_call_other", cost_call_other },
    { "cost_set_reg_reg", cost_set_reg_reg },
    { "cost_set_clr_mem_l", cost_set_clr_mem_l },
	{ "cost_neg_not_l", cost_neg_not_l },
	{ "cost_neg_not_w", cost_neg_not_w },
};

static void load_cost_overrides(const char *filename, int table[])
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return;

    char line[256];

    while (fgets(line, sizeof(line), f)) {
        char *p = line;

        /* Kommentare überspringen */
        if (*p == '#' || *p == '\n')
            continue;

        /* name = value parsen */
        char name[128];
        int value;

        if (sscanf(p, " %127[^=] = %d", name, &value) != 2)
            continue;

        /* Whitespace trimmen */
        for (char *q = name; *q; q++)
            if (*q == ' ' || *q == '\t')
                *q = 0;

        /* Lookup */
        for (size_t i = 0; i < sizeof(cost_map)/sizeof(cost_map[0]); i++) {
            if (strcmp(cost_map[i].name, name) == 0) {
                table[cost_map[i].index] = value;
                break;
            }
        }
    }

    fclose(f);
}

static void m68k_init_costs(void)
{
    load_cost_overrides("m68k-costs-speed.txt", m68k_costs_speed);
    load_cost_overrides("m68k-costs-size.txt",  m68k_costs_size);
}

bool
m68k_68000_10_costs (rtx x,
                     machine_mode mode,
                     int outer_code ATTRIBUTE_UNUSED,
                     int opno ATTRIBUTE_UNUSED,
                     int *total,
                     bool speed)
{
	static int initialized;
	if (!initialized) {
	    initialized = 1;
	    m68k_init_costs();
	}

	return m68k_68000_10_costs_table(x, mode, outer_code, opno, total, speed);
}
