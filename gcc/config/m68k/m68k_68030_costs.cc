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
   68030 KOSTEN-MAKROS (Speed / Size)
   ============================================ */
#define COST_REG                    (speed ? 1  : 5)
#define COST_MEM_PLUS_REG_DISP      (speed ? 0  : 1)
#define COST_MEM_PLUS_REG_REG       (speed ? 4  : 0)
#define COST_MEM_OTHER              (speed ? 9  : 1)
#define COST_CONST_INT_Q            (speed ? 0  : 20)
#define COST_CONST_INT_W            (speed ? 0  : 1)
#define COST_CONST_INT_L            (speed ? 5  : 0)
#define COST_CONST_DOUBLE           (speed ? 4  : 9)
#define COST_SYMBOL                 (speed ? 0  : 12)
#define COST_PLUS_REG_REG           (speed ? 3  : 5)
#define COST_PLUS_REG_CONSTQ        (speed ? 0  : 20)
#define COST_PLUS_REG_CONSTW        (speed ? 3  : 0)
#define COST_PLUS_REG_CONSTL        (speed ? 5  : 1)
#define COST_PLUS_OTHER             (speed ? 20 : 12)
#define COST_LOGIC_REG_CONST        (speed ? 11 : 8)
#define COST_SHIFT_CONST            (speed ? 6  : 2)
#define COST_SHIFT_CONST_W          (speed ? 8  : 0)
#define COST_BRANCH                 (speed ? 11 : 0)
#define COST_SET_REG_REG            (speed ? 5  : 1)
#define COST_CALL_OTHER             (speed ? 9  : 11)

/* ============================================
   68030 MULT-KOSTEN (basierend auf Messungen)
   ============================================ */
#define COST_MULU_L             44      /* mulu.l Dn, Dm (immer 44-46 Zyklen) */
#define COST_MULU_W             27      /* mulu.w Dn, Dm */
#define COST_ADD_L              2       /* add.l Dn, Dm */
#define COST_SUB_L              2       /* sub.l Dn, Dm */
#define COST_MOVE_L             2       /* move.l Dn, Dm */
#define COST_MOVEQ              2       /* moveq #0, Dn */
#define COST_LSL_SHIFT(shift)   (4 + (shift) * 2)  /* lsl.l #shift, Dn */

/* ============================================
   m68k_68030_costs_intern
   ============================================ */
