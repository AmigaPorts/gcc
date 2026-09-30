/* m68k-costs.cc - unified m68k RTX cost model.

   One engine, one pair of tables (speed, size) per CPU.
   The active pair is chosen once from TUNE_68000_10 / m68k_tune.

   Overrides are loaded from the file named by M68K_COSTS_FILE, into
   the active CPU's active table only. No fallback filenames.

   Adding a new CPU means adding one speed table and one size table,
   plus a case in m68k_cost_select(). The engine does not change. */

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
    IDX_REG,
    IDX_MEM_PLUS_REG_DISP,
    IDX_MEM_PLUS_REG_REG,
    IDX_MEM_OTHER,
    IDX_CONST_INT_Q,
    IDX_CONST_INT_W,
    IDX_CONST_INT_L,
    IDX_CONST_DOUBLE,
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
    IDX_MULU_L,
    IDX_MULU_W,
    IDX_ADD_L,
    IDX_SUB_L,
    IDX_MOVE_L,
    IDX_MOVEQ,
    IDX_LSL_SHIFT,
    IDX_COUNT
};

/* ============================================
   68000/010 - SPEED
   ============================================ */
static int cost_speed_68000_10[IDX_COUNT] = {
    [IDX_REG]                   = 3,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 0,
    [IDX_CONST_INT_Q]           = 0,
    [IDX_CONST_INT_W]           = 0,
    [IDX_CONST_INT_L]           = 5,
    [IDX_CONST_DOUBLE]          = 0,
    [IDX_SYMBOL]                = 7,
    [IDX_PLUS_REG_REG]          = 3,
    [IDX_PLUS_REG_CONSTQ]       = 1,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 0,
    [IDX_PLUS_OTHER]            = 6,
    [IDX_LOGIC_REG_CONST]       = 7,
    [IDX_SHIFT_CONST]           = 9,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 0,
    [IDX_CALL_OTHER]            = 5,
    /* 68000 uses a shift-based MULT formula, not the mulu_l/w fallbacks. */
    [IDX_MULU_L]                = 180,
    [IDX_MULU_W]                = 0,
    [IDX_ADD_L]                 = 0,
    [IDX_SUB_L]                 = 0,
    [IDX_MOVE_L]                = 0,
    [IDX_MOVEQ]                 = 0,
    [IDX_LSL_SHIFT]             = 8,
};

/* ============================================
   68000/010 - SIZE
   ============================================ */
static int cost_size_68000_10[IDX_COUNT] = {
    [IDX_REG]                   = 5,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 1,
    [IDX_CONST_INT_Q]           = 20,
    [IDX_CONST_INT_W]           = 1,
    [IDX_CONST_INT_L]           = 0,
    [IDX_CONST_DOUBLE]          = 9,
    [IDX_SYMBOL]                = 12,
    [IDX_PLUS_REG_REG]          = 5,
    [IDX_PLUS_REG_CONSTQ]       = 20,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 1,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 8,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 11,
    [IDX_MULU_L]                = 180,
    [IDX_MULU_W]                = 0,
    [IDX_ADD_L]                 = 0,
    [IDX_SUB_L]                 = 0,
    [IDX_MOVE_L]                = 0,
    [IDX_MOVEQ]                 = 0,
    [IDX_LSL_SHIFT]             = 8,
};

/* ============================================
   68020 - SPEED / SIZE
   Speed values from CMA run.
   ============================================ */
static int cost_speed_68020[IDX_COUNT] = {
    [IDX_REG]                   = 14,
    [IDX_MEM_PLUS_REG_DISP]     = 14,
    [IDX_MEM_PLUS_REG_REG]      = 13,
    [IDX_MEM_OTHER]             = 2,
    [IDX_CONST_INT_Q]           = 9,
    [IDX_CONST_INT_W]           = 23,
    [IDX_CONST_INT_L]           = 22,
    [IDX_CONST_DOUBLE]          = 2,
    [IDX_SYMBOL]                = 19,
    [IDX_PLUS_REG_REG]          = 9,
    [IDX_PLUS_REG_CONSTQ]       = 2,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 30,
    [IDX_PLUS_OTHER]            = 9,
    [IDX_LOGIC_REG_CONST]       = 6,
    [IDX_SHIFT_CONST]           = 7,
    [IDX_SHIFT_CONST_W]         = 29,
    [IDX_BRANCH]                = 11,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 13,
    [IDX_MULU_L]                = 25,
    [IDX_MULU_W]                = 26,
    [IDX_ADD_L]                 = 3,
    [IDX_SUB_L]                 = 11,
    [IDX_MOVE_L]                = 13,
    [IDX_MOVEQ]                 = 2,
    [IDX_LSL_SHIFT]             = 16,
};


