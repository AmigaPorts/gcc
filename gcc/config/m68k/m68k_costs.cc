#define IN_TARGET_CODE 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "cfghooks.h"
#include "tree.h"
#include "rtl.h"
#include <stdio.h>

#define USE_MOVQ(i)	((unsigned) ((i) + 128) <= 255)

/* ============================================
   COST ENUM
   ============================================ */
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
    cost_mult_l,
    cost_mult_w,
    cost_div_l,
    cost_div_w,
    cost_default_fallback,
    cost_sp_add,
    cost_mem_size_extra,
    cost_lsl_base,
    cost_lsl_per_shift,
    cost_mult_68000_word_factor,
    cost_max
};

/* ============================================
   ZENTRALE KONSTANTEN (CPU-unabhängig)
   ============================================ */

/* MULT: 68000 Bit-Popcount Kosten (bleiben als Makros, da sie nur für 68000 gelten) */
#define COST_MULT_68000_BASE_L     180     /* Basis für Long-MULT (Fallback) */
#define COST_MULT_68000_BASE_W     0       /* Basis für Word-MULT */
#define COST_MULT_68000_ZERO_EXT   24      /* Zero-Extend Basis */
#define COST_MULT_68000_ZERO_BIT   6       /* pro gesetztem Bit */
#define COST_MULT_68000_SIGN_EXT   28      /* Sign-Extend Basis */
#define COST_MULT_68000_TRANS      7       /* pro Transition */
#define COST_MULT_68000_POWER2     2       /* Potenz-von-2 Faktor */

/* DIV: Software-Fallback (nur 68000) */
#define COST_DIV_68000_L           260     /* Long-DIV Software-Fallback */
#define COST_DIV_68000_W           0       /* Word-DIV (nicht verwendet) */

/* ============================================
   EINE SIZE-TABELLE FÜR ALLE CPUS
   ============================================ */
