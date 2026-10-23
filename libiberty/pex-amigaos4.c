/* Utilities to execute a program in a subprocess (possibly linked by pipes
   with other subprocesses), and wait for it.  Generic AMIGAOS specialization.
   Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2003, 2005
   Free Software Foundation, Inc.

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
#include <ctype.h>

/* Use ECHILD if available, otherwise use EINVAL.  */
#ifdef ECHILD
#define PWAIT_ERROR ECHILD
#else
#define PWAIT_ERROR EINVAL
#endif

#if !defined(FD_CLOEXEC)
#define FD_CLOEXEC 1
#endif

static int pex_amiga_open_read (struct pex_obj *, const char *, int);
static int pex_amiga_open_write (struct pex_obj *, const char *, int, int);
static pid_t pex_amiga_exec_child (struct pex_obj *, int, const char *,
				 char * const *, char * const *,
				 int, int, int, int,
				 const char **, int *);
static int pex_amiga_close (struct pex_obj *, int);
static pid_t pex_amiga_wait (struct pex_obj *, pid_t, int *, struct pex_time *,
			   int, const char **, int *);
static FILE *pex_amiga_fdopenr (struct pex_obj *, int, int);
static FILE *pex_amiga_fdopenw (struct pex_obj *, int, int);

/* The list of functions we pass to the common routines.  */

const struct pex_funcs funcs =
{
  pex_amiga_open_read,
  pex_amiga_open_write,
  pex_amiga_exec_child,
  pex_amiga_close,
  pex_amiga_wait,
  NULL, /* pipe */
  pex_amiga_fdopenr,
  pex_amiga_fdopenw,
  NULL, /* cleanup */
};

/* Return a newly initialized pex_obj structure.  */

struct pex_obj *
pex_init (int flags, const char *pname, const char *tempbase)
{
  /* AMIGAOS does not support pipes.  */
  flags &= ~ PEX_USE_PIPES;
  return pex_init_common (flags, pname, tempbase, &funcs);
}

/* Open a file for reading.  */

static int
pex_amiga_open_read (struct pex_obj *obj ATTRIBUTE_UNUSED, const char *name,
		    int binary ATTRIBUTE_UNUSED)
{
  return open (name, O_RDONLY);
}

/* Open a file for writing.  */

static int
pex_amiga_open_write (struct pex_obj *obj ATTRIBUTE_UNUSED, const char *name,
		     int binary ATTRIBUTE_UNUSED, int append)
{
  /* Note that we can't use O_EXCL here because gcc may have already
     created the temporary file via make_temp_file.  */
  return open (name, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0666);
}

/* Close a file.  */

static int
pex_amiga_close (struct pex_obj *obj ATTRIBUTE_UNUSED, int fd)
{
  return close (fd);
}

/* Execute a child.  */

const unsigned char __shell_escape_character = '\\';