static int cost_size_68020[IDX_COUNT] = {
    [IDX_REG]                   = 5,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 1,
    [IDX_CONST_INT_Q]           = 20,
    [IDX_CONST_INT_W]           = 1,
    [IDX_CONST_INT_L]           = 0,
    [IDX_CONST_DOUBLE]          = 9,
    [IDX_SYMBOL]                = 12,
    [IDX_PLUS_REG_REG]          = 5,
    [IDX_PLUS_REG_CONSTQ]       = 20,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 1,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 8,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 11,
    [IDX_MULU_L]                = 43,
    [IDX_MULU_W]                = 27,
    [IDX_ADD_L]                 = 2,
    [IDX_SUB_L]                 = 2,
    [IDX_MOVE_L]                = 2,
    [IDX_MOVEQ]                 = 2,
    [IDX_LSL_SHIFT]             = 2,
};

/* ============================================
   68030 - SPEED / SIZE
   ============================================ */
static int cost_speed_68030[IDX_COUNT] = {
    [IDX_REG]                   = 1,
    [IDX_MEM_PLUS_REG_DISP]     = 0,
    [IDX_MEM_PLUS_REG_REG]      = 4,
    [IDX_MEM_OTHER]             = 9,
    [IDX_CONST_INT_Q]           = 0,
    [IDX_CONST_INT_W]           = 0,
    [IDX_CONST_INT_L]           = 5,
    [IDX_CONST_DOUBLE]          = 4,
    [IDX_SYMBOL]                = 0,
    [IDX_PLUS_REG_REG]          = 3,
    [IDX_PLUS_REG_CONSTQ]       = 0,
    [IDX_PLUS_REG_CONSTW]       = 3,
    [IDX_PLUS_REG_CONSTL]       = 5,
    [IDX_PLUS_OTHER]            = 20,
    [IDX_LOGIC_REG_CONST]       = 11,
    [IDX_SHIFT_CONST]           = 6,
    [IDX_SHIFT_CONST_W]         = 8,
    [IDX_BRANCH]                = 11,
    [IDX_SET_REG_REG]           = 5,
    [IDX_CALL_OTHER]            = 9,
    [IDX_MULU_L]                = 44,
    [IDX_MULU_W]                = 27,
    [IDX_ADD_L]                 = 2,
    [IDX_SUB_L]                 = 2,
    [IDX_MOVE_L]                = 2,
    [IDX_MOVEQ]                 = 2,
    [IDX_LSL_SHIFT]             = 4,
};

static int cost_size_68030[IDX_COUNT] = {
    [IDX_REG]                   = 5,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 1,
    [IDX_CONST_INT_Q]           = 20,
    [IDX_CONST_INT_W]           = 1,
    [IDX_CONST_INT_L]           = 0,
    [IDX_CONST_DOUBLE]          = 9,
    [IDX_SYMBOL]                = 12,
    [IDX_PLUS_REG_REG]          = 5,
    [IDX_PLUS_REG_CONSTQ]       = 20,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 1,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 8,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 11,
    [IDX_MULU_L]                = 44,
    [IDX_MULU_W]                = 27,
    [IDX_ADD_L]                 = 2,
    [IDX_SUB_L]                 = 2,
    [IDX_MOVE_L]                = 2,
    [IDX_MOVEQ]                 = 2,
    [IDX_LSL_SHIFT]             = 4,
};

/* ============================================
   68040 - SPEED / SIZE
   ============================================ */
