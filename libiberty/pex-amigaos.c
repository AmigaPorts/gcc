/* Utilities to execute a program in a subprocess (possibly linked by pipes
   with other subprocesses), and wait for it.  AmigaOS specialization.
   Copyright (C) 1996-2026 Free Software Foundation, Inc.

This file is part of the libiberty library.
Libiberty is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Libiberty is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with libiberty; see the file COPYING.LIB.  If not,
write to the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
Boston, MA 02110-1301, USA.  */

/* AmigaOS has no fork.  The C runtimes emulate fork/exec by running the
   command synchronously, and pex-unix's child branch then _exit()s the
   parent.  Run the child through SystemTagList() instead, like the DJGPP
   backend does with spawn(), and hand the redirected file handles to the
   shell.  The child inherits the process's local variables, which is how
   libnix's setenv() passes the environment on; an explicit env argument
   is not supported.  */

#include "config.h"
#include "libiberty.h"
#include "pex-common.h"

#include <stdio.h>
#include <errno.h>
#ifdef NEED_DECLARATION_ERRNO
extern int errno;
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <dos/dostags.h>
#include <proto/dos.h>

/* Map a file descriptor to its AmigaDOS file handle (libnix).  */
extern long fdtofh (int);

/* Stack handed to every child.  */
#ifndef PEX_AMIGAOS_CHILD_STACK
#define PEX_AMIGAOS_CHILD_STACK (8 * 1024 * 1024)
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif

static int pex_amigaos_open_read (struct pex_obj *, const char *, int);
static int pex_amigaos_open_write (struct pex_obj *, const char *, int, int);
static pid_t pex_amigaos_exec_child (struct pex_obj *, int, const char *,
				     char * const *, char * const *,
				     int, int, int, int,
				     const char **, int *);
static int pex_amigaos_close (struct pex_obj *, int);
static pid_t pex_amigaos_wait (struct pex_obj *, pid_t, int *,
			       struct pex_time *, int, const char **, int *);

/* The list of functions we pass to the common routines.  */

const struct pex_funcs funcs =
{
  pex_amigaos_open_read,
  pex_amigaos_open_write,
  pex_amigaos_exec_child,
  pex_amigaos_close,
  pex_amigaos_wait,
  NULL, /* pipe */
  NULL, /* fdopenr */
  NULL, /* fdopenw */
  NULL  /* cleanup */
};

/* Return a newly initialized pex_obj structure.  */

struct pex_obj *
pex_init (int flags, const char *pname, const char *tempbase)
{
  /* Children run to completion before we return, so pipes are out.  */
  flags &= ~ PEX_USE_PIPES;
  return pex_init_common (flags, pname, tempbase, &funcs);
}

/* Open a file for reading.  */

static int
pex_amigaos_open_read (struct pex_obj *obj ATTRIBUTE_UNUSED,
		       const char *name, int binary)
{
  return open (name, O_RDONLY | (binary ? O_BINARY : O_TEXT));
}

/* Open a file for writing.  */

static int
pex_amigaos_open_write (struct pex_obj *obj ATTRIBUTE_UNUSED,
		        const char *name, int binary, int append)
{
  /* Note that we can't use O_EXCL here because gcc may have already
     created the temporary file via make_temp_file.  */
  if (append)
    return -1;
  return open (name,
	       (O_WRONLY | O_CREAT | O_TRUNC
		| (binary ? O_BINARY : O_TEXT)),
	       S_IRUSR | S_IWUSR);
}

/* Close a file.  */

static int
pex_amigaos_close (struct pex_obj *obj ATTRIBUTE_UNUSED, int fd)
{
  return close (fd);
}

/* Append ARG to *P, quoted for the AmigaDOS shell when needed.  Inside
   double quotes the escape character is '*', so a literal quote or star
   becomes "*"" or "**".  */

