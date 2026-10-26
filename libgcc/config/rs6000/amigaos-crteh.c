/* Register the .eh_frame of an AmigaOS 4 program with the unwinder.
   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* AmigaOS has no dl_iterate_phdr, so the unwinder only finds the frames
   that are registered with __register_frame_info.  clib4's startup code
   registers the program's .eh_frame; newlib's and clib2's do not, and
   without it the unwinder finds no frame to unwind.  For those two the
   driver links crtehbegin.o, built from this file with CRT_BEGIN, ahead
   of every other object, and crtehend.o, built with CRT_END, after every
   library, as crtstuff.c's crtbegin.o and crtend.o do elsewhere.

   The registration functions are weak, so that a program that never
   unwinds does not pull in the unwinder.  */

#include "tconfig.h"
#include "tsystem.h"
#include "coretypes.h"
#include "tm.h"
#include "libgcc_tm.h"
#include "unwind-dw2-fde.h"

#if defined (CRT_BEGIN)

extern void __register_frame_info (const void *, struct object *)
  __attribute__ ((weak));
extern void *__deregister_frame_info (const void *)
  __attribute__ ((weak));

/* The start of the program's .eh_frame: nothing is linked before this
   that unwinds.  */
static const char __EH_FRAME_BEGIN__[]
  __attribute__ ((used, section (__LIBGCC_EH_FRAME_SECTION_NAME__),
		  aligned (4)))
  = { };

static struct object object;

/* The startup code runs the constructors from the end of .ctors, where
   priority 0 sorts, so this runs before any other, and the destructors
   from the start of .dtors, so the one below runs after any other.  */
#pragma GCC diagnostic ignored "-Wprio-ctor-dtor"

static void __attribute__ ((constructor (0)))
amigaos_register_frames (void)
{
  if (__register_frame_info)
    __register_frame_info (__EH_FRAME_BEGIN__, &object);
}

static void __attribute__ ((destructor (0)))
amigaos_deregister_frames (void)
{
  if (__deregister_frame_info)
    __deregister_frame_info (__EH_FRAME_BEGIN__);
}

#elif defined (CRT_END)

/* The zero length that ends the frames the unwinder walks.  */
static const int __FRAME_END__[]
  __attribute__ ((used, section (__LIBGCC_EH_FRAME_SECTION_NAME__),
		  aligned (4)))
  = { 0 };

#else
#error "compile with CRT_BEGIN or CRT_END"
#endif