static int m68k_costs_size[cost_max] = {
    [cost_reg] = 5,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 1,
    [cost_mem_plus_reg_reg] = 0,
    [cost_mem_other] = 1,
    [cost_const_int_q] = 20,
    [cost_const_int_w] = 1,
    [cost_const_int_l] = 0,
    [cost_const_double] = 9,
    [cost_symbol] = 12,
    [cost_plus_reg_reg] = 5,
    [cost_plus_reg_constq] = 20,
    [cost_plus_reg_constw] = 0,
    [cost_plus_reg_constl] = 1,
    [cost_plus_other] = 12,
    [cost_logic_reg_const] = 8,
    [cost_shift_const] = 2,
    [cost_shift_const_w] = 0,
    [cost_neg_not_l] = 4,
    [cost_neg_not_w] = 2,
    [cost_branch] = 0,
    [cost_set_reg_reg] = 1,
    [cost_set_clr_mem_l] = 0,
    [cost_call_reg] = 0,
    [cost_call_other] = 11,
    [cost_mult_l] = 44,
    [cost_mult_w] = 28,
    [cost_div_l] = 90,
    [cost_div_w] = 56,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* ============================================
   CPU-SPEZIFISCHE SPEED-TABELLEN
   ============================================ */

/* 68000 Speed (Score: 0.96813) */
static int m68k_costs_speed_000[cost_max] = {
    [cost_reg] = 4,
    [cost_mem_reg] = 1,
    [cost_mem_plus_reg_disp] = 2,
    [cost_mem_plus_reg_reg] = 0,
    [cost_mem_other] = 0,
    [cost_const_int_q] = 0,
    [cost_const_int_w] = 0,
    [cost_const_int_l] = 5,
    [cost_const_double] = 0,
    [cost_symbol] = 7,
    [cost_plus_reg_reg] = 3,
    [cost_plus_reg_constq] = 1,
    [cost_plus_reg_constw] = 0,
    [cost_plus_reg_constl] = 0,
    [cost_plus_other] = 9,
    [cost_logic_reg_const] = 7,
    [cost_shift_const] = 7,
    [cost_shift_const_w] = 0,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 0,
    [cost_set_reg_reg] = 0,
    [cost_set_clr_mem_l] = 3,
    [cost_call_reg] = 2,
    [cost_call_other] = 5,
    [cost_mult_l] = 44,
    [cost_mult_w] = 28,
    [cost_div_l] = 260,
    [cost_div_w] = 0,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* 68020 Speed (Score: 0.95894) */
static int m68k_costs_speed_020[cost_max] = {
    [cost_reg] = 3,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 1,
    [cost_mem_plus_reg_reg] = 8,
    [cost_mem_other] = 6,
    [cost_const_int_q] = 0,
    [cost_const_int_w] = 8,
    [cost_const_int_l] = 5,
    [cost_const_double] = 5,
    [cost_symbol] = 5,
    [cost_plus_reg_reg] = 3,
    [cost_plus_reg_constq] = 4,
    [cost_plus_reg_constw] = 3,
    [cost_plus_reg_constl] = 0,
    [cost_plus_other] = 11,
    [cost_logic_reg_const] = 12,
    [cost_shift_const] = 0,
    [cost_shift_const_w] = 0,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 0,
    [cost_set_reg_reg] = 0,
    [cost_set_clr_mem_l] = 7,
    [cost_call_reg] = 2,
    [cost_call_other] = 7,
    [cost_mult_l] = 44,
    [cost_mult_w] = 28,
    [cost_div_l] = 90,
    [cost_div_w] = 56,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* 68030 Speed (Score: 0.94219) */
static int m68k_costs_speed_030[cost_max] = {
    [cost_reg] = 1,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 0,
    [cost_mem_plus_reg_reg] = 4,
    [cost_mem_other] = 9,
    [cost_const_int_q] = 0,
    [cost_const_int_w] = 0,
    [cost_const_int_l] = 5,
    [cost_const_double] = 4,
    [cost_symbol] = 0,
    [cost_plus_reg_reg] = 3,
    [cost_plus_reg_constq] = 0,
    [cost_plus_reg_constw] = 3,
    [cost_plus_reg_constl] = 5,
    [cost_plus_other] = 20,
    [cost_logic_reg_const] = 11,
    [cost_shift_const] = 6,
    [cost_shift_const_w] = 8,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 11,
    [cost_set_reg_reg] = 5,
    [cost_set_clr_mem_l] = 0,
    [cost_call_reg] = 0,
    [cost_call_other] = 9,
    [cost_mult_l] = 44,
    [cost_mult_w] = 28,
    [cost_div_l] = 90,
    [cost_div_w] = 56,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* 68040 Speed (kopiert von 68030) */
static int m68k_costs_speed_040[cost_max] = {
    [cost_reg] = 1,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 0,
    [cost_mem_plus_reg_reg] = 4,
    [cost_mem_other] = 9,
    [cost_const_int_q] = 0,
    [cost_const_int_w] = 0,
    [cost_const_int_l] = 5,
    [cost_const_double] = 4,
    [cost_symbol] = 0,
    [cost_plus_reg_reg] = 3,
    [cost_plus_reg_constq] = 0,
    [cost_plus_reg_constw] = 3,
    [cost_plus_reg_constl] = 5,
    [cost_plus_other] = 20,
    [cost_logic_reg_const] = 11,
    [cost_shift_const] = 6,
    [cost_shift_const_w] = 8,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 11,
    [cost_set_reg_reg] = 5,
    [cost_set_clr_mem_l] = 0,
    [cost_call_reg] = 0,
    [cost_call_other] = 9,
    [cost_mult_l] = 44,
    [cost_mult_w] = 28,
    [cost_div_l] = 90,
    [cost_div_w] = 56,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* 68060 Speed (Score: 0.94109) */
static int m68k_costs_speed_060[cost_max] = {
    [cost_reg] = 0,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 0,
    [cost_mem_plus_reg_reg] = 0,
    [cost_mem_other] = 2,
    [cost_const_int_q] = 6,
    [cost_const_int_w] = 13,
    [cost_const_int_l] = 0,
    [cost_const_double] = 0,
    [cost_symbol] = 1,
    [cost_plus_reg_reg] = 7,
    [cost_plus_reg_constq] = 0,
    [cost_plus_reg_constw] = 0,
    [cost_plus_reg_constl] = 1,
    [cost_plus_other] = 12,
    [cost_logic_reg_const] = 0,
    [cost_shift_const] = 12,
    [cost_shift_const_w] = 0,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 6,
    [cost_set_reg_reg] = 5,
    [cost_set_clr_mem_l] = 2,
    [cost_call_reg] = 0,
    [cost_call_other] = 0,
    [cost_mult_l] = 4,
    [cost_mult_w] = 3,
    [cost_div_l] = 6,
    [cost_div_w] = 4,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* Apollo 68080 Speed (Score: 0.94109) */
static int m68k_costs_speed_080[cost_max] = {
    [cost_reg] = 0,
    [cost_mem_reg] = 0,
    [cost_mem_plus_reg_disp] = 0,
    [cost_mem_plus_reg_reg] = 0,
    [cost_mem_other] = 2,
    [cost_const_int_q] = 6,
    [cost_const_int_w] = 13,
    [cost_const_int_l] = 0,
    [cost_const_double] = 0,
    [cost_symbol] = 1,
    [cost_plus_reg_reg] = 7,
    [cost_plus_reg_constq] = 0,
    [cost_plus_reg_constw] = 0,
    [cost_plus_reg_constl] = 1,
    [cost_plus_other] = 12,
    [cost_logic_reg_const] = 0,
    [cost_shift_const] = 12,
    [cost_shift_const_w] = 0,
    [cost_neg_not_l] = 0,
    [cost_neg_not_w] = 0,
    [cost_branch] = 6,
    [cost_set_reg_reg] = 5,
    [cost_set_clr_mem_l] = 2,
    [cost_call_reg] = 0,
    [cost_call_other] = 0,
    [cost_mult_l] = 4,
    [cost_mult_w] = 3,
    [cost_div_l] = 6,
    [cost_div_w] = 4,
    [cost_default_fallback] = 4,
    [cost_sp_add] = 8,
    [cost_mem_size_extra] = 2,
    [cost_lsl_base] = 8,
    [cost_lsl_per_shift] = 2,
    [cost_mult_68000_word_factor] = 3,
};

/* ============================================
   COST MAP FÜR LADEFUNKTION
   ============================================ */
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
    { "cost_neg_not_l", cost_neg_not_l },
    { "cost_neg_not_w", cost_neg_not_w },
    { "cost_branch", cost_branch },
    { "cost_set_reg_reg", cost_set_reg_reg },
    { "cost_set_clr_mem_l", cost_set_clr_mem_l },
    { "cost_call_reg", cost_call_reg },
    { "cost_call_other", cost_call_other },
    { "cost_mult_l", cost_mult_l },
    { "cost_mult_w", cost_mult_w },
    { "cost_div_l", cost_div_l },
    { "cost_div_w", cost_div_w },
    { "cost_default_fallback", cost_default_fallback },
    { "cost_sp_add", cost_sp_add },
    { "cost_mem_size_extra", cost_mem_size_extra },
    { "cost_lsl_base", cost_lsl_base },
    { "cost_lsl_per_shift", cost_lsl_per_shift },
    { "cost_mult_68000_word_factor", cost_mult_68000_word_factor },
};

/* ============================================
   LADEFUNKTION FÜR TABELLEN-OVERRIDES
   ============================================ */
static void load_cost_overrides(const char *filename, int table[])
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return;

    char line[256];
    int line_num = 0;

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        char *p = line;

        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0')
            continue;

        char name[128];
        int value;

        if (sscanf(p, " %127[^=] = %d", name, &value) != 2) {
            fprintf(stderr, "Warning: m68k-costs: invalid line %d: '%s'\n",
                    line_num, line);
            continue;
        }

        char *end = name + strlen(name) - 1;
        while (end > name && (*end == ' ' || *end == '\t')) {
            *end = '\0';
            end--;
        }

        int found = 0;
        for (size_t i = 0; i < sizeof(cost_map)/sizeof(cost_map[0]); i++) {
            if (strcmp(cost_map[i].name, name) == 0) {
                table[cost_map[i].index] = value;
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Warning: m68k-costs: unknown cost '%s' in line %d\n",
                    name, line_num);
        }
    }

    fclose(f);
}

/* ============================================
   MULT-OPTIMIERUNG FÜR 68000 (NUR BIT-POPCOUNT)
   ============================================ */
static int
m68k_68000_mult_cost (rtx a, HOST_WIDE_INT n, machine_mode mode,
                      const int *thecosts)
{
    int bits = 0;
    int transitions = 0;
    int last = 0;
    HOST_WIDE_INT t = n;
    int f = GET_MODE_SIZE (mode) > 2
            ? COST_MULT_68000_BASE_L
            : COST_MULT_68000_BASE_W;

    if (GET_CODE (a) == ZERO_EXTEND)
    {
        while (t)
        {
            bits += (t & 1);
            t >>= 1;
        }
        f = COST_MULT_68000_ZERO_EXT + bits * COST_MULT_68000_ZERO_BIT;
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
        f = COST_MULT_68000_SIGN_EXT + transitions * COST_MULT_68000_TRANS;
    }

    /* Word-Mode Faktor (aus Tabelle) */
    if (GET_MODE_SIZE (mode) > 2)
        f = (f * thecosts[cost_mult_68000_word_factor]) / 2;

    return f;
}

/* ============================================
   ZENTRALE COST-FUNKTION
   ============================================ */
static bool
m68k_common_costs (rtx x,
                   machine_mode mode,
                   int outer_code ATTRIBUTE_UNUSED,
                   int opno ATTRIBUTE_UNUSED,
                   int *total,
                   const int *thecosts)
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
        default:
            *total = thecosts[cost_mem_other];
            break;
        }

        if (GET_MODE_SIZE (mode) > 2)
            *total += thecosts[cost_mem_size_extra];

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
                *total = thecosts[cost_sp_add];
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
            *total = (code == XOR) ? 0 : 1;
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
            *total = (GET_MODE_SIZE (mode) > 2
                      ? thecosts[cost_shift_const]
                      : thecosts[cost_shift_const_w]);
            return true;
        }

        *total = 0;
        return true;
    }

    case MULT:
    {
        rtx a = XEXP (x, 0);
        rtx b = XEXP (x, 1);

        /* 68000: Bit-Popcount Optimierung für Konstanten */
        if (CONST_INT_P (b) && thecosts == m68k_costs_speed_000)
        {
            HOST_WIDE_INT n = INTVAL (b);
            if (n > 0)
            {
                *total = m68k_68000_mult_cost (a, n, mode, thecosts);
                return true;
            }
        }

        /* Alle anderen CPUs: Kosten aus Tabelle */
        *total = GET_MODE_SIZE (mode) > 2
                 ? thecosts[cost_mult_l]
                 : thecosts[cost_mult_w];
        return true;
    }

    case DIV:
    case UDIV:
    case MOD:
    case UMOD:
        *total = GET_MODE_SIZE (mode) > 2
                 ? thecosts[cost_div_l]
                 : thecosts[cost_div_w];
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

        if (REG_P (dest) && REG_P (src))
        {
            *total = thecosts[cost_set_reg_reg];
            return true;
        }

        if (!m68k_common_costs (dest, mode, code, 0, total, thecosts))
            return false;
        if (!m68k_common_costs (src, mode, code, 1, &total2, thecosts))
            return false;
        *total += total2;

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
        rtx mem = XEXP (x, 0);
        rtx b = XEXP (mem, 0);

        if (REG_P (b) || GET_CODE (b) == PLUS)
            *total = 0;
        else
            *total = thecosts[cost_call_other];

        return true;
    }

    default:
        *total = thecosts[cost_default_fallback];
        return true;
    }
}