static pid_t
pex_amiga_exec_child (struct pex_obj *obj, int flags ATTRIBUTE_UNUSED, const char *executable ATTRIBUTE_UNUSED,
		     char * const * argv, char * const * env ATTRIBUTE_UNUSED,
                     int in ATTRIBUTE_UNUSED, int out ATTRIBUTE_UNUSED, int errdes ATTRIBUTE_UNUSED,
		     int toclose ATTRIBUTE_UNUSED, const char **errmsg, int *err)
{
  int rc;
  char *scmd,*s;
  int i,j,c,len,arglen;
  int need_quote;
  int already_have_quote;
  int escaped;
  int *statuses;

  len = 0;

  for(i = 0 ; argv[i] != NULL ; i++)
  {
    arglen = strlen(argv[i]);

    len += 1 + arglen;

    need_quote = already_have_quote = 0;

    /* Check if this parameter is already surrounded by double quotes.
       What counts is that the first character is a double quote. We
       hope that the last character is an unescaped double quote, but
       don't check for it. */
    if(argv[i][0] == '\"')
    {
      already_have_quote = 1;
    }
    else
    {
      /* Check if there's a blank space in the argument. If so, we will
         need to add double quote characters. */
      for (j = 0 ; j < arglen ; j++)
      {
        c = argv[i][j];

        if (isspace(c))
        {
          need_quote = 1;
          break;
        }
      }

      /* Make room for the double quote characters that we will have to add. */
      if(need_quote)
        len += 2;
    }

    /* Check if there are " or * characters in the quoted string which
       may have to be escaped. */
    if (need_quote || already_have_quote)
    {
      for (j = 0 ; j < arglen ; j++)
      {
        c = argv[i][j];

        /* We just might have to add an escape character in front of these two. */
        if (c == '\"' || c == '*')
	        len++;
      }
    }
  }

  s = scmd = (char *) xmalloc (len+1);

  for(i = 0 ; argv[i] != NULL ; i++)
  {
    arglen = strlen(argv[i]);

    need_quote = already_have_quote = 0;

    if (argv[i][0] == '\"')
    {
      already_have_quote = 1;
    }
    else
    {
      for (j = 0 ; j < arglen ; j++)
      {
        c = argv[i][j];

        if (isspace(c))
        {
          need_quote = 1;
          break;
        }
      }
    }

    if(s != scmd)
      (*s++) = ' ';

    if(need_quote)
      (*s++) = '\"';

    escaped = 0;

    for(j = 0 ; j < arglen ; j++)
    {
      c = argv[i][j];

      /* If this is a " or * and the parameter is quoted, try to
         add an escape character in front of it. */
      if((c == '\"' || c == '*') && (need_quote || already_have_quote))
	    {
        /* Careful, don't escape the first double
           quote character by mistake. */
        if(!already_have_quote || j > 0)
        {
          /* Don't add an escape character here if the previous character
             already was an escape character. */
          if(!escaped)
            (*s++) = '*';
	      }
      }

      (*s++) = c;

      /* Remember if the last character read was an escape character. */
      if (escaped)
        escaped = 0;
      else
        escaped = (c == __shell_escape_character && c != '*');
    }

    if(need_quote)
      (*s++) = '\"';
  }

  (*s) = '\0';

  rc = system (scmd);

  free (scmd);

  if (rc == -1)
  {
    *err = errno;
    *errmsg = install_error_msg;
    return -1;
  }

  /* Save the exit status for later.  When we are called, obj->count
     is the number of children which have executed before this
     one.  */
  statuses = (int *) obj->sysdep;
  statuses = XRESIZEVEC (int, statuses, obj->count + 1);
  statuses[obj->count] = (rc << 8); /* Tuck the status away for pwait */
  obj->sysdep = (void *) statuses;

  return obj->count;
}

/* Create a pipe.  */
/*
static int
pex_amiga_pipe (struct pex_obj *obj ATTRIBUTE_UNUSED, int *p,
	       int binary ATTRIBUTE_UNUSED)
{
  return pipe (p);
}
*/

/* Get a FILE pointer to read from a file descriptor.  */

static FILE *
pex_amiga_fdopenr (struct pex_obj *obj ATTRIBUTE_UNUSED, int fd,
		  int binary ATTRIBUTE_UNUSED)
{
  return fdopen (fd, "r");
}

/* Get a FILE pointer to write to a file descriptor.  */

static FILE *
pex_amiga_fdopenw (struct pex_obj *obj ATTRIBUTE_UNUSED, int fd,
		  int binary ATTRIBUTE_UNUSED)
{
  if (fcntl (fd, F_SETFD, FD_CLOEXEC) < 0)
    return NULL;
  return fdopen (fd, "w");
}


/* Wait for a child process to complete.  Actually the child process
   has already completed, and we just need to return the exit
   status.  */

static pid_t
pex_amiga_wait (struct pex_obj *obj, pid_t pid, int *status,
		struct pex_time *time, int done ATTRIBUTE_UNUSED,
		const char **errmsg ATTRIBUTE_UNUSED,
		int *err ATTRIBUTE_UNUSED)
{
  int *statuses;

  if (time != NULL)
    memset (time, 0, sizeof (struct pex_time));

  statuses = (int *) obj->sysdep;
  *status = statuses[pid];

  return 0;
}
