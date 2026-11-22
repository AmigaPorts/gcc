/* Conversions from long long to double, rounded, for the 68881.
   Copyright (C) 2026 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */

/* __floatdidf and __floatundidf return the integer in fp0 exactly, in
   extended precision.  With -fexcess-precision=standard the compiler calls
   these instead, which round it to double by storing and reloading it.  */

#ifdef __HAVE_68881__

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)
#define CONCAT1(a, b) CONCAT2 (a, b)
#define CONCAT2(a, b) a ## b

	.text

	.even
	.globl	SYM (__floatdidf_rounded)
SYM (__floatdidf_rounded):
	move.l	8(sp),-(sp)		| The argument again, low word first.
	move.l	8(sp),-(sp)
	jsr	SYM (__floatdidf)
	addq.l	#8,sp
	fmove.d	fp0,-(sp)
	fmove.d	(sp)+,fp0
	rts

	.even
	.globl	SYM (__floatundidf_rounded)
SYM (__floatundidf_rounded):
	move.l	8(sp),-(sp)
	move.l	8(sp),-(sp)
	jsr	SYM (__floatundidf)
	addq.l	#8,sp
	fmove.d	fp0,-(sp)
	fmove.d	(sp)+,fp0
	rts

#endif /* __HAVE_68881__ */
