/* m68k-costs.cc - m68k RTX cost model.

   Per-family tables (speed, size) select the cost model for the active
   CPU. Five families are supported:

     68000/010                    distinct (2-cycle shifts, no cache)
     68020/020_40/030             shared (Volamos does not model 68030
                                  caches, so the 68030 grid scores
                                  identically to 68020)
     68040                        distinct (barrel shifter, caches)
     68060                        distinct (superscalar, deeper pipeline)
     68080                        distinct (Apollo); uses 68060 values
                                  as a starting point (untuned) / unused

   The values were derived by CMA-ES tuning against a benchmark suite
   executed on a cycle-accurate simulator. See
   gcc/config/m68k/m68k-costs-tuning.md for the external harness and
   reproduction instructions.

   The M68K_COSTS_FILE environment variable can override the active
   table for tuning and diagnostics. This is not a supported feature;
   the .txt format is not stable and may change between GCC releases. */

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
   Cost index enum - order must match cost_map[]
   ============================================ */
enum m68k_cost_idx {
    IDX_REG_SRC,
    IDX_REG_DST,
    IDX_MEM_PLUS_REG_DISP,
    IDX_MEM_PLUS_REG_REG,
    IDX_MEM_OTHER,
    IDX_CONST_INT_Q,
    IDX_CONST_INT_W,
    IDX_CONST_INT_L_1,
    IDX_CONST_INT_L_2,
    IDX_SYMBOL,
    IDX_PLUS_REG_REG,
    IDX_PLUS_REG_CONSTQ,
    IDX_PLUS_REG_CONSTW,
    IDX_PLUS_REG_CONSTL,
    IDX_PLUS_OTHER,
    IDX_LOGIC_REG_CONST,
    IDX_SHIFT_CONST,
    IDX_SHIFT_CONST_W,
    IDX_BRANCH,
    IDX_SET_REG_REG,
    IDX_CALL_OTHER,
    IDX_DIV_L,
    IDX_DIV_W,
    IDX_ADD_L,
    IDX_SUB_L,
    IDX_MOVEQ,
    IDX_LSL_SHIFT,
    IDX_LSL_SHIFT_STEP,

    /* SBF structural constants */
    IDX_MEM_LONG_EXTRA,
    IDX_MEM_PREDEC_EXTRA,
    IDX_CLR_LONG_PENALTY,
    IDX_CLR_WORD_PENALTY,
    IDX_PLUS_LEA_OFFSET,
    IDX_PLUS_LONG_EXTRA,
    IDX_PLUS_SHORT_EXTRA,
    IDX_LOGIC_LONG_EXTRA,
    IDX_LOGIC_WORD_EXTRA,
    IDX_MULT_68000_BASE,
    IDX_MULT_68000_BITS,
    IDX_MULT_68000_TRANS,
    IDX_MULT_68000_WORD_BASE,
    IDX_MULT_68000_LONG_MUL,
    IDX_MULT_68000_WORD_MUL,
    IDX_EXTEND_EXTRA,

    IDX_COUNT
};

/* Runtime table buffers. The active pair is selected once in
   m68k_select_cost_tables(), then overridden from M68K_COSTS_FILE if set. */
static int cost_speed_68000_10[IDX_COUNT];
static int cost_size_68000_10[IDX_COUNT];

static int cost_speed_68020_30[IDX_COUNT];
static int cost_size_68020_30[IDX_COUNT];

static int cost_speed_68040[IDX_COUNT];
static int cost_size_68040[IDX_COUNT];

static int cost_speed_68060[IDX_COUNT];
static int cost_size_68060[IDX_COUNT];
#if 0
static int cost_speed_68080[IDX_COUNT];
static int cost_size_68080[IDX_COUNT];
#endif
static inline void
set_cost (int t[IDX_COUNT], enum m68k_cost_idx idx, int val)
{
    t[idx] = val;
}

/* ============================================
   68000/010 - SPEED / SIZE
   Winner: m68k-costs-speed.68000.winner.txt (0.859205)
   ============================================ */
static void
init_cost_speed_68000_10 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = -1;

    set_cost (t, IDX_REG_SRC,                2);
    set_cost (t, IDX_REG_DST,               38);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,      5);
    set_cost (t, IDX_MEM_PLUS_REG_REG,      12);
    set_cost (t, IDX_MEM_OTHER,              0);
    set_cost (t, IDX_CONST_INT_Q,            2);
    set_cost (t, IDX_CONST_INT_W,           34);
    set_cost (t, IDX_CONST_INT_L_1,         26);
    set_cost (t, IDX_CONST_INT_L_2,          4);
    set_cost (t, IDX_SYMBOL,                34);
    set_cost (t, IDX_PLUS_REG_REG,          23);
    set_cost (t, IDX_PLUS_REG_CONSTQ,        6);
    set_cost (t, IDX_PLUS_REG_CONSTW,       40);
    set_cost (t, IDX_PLUS_REG_CONSTL,        9);
    set_cost (t, IDX_PLUS_OTHER,            13);
    set_cost (t, IDX_LOGIC_REG_CONST,       38);
    set_cost (t, IDX_SHIFT_CONST,            2);
    set_cost (t, IDX_SHIFT_CONST_W,         36);
    set_cost (t, IDX_LSL_SHIFT,              2);
    set_cost (t, IDX_LSL_SHIFT_STEP,         1);
    set_cost (t, IDX_CALL_OTHER,            17);
    set_cost (t, IDX_DIV_L,                 23);
    set_cost (t, IDX_DIV_W,                 18);

    set_cost (t, IDX_MEM_LONG_EXTRA,         0);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,      18);
    set_cost (t, IDX_CLR_LONG_PENALTY,      14);
    set_cost (t, IDX_CLR_WORD_PENALTY,      15);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        6);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        0);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,      10);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       7);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       4);
    set_cost (t, IDX_MULT_68000_BASE,        3);
    set_cost (t, IDX_MULT_68000_BITS,       40);
    set_cost (t, IDX_MULT_68000_TRANS,       9);
    set_cost (t, IDX_MULT_68000_WORD_BASE,  38);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   39);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   37);
    set_cost (t, IDX_EXTEND_EXTRA,          33);
}

