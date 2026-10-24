/* A custom implementation of various env functions
 *
 * Used as a fallback when the host clib lacks unsetenv().  Modern clib2,
 * clib4, and newlib all provide setenv()/unsetenv() with proper cleanup, so
 * this file is only compiled when configure detects unsetenv() is absent.
 *
 * On the native AmigaOS shell, programs run within the same process as the
 * shell, so SetVar(GVF_LOCAL_ONLY) changes persist after the program exits.
 * An atexit handler therefore saves each variable's original state before
 * the first modification and restores everything on process exit.
 */
#define __USE_INLINE__

#include <proto/dos.h>
#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct saved_var {
    char *name;
    char *value;   /* original value, or NULL if the variable did not exist */
    LONG  len;     /* lv_Len of the original value (0 when value == NULL) */
    struct saved_var *next;
};

static struct saved_var *saved_list = NULL;
static int atexit_registered = 0;

static void
restore_vars_on_exit (void)
{
    struct saved_var *v = saved_list;
    while (v)
    {
        struct saved_var *next = v->next;
        if (v->value)
        {
            SetVar(v->name, v->value, v->len, GVF_LOCAL_ONLY);
            FreeVec(v->value);
        }
        else
        {
            DeleteVar(v->name, GVF_LOCAL_ONLY);
        }
        FreeVec(v->name);
        FreeVec(v);
        v = next;
    }
    saved_list = NULL;
}

/* Record the current value of 'name' the first time we are about to change it. */
static void
save_original (const char *name)
{
    struct LocalVar *lvar;
    struct saved_var *entry;
    struct saved_var *v = saved_list;

    while (v)
    {
        if (strcmp(v->name, name) == 0)
            return;   /* already saved — keep only the oldest value */
        v = v->next;
    }

    entry = (struct saved_var *)AllocVec(sizeof(struct saved_var), MEMF_CLEAR);
    if (!entry)
        return;

    entry->name = (char *)AllocVec(strlen(name) + 1, MEMF_ANY);
    if (!entry->name)
    {
        FreeVec(entry);
        return;
    }
    strcpy(entry->name, name);

    lvar = FindVar(name, GVF_LOCAL_ONLY);
    if (lvar)
    {
        entry->len   = lvar->lv_Len;
        entry->value = (char *)AllocVec(lvar->lv_Len + 1, MEMF_ANY);
        if (entry->value)
        {
            memcpy(entry->value, lvar->lv_Value, lvar->lv_Len);
            entry->value[lvar->lv_Len] = '\0';
        }
    }
    /* else: entry->value stays NULL — variable did not exist before */

    entry->next = saved_list;
    saved_list  = entry;

    if (!atexit_registered)
    {
        atexit(restore_vars_on_exit);
        atexit_registered = 1;
    }
}

int
setenv (const char *name, const char *value, int replace)
{
    if (!replace)
    {
        if (FindVar(name, GVF_LOCAL_ONLY))
            return 0;
    }

    save_original(name);
    return !SetVar(name, value, -1, GVF_LOCAL_ONLY);
}

int
unsetenv (const char *name)
{
    save_original(name);
    DeleteVar(name, GVF_LOCAL_ONLY);
    return 0;
}

int
putenv (char *str)
{
    int i;

    if (str[0] == '=')
        return -1;

    for (i = 0; str[i]; i++)
    {
        if (str[i] == '=')
        {
            char *name = (char *)AllocVec(i + 1, MEMF_ANY);
            if (name)
            {
                strncpy(name, str, i);
                name[i] = 0;
                setenv(name, &str[i] + 1, 1);
                FreeVec(name);
                return 0;
            }
            return -1;
        }
    }
    return -1;
}

char *
getenv (const char *name)
{
    struct LocalVar *lvar;

    if (!(lvar = FindVar(name, GVF_LOCAL_ONLY)))
        return NULL;
    return lvar->lv_Value;
}