static int cost_speed_68040[IDX_COUNT] = {
    [IDX_REG]                   = 1,
    [IDX_MEM_PLUS_REG_DISP]     = 0,
    [IDX_MEM_PLUS_REG_REG]      = 3,
    [IDX_MEM_OTHER]             = 6,
    [IDX_CONST_INT_Q]           = 0,
    [IDX_CONST_INT_W]           = 0,
    [IDX_CONST_INT_L]           = 4,
    [IDX_CONST_DOUBLE]          = 4,
    [IDX_SYMBOL]                = 0,
    [IDX_PLUS_REG_REG]          = 2,
    [IDX_PLUS_REG_CONSTQ]       = 0,
    [IDX_PLUS_REG_CONSTW]       = 2,
    [IDX_PLUS_REG_CONSTL]       = 3,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 4,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 3,
    [IDX_BRANCH]                = 3,
    [IDX_SET_REG_REG]           = 2,
    [IDX_CALL_OTHER]            = 4,
    [IDX_MULU_L]                = 20,   /* mul.l */
    [IDX_MULU_W]                = 14,   /* mul.w */
    [IDX_ADD_L]                 = 1,
    [IDX_SUB_L]                 = 1,
    [IDX_MOVE_L]                = 1,
    [IDX_MOVEQ]                 = 1,
    [IDX_LSL_SHIFT]             = 2,
};

static int cost_size_68040[IDX_COUNT] = {
    [IDX_REG]                   = 5,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 1,
    [IDX_CONST_INT_Q]           = 20,
    [IDX_CONST_INT_W]           = 1,
    [IDX_CONST_INT_L]           = 0,
    [IDX_CONST_DOUBLE]          = 9,
    [IDX_SYMBOL]                = 12,
    [IDX_PLUS_REG_REG]          = 5,
    [IDX_PLUS_REG_CONSTQ]       = 20,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 1,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 8,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 11,
    [IDX_MULU_L]                = 20,
    [IDX_MULU_W]                = 14,
    [IDX_ADD_L]                 = 1,
    [IDX_SUB_L]                 = 1,
    [IDX_MOVE_L]                = 1,
    [IDX_MOVEQ]                 = 1,
    [IDX_LSL_SHIFT]             = 2,
};

/* ============================================
   68080 (Apollo) - SPEED / SIZE
   ============================================ */
static int cost_speed_68080[IDX_COUNT] = {
    [IDX_REG]                   = 0,
    [IDX_MEM_PLUS_REG_DISP]     = 0,
    [IDX_MEM_PLUS_REG_REG]      = 3,
    [IDX_MEM_OTHER]             = 5,
    [IDX_CONST_INT_Q]           = 8,
    [IDX_CONST_INT_W]           = 0,
    [IDX_CONST_INT_L]           = 3,
    [IDX_CONST_DOUBLE]          = 0,
    [IDX_SYMBOL]                = 0,
    [IDX_PLUS_REG_REG]          = 13,
    [IDX_PLUS_REG_CONSTQ]       = 0,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 2,
    [IDX_PLUS_OTHER]            = 23,
    [IDX_LOGIC_REG_CONST]       = 0,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 10,
    [IDX_BRANCH]                = 10,
    [IDX_SET_REG_REG]           = 5,
    [IDX_CALL_OTHER]            = 0,
    [IDX_MULU_L]                = 4,
    [IDX_MULU_W]                = 3,
    [IDX_ADD_L]                 = 0,
    [IDX_SUB_L]                 = 0,
    [IDX_MOVE_L]                = 0,
    [IDX_MOVEQ]                 = 0,
    [IDX_LSL_SHIFT]             = 1,
};

