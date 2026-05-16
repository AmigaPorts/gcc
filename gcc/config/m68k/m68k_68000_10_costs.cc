#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "cfghooks.h"
#include "tree.h"
#include "rtl.h"

#define USE_MOVQ(i)	((unsigned) ((i) + 128) <= 255)

bool
m68k_68000_10_costs (rtx x,
                        machine_mode mode,
                        int outer_code ATTRIBUTE_UNUSED,
                        int opno ATTRIBUTE_UNUSED,
                        int *total,
                        bool speed)
{
  int code = GET_CODE (x);
  int total2;

  if (!speed)
    {
      *total = COSTS_N_INSNS (1);
      return true;
    }

  switch (code)
    {
    /* -------------------------------------------------- */
    /* Free / nearly free RTL nodes                       */
    /* -------------------------------------------------- */

    case REG:
    case SUBREG:
    case STRICT_LOW_PART:
    case TRUNCATE:
    case POST_INC:
      *total = 0;
      return true;

    case ZERO_EXTEND:
    case SIGN_EXTEND:
        *total = 4;
        return true;

    case PRE_DEC:
      *total = 1;
      return true;

    /* -------------------------------------------------- */
    /* Constants                                          */
    /* -------------------------------------------------- */

    case CONST_INT:
      {
        HOST_WIDE_INT v = INTVAL (x);

        if (USE_MOVQ (v))
          *total = 0;
        else if (v >= -32768 && v <= 32767)
          *total = 2;
        else
          *total = 4;

        return true;
      }

    case CONST_DOUBLE:
        *total = 16;
      return true;

    case SYMBOL_REF:
    case LABEL_REF:
      *total = 6;
      return true;

    case CONST:
      *total = 4;
      return true;

    /* -------------------------------------------------- */
    /* Memory -- THIS is the important balancing point    */
    /* -------------------------------------------------- */

    case MEM:
      {
        rtx addr = XEXP (x, 0);

        switch (GET_CODE (addr))
          {
          case REG:
          case POST_INC:
            *total = 4;
            break;

          case PRE_DEC:
            *total = 5;
            break;

          case PLUS:
            {
              rtx a = XEXP (addr, 0);
              rtx b = XEXP (addr, 1);

              /* reg + disp */
              if ((REG_P (a) && CONST_INT_P (b))
                  || (REG_P (b) && CONST_INT_P (a)))
                *total = 8;

              /* reg + reg */
              else if (REG_P (a) && REG_P (b))
                *total = 12;

              else
                *total = 14;

              break;
            }

          case SYMBOL_REF:
          case LABEL_REF:
          case CONST:
            *total = 10;
            break;

          default:
            *total = 16; //8;
            break;
          }

        /* important for sieve / crc */
        if (GET_MODE_SIZE (mode) > 2)
          *total += 2;

        return true;
      }

    /* -------------------------------------------------- */
    /* SET                                                */
    /* -------------------------------------------------- */

    case SET:
      {
        rtx dest = XEXP (x, 0);
        rtx src  = XEXP (x, 1);

        if (!m68k_68000_10_costs (dest, mode,
                                     code, 0,
                                     total, speed))
          return false;

        if (!m68k_68000_10_costs (src, mode,
                                     code, 1,
                                     &total2, speed))
          return false;

        *total += total2;

        /* strongly encourage clr/moveq */
        if (CONST_INT_P (src) && INTVAL (src) == 0)
          {
            if (REG_P (dest))
              *total = 2;
            else if (MEM_P (dest))
              *total = GET_MODE_SIZE (mode) > 2 ? 8 : 6;
          }

        return true;
      }

    /* -------------------------------------------------- */
    /* Arithmetic                                         */
    /* -------------------------------------------------- */

    case PLUS:
    case MINUS:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* keep combine alive, but not TOO alive */
        if (REG_P (a) && REG_P (b))
          {
            *total = 4; // GET_MODE_SIZE (mode) > 2 ? 6 : 4;
            return true;
          }

        /* ADDQ/SUBQ */
        if (REG_P (a) && CONST_INT_P (b))
          {
            HOST_WIDE_INT v = INTVAL (b);

            if (v >= -8 && v <= 8)
              {
                *total = 4;
                return true;
              }

            if (REGNO (a) == SP_REG)
			  {
				*total = 8;
				return true;
			  }

            *total = GET_MODE_SIZE (mode) > 2 ? 12 : 8;
            return true;
          }

        /* critical: NOT too cheap */
        *total = GET_CODE (a) == PLUS ? 12 : 8;
        return true;
      }

    /* -------------------------------------------------- */
    /* Logic                                              */
    /* -------------------------------------------------- */

    case AND:
    case IOR:
    case XOR:
    case COMPARE:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* SHA/CRC friendliness */
        if (REG_P (a) && REG_P (b))
          {
            if (code == XOR)
              *total = 5; // 3
            else
              *total = 6; // 4

            return true;
          }

        if ((REG_P (a) && CONST_INT_P (b))
            || (REG_P (b) && CONST_INT_P (a)))
          {
            *total = GET_MODE_SIZE (mode) > 2 ? 10 : 6; // 12 : 8
            return true;
          }

        if (MEM_P (a) || MEM_P (b))
          {
            *total = 12;
            return true;
          }

        *total = 8;
        return true;
      }

    /* -------------------------------------------------- */
    /* Shifts / rotates                                   */
    /* -------------------------------------------------- */

    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* IMPORTANT:
           Cheap enough for SHA1,
           expensive enough to avoid spill hell. */

        if (REG_P (a) && CONST_INT_P (b))
          {
            int n = INTVAL (b);

//            *total = (GET_MODE_SIZE (mode) > 2 ? 6 : 4)
//                     + (n >> 1);
                    *total = (GET_MODE_SIZE (mode) > 2 ? 8 : 6)
            + (n << 1);

            return true;
          }

        if (REG_P (a))
          {
            *total = GET_MODE_SIZE (mode) > 2 ? 12 : 8;
            return true;
          }

        *total = 10;
        return true;
      }

    case ROTATE:
    case ROTATERT:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* targeted SHA1 optimization */

        if (REG_P (a) && CONST_INT_P (b))
          {
            *total = GET_MODE_SIZE (mode) > 2 ? 5 : 4;
            return true;
          }

        *total = 10;
        return true;
      }

    /* -------------------------------------------------- */
    /* Unary                                              */
    /* -------------------------------------------------- */

    case NEG:
    case NOT:
      *total = GET_MODE_SIZE (mode) > 2 ? 6 : 4;
      return true;

    /* -------------------------------------------------- */
    /* Multiply / divide                                  */
    /* -------------------------------------------------- */
    case MULT:
      {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* power-of-two => shifts */

        if (CONST_INT_P (b))
          {
            HOST_WIDE_INT n = INTVAL (b);
            HOST_WIDE_INT p = 1;
            int shift = 0;

            if (n < 0)
              n = -n;

            while (p < n)
              {
                p <<= 1;
                shift++;
              }

            if (p == n)
              {
                *total = 8 + (shift * 2);
                return true;
              }
          }

        if (speed)
          {
            int f;

            /* hybridV3 baseline behalten */
            f = GET_MODE_SIZE (mode) > 2 ? 180 : 48;

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

                        /* leichte Verbesserung */
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

                        /* signed synthese etwas teurer */
                        f = 28 + transitions * 7;
                      }

                    /* große Modi bleiben teuer */
                    if (GET_MODE_SIZE (mode) > 2)
                      f = (f * 3) / 2;

                    /* power-of-two massiv bevorzugen */
                    if ((i & (i - 1)) == 0)
                      f >>= 1;
                  }
              }

            *total += f;
          }
        else
          {
            *total += GET_MODE_SIZE (mode) > 2 ? 48 : 16;
          }

        return true;
      }
    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
      *total = GET_MODE_SIZE (mode) > 2 ? 260 : 120;
      return true;

    /* -------------------------------------------------- */
    /* Branches                                           */
    /* -------------------------------------------------- */

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
      *total = 6;
      return true;

    case IF_THEN_ELSE:
      *total = 6;
      return true;

    /* -------------------------------------------------- */
    /* Calls                                              */
    /* -------------------------------------------------- */

    case CALL:
      {
        rtx a = XEXP (x, 0);

        if (MEM_P (a))
          {
            rtx b = XEXP (a, 0);

            if (REG_P (b))
              *total = 16;
            else if (GET_CODE (b) == PLUS)
              *total = 18;
            else
              *total = 20;
          }
        else
          *total = 18;

        return true;
      }

    default:
      *total = 4;
      return true;
    }
}
