#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "cfghooks.h"
#include "tree.h"
#include "rtl.h"

#define USE_MOVQ(i)	((unsigned) ((i) + 128) <= 255)

/* ============================================
   68000 KOSTEN (Speed / Size) - OPTIMIERT
   ============================================ */
#define COST_REG                    (speed ? 3  : 5)
#define COST_MEM_REG                (speed ? 1  : 0)
#define COST_MEM_PLUS_REG_DISP      (speed ? 1  : 1)
#define COST_MEM_PLUS_REG_REG       (speed ? 0  : 0)
#define COST_MEM_OTHER              (speed ? 0  : 1)
#define COST_CONST_INT_Q            (speed ? 0  : 20)
#define COST_CONST_INT_W            (speed ? 0  : 1)
#define COST_CONST_INT_L            (speed ? 5  : 0)
#define COST_CONST_DOUBLE           (speed ? 0  : 9)
#define COST_SYMBOL                 (speed ? 7  : 12)
#define COST_PLUS_REG_REG           (speed ? 3  : 5)
#define COST_PLUS_REG_CONSTQ        (speed ? 1  : 20)
#define COST_PLUS_REG_CONSTW        (speed ? 0  : 0)
#define COST_PLUS_REG_CONSTL        (speed ? 0  : 1)
#define COST_PLUS_OTHER             (speed ? 6  : 12)
#define COST_LOGIC_REG_CONST        (speed ? 7  : 8)
#define COST_SHIFT_CONST            (speed ? 9  : 2)
#define COST_SHIFT_CONST_W          (speed ? 0  : 0)
#define COST_NEG_NOT_L              (speed ? 0  : 4)
#define COST_NEG_NOT_W              (speed ? 0  : 2)
#define COST_BRANCH                 (speed ? 0  : 0)
#define COST_SET_REG_REG            (speed ? 0  : 1)
#define COST_SET_CLR_MEM_L          (speed ? 0  : 0)
#define COST_CALL_REG               (speed ? 0  : 0)
#define COST_CALL_OTHER             (speed ? 5  : 11)

/* ============================================
   68000 MULT KOSTEN (aus alter Implementierung)
   ============================================ */
#define COST_MULU_L_SPEED(shift)    (8 + (shift) * 2)  /* lsl.l + add/sub */
#define COST_MULU_L_FALLBACK        180                 /* mulu.l Fallback */
#define COST_MULU_W_FALLBACK        0                   /* mulu.w Fallback (nicht verwendet) */

