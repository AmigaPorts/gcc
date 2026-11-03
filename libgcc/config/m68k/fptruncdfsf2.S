/* Conversion from double to float for the 68000 family without an FPU.
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
   fpgnulib.c takes on a 68000, and everything else goes to __truncdfsf2_slow,
   the function in fpgnulib.c under the name t-floatlib-amiga gives it.  With
   an FPU a float comes back in fp0, and the compiler doesn't call this.

   This has an object of its own because libnix's libm.a can define
   __extendsfdf2 by itself: one object for both would then be a
   multiple definition.  */

#ifndef __HAVE_68881__

#define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)
#define CONCAT1(a, b) CONCAT2 (a, b)
#define CONCAT2(a, b) a ## b

	.text

	.even
	.globl	SYM (__truncdfsf2)
SYM (__truncdfsf2):
	movem.l	4(sp),d0-d1
	move.l	d0,a0			| For the sign.
	add.l	d1,d1
	addx.l	d0,d0			| The first of 3 bits, and the sign goes.
	sub.l	#0x70200000,d0		| (1023 - 127 + 1) << 21
	cmp.l	#0x1fc00000,d0		| 254 - 1 exponents, << 21
	jcc	.Ltrunc_slow		| Out of range, or NaN or infinity or zero.
	add.l	d1,d1
	addx.l	d0,d0
	add.l	d1,d1
	addx.l	d0,d0
	tst.l	d1			| To nearest, ties to even.
	jpl	.Ltrunc_rounded
	add.l	d1,d1
	jne	.Ltrunc_up
	btst	#0,d0
	jeq	.Ltrunc_rounded
.Ltrunc_up:
	addq.l	#1,d0			| Carries into the exponent if it has to.
.Ltrunc_rounded:
	add.l	#0x00800000,d0		| The exponent's 1 from the sub.l.
	move.l	a0,d1
	jpl	.Ltrunc_positive
	bset	#31,d0
.Ltrunc_positive:
	rts
.Ltrunc_slow:
	move.l	4(sp),d0
	add.l	d0,d0
	or.l	8(sp),d0
	jne	.Ltrunc_c
	move.l	a0,d0			| Zero keeps its sign.
	and.l	#0x80000000,d0
	rts
.Ltrunc_c:
	jra	SYM (__truncdfsf2_slow)

#endif /* !__HAVE_68881__ */