static void
init_cost_size_68000_10 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = -1;

    /* Size not tuned; SBF baseline. */
    set_cost (t, IDX_REG_SRC,                4);
    set_cost (t, IDX_REG_DST,                0);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,      4);
    set_cost (t, IDX_MEM_PLUS_REG_REG,       8);
    set_cost (t, IDX_MEM_OTHER,             12);
    set_cost (t, IDX_CONST_INT_Q,            4);
    set_cost (t, IDX_CONST_INT_W,            8);
    set_cost (t, IDX_CONST_INT_L_1,         10);
    set_cost (t, IDX_CONST_INT_L_2,         12);
    set_cost (t, IDX_SYMBOL,                12);
    set_cost (t, IDX_PLUS_REG_REG,           4);
    set_cost (t, IDX_PLUS_REG_CONSTQ,        4);
    set_cost (t, IDX_PLUS_REG_CONSTW,        8);
    set_cost (t, IDX_PLUS_REG_CONSTL,        8);
    set_cost (t, IDX_PLUS_OTHER,            12);
    set_cost (t, IDX_LOGIC_REG_CONST,        8);
    set_cost (t, IDX_SHIFT_CONST,            8);
    set_cost (t, IDX_SHIFT_CONST_W,          6);
    set_cost (t, IDX_CALL_OTHER,            22);
    set_cost (t, IDX_DIV_L,                108);
    set_cost (t, IDX_DIV_W,                 36);
    set_cost (t, IDX_LSL_SHIFT,              8);
    set_cost (t, IDX_LSL_SHIFT_STEP,         2);

    set_cost (t, IDX_MEM_LONG_EXTRA,         4);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,       2);
    set_cost (t, IDX_CLR_LONG_PENALTY,       8);
    set_cost (t, IDX_CLR_WORD_PENALTY,       4);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        8);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        6);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,       4);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       6);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       6);
    set_cost (t, IDX_MULT_68000_BASE,       24);
    set_cost (t, IDX_MULT_68000_BITS,        6);
    set_cost (t, IDX_MULT_68000_TRANS,       7);
    set_cost (t, IDX_MULT_68000_WORD_BASE,  38);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   48);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   16);
    set_cost (t, IDX_EXTEND_EXTRA,           8);
}

/* ============================================
   68020/030 - SPEED / SIZE
   Winner: m68k-costs-speed.68020.winner.txt (0.988126)
   (identical to 68030 winner - Volamos doesn't model 68030 caches)
   ============================================ */
static void
init_cost_speed_68020_30 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    set_cost (t, IDX_REG_SRC,               10);
    set_cost (t, IDX_REG_DST,               36);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,     10);
    set_cost (t, IDX_MEM_PLUS_REG_REG,      33);
    set_cost (t, IDX_MEM_OTHER,              0);
    set_cost (t, IDX_CONST_INT_Q,            3);
    set_cost (t, IDX_CONST_INT_W,            9);
    set_cost (t, IDX_CONST_INT_L_1,          4);
    set_cost (t, IDX_CONST_INT_L_2,          4);
    set_cost (t, IDX_SYMBOL,                10);
    set_cost (t, IDX_PLUS_REG_REG,          14);
    set_cost (t, IDX_PLUS_REG_CONSTQ,        0);
    set_cost (t, IDX_PLUS_REG_CONSTW,       33);
    set_cost (t, IDX_PLUS_REG_CONSTL,       18);
    set_cost (t, IDX_PLUS_OTHER,             9);
    set_cost (t, IDX_LOGIC_REG_CONST,       15);
    set_cost (t, IDX_SHIFT_CONST,            6);
    set_cost (t, IDX_SHIFT_CONST_W,         16);
    set_cost (t, IDX_SET_REG_REG,            0);
    set_cost (t, IDX_CALL_OTHER,            18);
    set_cost (t, IDX_DIV_L,                 39);
    set_cost (t, IDX_DIV_W,                  9);
    set_cost (t, IDX_LSL_SHIFT,             27);
    set_cost (t, IDX_LSL_SHIFT_STEP,         3);

    set_cost (t, IDX_MEM_LONG_EXTRA,        12);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,      10);
    set_cost (t, IDX_CLR_LONG_PENALTY,      31);
    set_cost (t, IDX_CLR_WORD_PENALTY,       0);
    set_cost (t, IDX_PLUS_LEA_OFFSET,       22);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        1);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,       2);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       7);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,      11);
    set_cost (t, IDX_MULT_68000_BASE,        6);
    set_cost (t, IDX_MULT_68000_BITS,       28);
    set_cost (t, IDX_MULT_68000_TRANS,      23);
    set_cost (t, IDX_MULT_68000_WORD_BASE,   4);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   32);
    set_cost (t, IDX_MULT_68000_WORD_MUL,    0);
    set_cost (t, IDX_EXTEND_EXTRA,           8);
}

static void
init_cost_size_68020_30 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    /* Size not tuned; SBF baseline. */
    set_cost (t, IDX_REG_SRC,                4);
    set_cost (t, IDX_REG_DST,                0);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,      1);
    set_cost (t, IDX_MEM_OTHER,              1);
    set_cost (t, IDX_CONST_INT_Q,           20);
    set_cost (t, IDX_CONST_INT_W,            1);
    set_cost (t, IDX_CONST_INT_L_1,          0);
    set_cost (t, IDX_CONST_INT_L_2,          0);
    set_cost (t, IDX_SYMBOL,                12);
    set_cost (t, IDX_PLUS_REG_REG,           5);
    set_cost (t, IDX_PLUS_REG_CONSTQ,       20);
    set_cost (t, IDX_PLUS_REG_CONSTL,        1);
    set_cost (t, IDX_PLUS_OTHER,            12);
    set_cost (t, IDX_LOGIC_REG_CONST,        8);
    set_cost (t, IDX_SHIFT_CONST,            2);
    set_cost (t, IDX_SET_REG_REG,            1);
    set_cost (t, IDX_CALL_OTHER,            11);
    set_cost (t, IDX_DIV_L,                 43);
    set_cost (t, IDX_DIV_W,                 27);
    set_cost (t, IDX_ADD_L,                  2);
    set_cost (t, IDX_SUB_L,                  2);
    set_cost (t, IDX_MOVEQ,                  2);
    set_cost (t, IDX_LSL_SHIFT,              2);
    set_cost (t, IDX_LSL_SHIFT_STEP,         2);

    set_cost (t, IDX_MEM_LONG_EXTRA,         2);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,       2);
    set_cost (t, IDX_CLR_LONG_PENALTY,       8);
    set_cost (t, IDX_CLR_WORD_PENALTY,       4);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        8);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        4);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,       4);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       4);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       4);
    set_cost (t, IDX_MULT_68000_BASE,       24);
    set_cost (t, IDX_MULT_68000_BITS,        6);
    set_cost (t, IDX_MULT_68000_TRANS,       7);
    set_cost (t, IDX_MULT_68000_WORD_BASE,  38);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   43);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   27);
    set_cost (t, IDX_EXTEND_EXTRA,           8);
}