static bool
m68k_68030_costs_intern (rtx x,
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
        HOST_WIDE_INT v = INTVAL(x);

        if (USE_MOVQ(v))
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
        rtx inner = XEXP(x, 0);
        if (GET_CODE(inner) == PLUS
            && SYMBOL_REF_P(XEXP(inner, 0))
            && CONST_INT_P(XEXP(inner, 1)))
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
        rtx addr = XEXP(x, 0);

        switch (GET_CODE(addr))
        {
        case REG:
        case POST_INC:
        case PRE_DEC:
            *total = 0;
            break;

        case PLUS:
        {
            rtx a = XEXP(addr, 0);
            rtx b = XEXP(addr, 1);

            if ((REG_P(a) && CONST_INT_P(b)))
            {
                HOST_WIDE_INT off = INTVAL(b);
                if (off >= -32768 && off <= 32767)
                    *total = COST_MEM_PLUS_REG_DISP;
                else
                    *total = COST_MEM_PLUS_REG_REG;
            }
            else if (REG_P(a) && REG_P(b))
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

        if (GET_MODE_SIZE(mode) > 2)
            *total += 2;

        return true;
    }

    case PLUS:
    case MINUS:
    {
        rtx a = XEXP(x, 0);
        rtx b = XEXP(x, 1);

        if (REG_P(a) && REG_P(b))
        {
            *total = COST_PLUS_REG_REG;
            return true;
        }

        if (REG_P(a) && CONST_INT_P(b))
        {
            HOST_WIDE_INT v = INTVAL(b);

            if (v >= -8 && v <= 8)
            {
                *total = COST_PLUS_REG_CONSTQ;
                return true;
            }

            if (REGNO(a) == SP_REG)
            {
                *total = 8;
                return true;
            }

            *total = GET_MODE_SIZE(mode) > 2
                     ? COST_PLUS_REG_CONSTL
                     : COST_PLUS_REG_CONSTW;
            return true;
        }

        *total = (GET_CODE(a) == PLUS
                  ? COST_PLUS_OTHER
                  : COST_PLUS_OTHER / 2);
        return true;
    }

    case AND:
    case IOR:
    case XOR:
    case COMPARE:
    {
        rtx a = XEXP(x, 0);
        rtx b = XEXP(x, 1);

        if (REG_P(a) && REG_P(b))
        {
            *total = (code == XOR) ? 0 : 1;
            return true;
        }

        if ((REG_P(a) && CONST_INT_P(b))
            || (REG_P(b) && CONST_INT_P(a)))
        {
            *total = GET_MODE_SIZE(mode) > 2
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
        rtx a = XEXP(x, 0);
        rtx b = XEXP(x, 1);

        if (REG_P(a) && CONST_INT_P(b))
        {
            *total = (GET_MODE_SIZE(mode) > 2
                      ? COST_SHIFT_CONST
                      : COST_SHIFT_CONST_W);
            return true;
        }

        *total = 0;
        return true;
    }

    case MULT:
      {
        rtx dst = XEXP (x, 0);
        rtx src = XEXP (x, 1);

        if (CONST_INT_P (src))
          {
            HOST_WIDE_INT n = INTVAL (src);

            /* 1. Special case: multiplication by 0 or 1. */
            if (n == 0 || n == 1)
              {
                *total = COST_MOVEQ;
                return true;
              }

            unsigned HOST_WIDE_INT un;

            if (n < 0)
              un = -(unsigned HOST_WIDE_INT) n;
            else
              un = (unsigned HOST_WIDE_INT) n;

            /* 2. Special case: power of 2 -> lsl.l #shift, Dn.
               Negative constants are excluded because they also need neg.l. */
            if (n > 0)
              {
                int shift = exact_log2 (un);

                if (shift >= 0)
                  {
                    *total = COST_LSL_SHIFT (shift);
                    return true;
                  }
              }

            /* 3. Special case: (2^shift + 1) -> lsl.l + add.l */
            if (n > 0 && un > 1 && (un & (un - 1)) == 1)
              {
                int shift = exact_log2 (un - 1);

                if (shift >= 0)
                  {
                    *total = COST_LSL_SHIFT (shift) + COST_ADD_L;
                    return true;
                  }
              }

            /* 4. Special case: (2^shift - 1) -> lsl.l + sub.l */
            if (n > 0 && un > 2 && (un & (un + 1)) == 0)
              {
                int shift = exact_log2 (un + 1);

                if (shift >= 0)
                  {
                    *total = COST_LSL_SHIFT (shift) + COST_SUB_L;
                    return true;
                  }
              }

            /* 5. General case with bit population count. */
            {
              int bits = 0;
              int l = 0;
              HOST_WIDE_INT nn = n;

              if (nn > 0)
                {
                  if (GET_CODE (dst) == ZERO_EXTEND || REG_P (dst))
                    {
                      while (nn)
                        {
                          if (nn & 1)
                            ++bits;
                          nn >>= 1;
                        }

                      if (bits == 1 && REG_P (dst))
                        {
                          *total = COST_MOVEQ;
                          return true;
                        }
                    }
                  else
                    {
                      while (nn || l)
                        {
                          if ((nn & 1) != l)
                            {
                              l = !l;
                              ++bits;
                            }

                          nn >>= 1;
                        }
                    }

                  *total = 12 + bits;
                  return true;
                }
            }
          }

        /* 6. Fallback: mulu.l / muls.l */
        *total = GET_MODE_SIZE (mode) > 2
                 ? COST_MULU_L
                 : COST_MULU_W;
        return true;
      }
    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
        *total = GET_MODE_SIZE(mode) > 2 ? 260 : 0;
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
        rtx dest = XEXP(x, 0);
        rtx src = XEXP(x, 1);

        if (REG_P(dest) && REG_P(src))
        {
            *total = COST_SET_REG_REG;
            return true;
        }

        if (!m68k_68030_costs_intern(dest, mode, code, 0, total, speed))
            return false;
        if (!m68k_68030_costs_intern(src, mode, code, 1, &total2, speed))
            return false;
        *total += total2;

        /* CLR - überschreibt Summe */
        if (CONST_INT_P(src) && INTVAL(src) == 0)
        {
            if (REG_P(dest))
                *total = 0;
            else if (MEM_P(dest))
                *total = 0;  /* COST_SET_CLR_MEM_L = 0 */
        }

        return true;
    }

    case CALL:
    {
        rtx mem = XEXP(x, 0);
        rtx b = XEXP(mem, 0);

        if (REG_P(b) || GET_CODE(b) == PLUS)
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
m68k_68030_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    return m68k_68030_costs_intern(x, mode, outer_code, opno, total, speed);
}