static bool
m68k_68000_10_costs_intern (rtx x,
                     machine_mode mode,
                     int outer_code ATTRIBUTE_UNUSED,
                     int opno ATTRIBUTE_UNUSED,
                     int *total,
                     bool speed)
{
  int code = GET_CODE (x);
  int total2;

  switch (code)
    {
  /* Alle Fälle, die 0 zurückgeben */
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
  case NEG:
  case NOT:
      *total = 0;
    return true;

    case CONST_INT:
      {
        HOST_WIDE_INT v = INTVAL (x);

        if (USE_MOVQ (v))
          *total = COST_CONST_INT_Q;
        else if (v >= -32768 && v <= 32767)
          *total = COST_CONST_INT_W;
        else
          *total = COST_CONST_INT_L;

        return true;
      }

    case CONST_DOUBLE:
      *total = COST_CONST_DOUBLE;
      return true;

    case SYMBOL_REF:
      *total = COST_SYMBOL;
      return true;

    case CONST:
      {
        rtx inner = XEXP (x, 0);
        if (GET_CODE (inner) == PLUS
            && SYMBOL_REF_P (XEXP (inner, 0))
            && CONST_INT_P (XEXP (inner, 1)))
          *total = COST_SYMBOL;
        else
          *total = 0;
        return true;
      }

    case REG:
      *total = COST_REG;
      return true;

    case MEM:
      {
        rtx addr = XEXP (x, 0);

        switch (GET_CODE (addr))
          {
          case REG:
          case POST_INC:
          case PRE_DEC:
            *total = 0;
            break;

          case PLUS:
            {
              rtx a = XEXP (addr, 0);
              rtx b = XEXP (addr, 1);

              if ((REG_P (a) && CONST_INT_P (b)))
                {
                  HOST_WIDE_INT off = INTVAL (b);
                  if (off >= -32768 && off <= 32767)
                    *total = COST_MEM_PLUS_REG_DISP;
                  else
                    *total = COST_MEM_PLUS_REG_REG;
                }
              else if (REG_P (a) && REG_P (b))
                *total = COST_MEM_PLUS_REG_REG;
              else
                *total = COST_MEM_OTHER;

              break;
            }

          case SYMBOL_REF:
          case LABEL_REF:
          case CONST:
          default:
            *total = COST_MEM_OTHER;
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
            *total = COST_PLUS_REG_REG;
            return true;
          }

        if (REG_P (a) && CONST_INT_P (b))
          {
            HOST_WIDE_INT v = INTVAL (b);

            if (v >= -8 && v <= 8)
              {
                *total = COST_PLUS_REG_CONSTQ;
                return true;
              }

            if (REGNO (a) == SP_REG)
              {
                *total = 8;
                return true;
              }

            *total = GET_MODE_SIZE (mode) > 2
                     ? COST_PLUS_REG_CONSTL
                     : COST_PLUS_REG_CONSTW;
            return true;
          }

        *total = (GET_CODE (a) == PLUS
                  ? COST_PLUS_OTHER
                  : COST_PLUS_OTHER / 2);
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
            *total = (code == XOR) ? 0 : 1;
            return true;
          }

        if ((REG_P (a) && CONST_INT_P (b))
            || (REG_P (b) && CONST_INT_P (a)))
          {
            *total = GET_MODE_SIZE (mode) > 2
                     ? COST_LOGIC_REG_CONST + 2
                     : COST_LOGIC_REG_CONST;
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
            *total = (GET_MODE_SIZE (mode) > 2
                      ? COST_SHIFT_CONST
                      : COST_SHIFT_CONST_W);
            return true;
          }

        *total = 0;
        return true;
      }

	case MULT:
	  {
		rtx a = XEXP (x, 0);
		rtx b = XEXP (x, 1);

		/* Speed optimization: constant multiplication. */
		if (CONST_INT_P (b) && speed)
		  {
			HOST_WIDE_INT n = INTVAL (b);

			if (n > 0)
			  {
				int shift = exact_log2 (n);

				if (shift >= 0)
				  {
					*total = COST_MULU_L_SPEED (shift);
					return true;
				  }
			  }

			/* General case using bit population count / transitions. */
			{
			  int f = GET_MODE_SIZE (mode) > 2 ? 180 : 0;
			  HOST_WIDE_INT i = n;

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
						  bits += t & 1;
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
				}

			  *total = f;
			  return true;
			}
		  }

		/* Fallback: mulu.l (not speed or no constant). */
		*total = GET_MODE_SIZE (mode) > 2
				 ? COST_MULU_L_FALLBACK
				 : COST_MULU_W_FALLBACK;
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
      *total = COST_BRANCH;
      return true;

    case SET:
          {
            rtx dest = XEXP (x, 0);
            rtx src  = XEXP (x, 1);

            if (REG_P (dest) && REG_P (src))
              {
                *total = COST_SET_REG_REG;
                return true;
              }

            if (!m68k_68000_10_costs_intern (dest, mode, code, 0, total, speed))
              return false;
            if (!m68k_68000_10_costs_intern (src, mode, code, 1, &total2, speed))
              return false;
            *total += total2;

            if (CONST_INT_P (src) && INTVAL (src) == 0)
              {
                if (REG_P (dest))
                  *total = 0;
                else if (MEM_P (dest))
                  *total = GET_MODE_SIZE (mode) > 2
                           ? COST_SET_CLR_MEM_L
                           : 0;
              }

            return true;
          }

    case CALL:
      {
        rtx mem = XEXP (x, 0);
        rtx b = XEXP (mem, 0);

        if (REG_P (b) || GET_CODE (b) == PLUS)
          *total = 0;
        else
          *total = COST_CALL_OTHER;

        return true;
      }

    default:
      *total = 4;
      return true;
    }
}

/* ============================================
   EXPORTIERTE FUNKTION
   ============================================ */
bool
m68k_68000_10_costs (rtx x,
                     machine_mode mode,
                     int outer_code ATTRIBUTE_UNUSED,
                     int opno ATTRIBUTE_UNUSED,
                     int *total,
                     bool speed)
{
  return m68k_68000_10_costs_intern (x, mode, outer_code, opno, total, speed);
}
