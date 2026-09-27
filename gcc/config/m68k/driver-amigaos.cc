/* Driver-side checks for the AmigaOS C runtime options.
   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#define IN_GCC_FRONTEND 1

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "diagnostic-core.h"

/* Spec function "amigaos-crt-check", called once with the spelling of every
   runtime selector on the command line: "-mcrt=<name>" for each -mcrt= and
   "-noixemul" for the obsolete flag.

   A program gets one C runtime: the selector picks the header search path,
   the startup file and the libraries, so two of them cannot both be
   honoured.  The specs would expand both branches and hand the linker two
   startup files, which fails later with a puzzling duplicate-symbol error,
   so say it here instead.

   Repeating the same selector is allowed, both because it is harmless and
   because the driver expands this from the preprocessor and the link spec
   of a single invocation.  Returns NULL: it adds nothing to the spec.  */

const char *
amigaos_crt_check (int argc, const char **argv)
{
  static const char *chosen;

  if (argc != 1)
    fatal_error (input_location, "%<%%:amigaos-crt-check%> takes one argument");

  if (chosen == NULL)
    chosen = argv[0];
  else if (strcmp (chosen, argv[0]) != 0)
    fatal_error (input_location,
		 "%qs and %qs both select a C runtime, pass only one",
		 chosen, argv[0]);

  return NULL;
}
