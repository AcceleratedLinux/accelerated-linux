! Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB
! 
! This library is free software; you can redistribute it and/or
! modify it under the terms of the GNU Library General Public
! License as published by the Free Software Foundation; either
! version 2 of the License, or (at your option) any later version.
! 
! This library is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Library General Public License for more details.
! 
! You should have received a copy of the GNU Library General Public
! License along with this library; if not, write to the Free
! Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
! MA 02111-1307, USA

	.file	"strmake-sparc.s"
.section	".text"
	.align 4
	.global strmake
	.type	 strmake,#function
	.proc	0102
strmake:
	orcc	%g0,%o2,%g0
	be,a	.end
	nop
	ldsb	[%o1],%o3
.loop:	
	stb	%o3,[%o0]
	cmp	%o3,0
	be	.end		! Jump to end on end of string
	add	%o1,1,%o1
	add	%o0,1,%o0
	subcc	%o2,1,%o2
	bne,a	.loop
	ldsb	[%o1],%o3
.end:
	retl
	stb	%g0,[%o0]
.strmake_end:
	.size	 strmake,.strmake_end-strmake
	.ident	"Matt Wagner & Monty"
