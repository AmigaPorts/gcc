/* Conversion from float to double for the 68000 family without an FPU.
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

/* The normal numbers are handled here, in less than half the time
   fpgnulib.c takes on a 68000, and everything else goes to __extendsfdf2_slow,
   the function in fpgnulib.c under the name t-floatlib-amiga gives it.  With
   an FPU a float comes back in fp0, and the compiler doesn't call this.

   This has an object of its own because libnix's libm.a can define
   __truncdfsf2 by itself: one object for both would then be a
   multiple definition.  */

#ifndef __HAVE_68881__

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)
#define CONCAT1(a, b) CONCAT2 (a, b)
#define CONCAT2(a, b) a ## b

	.text

	.even
	.globl	SYM (__extendsfdf2)
SYM (__extendsfdf2):
	move.l	4(sp),d0
	move.l	d0,d1
	add.l	d1,d1			| The sign goes.
	cmp.l	#0xff000000,d1
	jcc	.Lextend_slow		| NaN or infinity.
	cmp.l	#0x01000000,d1
	jcs	.Lextend_small		| Zero or denormal.
	moveq	#7,d1
	and.l	d0,d1
	ror.l	#3,d1			| The last 3 bits start the low word.
	lsr.l	#3,d0			| That leaves the sign in bit 28.
	btst	#28,d0
	jne	.Lextend_negative
	add.l	#0x38000000,d0		| (1023 - 127) << 20
	rts
.Lextend_negative:
	add.l	#0xa8000000,d0		| The same, and the sign back in bit 31.
	rts
.Lextend_small:
	tst.l	d1
	jne	.Lextend_slow
	and.l	#0x80000000,d0
	moveq	#0,d1
	rts
.Lextend_slow:
	jra	SYM (__extendsfdf2_slow)

#endif /* !__HAVE_68881__ */