/* ============================================
   68040 - SPEED / SIZE
   Winner: m68k-costs-speed.68040.winner.txt (0.990612)
   ============================================ */
static void
init_cost_speed_68040 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    set_cost (t, IDX_REG_SRC,                9);
    set_cost (t, IDX_REG_DST,               40);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,     17);
    set_cost (t, IDX_MEM_PLUS_REG_REG,      23);
    set_cost (t, IDX_MEM_OTHER,              0);
    set_cost (t, IDX_CONST_INT_Q,            1);
    set_cost (t, IDX_CONST_INT_W,           12);
    set_cost (t, IDX_CONST_INT_L_1,          0);
    set_cost (t, IDX_CONST_INT_L_2,         27);
    set_cost (t, IDX_SYMBOL,                 1);
    set_cost (t, IDX_PLUS_REG_REG,          14);
    set_cost (t, IDX_PLUS_REG_CONSTQ,        0);
    set_cost (t, IDX_PLUS_REG_CONSTW,        6);
    set_cost (t, IDX_PLUS_REG_CONSTL,        1);
    set_cost (t, IDX_PLUS_OTHER,            14);
    set_cost (t, IDX_LOGIC_REG_CONST,        5);
    set_cost (t, IDX_SHIFT_CONST,            3);
    set_cost (t, IDX_SHIFT_CONST_W,         27);
    set_cost (t, IDX_LSL_SHIFT,              0);
    set_cost (t, IDX_LSL_SHIFT_STEP,         2);
    set_cost (t, IDX_CALL_OTHER,            27);
    set_cost (t, IDX_DIV_L,                 20);
    set_cost (t, IDX_DIV_W,                  0);

    set_cost (t, IDX_MEM_LONG_EXTRA,        10);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,       0);
    set_cost (t, IDX_CLR_LONG_PENALTY,      27);
    set_cost (t, IDX_CLR_WORD_PENALTY,       0);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        1);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        5);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,      20);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       6);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       0);
    set_cost (t, IDX_MULT_68000_BASE,       10);
    set_cost (t, IDX_MULT_68000_BITS,        0);
    set_cost (t, IDX_MULT_68000_TRANS,       0);
    set_cost (t, IDX_MULT_68000_WORD_BASE,   7);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   30);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   26);
    set_cost (t, IDX_EXTEND_EXTRA,          24);
}

static void
init_cost_size_68040 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    /* Size not tuned; SBF baseline. */
    set_cost (t, IDX_REG_SRC,                4);
    set_cost (t, IDX_REG_DST,                0);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,      1);
    set_cost (t, IDX_MEM_OTHER,              1);
    set_cost (t, IDX_CONST_INT_Q,           20);
    set_cost (t, IDX_CONST_INT_W,            1);
    set_cost (t, IDX_CONST_INT_L_1,          0);
    set_cost (t, IDX_CONST_INT_L_2,          0);
    set_cost (t, IDX_SYMBOL,                12);
    set_cost (t, IDX_PLUS_REG_REG,           5);
    set_cost (t, IDX_PLUS_REG_CONSTQ,       20);
    set_cost (t, IDX_PLUS_REG_CONSTL,        1);
    set_cost (t, IDX_PLUS_OTHER,            12);
    set_cost (t, IDX_LOGIC_REG_CONST,        8);
    set_cost (t, IDX_SHIFT_CONST,            2);
    set_cost (t, IDX_SET_REG_REG,            1);
    set_cost (t, IDX_CALL_OTHER,            11);
    set_cost (t, IDX_DIV_L,                 10);
    set_cost (t, IDX_DIV_W,                  5);
    set_cost (t, IDX_ADD_L,                  1);
    set_cost (t, IDX_SUB_L,                  1);
    set_cost (t, IDX_MOVEQ,                  1);
    set_cost (t, IDX_LSL_SHIFT,              2);
    set_cost (t, IDX_LSL_SHIFT_STEP,         0);

    set_cost (t, IDX_MEM_LONG_EXTRA,         2);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,       2);
    set_cost (t, IDX_CLR_LONG_PENALTY,       8);
    set_cost (t, IDX_CLR_WORD_PENALTY,       4);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        8);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        4);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,       4);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       4);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       4);
    set_cost (t, IDX_MULT_68000_BASE,       24);
    set_cost (t, IDX_MULT_68000_BITS,        6);
    set_cost (t, IDX_MULT_68000_TRANS,       7);
    set_cost (t, IDX_MULT_68000_WORD_BASE,  38);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   20);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   14);
    set_cost (t, IDX_EXTEND_EXTRA,           8);
}

/* ============================================
   68060 - SPEED / SIZE
   Winner: m68k-costs-speed.68060.winner.txt (0.981251)
   ============================================ */
