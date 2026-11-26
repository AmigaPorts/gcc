/* The 68000's 32-bit multiply and divide helpers for AmigaOS.
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

/* Faster division helpers than lb1sf68.S's, for the 68000 and 68010.
   They are built on one routine that returns both the quotient and the
   remainder, so % costs only some 30 cycles more than /, where lb1sf68.S's
   __umodsi3 divides, multiplies and subtracts: 2 to 2.6 times as slow.
   And their time doesn't grow with the divisor's width, where lb1sf68.S's
   __udivsi3 shifts a divisor of more than 16 bits right one bit at a time:
   twice as slow with a 31-bit one.  The price is size: the four division
   helpers are 272 bytes, lb1sf68.S's 236, and a program that only divides
   links 80 bytes more.  __mulsi3 is lb1sf68.S's.

   Only AmigaOS uses this file, so that other m68k targets keep upstream's
   code.  There every C library defines these helpers too, and its own must
   win: they are weak here, for programs without one.

   The helpers reach the cores in other objects with absolute jsr and jmp,
   not lb1sf68.S's PICCALL and PICJUMP: with -fbaserel and -resident those
   make a 16-bit bra, which fails to link once the linker places the two
   more than 32 KB apart.  */

#include "lb1sf68.S"

#ifdef L_amigaos_udivmodsi
/* d0 = N, d1 = D -> d0 = N / D, d1 = N % D, unsigned.  Clobbers a0 and a1
   only, besides d0 and d1.  */
	.text
	.globl	SYM (__udivmodsi_internal)
SYM (__udivmodsi_internal):
	movel	d2, a0
	movel	d1, d2
	swap	d2
	tstw	d2
	jne	3f

/* D < 2^16: divide the high word of N, then the remainder and the low
   word, or N alone if its high word is 0.  */
	movel	d0, d2
	clrw	d0
	swap	d0
	jeq	1f
	divu	d1, d0		| d0 = high remainder : high quotient
	movel	d0, a1
	movew	d2, d0		| d0 = high remainder : low word of N
	divu	d1, d0		| d0 = remainder : low quotient
	movel	d0, d1
	movel	a1, d2
	swap	d2
	movew	d0, d2		| d2 = quotient
	movel	d2, d0
	clrw	d1
	swap	d1
	movel	a0, d2
	rts
1:	divu	d1, d2		| d2 = remainder : quotient
	movew	d2, d0
	clrw	d2
	swap	d2
	movel	d2, d1
	movel	a0, d2
	rts

/* D >= 2^16, so the quotient fits in 16 bits.  Shift D left by s until
   its top bit is set.  The top 32 bits of N << s divided by the top word
   of D << s give an estimate q of the quotient, at most one too large.
   N << s - q * (D << s) comes from that divu's remainder, the low word of
   N << s and one multiply; while it is negative, q is one too large.  The
   remainder is that difference shifted back.  d2 counts c = 16 - s.  */
3:	movel	d3, sp@-
	moveq	IMM (16), d2
	cmpl	IMM (0x01000000), d1
	jcc	4f
	lsll	IMM (8), d1
	subqw	IMM (8), d2
4:	cmpl	IMM (0x10000000), d1
	jcc	5f
	lsll	IMM (4), d1
	subqw	IMM (4), d2
5:	cmpl	IMM (0x40000000), d1
	jcc	6f
	lsll	IMM (2), d1
	subqw	IMM (2), d2
6:	tstl	d1
	jmi	7f
	addl	d1, d1
	subqw	IMM (1), d2
7:	movel	d0, d3		| the low word of N << s
	swap	d3
	clrw	d3
	lsrl	d2, d3
	lsrl	d2, d0		| the top 32 bits of N << s
	swap	d1
	divu	d1, d0		| by the top word of D << s
	movew	d0, a1		| q
	movew	d3, d0
	swap	d1		| d1 = D << s
	movew	a1, d3
	mulu	d1, d3		| q times the low word of D << s
	subl	d3, d0		| N << s - q * (D << s), modulo 2^32
	jcc	9f
8:	subqw	IMM (1), a1
	addl	d1, d0
	jcc	8b
9:	negw	d2
	addw	IMM (16), d2	| s
	lsrl	d2, d0
	movel	d0, d1
	moveq	IMM (0), d0
	movew	a1, d0
	movel	sp@+, d3
	movel	a0, d2
	rts
#endif /* L_amigaos_udivmodsi */

#ifdef L_amigaos_divmodsi
/* d0 = N, d1 = D -> d0 = N / D, truncated, and d1 = N % D, with N's
   sign.  */
	.text
	.globl	SYM (__divmodsi_internal)
SYM (__divmodsi_internal):
	tstl	d0
	jpl	2f
	negl	d0
	tstl	d1
	jpl	1f
	negl	d1
	jsr	SYM (__udivmodsi_internal)
	negl	d1
	rts
1:	jsr	SYM (__udivmodsi_internal)
	negl	d0
	negl	d1
	rts
2:	tstl	d1
	jmi	3f
	jmp	SYM (__udivmodsi_internal)
3:	negl	d1
	jsr	SYM (__udivmodsi_internal)
	negl	d0
	rts
#endif /* L_amigaos_divmodsi */

#ifdef L_amigaos_udivsi3
	.text
	FUNC (__udivsi3)
	.globl	SYM (__udivsi3)
	.weak	SYM (__udivsi3)
SYM (__udivsi3):
	moveml	sp@(4), d0/d1
	jmp	SYM (__udivmodsi_internal)
#endif /* L_amigaos_udivsi3 */

#ifdef L_amigaos_umodsi3
	.text
	FUNC (__umodsi3)
	.globl	SYM (__umodsi3)
	.weak	SYM (__umodsi3)
SYM (__umodsi3):
	moveml	sp@(4), d0/d1
	jsr	SYM (__udivmodsi_internal)
	movel	d1, d0
	rts
#endif /* L_amigaos_umodsi3 */

#ifdef L_amigaos_divsi3
	.text
	FUNC (__divsi3)
	.globl	SYM (__divsi3)
	.weak	SYM (__divsi3)
SYM (__divsi3):
	moveml	sp@(4), d0/d1
	jmp	SYM (__divmodsi_internal)
#endif /* L_amigaos_divsi3 */

#ifdef L_amigaos_modsi3
	.text
	FUNC (__modsi3)
	.globl	SYM (__modsi3)
	.weak	SYM (__modsi3)
SYM (__modsi3):
	moveml	sp@(4), d0/d1
	jsr	SYM (__divmodsi_internal)
	movel	d1, d0
	rts
#endif /* L_amigaos_modsi3 */
