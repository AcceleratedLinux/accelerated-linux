/* Target-dependent definitions for AMD64.

   Copyright (C) 2001, 2003, 2004, 2007, 2008 Free Software Foundation, Inc.
   Contributed by Jiri Smid, SuSE Labs.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef AMD64_TDEP_H
#define AMD64_TDEP_H

struct gdbarch;
struct frame_info;
struct regcache;

#include "i386-tdep.h"

/* Register numbers of various important registers.  */

enum amd64_regnum
{
  AMD64_RAX_REGNUM,		/* %rax */
  AMD64_RBX_REGNUM,		/* %rbx */
  AMD64_RCX_REGNUM,		/* %rcx */
  AMD64_RDX_REGNUM,		/* %rdx */
  AMD64_RSI_REGNUM,		/* %rsi */
  AMD64_RDI_REGNUM,		/* %rdi */
  AMD64_RBP_REGNUM,		/* %rbp */
  AMD64_RSP_REGNUM,		/* %rsp */
  AMD64_R8_REGNUM = 8,		/* %r8 */
  AMD64_R15_REGNUM = 15,	/* %r15 */
  AMD64_RIP_REGNUM,		/* %rip */
  AMD64_EFLAGS_REGNUM,		/* %eflags */
  AMD64_CS_REGNUM,		/* %cs */
  AMD64_SS_REGNUM,		/* %ss */
  AMD64_DS_REGNUM,		/* %ds */
  AMD64_ES_REGNUM,		/* %es */
  AMD64_FS_REGNUM,		/* %fs */
  AMD64_GS_REGNUM,		/* %gs */
  AMD64_ST0_REGNUM = 24,	/* %st0 */
  AMD64_FCTRL_REGNUM = AMD64_ST0_REGNUM + 8,
  AMD64_FSTAT_REGNUM = AMD64_ST0_REGNUM + 9,
  AMD64_XMM0_REGNUM = 40,	/* %xmm0 */
  AMD64_XMM1_REGNUM,		/* %xmm1 */
  AMD64_MXCSR_REGNUM = AMD64_XMM0_REGNUM + 16
};

/* Number of general purpose registers.  */
#define AMD64_NUM_GREGS		24

extern void amd64_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch);

/* Functions from amd64-tdep.c which may be needed on architectures
   with extra registers.  */

extern const char *amd64_register_name (struct gdbarch *gdbarch, int regnum);
extern struct type *amd64_register_type (struct gdbarch *gdbarch, int regnum);

/* Fill register REGNUM in REGCACHE with the appropriate
   floating-point or SSE register value from *FXSAVE.  If REGNUM is
   -1, do this for all registers.  This function masks off any of the
   reserved bits in *FXSAVE.  */

extern void amd64_supply_fxsave (struct regcache *regcache, int regnum,
				 const void *fxsave);

/* Fill register REGNUM (if it is a floating-point or SSE register) in
   *FXSAVE with the value from REGCACHE.  If REGNUM is -1, do this for
   all registers.  This function doesn't touch any of the reserved
   bits in *FXSAVE.  */

extern void amd64_collect_fxsave (const struct regcache *regcache, int regnum,
				  void *fxsave);


/* Variables exported from amd64nbsd-tdep.c.  */
extern int amd64nbsd_r_reg_offset[];

/* Variables exported from amd64obsd-tdep.c.  */
extern int amd64obsd_r_reg_offset[];

/* Variables exported from amd64fbsd-tdep.c.  */
extern CORE_ADDR amd64fbsd_sigtramp_start_addr;
extern CORE_ADDR amd64fbsd_sigtramp_end_addr;
extern int amd64fbsd_sc_reg_offset[];

#endif /* amd64-tdep.h */