static int cost_size_68080[IDX_COUNT] = {
    [IDX_REG]                   = 5,
    [IDX_MEM_PLUS_REG_DISP]     = 1,
    [IDX_MEM_PLUS_REG_REG]      = 0,
    [IDX_MEM_OTHER]             = 1,
    [IDX_CONST_INT_Q]           = 20,
    [IDX_CONST_INT_W]           = 1,
    [IDX_CONST_INT_L]           = 0,
    [IDX_CONST_DOUBLE]          = 9,
    [IDX_SYMBOL]                = 12,
    [IDX_PLUS_REG_REG]          = 5,
    [IDX_PLUS_REG_CONSTQ]       = 20,
    [IDX_PLUS_REG_CONSTW]       = 0,
    [IDX_PLUS_REG_CONSTL]       = 1,
    [IDX_PLUS_OTHER]            = 12,
    [IDX_LOGIC_REG_CONST]       = 8,
    [IDX_SHIFT_CONST]           = 2,
    [IDX_SHIFT_CONST_W]         = 0,
    [IDX_BRANCH]                = 0,
    [IDX_SET_REG_REG]           = 1,
    [IDX_CALL_OTHER]            = 11,
    [IDX_MULU_L]                = 4,
    [IDX_MULU_W]                = 3,
    [IDX_ADD_L]                 = 0,
    [IDX_SUB_L]                 = 0,
    [IDX_MOVE_L]                = 0,
    [IDX_MOVEQ]                 = 0,
    [IDX_LSL_SHIFT]             = 1,
};

/* ============================================
   Cost file (name -> index in the active table)
   ============================================ */

struct cost_map_entry {
    const char *name;
    int         index;
};

static const struct cost_map_entry cost_map[] = {
    { "cost_reg",               IDX_REG },
    { "cost_mem_plus_reg_disp", IDX_MEM_PLUS_REG_DISP },
    { "cost_mem_plus_reg_reg",  IDX_MEM_PLUS_REG_REG },
    { "cost_mem_other",         IDX_MEM_OTHER },
    { "cost_const_int_q",       IDX_CONST_INT_Q },
    { "cost_const_int_w",       IDX_CONST_INT_W },
    { "cost_const_int_l",       IDX_CONST_INT_L },
    { "cost_const_double",      IDX_CONST_DOUBLE },
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
    { "cost_mulu_l",            IDX_MULU_L },
    { "cost_mulu_w",            IDX_MULU_W },
    { "cost_add_l",             IDX_ADD_L },
    { "cost_sub_l",             IDX_SUB_L },
    { "cost_move_l",            IDX_MOVE_L },
    { "cost_moveq",             IDX_MOVEQ },
    { "cost_lsl_shift",         IDX_LSL_SHIFT },
};

/* ============================================
   Active table selection
   ============================================ */

static int *active_speed;
static int *active_size;

static void
m68k_select_cost_tables (void)
{
    if (TUNE_68000_10)
      {
        active_speed = cost_speed_68000_10;
        active_size  = cost_size_68000_10;
      }
    else if (m68k_tune == u68020 || m68k_tune == u68020_40)
      {
        active_speed = cost_speed_68020;
        active_size  = cost_size_68020;
      }
    else if (m68k_tune == u68030)
      {
        active_speed = cost_speed_68030;
        active_size  = cost_size_68030;
      }
    else if (m68k_tune == u68040)
      {
        active_speed = cost_speed_68040;
        active_size  = cost_size_68040;
      }
    else
      {
        active_speed = cost_speed_68080;
        active_size  = cost_size_68080;
      }
}

/* ============================================
   Override loader - fills the active table only
   ============================================ */

static void
load_cost_overrides (const char *filename, int table[])
{
    FILE *f = fopen (filename, "r");
    if (!f)
        return;

    char line[256];
    int  lineno = 0;

    while (fgets (line, sizeof (line), f))
      {
        char *p = line;
        lineno++;

        while (*p == ' ' || *p == '\t')
          p++;

        if (*p == '#' || *p == '\n' || *p == '\r' || *p == 0)
          continue;

        char name[128];
        int  value;

        if (sscanf (p, " %127[^= \t] = %d", name, &value) != 2)
          {
            fprintf (stderr, "%s:%d: syntax error: %s", filename, lineno, line);
            continue;
          }

        for (char *q = name; *q; q++)
          if (*q >= 'A' && *q <= 'Z')
            *q += 0x20;

        int found = 0;
        for (size_t i = 0; i < sizeof (cost_map) / sizeof (cost_map[0]); i++)
          {
            if (strcmp (cost_map[i].name, name) == 0)
              {
                table[cost_map[i].index] = value;
                found = 1;
                break;
              }
          }

        if (!found)
          fprintf (stderr, "%s:%d: unknown cost name '%s'\n",
                   filename, lineno, name);
      }

    fclose (f);
}