static void
init_cost_speed_68060 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    set_cost (t, IDX_REG_SRC,               14);
    set_cost (t, IDX_REG_DST,               40);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,     13);
    set_cost (t, IDX_MEM_PLUS_REG_REG,      28);
    set_cost (t, IDX_MEM_OTHER,              1);
    set_cost (t, IDX_CONST_INT_Q,            3);
    set_cost (t, IDX_CONST_INT_W,            9);
    set_cost (t, IDX_CONST_INT_L_1,          1);
    set_cost (t, IDX_CONST_INT_L_2,         28);
    set_cost (t, IDX_SYMBOL,                 1);
    set_cost (t, IDX_PLUS_REG_REG,          21);
    set_cost (t, IDX_PLUS_REG_CONSTQ,        2);
    set_cost (t, IDX_PLUS_REG_CONSTW,        3);
    set_cost (t, IDX_PLUS_REG_CONSTL,       17);
    set_cost (t, IDX_PLUS_OTHER,            21);
    set_cost (t, IDX_LOGIC_REG_CONST,        6);
    set_cost (t, IDX_SHIFT_CONST,            7);
    set_cost (t, IDX_SHIFT_CONST_W,         25);
    set_cost (t, IDX_SET_REG_REG,            0);
    set_cost (t, IDX_CALL_OTHER,            28);
    set_cost (t, IDX_DIV_L,                 16);
    set_cost (t, IDX_DIV_W,                 12);
    set_cost (t, IDX_LSL_SHIFT,              1);
    set_cost (t, IDX_LSL_SHIFT_STEP,         3);

    set_cost (t, IDX_MEM_LONG_EXTRA,         0);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,      11);
    set_cost (t, IDX_CLR_LONG_PENALTY,      36);
    set_cost (t, IDX_CLR_WORD_PENALTY,      10);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        2);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        5);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,      22);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       8);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,      13);
    set_cost (t, IDX_MULT_68000_BASE,       32);
    set_cost (t, IDX_MULT_68000_BITS,       15);
    set_cost (t, IDX_MULT_68000_TRANS,      12);
    set_cost (t, IDX_MULT_68000_WORD_BASE,   0);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   24);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   40);
    set_cost (t, IDX_EXTEND_EXTRA,          31);
}

static void
init_cost_size_68060 (int t[IDX_COUNT])
{
    for (int i = 0; i < IDX_COUNT; ++i) t[i] = 0;

    /* Size not tuned; SBF baseline. */
    set_cost (t, IDX_REG_SRC,                4);
    set_cost (t, IDX_REG_DST,                0);
    set_cost (t, IDX_MEM_PLUS_REG_DISP,      1);
    set_cost (t, IDX_MEM_OTHER,              1);
    set_cost (t, IDX_CONST_INT_Q,           20);
    set_cost (t, IDX_CONST_INT_W,            1);
    set_cost (t, IDX_CONST_INT_L_1,          0);
    set_cost (t, IDX_CONST_INT_L_2,          0);
    set_cost (t, IDX_SYMBOL,                12);
    set_cost (t, IDX_PLUS_REG_REG,           5);
    set_cost (t, IDX_PLUS_REG_CONSTQ,       20);
    set_cost (t, IDX_PLUS_REG_CONSTL,        1);
    set_cost (t, IDX_PLUS_OTHER,            12);
    set_cost (t, IDX_LOGIC_REG_CONST,        8);
    set_cost (t, IDX_SHIFT_CONST,            2);
    set_cost (t, IDX_SET_REG_REG,            1);
    set_cost (t, IDX_CALL_OTHER,            11);
    set_cost (t, IDX_DIV_L,                 10);
    set_cost (t, IDX_DIV_W,                  5);
    set_cost (t, IDX_ADD_L,                  1);
    set_cost (t, IDX_SUB_L,                  1);
    set_cost (t, IDX_MOVEQ,                  1);
    set_cost (t, IDX_LSL_SHIFT,              2);
    set_cost (t, IDX_LSL_SHIFT_STEP,         0);

    set_cost (t, IDX_MEM_LONG_EXTRA,         2);
    set_cost (t, IDX_MEM_PREDEC_EXTRA,       2);
    set_cost (t, IDX_CLR_LONG_PENALTY,       8);
    set_cost (t, IDX_CLR_WORD_PENALTY,       4);
    set_cost (t, IDX_PLUS_LEA_OFFSET,        8);
    set_cost (t, IDX_PLUS_LONG_EXTRA,        4);
    set_cost (t, IDX_PLUS_SHORT_EXTRA,       4);
    set_cost (t, IDX_LOGIC_LONG_EXTRA,       4);
    set_cost (t, IDX_LOGIC_WORD_EXTRA,       4);
    set_cost (t, IDX_MULT_68000_BASE,       24);
    set_cost (t, IDX_MULT_68000_BITS,        6);
    set_cost (t, IDX_MULT_68000_TRANS,       7);
    set_cost (t, IDX_MULT_68000_WORD_BASE,  38);
    set_cost (t, IDX_MULT_68000_LONG_MUL,   24);
    set_cost (t, IDX_MULT_68000_WORD_MUL,   40);
    set_cost (t, IDX_EXTEND_EXTRA,           8);
}
#if 0
/* ============================================
   68080 (Apollo) - SPEED / SIZE
   No tuning data - uses 68060 values as starting point.
   Update when 68080 hardware/simulator becomes available.
   ============================================ */
static void
init_cost_speed_68080 (int t[IDX_COUNT])
{
    /* Copy from 68060: no 68080 tuning exists yet, and the 68080
       architecture is closest to the 68060 (superscalar, caches). */
    init_cost_speed_68060 (t);
}

static void
init_cost_size_68080 (int t[IDX_COUNT])
{
    init_cost_size_68060 (t);
}
#endif

/* ============================================
   Cost map for M68K_COSTS_FILE overrides (tuning only)
   ============================================ */