static char *
append_arg (char *p, const char *arg)
{
  const char *s;
  int quote = (*arg == '\0');

  for (s = arg; *s; s++)
    if (*s == ' ' || *s == '\t' || *s == '"' || *s == '*')
      quote = 1;

  if (!quote)
    {
      size_t len = strlen (arg);
      memcpy (p, arg, len);
      return p + len;
    }

  *p++ = '"';
  for (s = arg; *s; s++)
    {
      if (*s == '"' || *s == '*')
	*p++ = '*';
      *p++ = *s;
    }
  *p++ = '"';
  return p;
}

/* Execute a child.  */

static pid_t
pex_amigaos_exec_child (struct pex_obj *obj, int flags,
			const char *executable, char * const * argv,
			char * const * env ATTRIBUTE_UNUSED,
			int in, int out, int errdes,
			int toclose ATTRIBUTE_UNUSED, const char **errmsg,
			int *err)
{
  struct TagItem tags[5];
  int ntags = 0;
  size_t len;
  char *cmd, *p;
  int i, status;
  int *statuses;

  /* Worst case every character is escaped and every argument quoted.  */
  len = 2 * strlen (executable) + 3;
  for (i = 1; argv[i] != NULL; i++)
    len += 2 * strlen (argv[i]) + 3;
  cmd = XNEWVEC (char, len + 1);

  p = append_arg (cmd, executable);
  for (i = 1; argv[i] != NULL; i++)
    {
      *p++ = ' ';
      p = append_arg (p, argv[i]);
    }
  *p = '\0';

  if (in != STDIN_FILE_NO)
    {
      tags[ntags].ti_Tag = SYS_Input;
      tags[ntags++].ti_Data = fdtofh (in);
    }
  if (out != STDOUT_FILE_NO)
    {
      tags[ntags].ti_Tag = SYS_Output;
      tags[ntags++].ti_Data = fdtofh (out);
    }
  if ((flags & PEX_STDERR_TO_STDOUT) != 0)
    {
      tags[ntags].ti_Tag = SYS_Error;
      tags[ntags++].ti_Data = fdtofh (out);
    }
  else if (errdes != STDERR_FILE_NO)
    {
      tags[ntags].ti_Tag = SYS_Error;
      tags[ntags++].ti_Data = fdtofh (errdes);
    }
  /* The child would otherwise inherit the shell's default stack, a few
     KB, and the compilers recurse deeply.  libnix does not grow the stack
     on its own, so size it here.  */
  tags[ntags].ti_Tag = NP_StackSize;
  tags[ntags++].ti_Data = PEX_AMIGAOS_CHILD_STACK;
  tags[ntags].ti_Tag = TAG_END;
  tags[ntags].ti_Data = 0;

  /* Flush our own buffered output so it stays ordered with the child's.  */
  fflush (stdout);
  fflush (stderr);

  status = SystemTagList ((CONST_STRPTR) cmd, tags);
  free (cmd);

  if (status == -1)
    {
      *err = ENOENT;
      *errmsg = "SystemTagList";
      return (pid_t) -1;
    }

  /* Save the exit status for later, in the wait() encoding gcc decodes
     with WIFEXITED/WEXITSTATUS.  When we are called, obj->count is the
     number of children which have executed before this one.  */
  statuses = (int *) obj->sysdep;
  statuses = XRESIZEVEC (int, statuses, obj->count + 1);
  statuses[obj->count] = (status & 0xff) << 8;
  obj->sysdep = (void *) statuses;

  return (pid_t) obj->count;
}

/* Wait for a child process to complete.  Actually the child process
   has already completed, and we just need to return the exit
   status.  */

static pid_t
pex_amigaos_wait (struct pex_obj *obj, pid_t pid, int *status,
		  struct pex_time *time, int done ATTRIBUTE_UNUSED,
		  const char **errmsg ATTRIBUTE_UNUSED,
		  int *err ATTRIBUTE_UNUSED)
{
  int *statuses;

  if (time != NULL)
    memset (time, 0, sizeof *time);

  statuses = (int *) obj->sysdep;
  *status = statuses[pid];

  return 0;
}