/* ============================================
   One-time initialisation
   ============================================ */

static void
m68k_init_costs (bool speed)
{
    m68k_select_cost_tables ();

    /* Overrides apply to whichever table is active for this build. */
    const char *env = getenv ("M68K_COSTS_FILE");
    if (env && *env)
      {
        load_cost_overrides (env, active_speed);
        load_cost_overrides (env, active_size);
        return;
      }

    /* Look for override files in the current directory */
    load_cost_overrides ("m68k-costs-speed.txt", active_speed);
    load_cost_overrides ("m68k-costs-size.txt",  active_size);
}

/* ============================================
   Drop-in access macros - read from active table
   ============================================ */

#define COST_TABLE  (speed ? active_speed : active_size)

#define COST_REG                (COST_TABLE[IDX_REG])
#define COST_MEM_PLUS_REG_DISP  (COST_TABLE[IDX_MEM_PLUS_REG_DISP])
#define COST_MEM_PLUS_REG_REG   (COST_TABLE[IDX_MEM_PLUS_REG_REG])
#define COST_MEM_OTHER          (COST_TABLE[IDX_MEM_OTHER])
#define COST_CONST_INT_Q        (COST_TABLE[IDX_CONST_INT_Q])
#define COST_CONST_INT_W        (COST_TABLE[IDX_CONST_INT_W])
#define COST_CONST_INT_L        (COST_TABLE[IDX_CONST_INT_L])
#define COST_CONST_DOUBLE       (COST_TABLE[IDX_CONST_DOUBLE])
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

#define COST_MULU_L             (COST_TABLE[IDX_MULU_L])
#define COST_MULU_W             (COST_TABLE[IDX_MULU_W])
#define COST_ADD_L              (COST_TABLE[IDX_ADD_L])
#define COST_SUB_L              (COST_TABLE[IDX_SUB_L])
#define COST_MOVE_L             (COST_TABLE[IDX_MOVE_L])
#define COST_MOVEQ              (COST_TABLE[IDX_MOVEQ])

/* Base + per-shift term; the per-shift constant is CPU-specific. */
#define COST_LSL_SHIFT(shift)   \
    (COST_TABLE[IDX_LSL_SHIFT] + (shift) * m68k_lsl_shift_per_step ())

static int
m68k_lsl_shift_per_step (void)
{
    if (TUNE_68000_10)                      return 2;
    if (m68k_tune == u68020 || m68k_tune == u68020_40) return 2;
    if (m68k_tune == u68030)                return 2;
    if (m68k_tune == u68040)                return 0;
    return 1;                               /* 68080 */
}

/* ============================================
   MULT helpers - the two different formulas
   ============================================ */

/* 68020/030/040/080 form: exact power-of-2 -> lsl, 2^n +/- 1 -> lsl+add/sub,
   otherwise popcount-based estimate. */
static int
m68k_mult_const_cost_common (HOST_WIDE_INT n, rtx dst, machine_mode mode, bool speed)
{
    if (n == 0 || n == 1)
        return COST_MOVEQ;

    unsigned HOST_WIDE_INT un = (n < 0) ? -(unsigned HOST_WIDE_INT) n
                                        : (unsigned HOST_WIDE_INT) n;

    if (n > 0)
      {
        int shift = exact_log2 (un);
        if (shift >= 0)
            return COST_LSL_SHIFT (shift);
      }

    if (n > 0 && un > 1 && (un & (un - 1)) == 1)
      {
        int shift = exact_log2 (un - 1);
        if (shift >= 0)
            return COST_LSL_SHIFT (shift) + COST_ADD_L;
      }

    if (n > 0 && un > 2 && (un & (un + 1)) == 0)
      {
        int shift = exact_log2 (un + 1);
        if (shift >= 0)
            return COST_LSL_SHIFT (shift) + COST_SUB_L;
      }

    int bits = 0;
    int l = 0;
    HOST_WIDE_INT nn = n;

    if (nn > 0)
      {
        if (GET_CODE (dst) == ZERO_EXTEND || REG_P (dst))
          {
            while (nn)
              {
                if (nn & 1) ++bits;
                nn >>= 1;
              }
            if (bits == 1 && REG_P (dst))
                return COST_MOVEQ;
          }
        else
          {
            while (nn || l)
              {
                if ((nn & 1) != l) { l = !l; ++bits; }
                nn >>= 1;
              }
          }
        return 12 + bits;
      }
    return -1;
}