struct cost_map_entry { const char *name; int index; };
static const struct cost_map_entry cost_map[] = {
    { "cost_reg_src",           IDX_REG_SRC },
    { "cost_reg_dst",           IDX_REG_DST },
    { "cost_mem_plus_reg_disp", IDX_MEM_PLUS_REG_DISP },
    { "cost_mem_plus_reg_reg",  IDX_MEM_PLUS_REG_REG },
    { "cost_mem_other",         IDX_MEM_OTHER },
    { "cost_const_int_q",       IDX_CONST_INT_Q },
    { "cost_const_int_w",       IDX_CONST_INT_W },
    { "cost_const_int_l_1",     IDX_CONST_INT_L_1 },
    { "cost_const_int_l_2",     IDX_CONST_INT_L_2 },
    { "cost_symbol",            IDX_SYMBOL },
    { "cost_plus_reg_reg",      IDX_PLUS_REG_REG },
    { "cost_plus_reg_constq",   IDX_PLUS_REG_CONSTQ },
    { "cost_plus_reg_constw",   IDX_PLUS_REG_CONSTW },
    { "cost_plus_reg_constl",   IDX_PLUS_REG_CONSTL },
    { "cost_plus_other",        IDX_PLUS_OTHER },
    { "cost_logic_reg_const",   IDX_LOGIC_REG_CONST },
    { "cost_shift_const",       IDX_SHIFT_CONST },
    { "cost_shift_const_w",     IDX_SHIFT_CONST_W },
    { "cost_branch",            IDX_BRANCH },
    { "cost_set_reg_reg",       IDX_SET_REG_REG },
    { "cost_call_other",        IDX_CALL_OTHER },
    { "cost_div_l",             IDX_DIV_L },
    { "cost_div_w",             IDX_DIV_W },
    { "cost_add_l",             IDX_ADD_L },
    { "cost_sub_l",             IDX_SUB_L },
    { "cost_moveq",             IDX_MOVEQ },
    { "cost_lsl_shift",         IDX_LSL_SHIFT },
    { "cost_lsl_shift_step",    IDX_LSL_SHIFT_STEP },

    { "cost_mem_long_extra",    IDX_MEM_LONG_EXTRA },
    { "cost_mem_predec_extra",  IDX_MEM_PREDEC_EXTRA },
    { "cost_clr_long_penalty",  IDX_CLR_LONG_PENALTY },
    { "cost_clr_word_penalty",  IDX_CLR_WORD_PENALTY },
    { "cost_plus_lea_offset",   IDX_PLUS_LEA_OFFSET },
    { "cost_plus_long_extra",   IDX_PLUS_LONG_EXTRA },
    { "cost_plus_short_extra",  IDX_PLUS_SHORT_EXTRA },
    { "cost_logic_long_extra",  IDX_LOGIC_LONG_EXTRA },
    { "cost_logic_word_extra",  IDX_LOGIC_WORD_EXTRA },
    { "cost_mult_68000_base",   IDX_MULT_68000_BASE },
    { "cost_mult_68000_bits",   IDX_MULT_68000_BITS },
    { "cost_mult_68000_trans",  IDX_MULT_68000_TRANS },
    { "cost_mult_68000_word_base", IDX_MULT_68000_WORD_BASE },
    { "cost_mult_68000_long_mul",  IDX_MULT_68000_LONG_MUL },
    { "cost_mult_68000_word_mul",  IDX_MULT_68000_WORD_MUL },
    { "cost_extend_extra",      IDX_EXTEND_EXTRA },
};

static int *active_speed;
static int *active_size;

/* Cost-table selection by m68k_tune value.

   m68k_tune is one of: u68000, u68010, u68020, u68030, u68040,
   u68060, u68020_40, u68020_60, u68080.

   The tables are grouped by architectural family:

     68000/010      -> 68000_10   (distinct: 2-cycle shifts, emulated mul/div)
     68020, 68030   -> 68020_30   (shared: Volamos does not model 68030 caches)
     68040, 68060   -> 68040_60   (shared: 68060 superscalar needs a similar mix)
     u68020_40      -> 68040_60   (tune for 68040; the target is the 68040)
     u68020_60      -> 68040_60   (tune for 68060; the target is the 68060)
     68080          -> 68080      (distinct: Apollo)  */

static void
m68k_select_cost_tables (void)
{
    if (m68k_tune == u68000 || m68k_tune == u68010)
      {
        init_cost_speed_68000_10 (cost_speed_68000_10);
        init_cost_size_68000_10  (cost_size_68000_10);
        active_speed = cost_speed_68000_10;
        active_size  = cost_size_68000_10;
      }
    else if (m68k_tune == u68020 || m68k_tune == u68030)
      {
        init_cost_speed_68020_30 (cost_speed_68020_30);
        init_cost_size_68020_30  (cost_size_68020_30);
        active_speed = cost_speed_68020_30;
        active_size  = cost_size_68020_30;
      }
    else if (m68k_tune == u68040  || m68k_tune == u68020_40)
      {
        init_cost_speed_68040 (cost_speed_68040);
        init_cost_size_68040  (cost_size_68040);
        active_speed = cost_speed_68040;
        active_size  = cost_size_68040;
      }
    else if (m68k_tune == u68060 || m68k_tune == u68020_60)
      {
        init_cost_speed_68060 (cost_speed_68060);
        init_cost_size_68060  (cost_size_68060);
        active_speed = cost_speed_68060;
        active_size  = cost_size_68060;
      }
#if 0
    else if (m68k_tune == u68080)
      {
        init_cost_speed_68080 (cost_speed_68080);
        init_cost_size_68080  (cost_size_68080);
        active_speed = cost_speed_68080;
        active_size  = cost_size_68080;
      }
#endif
    else
      {
        /* Unreachable given all current m68k_tune values, but keeps
           active_speed / active_size valid if a future GCC adds one. */
        init_cost_speed_68020_30 (cost_speed_68020_30);
        init_cost_size_68020_30  (cost_size_68020_30);
        active_speed = cost_speed_68020_30;
        active_size  = cost_size_68020_30;
      }
}

/* Override loader - for tuning and diagnostics only. Not a supported
   user-facing feature; the .txt format is not stable across releases. */
static void
load_cost_overrides (const char *filename, int table[])
{
    FILE *f = fopen (filename, "r");
    if (!f) return;

    char line[256];
    int  lineno = 0;

    while (fgets (line, sizeof (line), f))
      {
        char *p = line;
        lineno++;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == 0) continue;

        char name[128];
        int  value;
        if (sscanf (p, " %127[^= \t] = %d", name, &value) != 2) {
            fprintf (stderr, "%s:%d: syntax error: %s", filename, lineno, line);
            continue;
        }

        for (char *q = name; *q; q++) if (*q >= 'A' && *q <= 'Z') *q += 0x20;

        int found = 0;
        for (size_t i = 0; i < sizeof (cost_map) / sizeof (cost_map[0]); i++) {
            if (strcmp (cost_map[i].name, name) == 0) {
                table[cost_map[i].index] = value;
                found = 1;
                break;
            }
        }
        if (!found) fprintf (stderr, "%s:%d: unknown cost name '%s'\n", filename, lineno, name);
      }
    fclose (f);
}

