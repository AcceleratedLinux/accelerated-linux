/* Target-dependent code for GDB, the GNU debugger.

   Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008
   Free Software Foundation, Inc.

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

#ifndef PPC_TDEP_H
#define PPC_TDEP_H

struct gdbarch;
struct frame_info;
struct value;
struct regcache;
struct type;

/* From ppc-linux-tdep.c... */
enum return_value_convention ppc_sysv_abi_return_value (struct gdbarch *gdbarch,
							struct type *valtype,
							struct regcache *regcache,
							gdb_byte *readbuf,
							const gdb_byte *writebuf);
enum return_value_convention ppc_sysv_abi_broken_return_value (struct gdbarch *gdbarch,
							       struct type *valtype,
							       struct regcache *regcache,
							       gdb_byte *readbuf,
							       const gdb_byte *writebuf);
CORE_ADDR ppc_sysv_abi_push_dummy_call (struct gdbarch *gdbarch,
					struct value *function,
					struct regcache *regcache,
					CORE_ADDR bp_addr, int nargs,
					struct value **args, CORE_ADDR sp,
					int struct_return,
					CORE_ADDR struct_addr);
CORE_ADDR ppc64_sysv_abi_push_dummy_call (struct gdbarch *gdbarch,
					  struct value *function,
					  struct regcache *regcache,
					  CORE_ADDR bp_addr, int nargs,
					  struct value **args, CORE_ADDR sp,
					  int struct_return,
					  CORE_ADDR struct_addr);
CORE_ADDR ppc64_sysv_abi_adjust_breakpoint_address (struct gdbarch *gdbarch,
						    CORE_ADDR bpaddr);
int ppc_linux_memory_remove_breakpoint (struct gdbarch *, struct bp_target_info *);
struct link_map_offsets *ppc_linux_svr4_fetch_link_map_offsets (void);
const struct regset *ppc_linux_gregset (int);
const struct regset *ppc_linux_fpregset (void);

enum return_value_convention ppc64_sysv_abi_return_value (struct gdbarch *gdbarch,
							  struct type *valtype,
							  struct regcache *regcache,
							  gdb_byte *readbuf,
							  const gdb_byte *writebuf);

/* From rs6000-tdep.c... */
int altivec_register_p (struct gdbarch *gdbarch, int regno);
int spe_register_p (struct gdbarch *gdbarch, int regno);

/* Return non-zero if the architecture described by GDBARCH has
   floating-point registers (f0 --- f31 and fpscr).  */
int ppc_floating_point_unit_p (struct gdbarch *gdbarch);

/* Return non-zero if the architecture described by GDBARCH has
   Altivec registers (vr0 --- vr31, vrsave and vscr).  */
int ppc_altivec_support_p (struct gdbarch *gdbarch);

/* Register set description.  */

struct ppc_reg_offsets
{
  /* General-purpose registers.  */
  int r0_offset;
  int gpr_size; /* size for r0-31, pc, ps, lr, ctr.  */
  int xr_size;  /* size for cr, xer, mq.  */
  int pc_offset;
  int ps_offset;
  int cr_offset;
  int lr_offset;
  int ctr_offset;
  int xer_offset;
  int mq_offset;

  /* Floating-point registers.  */
  int f0_offset;
  int fpscr_offset;
  int fpscr_size;

  /* AltiVec registers.  */
  int vr0_offset;
  int vscr_offset;
  int vrsave_offset;
};

/* Supply register REGNUM in the general-purpose register set REGSET
   from the buffer specified by GREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

extern void ppc_supply_gregset (const struct regset *regset,
				struct regcache *regcache,
				int regnum, const void *gregs, size_t len);

/* Supply register REGNUM in the floating-point register set REGSET
   from the buffer specified by FPREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

extern void ppc_supply_fpregset (const struct regset *regset,
				 struct regcache *regcache,
				 int regnum, const void *fpregs, size_t len);

/* Supply register REGNUM in the Altivec register set REGSET
   from the buffer specified by VRREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

extern void ppc_supply_vrregset (const struct regset *regset,
				 struct regcache *regcache,
				 int regnum, const void *vrregs, size_t len);

/* Collect register REGNUM in the general-purpose register set
   REGSET. from register cache REGCACHE into the buffer specified by
   GREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

extern void ppc_collect_gregset (const struct regset *regset,
				 const struct regcache *regcache,
				 int regnum, void *gregs, size_t len);

/* Collect register REGNUM in the floating-point register set
   REGSET. from register cache REGCACHE into the buffer specified by
   FPREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

extern void ppc_collect_fpregset (const struct regset *regset,
				  const struct regcache *regcache,
				  int regnum, void *fpregs, size_t len);

/* Collect register REGNUM in the Altivec register set
   REGSET from register cache REGCACHE into the buffer specified by
   VRREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

extern void ppc_collect_vrregset (const struct regset *regset,
				  const struct regcache *regcache,
				  int regnum, void *vrregs, size_t len);

/* Private data that this module attaches to struct gdbarch. */

