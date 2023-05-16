# FSK86 Bootloader
# ========================

# David Leonard, 2020
# Heavily based on mlb.asm by Sebastian Plotz
# Copyright (C) 2012 Sebastian Plotz
# Copyright (C) 2014 Wiktor W Brodlo
# Copyright (C) 2020 Digi International

# FSK86 Bootloader is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This is an FSK (filesystem+kernel) variant of Sebastian Plotz's MLB
# (Minimal Linux Bootloader). It has the following changes:
#   - includes a DOS partition table
#   - occupies 3 sectors at start of disk (ie no longer 1 sector)
#   - provides sector 1 as an environment (i.e. no mlbinstall)
#     structured as a sequence of var=value\0 strings, mostly compatible
#     with U-Boot config areas
#   - boots the FSK in partition 2 (sda2)
#   - skips over CRAMFS or SQUASHFS file systems to locate
#     the bzImage (ie FSK format)
#   - passes a linux cmdline from the environment sector
#
# How to use:
#   - Install this as the first 3 sectors of a disk (reserve 4)
#   - Use a partition editor to adjust partition 2 if needed
#   - concatenate a squashfs root with bzImage and write to parition 2
#   - arrange to pass root=/dev/sda2 to the kernel cmdline
#   - optionally replace partition 1 with config vars such as cmdline=...
#
# On error a letter is shown followed by a hex code (eg R01) then halt.
# Errors are:
#    F - boot partition does not contain a cramfs or squashfs header
#    M - memory transfer error;
#        see status codes at http://www.ctyme.com/intr/rb-1527.htm
#    R - sector read error; see http://www.ctyme.com/intr/rb-0606.htm

# Memory layout
# =============

# 0x07c00 - 0x081ff	FSK86 Bootloader
#    7dbe		+ partition table
#    7dfe		+ MBR signature
#    7e00               + environment strings
#    8000 		+ code, data
# 0x10000 - 0x17fff	Real mode kernel
# 0x18000 - 0x1dfff	Stack and heap
# 0x1e000 - 0x1ffff	Kernel command line
# 0x20000 - 0x2fdff	temporary space for loading
#			protected-mode kernel

# base_ptr = 0x10000
# heap_end = 0xe000
# heap_end_ptr = heap_end - 0x200 = 0xde00
# cmd_line_ptr = base_ptr + heap_end = 0x1e000

	.code16
	.global _start

# Assumes BIOS Boot Specification 1.01 environment:
#  8086 16-bit real mode
#  %dl = boot disk for use with INT 13h
#  %es:di = PnP info (unused)

_start:
	cli					# stop BIOS from hurting us
	xor	%ax, %ax
	mov	%ax, %ds			# zero segments, to make
	mov	%ax, %ss			#  gdb easier
	mov	$_start, %sp			# set up the stack
	sti

read_bootloader:

	mov	%ax, %es
	mov	$environ, %bx
	mov	$3, %ax				# read next 3 sectors of fsk86
	call	read_from_hdd			#  into 7e00..83ff

read_fs_header:

	mov	part2.lba, %eax			# locate fsk part start sector
	mov	%eax, dap.lba
	mov	$0x10000 >>4, %ax		# read fsk's first sector into
	mov	%ax, %es			#  1000:0000, i.e. 1_0000 is a
	xor	%bx, %bx			#  temp area
	mov	$1, %ax
	call	read_from_hdd

skip_filesystem:

	movl	%es:0, %eax			# check filesystem type
	cmpl	$0x28cd3d45, %eax		# CRAMFS?
	mov	$4, %di				# load cramfs's filesystem size
	je	0f
	cmpl	$0x73717368, %eax		# SQUASHFS?
	mov	$40, %di			# load squashfs's fs size
	je	0f
	mov	$'F', %al			# Unknown fstype, error 'F'
	jmp	error
0:
	movl	%es:0(%di), %eax		# load hdr's size_bytes field
	shr	$9, %eax			# convert to sectors
	dec	%eax				# fixup dap.lba postincrement
	add	%eax, dap.lba			# step over filesystem sectors

read_kernel_bootsector:

	mov	$1, %ax				# read first kernel sector
	call	read_from_hdd			#  into 0x10000-0x101ff