static void
m68k_init_costs (bool speed ATTRIBUTE_UNUSED)
{
    m68k_select_cost_tables ();

    /* Opt-in override for tuning: M68K_COSTS_FILE points to a text file
       of "name = value" pairs. No fallback filenames. */
    const char *env = getenv ("M68K_COSTS_FILE");
    if (env && *env)
      {
        load_cost_overrides (env, active_speed);
        load_cost_overrides (env, active_size);
      }
}

#define COST_TABLE  (speed ? active_speed : active_size)
#define COST_IS_UNDEFINED(idx)  (COST_TABLE[idx] < 0)

#define COST_CONST_INT_W        (COST_TABLE[IDX_CONST_INT_W])
#define COST_CONST_INT_L_1      (COST_TABLE[IDX_CONST_INT_L_1])
#define COST_CONST_INT_L_2      (COST_TABLE[IDX_CONST_INT_L_2])
#define COST_SYMBOL             (COST_TABLE[IDX_SYMBOL])
#define COST_PLUS_REG_REG       (COST_TABLE[IDX_PLUS_REG_REG])
#define COST_PLUS_REG_CONSTQ    (COST_TABLE[IDX_PLUS_REG_CONSTQ])
#define COST_PLUS_REG_CONSTW    (COST_TABLE[IDX_PLUS_REG_CONSTW])
#define COST_PLUS_REG_CONSTL    (COST_TABLE[IDX_PLUS_REG_CONSTL])
#define COST_PLUS_OTHER         (COST_TABLE[IDX_PLUS_OTHER])
#define COST_LOGIC_REG_CONST    (COST_TABLE[IDX_LOGIC_REG_CONST])
#define COST_SHIFT_CONST        (COST_TABLE[IDX_SHIFT_CONST])
#define COST_SHIFT_CONST_W      (COST_TABLE[IDX_SHIFT_CONST_W])
#define COST_BRANCH             (COST_TABLE[IDX_BRANCH])
#define COST_SET_REG_REG        (COST_TABLE[IDX_SET_REG_REG])
#define COST_CALL_OTHER         (COST_TABLE[IDX_CALL_OTHER])

/* Divide cost table holds divide-cost / 4; multiply by 4 to get
   the actual cost. This normalizes the range for CMA-ES (the raw
   divide costs are > 100, which is out of scale with the
   other entries). */
#define COST_DIV_L              (COST_TABLE[IDX_DIV_L]*4)
#define COST_DIV_W              (COST_TABLE[IDX_DIV_W]*4)
#define COST_ADD_L              (COST_TABLE[IDX_ADD_L])
#define COST_SUB_L              (COST_TABLE[IDX_SUB_L])
#define COST_MOVEQ              (COST_TABLE[IDX_MOVEQ])
#define COST_LSL_SHIFT(shift)   (COST_TABLE[IDX_LSL_SHIFT] + (shift) * COST_TABLE[IDX_LSL_SHIFT_STEP])

static int
const_int_cost (HOST_WIDE_INT i)
{
  switch (m68k_const_method (i))
    {
    case MVZ: case MVS: case NOTB: case NOTW: case NEGW: case SWAP: return 1;
    case MOVL: return 2;
    }
  // case MOVQ:
  return 0;
}


/* ============================================
   SBF Engine (Fully table-driven)
   ============================================ */
   static bool