/* 68000 form: use the shift-based formula; fall back to COST_MULU_L
   for the general case. */
static int
m68k_mult_const_cost_68000 (HOST_WIDE_INT n, rtx a, machine_mode mode, bool speed)
{
    if (n > 0)
      {
        int shift = exact_log2 (n);
        if (shift >= 0)
            return COST_LSL_SHIFT (shift);
      }

    int f = (GET_MODE_SIZE (mode) > 2) ? 180 : 0;
    HOST_WIDE_INT i = n;

    if (i > 0)
      {
        int bits = 0;
        int transitions = 0;
        int last = 0;
        HOST_WIDE_INT t = i;

        if (GET_CODE (a) == ZERO_EXTEND)
          {
            while (t) { bits += t & 1; t >>= 1; }
            f = 24 + bits * 6;
          }
        else
          {
            while (t || last)
              {
                int bit = t & 1;
                if (bit != last) { transitions++; last = bit; }
                t >>= 1;
              }
            f = 28 + transitions * 7;
          }

        if (GET_MODE_SIZE (mode) > 2)
            f = (f * 3) / 2;
      }
    return f;
}

/* ============================================
   Shared engine
   ============================================ */

static bool
m68k_costs_engine (rtx x,
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
                if (REG_P (a) && CONST_INT_P (b))
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

          *total = (GET_CODE (a) == PLUS)
                   ? COST_PLUS_OTHER
                   : COST_PLUS_OTHER / 2;
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
              *total = GET_MODE_SIZE (mode) > 2
                       ? COST_SHIFT_CONST
                       : COST_SHIFT_CONST_W;
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
              int c = TUNE_68000_10
                        ? m68k_mult_const_cost_68000 (n, dst, mode, speed)
                        : m68k_mult_const_cost_common (n, dst, mode, speed);
              if (c >= 0)
                {
                  *total = c;
                  return true;
                }
            }

          *total = GET_MODE_SIZE (mode) > 2 ? COST_MULU_L : COST_MULU_W;
          return true;
        }

      case DIV:
      case UDIV:
      case MOD:
      case UMOD:
        /* 68000 and 68020 have no div-specific cost; reuse 260 / 0 as before. */
        if (TUNE_68000_10 || m68k_tune == u68020 || m68k_tune == u68020_40
            || m68k_tune == u68030)
            *total = GET_MODE_SIZE (mode) > 2 ? 260 : 0;
        else if (m68k_tune == u68040)
            *total = GET_MODE_SIZE (mode) > 2 ? 38 : 44;
        else
            *total = GET_MODE_SIZE (mode) > 2 ? 6 : 4;
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
          rtx src = XEXP (x, 1);

          if (REG_P (dest) && REG_P (src))
            {
              *total = COST_SET_REG_REG;
              return true;
            }

          if (!m68k_costs_engine (dest, mode, code, 0, total, speed))
              return false;
          if (!m68k_costs_engine (src, mode, code, 1, &total2, speed))
              return false;
          *total += total2;

          if (CONST_INT_P (src) && INTVAL (src) == 0)
            {
              if (REG_P (dest))
                  *total = 0;
              else if (MEM_P (dest))
                  *total = 0;
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
   Exported entry point
   ============================================ */

bool
m68k_costs (rtx x,
	    machine_mode mode,
	    int outer_code ATTRIBUTE_UNUSED,
	    int opno ATTRIBUTE_UNUSED,
	    int *total,
	    bool speed)
{
    static bool initialized;
    if (!initialized)
      {
        initialized = true;
        m68k_init_costs (speed);
      }
    return m68k_costs_engine (x, mode, outer_code, opno, total, speed);
}