read_kernel_setup:

	xor	%ax, %ax
	add	%es:0x1f1, %al			# nr of extra sectors to load
	jnz 	0f
	mov	$4, %al
0:
	mov	$0x200, %bx			# read extra sectors to 0x10200
	call	read_from_hdd

set_header_fields:
	movb	$0xe1, %es:0x210		# set type_of_loader
	orb	$0x80, %es:0x211		# set CAN_USE_HEAP
	movw	$0xde00, %es:0x224		# set heap_end_ptr
	movl	$0x1e000, %es:0x228		# set cmd_line_ptr

copy_cmd_line:
	mov	$S_cmdline, %si
	call	getenv
	mov	$0xe000, %di
#	cld					# (side-effect of getenv)
	rep movsb				# copy cmdline into 0x1e000
	movb	$0, 0(%di)			# NUL-terminate

read_protected_mode_kernel:

	movl	%es:0x1f4, %eax			# nr of 16-byte pars to read
	shl	$4, %eax			# nr of bytes left to read
0:
	test	%eax, %eax
	jz	run_kernel

	mov	%eax, %ebx
	cmp	$(127 * 0x200), %eax		# clamp %eax to 0xfe00
	jb	1f
	mov	$(127 * 0x200), %eax

1:	sub	%eax, %ebx			# update bytes remaining
	push	%ebx
	push	%ax

	add	$511, %ax
	shr	$9, %ax
	mov	$0x20000 >>4, %bx
	mov	%bx, %es
	xor	%bx, %bx
	call	read_from_hdd			# read %ax sectors to 0x20000

	pop	%cx
	call	do_move

	pop	%eax
	jmp	0b

run_kernel:

	cli
	mov	$0x10000 >>4, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss
	mov	$0xe000, %sp
	ljmp	$0x1020, $0

read_from_hdd:					# read next %ax sectors
						#  from disk %dl
						#  to memory %es:%bx
						#  destroys reg %eax
						#  updates dap.lba

	mov	%ax, dap.count
	mov	%bx, dap.offset
	mov	%es, dap.segment
	mov	$0x42, %ah
	mov	$dap, %si
#	mov	$0x80, %dl			# BIOS will have left %dl to
						#  the disk that is booting
	int	$0x13
	mov	$'R', %al
	jc	error
#	movzw	dap.count, %eax			# old GAS doesn't know movzw?
	xor	%eax, %eax
	mov	dap.count, %ax
	add	%eax, dap.lba
	ret

do_move:					# copies lo-mem data to hi-mem
						#  Copies %cx bytes from
						#    address in gdt structure,
						#  updates gdt$dst after copy,
						#  destroys %eax, %cx, %es, %si

	push	%cx
	inc	%cx				# convert %cx to words
	shr	$1, %cx
	xor	%ax, %ax
	mov	%ax, %es			# point %es:%si at gdt
	mov	$gdt, %si
	mov	$0x87, %ah			# http://www.ctyme.com/intr/rb-1527.htm
	int	$0x15
	mov	$'M', %al
	jc	error
	xor	%eax, %eax
	pop	%ax
	addl	%eax, gdt$dst			# (assumes copylen < 4GB)
	ret

errno:	.byte	0				# last error

error:						# print %al and %ah in hex

	mov	%ah, errno			# (don't use stack)

	mov	$0xe, %ah
	xor	%bh, %bh
	int	$0x10

	mov	errno, %al
	shr	$4, %al
	or	$'0', %al
	cmp	$'9', %al
	jle	0f
	add	$'a'-'0'-10,%al
0:
	mov	$0xe, %ah
	xor	%bh, %bh
	int	$0x10

	mov	errno, %al
	and	$0xf, %al
	or	$'0', %al
	cmp	$'9', %al
	jle	0f
	add	$'a'-'0'-10,%al
0:
	mov	$0xe, %ah
	xor	%bh, %bh
	int	$0x10
halt:
	hlt
	jmp	halt


# Global Descriptor Table
# (used by do_move)

gdt:

	.zero	16
	.word	0xffff				# src seg limit in bytes
						#  (this must be >= 2*%cx-1)