/* Vector ABI used by the inferior.  */
enum powerpc_vector_abi
{
  POWERPC_VEC_AUTO,
  POWERPC_VEC_GENERIC,
  POWERPC_VEC_ALTIVEC,
  POWERPC_VEC_SPE,
  POWERPC_VEC_LAST
};

struct gdbarch_tdep
  {
    int wordsize;		/* Size in bytes of fixed-point word.  */
    int soft_float;		/* Avoid FP registers for arguments?  */

    /* How to pass vector arguments.  Never set to AUTO or LAST.  */
    enum powerpc_vector_abi vector_abi;

    int ppc_gp0_regnum;		/* GPR register 0 */
    int ppc_toc_regnum;		/* TOC register */
    int ppc_ps_regnum;	        /* Processor (or machine) status (%msr) */
    int ppc_cr_regnum;		/* Condition register */
    int ppc_lr_regnum;		/* Link register */
    int ppc_ctr_regnum;		/* Count register */
    int ppc_xer_regnum;		/* Integer exception register */

    /* Not all PPC and RS6000 variants will have the registers
       represented below.  A -1 is used to indicate that the register
       is not present in this variant.  */

    /* Floating-point registers.  */
    int ppc_fp0_regnum;         /* floating-point register 0 */
    int ppc_fpscr_regnum;	/* fp status and condition register */

    /* Multiplier-Quotient Register (older POWER architectures only).  */
    int ppc_mq_regnum;

    /* Altivec registers.  */
    int ppc_vr0_regnum;		/* First AltiVec register */
    int ppc_vrsave_regnum;	/* Last AltiVec register */

    /* SPE registers.  */
    int ppc_ev0_upper_regnum;   /* First GPR upper half register */
    int ppc_ev0_regnum;         /* First ev register */
    int ppc_acc_regnum;         /* SPE 'acc' register */
    int ppc_spefscr_regnum;     /* SPE 'spefscr' register */

    /* Decimal 128 registers.  */
    int ppc_dl0_regnum;		/* First Decimal128 argument register pair.  */

    /* Offset to ABI specific location where link register is saved.  */
    int lr_frame_offset;	

    /* An array of integers, such that sim_regno[I] is the simulator
       register number for GDB register number I, or -1 if the
       simulator does not implement that register.  */
    int *sim_regno;

    /* Minimum possible text address.  */
    CORE_ADDR text_segment_base;

    /* ISA-specific types.  */
    struct type *ppc_builtin_type_vec64;
};


/* Constants for register set sizes.  */
enum
  {
    ppc_num_gprs = 32,          /* 32 general-purpose registers */
    ppc_num_fprs = 32,          /* 32 floating-point registers */
    ppc_num_srs = 16,           /* 16 segment registers */
    ppc_num_vrs = 32            /* 32 Altivec vector registers */
  };


/* Register number constants.  These are GDB internal register
   numbers; they are not used for the simulator or remote targets.
   Extra SPRs (those other than MQ, CTR, LR, XER, SPEFSCR) are given
   numbers above PPC_NUM_REGS.  So are segment registers and other
   target-defined registers.  */
enum {
  PPC_R0_REGNUM = 0,
  PPC_F0_REGNUM = 32,
  PPC_PC_REGNUM = 64,
  PPC_MSR_REGNUM = 65,
  PPC_CR_REGNUM = 66,
  PPC_LR_REGNUM = 67,
  PPC_CTR_REGNUM = 68,
  PPC_XER_REGNUM = 69,
  PPC_FPSCR_REGNUM = 70,
  PPC_MQ_REGNUM = 71,
  PPC_SPE_UPPER_GP0_REGNUM = 72,
  PPC_SPE_ACC_REGNUM = 104,
  PPC_SPE_FSCR_REGNUM = 105,
  PPC_VR0_REGNUM = 106,
  PPC_VSCR_REGNUM = 138,
  PPC_VRSAVE_REGNUM = 139,
  PPC_NUM_REGS
};


/* Instruction size.  */
#define PPC_INSN_SIZE 4

/* Estimate for the maximum number of instrctions in a function epilogue.  */
#define PPC_MAX_EPILOGUE_INSTRUCTIONS  52

extern struct target_desc *tdesc_powerpc_e500;

#endif /* ppc-tdep.h */