/* ============================================
   INIT-FUNKTIONEN FÜR JEDE CPU
   ============================================ */

static void m68k_init_costs_000(void)
{
    load_cost_overrides("m68k-costs-000.txt", m68k_costs_speed_000);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

static void m68k_init_costs_020(void)
{
    load_cost_overrides("m68k-costs-020.txt", m68k_costs_speed_020);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

static void m68k_init_costs_030(void)
{
    load_cost_overrides("m68k-costs-030.txt", m68k_costs_speed_030);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

static void m68k_init_costs_040(void)
{
    load_cost_overrides("m68k-costs-040.txt", m68k_costs_speed_040);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

static void m68k_init_costs_060(void)
{
    load_cost_overrides("m68k-costs-060.txt", m68k_costs_speed_060);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

static void m68k_init_costs_080(void)
{
    load_cost_overrides("m68k-costs-080.txt", m68k_costs_speed_080);
    load_cost_overrides("m68k-costs-size.txt", m68k_costs_size);
}

/* ============================================
   EXPORTIERTE FUNKTIONEN
   ============================================ */

bool
m68k_68000_10_costs (rtx x,
                     machine_mode mode,
                     int outer_code ATTRIBUTE_UNUSED,
                     int opno ATTRIBUTE_UNUSED,
                     int *total,
                     bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_000();
    }

    const int *thecosts = speed ? m68k_costs_speed_000 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}

bool
m68k_68020_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_020();
    }

    const int *thecosts = speed ? m68k_costs_speed_020 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}

bool
m68k_68030_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_030();
    }

    const int *thecosts = speed ? m68k_costs_speed_030 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}

bool
m68k_68040_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_040();
    }

    const int *thecosts = speed ? m68k_costs_speed_040 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}

bool
m68k_68060_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_060();
    }

    const int *thecosts = speed ? m68k_costs_speed_060 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}

bool
m68k_68080_costs (rtx x,
                  machine_mode mode,
                  int outer_code ATTRIBUTE_UNUSED,
                  int opno ATTRIBUTE_UNUSED,
                  int *total,
                  bool speed)
{
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
        m68k_init_costs_080();
    }

    const int *thecosts = speed ? m68k_costs_speed_080 : m68k_costs_size;
    return m68k_common_costs (x, mode, outer_code, opno, total, thecosts);
}