gdt$src:.long	0x20000 | (0x93<<24)		# src address [0:24] and
						#  data rights bits
	.word	0
	.word	0xffff				# dest segment limit
gdt$dst:.long	0x100000 | (0x93<<24)
	.word	0
	.zero	16

# Disk Address Packet
# (used by read_from_hdd)

dap:

	.byte	0x10				# size of DAP
	.byte	0				# unused
dap.count:
	.word	0				# number of sectors
dap.offset:
	.word	0				# destination: offset
dap.segment:
	.word	0				# destination: segment
dap.lba:
	.long	1				# low bytes of LBA address
	.long	0				# high bytes of LBA address


	.org	0x1b8-5
brag:	.ascii	"FSK86"

	.org	0x1b8, 0
disksig:.long	0				# filled in by sfdisk
	.word	0

# DOS partition table
#  This table occupies a reserved range of bytes in sector 0
#  and may be updated by partition editing tools. The boot sector
#  code has to 'wrap around' this table.

# A sample partition layout for a 64MB disk is hard-coded below:
#          type start  size
#    sda1    ff     1  512 bytes	- config partition
#    sda2    83*    4  31.9 MB		- FSK partition
#    sda3    83 65536  32 MB		- /etc/config
#    sda4    00        0		- unused

	.org	0x1be, 0			# Beginning of DOS part table

part1:
	.byte	0				# status
	.zero	3				# chs_start
	.byte	0x7f				# type
	.zero	3				# chs_end
part1.lba:
	.long	1				# LBA of start
part1.nsectors:
	.long	1

part2:
	.byte	0x80				# status
	.zero	3				# chs_start
	.byte	0x83				# type
	.zero	3				# chs_end
part2.lba:
	.long	4				# LBA of start
part2.nsectors:
	.long	(32 *1024*1024/512) - 4		# 32MB - 4s

part3:
	.byte	0				# status
	.zero	3				# chs_start
	.byte	0x83				# type
	.zero	3				# chs_end
part3.lba:
	.long	(32 *1024*1024/512)		# LBA of start
part3.nsectors:
	.long	(32 *1024*1024/512)		# 32MB


	.org	0x1fe, 0
signature:
	.word	0xaa55				# required by BIOS


# The environment sector is modeled after U-Boot's environ/config block.
# Sector 1 starts with a 32-bit checksum and is followed by an unsorted
# sequence of NUL-terminated "variable=value" strings. The sequence is
# terminated by "". For example, "v1=abc\0v2=xyz\0\0" represents
#  { "v1": "abc", "v2": "xyz" }

	.org	0x200, 0
environ:
	.long	0				# reserved for checksum
						# (matches U-BOOT)
	.byte	0

	.org	0x400, 0
getenv:						# finds %ds:%si's "key"
						#  definition in environ.
						#  returns "value" in %ds:%si,
						#  sets %cx to the strlen
						#    of %ds:%si,
						#  sets %cx to zero if key was
						#    not found,
						#  destroys %ax,%di,
						#  returns %si,%cx,
						#  clears d flag (cld)

	push	%es
	xor	%ax, %ax			# %ax=0 for this procedure
	mov	%ax, %es
	mov	$environ+4, %di
	mov	$0x200-4, %cx			# sizeof environ
	cld
0:
	cmp	%al, (%di)			# exhausted environ? (ax=0)
	jne	1f
	xor	%cx, %cx			# return with cx=0
	pop	%es
	ret
1:
	push	%si
	repe cmpsb				# advance until (%si) != (%di)
	cmpb	%al, -1(%si)			# consumed all of key?
	pop	%si
	jne	2f				# branch if key not exhausted
	cmpb	$'=', -1(%di)			# matched "key=" in environ?
	je	3f				# branch if matched "key="
2:
#	mov	$'\n', %al
	repne scasb				# advance %di past the next EOS
						#  byte in environ
#	xor	%al, %al
	jmp	0b
3:
	mov	%di, %si			# %si = "value\n"
#	mov	$'\n', %al
	repne scasb				# advance %di over next EOS byte
	sub	%si, %di			# %di = strlen("value")+1
	mov	%di, %cx
	dec	%cx
	pop	%es
	ret

S_cmdline:
	.asciz	"cmdline"

	.org	0x600, 0			# pad to end of sector