m68k_costs_engine (rtx x, machine_mode mode, int outer_code,
                            int opno, int *total, bool speed)
{
    int code = GET_CODE (x);
    int total2 = 0;

    switch (code)
      {
      case CALL:
        {
          if (COST_IS_UNDEFINED (IDX_CALL_OTHER)) return false;
          *total = COST_CALL_OTHER;
          return true;
        }
      case EQ: case NE: case GE: case GT: case GTU: case LE: case LT:
        if (COST_IS_UNDEFINED (IDX_BRANCH)) return false;
        *total = COST_BRANCH;
        return true;
      case ZERO_EXTRACT:
        if (outer_code == COMPARE && GET_CODE (XEXP (x, 1)) == CONST_INT && INTVAL (XEXP (x, 1)) == 1) {
            *total = 2; return true;
        }
        return false;
      case POST_INC:
      case TRUNCATE:
        *total = 0;
        return true;
      case PRE_DEC:
        *total = 2;
        return true;
      case REG:
        if (opno) {
            if (COST_IS_UNDEFINED (IDX_REG_SRC)) return false;
            *total = COST_TABLE[IDX_REG_SRC];
        } else {
            if (COST_IS_UNDEFINED (IDX_REG_DST)) return false;
            *total = COST_TABLE[IDX_REG_DST];
        }
        return true;
      case SYMBOL_REF:
        if (COST_IS_UNDEFINED (IDX_SYMBOL)) return false;
        *total = COST_SYMBOL;
        return true;
      case IF_THEN_ELSE:
        *total = 10;
        return true;
      case SET:
        {
          rtx a = XEXP(x, 0);
          rtx b = XEXP(x, 1);
          if (m68k_costs_engine (a, mode, code, 0, total, speed)
              && m68k_costs_engine (b, mode, code, 1, &total2, speed))
            {
              *total += total2;
              if (MEM_P(a) && GET_CODE(b) == CONST_INT && INTVAL(b) == 0) {
                  *total += GET_MODE_SIZE(mode) > 2 ? COST_TABLE[IDX_CLR_LONG_PENALTY] : COST_TABLE[IDX_CLR_WORD_PENALTY];
                  if (GET_CODE(XEXP(a, 0)) == PRE_DEC) *total += 2;
              }
              return true;
            }
          break;
        }
      case CONST:
        {
          rtx a = XEXP(x, 0);
          if (GET_CODE(a) == PLUS && SYMBOL_REF_P(XEXP(a,0)) && GET_CODE(XEXP(a, 1)) == CONST_INT) {
              if (COST_IS_UNDEFINED (IDX_SYMBOL)) return false;
              *total = COST_SYMBOL;
              return true;
          }
          *total = 0;
          break;
        }
      case MULT:
        {
          rtx a = XEXP(x, 0);
          rtx b = XEXP(x, 1);

          if (GET_CODE(b) == CONST_INT)
            {
              int n = INTVAL(b);
              if (n < 0) n = -n;
              int x_shift = 0;
              unsigned y = 1;
              for (; y < (unsigned)n; y += y, ++x_shift) ;
              if (y == (unsigned)n) {
                  *total = (GET_MODE_SIZE(mode) > 2 ? COST_SHIFT_CONST : COST_SHIFT_CONST_W);
                  *total += COST_TABLE[IDX_LSL_SHIFT_STEP] * x_shift;
                  return true;
              }
            }

          if (GET_CODE (a) == ZERO_EXTEND || GET_CODE (a) == SIGN_EXTEND) {
              *total = 0;
              mode = HImode;
          } else if (!m68k_costs_engine (a, mode, code, 0, total, speed)) {
              break;
          }

          if (speed) {
              int f = (GET_MODE_SIZE(mode) > 2) ? COST_TABLE[IDX_MULT_68000_LONG_MUL] : COST_TABLE[IDX_MULT_68000_WORD_MUL];
              if (GET_CODE(b) == CONST_INT && GET_MODE_SIZE(mode) == 2) {
                  int i = INTVAL(b);
                  if (i < 0) i = -i;
                  int bits = 0, l = 0;
                  if (i > 0) {
                      if (GET_CODE (a) == ZERO_EXTEND) {
                          while (i) { ++bits; i>>=1; }
                      } else {
                          while (i || l) { if ((i&1) != l) { l = !l; ++bits; } i >>= 1; }
                      }
                      f = COST_TABLE[IDX_MULT_68000_WORD_BASE] + 2 * bits;
                  }
              }
              *total += f;
          } else {
              *total = GET_MODE_SIZE(mode) > 2 ? COST_TABLE[IDX_MULT_68000_LONG_MUL] : COST_TABLE[IDX_MULT_68000_WORD_MUL];
          }
          return true;
        }
      case DIV:
      case UDIV:
      case MOD:
      case UMOD:
        {
          rtx a = XEXP(x, 0);
          if (m68k_costs_engine (a, mode, code, 0, total, speed)) {
              if (speed) *total += GET_MODE_SIZE(mode) > 2 ? COST_DIV_L : COST_DIV_W;
              else *total += GET_MODE_SIZE(mode) > 2 ? COST_DIV_L : COST_DIV_W;
              return true;
          }
          break;
        }
      case ASHIFT:
      case ASHIFTRT:
      case LSHIFTRT:
        {
          rtx a = XEXP(x, 0);
          rtx b = XEXP(x, 1);

          if (REG_P(a) || SUBREG_P(a)) {

              if (GET_CODE(b) == CONST_INT) {
                  int sv = INTVAL(b);
                  *total = (GET_MODE_SIZE(mode) > 2 ? COST_SHIFT_CONST : COST_SHIFT_CONST_W);

                  // non linear - based on swap
                  if (sv < 16) {
                      *total += COST_TABLE[IDX_LSL_SHIFT_STEP] * sv;
                  } else {
                      *total += COST_TABLE[IDX_LSL_SHIFT_STEP] * (4 + (sv - 16));
                  }
                  return true;
              }
              *total = (GET_MODE_SIZE(mode) > 2 ? COST_SHIFT_CONST : COST_SHIFT_CONST_W) ;
              if (speed) *total += 16;
              return true;
          }
          if (m68k_costs_engine (XEXP(x, 0), mode, code, 0, total, speed)) {
              *total += 8;
              return true;
          }
          break;
        }
      case SUBREG:
      case STRICT_LOW_PART:
        if (GET_CODE (XEXP(x, 0)) == SUBREG || GET_CODE (XEXP(x, 0)) == STRICT_LOW_PART) return false;
        return m68k_costs_engine (XEXP(x, 0), GET_MODE(XEXP(x, 0)), outer_code, 0, total, speed);
      case ZERO_EXTEND:
      case SIGN_EXTEND:
        if (GET_CODE (XEXP(x, 0)) == ZERO_EXTEND || GET_CODE (XEXP(x, 0)) == SIGN_EXTEND) return false;
        if (m68k_costs_engine (XEXP(x, 0), GET_MODE(XEXP(x, 0)), outer_code, 0, total, speed)) {
            *total += COST_TABLE[IDX_EXTEND_EXTRA];
            return true;
        }
        return false;
      case CONST_INT:
        {
          int kind = const_int_cost (INTVAL (x));
          bool is_long = GET_MODE_SIZE (mode) > 2;
          if (kind == 0) {
            if (COST_IS_UNDEFINED (IDX_CONST_INT_Q)) return false;
            *total = COST_TABLE[IDX_CONST_INT_Q];
          } else if (!is_long) {
            if (COST_IS_UNDEFINED (IDX_CONST_INT_W)) return false;
            *total = COST_TABLE[IDX_CONST_INT_W];
          } else if (kind == 1) {
            if (COST_IS_UNDEFINED (IDX_CONST_INT_L_1)) return false;
            *total = COST_TABLE[IDX_CONST_INT_L_1];
          } else {
            if (COST_IS_UNDEFINED (IDX_CONST_INT_L_2)) return false;
            *total = COST_TABLE[IDX_CONST_INT_L_2];
          }
          return true;
        }
      case NEG:
      case NOT:
        {
          rtx a = XEXP(x, 0);
          if (REG_P(a) || SUBREG_P(a)) {
              *total = GET_MODE_SIZE(mode) > 2 ? 6 : 4;
              return true;
          }
          if (m68k_costs_engine (a, mode, code, 0, total, speed)) {
              *total += GET_MODE_SIZE(mode) > 2 ? 12 : 8;
              return true;
          }
          break;
        }
      case MINUS:
      case PLUS:
        {
          rtx a = XEXP(x, 0);
          rtx b = XEXP(x, 1);

          if (GET_CODE(a) == PLUS && GET_CODE(b) == CONST_INT && (unsigned)(INTVAL(b) + 128) < 255) {
              if (REG_P(XEXP(a,0)) && REG_P(XEXP(a,1))) {
                  if (COST_IS_UNDEFINED (IDX_PLUS_REG_REG)) return false;
                  *total = COST_PLUS_REG_REG + COST_TABLE[IDX_PLUS_LEA_OFFSET];
                  return true;
              }
          }

          if (SUBREG_P(a)) a = XEXP(a, 0);
          if (SUBREG_P(b)) b = XEXP(b, 0);

          if (REG_P(a) && REGNO(a) >= 8 && REGNO(a) < 16) {
              if (GET_CODE(b) == CONST && GET_CODE(XEXP(b, 0)) == UNSPEC) { *total = 8; return true; }
              if (GET_CODE(b) == CONST_INT && (unsigned)(INTVAL(b) + 32768) < 65535) { *total = 8; return true; }
          }

          if (REG_P(a) && REG_P(b)) {
              if (COST_IS_UNDEFINED (IDX_PLUS_REG_REG)) return false;
              *total = GET_MODE_SIZE(mode) > 2 ? COST_PLUS_REG_REG + 4 : COST_PLUS_REG_REG;
              return true;
          }

          if (GET_CODE(b) == CONST_INT) {
              int i = INTVAL(b);
              if (REG_P(a) && (USE_MOVQ(i) || (REGNO(a) >= 8 && REGNO(a) < 16 && (unsigned)(INTVAL(b) + 32768) < 65535))) {
                  if (COST_IS_UNDEFINED (IDX_PLUS_REG_CONSTQ)) return false;
                  *total = GET_MODE_SIZE(mode) > 2 ? COST_PLUS_REG_CONSTQ + 4 : COST_PLUS_REG_CONSTQ;
              } else {
                  if (COST_IS_UNDEFINED (IDX_PLUS_OTHER)) return false;
                  *total = GET_MODE_SIZE(mode) > 2 ? COST_PLUS_OTHER + 4 : COST_PLUS_OTHER;
              }
              return true;
          }

          if (GET_CODE(b) == SYMBOL_REF || (GET_CODE(b) == CONST && GET_CODE(XEXP(b,0)) == PLUS && GET_CODE(XEXP(XEXP(b,0),0)) == SYMBOL_REF)) {
              if (COST_IS_UNDEFINED (IDX_PLUS_OTHER)) return false;
              *total = COST_PLUS_OTHER + 4;
              return true;
          }

          if (!m68k_costs_engine (a, mode, code, 0, total, speed)) break;
          if (!m68k_costs_engine (b, mode, code, 1, &total2, speed)) break;

          *total += total2 + (GET_MODE_SIZE(mode) > 2 ? COST_TABLE[IDX_PLUS_LONG_EXTRA] : COST_TABLE[IDX_PLUS_SHORT_EXTRA]);
          return true;
        }
      case COMPARE:
      case AND:
      case XOR:
      case IOR:
        {
          rtx a = XEXP(x, 0);
          rtx b = XEXP(x, 1);
          if (GET_CODE(b) == CONST_INT) {
              if (REG_P(a)) {
                  if (COST_IS_UNDEFINED (IDX_LOGIC_REG_CONST)) return false;
                  *total = GET_MODE_SIZE(mode) > 2 ? COST_LOGIC_REG_CONST + 6 : COST_LOGIC_REG_CONST;
                  return true;
              }
              if (m68k_costs_engine (a, mode, code, 0, total, speed)) {
                  *total += GET_MODE_SIZE(mode) > 2 ? 20 : 12;
                  return true;
              }
          } else if (REG_P(a)) {
              if (m68k_costs_engine (b, mode, code, 1, total, speed)) {
                  *total += GET_MODE_SIZE(mode) > 2 ? 4 : (code == XOR ? 8 : 6);
                  return true;
              }
          } else if (REG_P(b)) {
              if (m68k_costs_engine (a, mode, code, 1, total, speed)) {
                  *total += GET_MODE_SIZE(mode) > 2 ? 12 : 8;
                  return true;
              }
          }

          if (m68k_costs_engine (a, mode, code, 0, total, speed)
              && m68k_costs_engine (b, mode, code, 1, &total2, speed)) {
              *total += total2 + (GET_MODE_SIZE(mode) > 2 ? 4 : (code == XOR ? 8 : 6));
              return true;
          }
          break;
        }
      case MEM:
        {
          rtx y = XEXP (x, 0);
          int yc = GET_CODE (y);
          int idx = IDX_MEM_OTHER;

          if (yc == REG || yc == PRE_INC || yc == POST_INC || yc == POST_DEC || yc == PRE_DEC)
            idx = IDX_MEM_PLUS_REG_DISP;
          else if (yc == CONST_INT || yc == SYMBOL_REF
                   || (yc == PLUS && GET_CODE (XEXP (y, 0)) == SYMBOL_REF))
            idx = IDX_MEM_OTHER;
          else if (yc == PLUS)
            {
              rtx a0 = XEXP (y, 0);
              rtx a1 = XEXP (y, 1);
              if (REG_P (a0) && CONST_INT_P (a1))
                {
                  HOST_WIDE_INT off = INTVAL (a1);
                  idx = (off >= -32768 && off <= 32767)
                          ? IDX_MEM_PLUS_REG_DISP     /* (d16,An) - same cost as (An) */
                          : IDX_MEM_PLUS_REG_REG;     /* out-of-range: needs extra add */
                }
              else
                idx = IDX_MEM_PLUS_REG_REG;       /* (An,Xn) or (plus (plus reg reg) c) */
            }
          else
            idx = IDX_MEM_PLUS_REG_REG;

          if (COST_IS_UNDEFINED (idx)) return false;
          *total = COST_TABLE[idx];
          if (mode != QImode && mode != HImode)
            *total += COST_TABLE[IDX_MEM_LONG_EXTRA];
          if (opno && yc == PRE_DEC)
            *total += COST_TABLE[IDX_MEM_PREDEC_EXTRA];
          return true;
        }
      default:
        break;
      }
    return false;
}
   
/* DImode wrapper: SBF was designed for 32-bit operands; on DImode
   (emulated 64-bit) the cost is approximately doubled. */
static bool
m68k_costs_engine_di (rtx x, machine_mode mode, int outer_code,
                      int opno, int *total, bool speed)
{
  bool r = m68k_costs_engine (x, mode, outer_code, opno, total, speed);
  if (r && mode == DImode)
    *total *= 2;
  return r;
}

/* ============================================
   Exported entry point
   ============================================ */

bool
m68k_costs (rtx x,
	    machine_mode mode,
	    int outer_code,
	    int opno,
	    int *total,
	    bool speed)
{
    static bool initialized;
    if (!initialized)
      {
        initialized = true;
        m68k_init_costs (speed);
      }

    return m68k_costs_engine_di (x, mode, outer_code, opno, total, speed);
}
