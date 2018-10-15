/* Target-dependent code for GDB, the GNU debugger.

   Copyright (C) 1986, 1987, 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
   1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
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

#include "defs.h"
#include "frame.h"
#include "inferior.h"
#include "symtab.h"
#include "target.h"
#include "gdbcore.h"
#include "gdbcmd.h"
#include "objfiles.h"
#include "arch-utils.h"
#include "regcache.h"
#include "regset.h"
#include "doublest.h"
#include "value.h"
#include "parser-defs.h"
#include "osabi.h"
#include "infcall.h"
#include "sim-regno.h"
#include "gdb/sim-ppc.h"
#include "reggroups.h"
#include "dwarf2-frame.h"
#include "target-descriptions.h"
#include "user-regs.h"

#include "libbfd.h"		/* for bfd_default_set_arch_mach */
#include "coff/internal.h"	/* for libcoff.h */
#include "libcoff.h"		/* for xcoff_data */
#include "coff/xcoff.h"
#include "libxcoff.h"

#include "elf-bfd.h"
#include "elf/ppc.h"

#include "solib-svr4.h"
#include "ppc-tdep.h"

#include "gdb_assert.h"
#include "dis-asm.h"

#include "trad-frame.h"
#include "frame-unwind.h"
#include "frame-base.h"

#include "rs6000-tdep.h"

#include "features/rs6000/powerpc-32.c"
#include "features/rs6000/powerpc-403.c"
#include "features/rs6000/powerpc-403gc.c"
#include "features/rs6000/powerpc-505.c"
#include "features/rs6000/powerpc-601.c"
#include "features/rs6000/powerpc-602.c"
#include "features/rs6000/powerpc-603.c"
#include "features/rs6000/powerpc-604.c"
#include "features/rs6000/powerpc-64.c"
#include "features/rs6000/powerpc-7400.c"
#include "features/rs6000/powerpc-750.c"
#include "features/rs6000/powerpc-860.c"
#include "features/rs6000/powerpc-e500.c"
#include "features/rs6000/rs6000.c"

/* Determine if regnum is an SPE pseudo-register.  */
#define IS_SPE_PSEUDOREG(tdep, regnum) ((tdep)->ppc_ev0_regnum >= 0 \
    && (regnum) >= (tdep)->ppc_ev0_regnum \
    && (regnum) < (tdep)->ppc_ev0_regnum + 32)

/* Determine if regnum is a decimal float pseudo-register.  */
#define IS_DFP_PSEUDOREG(tdep, regnum) ((tdep)->ppc_dl0_regnum >= 0 \
    && (regnum) >= (tdep)->ppc_dl0_regnum \
    && (regnum) < (tdep)->ppc_dl0_regnum + 16)

/* The list of available "set powerpc ..." and "show powerpc ..."
   commands.  */
static struct cmd_list_element *setpowerpccmdlist = NULL;
static struct cmd_list_element *showpowerpccmdlist = NULL;

static enum auto_boolean powerpc_soft_float_global = AUTO_BOOLEAN_AUTO;

/* The vector ABI to use.  Keep this in sync with powerpc_vector_abi.  */
static const char *powerpc_vector_strings[] =
{
  "auto",
  "generic",
  "altivec",
  "spe",
  NULL
};

/* A variable that can be configured by the user.  */
static enum powerpc_vector_abi powerpc_vector_abi_global = POWERPC_VEC_AUTO;
static const char *powerpc_vector_abi_string = "auto";

/* If the kernel has to deliver a signal, it pushes a sigcontext
   structure on the stack and then calls the signal handler, passing
   the address of the sigcontext in an argument register. Usually
   the signal handler doesn't save this register, so we have to
   access the sigcontext structure via an offset from the signal handler
   frame.
   The following constants were determined by experimentation on AIX 3.2.  */
#define SIG_FRAME_PC_OFFSET 96
#define SIG_FRAME_LR_OFFSET 108
#define SIG_FRAME_FP_OFFSET 284

/* To be used by skip_prologue. */

struct rs6000_framedata
  {
    int offset;			/* total size of frame --- the distance
				   by which we decrement sp to allocate
				   the frame */
    int saved_gpr;		/* smallest # of saved gpr */
    int saved_fpr;		/* smallest # of saved fpr */
    int saved_vr;               /* smallest # of saved vr */
    int saved_ev;               /* smallest # of saved ev */
    int alloca_reg;		/* alloca register number (frame ptr) */
    char frameless;		/* true if frameless functions. */
    char nosavedpc;		/* true if pc not saved. */
    int gpr_offset;		/* offset of saved gprs from prev sp */
    int fpr_offset;		/* offset of saved fprs from prev sp */
    int vr_offset;              /* offset of saved vrs from prev sp */
    int ev_offset;              /* offset of saved evs from prev sp */
    int lr_offset;		/* offset of saved lr */
    int cr_offset;		/* offset of saved cr */
    int vrsave_offset;          /* offset of saved vrsave register */
  };

/* Description of a single register. */

struct reg
  {
    char *name;			/* name of register */
    unsigned char sz32;		/* size on 32-bit arch, 0 if nonexistent */
    unsigned char sz64;		/* size on 64-bit arch, 0 if nonexistent */
    unsigned char fpr;		/* whether register is floating-point */
    unsigned char pseudo;       /* whether register is pseudo */
    int spr_num;                /* PowerPC SPR number, or -1 if not an SPR.
                                   This is an ISA SPR number, not a GDB
                                   register number.  */
  };

/* Hook for determining the TOC address when calling functions in the
   inferior under AIX. The initialization code in rs6000-nat.c sets
   this hook to point to find_toc_address.  */

CORE_ADDR (*rs6000_find_toc_address_hook) (CORE_ADDR) = NULL;

/* Static function prototypes */

static CORE_ADDR branch_dest (struct frame_info *frame, int opcode,
			      int instr, CORE_ADDR pc, CORE_ADDR safety);
static CORE_ADDR skip_prologue (struct gdbarch *, CORE_ADDR, CORE_ADDR,
                                struct rs6000_framedata *);

/* Is REGNO an AltiVec register?  Return 1 if so, 0 otherwise.  */
int
altivec_register_p (struct gdbarch *gdbarch, int regno)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  if (tdep->ppc_vr0_regnum < 0 || tdep->ppc_vrsave_regnum < 0)
    return 0;
  else
    return (regno >= tdep->ppc_vr0_regnum && regno <= tdep->ppc_vrsave_regnum);
}


/* Return true if REGNO is an SPE register, false otherwise.  */
int
spe_register_p (struct gdbarch *gdbarch, int regno)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  
  /* Is it a reference to EV0 -- EV31, and do we have those?  */
  if (IS_SPE_PSEUDOREG (tdep, regno))
    return 1;

  /* Is it a reference to one of the raw upper GPR halves?  */
  if (tdep->ppc_ev0_upper_regnum >= 0
      && tdep->ppc_ev0_upper_regnum <= regno
      && regno < tdep->ppc_ev0_upper_regnum + ppc_num_gprs)
    return 1;

  /* Is it a reference to the 64-bit accumulator, and do we have that?  */
  if (tdep->ppc_acc_regnum >= 0
      && tdep->ppc_acc_regnum == regno)
    return 1;

  /* Is it a reference to the SPE floating-point status and control register,
     and do we have that?  */
  if (tdep->ppc_spefscr_regnum >= 0
      && tdep->ppc_spefscr_regnum == regno)
    return 1;

  return 0;
}


/* Return non-zero if the architecture described by GDBARCH has
   floating-point registers (f0 --- f31 and fpscr).  */
int
ppc_floating_point_unit_p (struct gdbarch *gdbarch)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  return (tdep->ppc_fp0_regnum >= 0
          && tdep->ppc_fpscr_regnum >= 0);
}

/* Return non-zero if the architecture described by GDBARCH has
   Altivec registers (vr0 --- vr31, vrsave and vscr).  */
int
ppc_altivec_support_p (struct gdbarch *gdbarch)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  return (tdep->ppc_vr0_regnum >= 0
          && tdep->ppc_vrsave_regnum >= 0);
}

/* Check that TABLE[GDB_REGNO] is not already initialized, and then
   set it to SIM_REGNO.

   This is a helper function for init_sim_regno_table, constructing
   the table mapping GDB register numbers to sim register numbers; we
   initialize every element in that table to -1 before we start
   filling it in.  */
static void
set_sim_regno (int *table, int gdb_regno, int sim_regno)
{
  /* Make sure we don't try to assign any given GDB register a sim
     register number more than once.  */
  gdb_assert (table[gdb_regno] == -1);
  table[gdb_regno] = sim_regno;
}


/* Initialize ARCH->tdep->sim_regno, the table mapping GDB register
   numbers to simulator register numbers, based on the values placed
   in the ARCH->tdep->ppc_foo_regnum members.  */
static void
init_sim_regno_table (struct gdbarch *arch)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (arch);
  int total_regs = gdbarch_num_regs (arch);
  int *sim_regno = GDBARCH_OBSTACK_CALLOC (arch, total_regs, int);
  int i;
  static const char *const segment_regs[] = {
    "sr0", "sr1", "sr2", "sr3", "sr4", "sr5", "sr6", "sr7",
    "sr8", "sr9", "sr10", "sr11", "sr12", "sr13", "sr14", "sr15"
  };

  /* Presume that all registers not explicitly mentioned below are
     unavailable from the sim.  */
  for (i = 0; i < total_regs; i++)
    sim_regno[i] = -1;

  /* General-purpose registers.  */
  for (i = 0; i < ppc_num_gprs; i++)
    set_sim_regno (sim_regno, tdep->ppc_gp0_regnum + i, sim_ppc_r0_regnum + i);
  
  /* Floating-point registers.  */
  if (tdep->ppc_fp0_regnum >= 0)
    for (i = 0; i < ppc_num_fprs; i++)
      set_sim_regno (sim_regno,
                     tdep->ppc_fp0_regnum + i,
                     sim_ppc_f0_regnum + i);
  if (tdep->ppc_fpscr_regnum >= 0)
    set_sim_regno (sim_regno, tdep->ppc_fpscr_regnum, sim_ppc_fpscr_regnum);

  set_sim_regno (sim_regno, gdbarch_pc_regnum (arch), sim_ppc_pc_regnum);
  set_sim_regno (sim_regno, tdep->ppc_ps_regnum, sim_ppc_ps_regnum);
  set_sim_regno (sim_regno, tdep->ppc_cr_regnum, sim_ppc_cr_regnum);

  /* Segment registers.  */
  for (i = 0; i < ppc_num_srs; i++)
    {
      int gdb_regno;

      gdb_regno = user_reg_map_name_to_regnum (arch, segment_regs[i], -1);
      if (gdb_regno >= 0)
	set_sim_regno (sim_regno, gdb_regno, sim_ppc_sr0_regnum + i);
    }

  /* Altivec registers.  */
  if (tdep->ppc_vr0_regnum >= 0)
    {
      for (i = 0; i < ppc_num_vrs; i++)
        set_sim_regno (sim_regno,
                       tdep->ppc_vr0_regnum + i,
                       sim_ppc_vr0_regnum + i);

      /* FIXME: jimb/2004-07-15: when we have tdep->ppc_vscr_regnum,
         we can treat this more like the other cases.  */
      set_sim_regno (sim_regno,
                     tdep->ppc_vr0_regnum + ppc_num_vrs,
                     sim_ppc_vscr_regnum);
    }
  /* vsave is a special-purpose register, so the code below handles it.  */

  /* SPE APU (E500) registers.  */
  if (tdep->ppc_ev0_upper_regnum >= 0)
    for (i = 0; i < ppc_num_gprs; i++)
      set_sim_regno (sim_regno,
                     tdep->ppc_ev0_upper_regnum + i,
                     sim_ppc_rh0_regnum + i);
  if (tdep->ppc_acc_regnum >= 0)
    set_sim_regno (sim_regno, tdep->ppc_acc_regnum, sim_ppc_acc_regnum);
  /* spefscr is a special-purpose register, so the code below handles it.  */

#ifdef WITH_SIM
  /* Now handle all special-purpose registers.  Verify that they
     haven't mistakenly been assigned numbers by any of the above
     code.  */
  for (i = 0; i < sim_ppc_num_sprs; i++)
    {
      const char *spr_name = sim_spr_register_name (i);
      int gdb_regno = -1;

      if (spr_name != NULL)
	gdb_regno = user_reg_map_name_to_regnum (arch, spr_name, -1);

      if (gdb_regno != -1)
	set_sim_regno (sim_regno, gdb_regno, sim_ppc_spr0_regnum + i);
    }
#endif

  /* Drop the initialized array into place.  */
  tdep->sim_regno = sim_regno;
}


/* Given a GDB register number REG, return the corresponding SIM
   register number.  */
static int
rs6000_register_sim_regno (struct gdbarch *gdbarch, int reg)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int sim_regno;

  if (tdep->sim_regno == NULL)
    init_sim_regno_table (gdbarch);

  gdb_assert (0 <= reg 
	      && reg <= gdbarch_num_regs (gdbarch)
			+ gdbarch_num_pseudo_regs (gdbarch));
  sim_regno = tdep->sim_regno[reg];

  if (sim_regno >= 0)
    return sim_regno;
  else
    return LEGACY_SIM_REGNO_IGNORE;
}



/* Register set support functions.  */

/* REGS + OFFSET contains register REGNUM in a field REGSIZE wide.
   Write the register to REGCACHE.  */

static void
ppc_supply_reg (struct regcache *regcache, int regnum, 
		const gdb_byte *regs, size_t offset, int regsize)
{
  if (regnum != -1 && offset != -1)
    {
      if (regsize > 4)
	{
	  struct gdbarch *gdbarch = get_regcache_arch (regcache);
	  int gdb_regsize = register_size (gdbarch, regnum);
	  if (gdb_regsize < regsize
	      && gdbarch_byte_order (gdbarch) == BFD_ENDIAN_BIG)
	    offset += regsize - gdb_regsize;
	}
      regcache_raw_supply (regcache, regnum, regs + offset);
    }
}

/* Read register REGNUM from REGCACHE and store to REGS + OFFSET
   in a field REGSIZE wide.  Zero pad as necessary.  */

static void
ppc_collect_reg (const struct regcache *regcache, int regnum,
		 gdb_byte *regs, size_t offset, int regsize)
{
  if (regnum != -1 && offset != -1)
    {
      if (regsize > 4)
	{
	  struct gdbarch *gdbarch = get_regcache_arch (regcache);
	  int gdb_regsize = register_size (gdbarch, regnum);
	  if (gdb_regsize < regsize)
	    {
	      if (gdbarch_byte_order (gdbarch) == BFD_ENDIAN_BIG)
		{
		  memset (regs + offset, 0, regsize - gdb_regsize);
		  offset += regsize - gdb_regsize;
		}
	      else
		memset (regs + offset + regsize - gdb_regsize, 0,
			regsize - gdb_regsize);
	    }
	}
      regcache_raw_collect (regcache, regnum, regs + offset);
    }
}
    
static int
ppc_greg_offset (struct gdbarch *gdbarch,
		 struct gdbarch_tdep *tdep,
		 const struct ppc_reg_offsets *offsets,
		 int regnum,
		 int *regsize)
{
  *regsize = offsets->gpr_size;
  if (regnum >= tdep->ppc_gp0_regnum
      && regnum < tdep->ppc_gp0_regnum + ppc_num_gprs)
    return (offsets->r0_offset
	    + (regnum - tdep->ppc_gp0_regnum) * offsets->gpr_size);

  if (regnum == gdbarch_pc_regnum (gdbarch))
    return offsets->pc_offset;

  if (regnum == tdep->ppc_ps_regnum)
    return offsets->ps_offset;

  if (regnum == tdep->ppc_lr_regnum)
    return offsets->lr_offset;

  if (regnum == tdep->ppc_ctr_regnum)
    return offsets->ctr_offset;

  *regsize = offsets->xr_size;
  if (regnum == tdep->ppc_cr_regnum)
    return offsets->cr_offset;

  if (regnum == tdep->ppc_xer_regnum)
    return offsets->xer_offset;

  if (regnum == tdep->ppc_mq_regnum)
    return offsets->mq_offset;

  return -1;
}

static int
ppc_fpreg_offset (struct gdbarch_tdep *tdep,
		  const struct ppc_reg_offsets *offsets,
		  int regnum)
{
  if (regnum >= tdep->ppc_fp0_regnum
      && regnum < tdep->ppc_fp0_regnum + ppc_num_fprs)
    return offsets->f0_offset + (regnum - tdep->ppc_fp0_regnum) * 8;

  if (regnum == tdep->ppc_fpscr_regnum)
    return offsets->fpscr_offset;

  return -1;
}

static int
ppc_vrreg_offset (struct gdbarch_tdep *tdep,
		  const struct ppc_reg_offsets *offsets,
		  int regnum)
{
  if (regnum >= tdep->ppc_vr0_regnum
      && regnum < tdep->ppc_vr0_regnum + ppc_num_vrs)
    return offsets->vr0_offset + (regnum - tdep->ppc_vr0_regnum) * 16;

  if (regnum == tdep->ppc_vrsave_regnum - 1)
    return offsets->vscr_offset;

  if (regnum == tdep->ppc_vrsave_regnum)
    return offsets->vrsave_offset;

  return -1;
}

/* Supply register REGNUM in the general-purpose register set REGSET
   from the buffer specified by GREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

void
ppc_supply_gregset (const struct regset *regset, struct regcache *regcache,
		    int regnum, const void *gregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  const struct ppc_reg_offsets *offsets = regset->descr;
  size_t offset;
  int regsize;

  if (regnum == -1)
    {
      int i;
      int gpr_size = offsets->gpr_size;

      for (i = tdep->ppc_gp0_regnum, offset = offsets->r0_offset;
	   i < tdep->ppc_gp0_regnum + ppc_num_gprs;
	   i++, offset += gpr_size)
	ppc_supply_reg (regcache, i, gregs, offset, gpr_size);

      ppc_supply_reg (regcache, gdbarch_pc_regnum (gdbarch),
		      gregs, offsets->pc_offset, gpr_size);
      ppc_supply_reg (regcache, tdep->ppc_ps_regnum,
		      gregs, offsets->ps_offset, gpr_size);
      ppc_supply_reg (regcache, tdep->ppc_lr_regnum,
		      gregs, offsets->lr_offset, gpr_size);
      ppc_supply_reg (regcache, tdep->ppc_ctr_regnum,
		      gregs, offsets->ctr_offset, gpr_size);
      ppc_supply_reg (regcache, tdep->ppc_cr_regnum,
		      gregs, offsets->cr_offset, offsets->xr_size);
      ppc_supply_reg (regcache, tdep->ppc_xer_regnum,
		      gregs, offsets->xer_offset, offsets->xr_size);
      ppc_supply_reg (regcache, tdep->ppc_mq_regnum,
		      gregs, offsets->mq_offset, offsets->xr_size);
      return;
    }

  offset = ppc_greg_offset (gdbarch, tdep, offsets, regnum, &regsize);
  ppc_supply_reg (regcache, regnum, gregs, offset, regsize);
}

/* Supply register REGNUM in the floating-point register set REGSET
   from the buffer specified by FPREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

void
ppc_supply_fpregset (const struct regset *regset, struct regcache *regcache,
		     int regnum, const void *fpregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep;
  const struct ppc_reg_offsets *offsets;
  size_t offset;

  if (!ppc_floating_point_unit_p (gdbarch))
    return;

  tdep = gdbarch_tdep (gdbarch);
  offsets = regset->descr;
  if (regnum == -1)
    {
      int i;

      for (i = tdep->ppc_fp0_regnum, offset = offsets->f0_offset;
	   i < tdep->ppc_fp0_regnum + ppc_num_fprs;
	   i++, offset += 8)
	ppc_supply_reg (regcache, i, fpregs, offset, 8);

      ppc_supply_reg (regcache, tdep->ppc_fpscr_regnum,
		      fpregs, offsets->fpscr_offset, offsets->fpscr_size);
      return;
    }

  offset = ppc_fpreg_offset (tdep, offsets, regnum);
  ppc_supply_reg (regcache, regnum, fpregs, offset,
		  regnum == tdep->ppc_fpscr_regnum ? offsets->fpscr_size : 8);
}

/* Supply register REGNUM in the Altivec register set REGSET
   from the buffer specified by VRREGS and LEN to register cache
   REGCACHE.  If REGNUM is -1, do this for all registers in REGSET.  */

void
ppc_supply_vrregset (const struct regset *regset, struct regcache *regcache,
		     int regnum, const void *vrregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep;
  const struct ppc_reg_offsets *offsets;
  size_t offset;

  if (!ppc_altivec_support_p (gdbarch))
    return;

  tdep = gdbarch_tdep (gdbarch);
  offsets = regset->descr;
  if (regnum == -1)
    {
      int i;

      for (i = tdep->ppc_vr0_regnum, offset = offsets->vr0_offset;
	   i < tdep->ppc_vr0_regnum + ppc_num_vrs;
	   i++, offset += 16)
        ppc_supply_reg (regcache, i, vrregs, offset, 16);

      ppc_supply_reg (regcache, (tdep->ppc_vrsave_regnum - 1),
		      vrregs, offsets->vscr_offset, 4);

      ppc_supply_reg (regcache, tdep->ppc_vrsave_regnum,
		      vrregs, offsets->vrsave_offset, 4);
      return;
    }

  offset = ppc_vrreg_offset (tdep, offsets, regnum);
  if (regnum != tdep->ppc_vrsave_regnum
      && regnum != tdep->ppc_vrsave_regnum - 1)
    ppc_supply_reg (regcache, regnum, vrregs, offset, 16);
  else
    ppc_supply_reg (regcache, regnum,
		    vrregs, offset, 4);
}

/* Collect register REGNUM in the general-purpose register set
   REGSET from register cache REGCACHE into the buffer specified by
   GREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

void
ppc_collect_gregset (const struct regset *regset,
		     const struct regcache *regcache,
		     int regnum, void *gregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  const struct ppc_reg_offsets *offsets = regset->descr;
  size_t offset;
  int regsize;

  if (regnum == -1)
    {
      int i;
      int gpr_size = offsets->gpr_size;

      for (i = tdep->ppc_gp0_regnum, offset = offsets->r0_offset;
	   i < tdep->ppc_gp0_regnum + ppc_num_gprs;
	   i++, offset += gpr_size)
	ppc_collect_reg (regcache, i, gregs, offset, gpr_size);

      ppc_collect_reg (regcache, gdbarch_pc_regnum (gdbarch),
		       gregs, offsets->pc_offset, gpr_size);
      ppc_collect_reg (regcache, tdep->ppc_ps_regnum,
		       gregs, offsets->ps_offset, gpr_size);
      ppc_collect_reg (regcache, tdep->ppc_lr_regnum,
		       gregs, offsets->lr_offset, gpr_size);
      ppc_collect_reg (regcache, tdep->ppc_ctr_regnum,
		       gregs, offsets->ctr_offset, gpr_size);
      ppc_collect_reg (regcache, tdep->ppc_cr_regnum,
		       gregs, offsets->cr_offset, offsets->xr_size);
      ppc_collect_reg (regcache, tdep->ppc_xer_regnum,
		       gregs, offsets->xer_offset, offsets->xr_size);
      ppc_collect_reg (regcache, tdep->ppc_mq_regnum,
		       gregs, offsets->mq_offset, offsets->xr_size);
      return;
    }

  offset = ppc_greg_offset (gdbarch, tdep, offsets, regnum, &regsize);
  ppc_collect_reg (regcache, regnum, gregs, offset, regsize);
}

/* Collect register REGNUM in the floating-point register set
   REGSET from register cache REGCACHE into the buffer specified by
   FPREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

void
ppc_collect_fpregset (const struct regset *regset,
		      const struct regcache *regcache,
		      int regnum, void *fpregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep;
  const struct ppc_reg_offsets *offsets;
  size_t offset;

  if (!ppc_floating_point_unit_p (gdbarch))
    return;

  tdep = gdbarch_tdep (gdbarch);
  offsets = regset->descr;
  if (regnum == -1)
    {
      int i;

      for (i = tdep->ppc_fp0_regnum, offset = offsets->f0_offset;
	   i < tdep->ppc_fp0_regnum + ppc_num_fprs;
	   i++, offset += 8)
	ppc_collect_reg (regcache, i, fpregs, offset, 8);

      ppc_collect_reg (regcache, tdep->ppc_fpscr_regnum,
		       fpregs, offsets->fpscr_offset, offsets->fpscr_size);
      return;
    }

  offset = ppc_fpreg_offset (tdep, offsets, regnum);
  ppc_collect_reg (regcache, regnum, fpregs, offset,
		   regnum == tdep->ppc_fpscr_regnum ? offsets->fpscr_size : 8);
}

/* Collect register REGNUM in the Altivec register set
   REGSET from register cache REGCACHE into the buffer specified by
   VRREGS and LEN.  If REGNUM is -1, do this for all registers in
   REGSET.  */

void
ppc_collect_vrregset (const struct regset *regset,
		      const struct regcache *regcache,
		      int regnum, void *vrregs, size_t len)
{
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep;
  const struct ppc_reg_offsets *offsets;
  size_t offset;

  if (!ppc_altivec_support_p (gdbarch))
    return;

  tdep = gdbarch_tdep (gdbarch);
  offsets = regset->descr;
  if (regnum == -1)
    {
      int i;

      for (i = tdep->ppc_vr0_regnum, offset = offsets->vr0_offset;
	   i < tdep->ppc_vr0_regnum + ppc_num_vrs;
	   i++, offset += 16)
	ppc_collect_reg (regcache, i, vrregs, offset, 16);

      ppc_collect_reg (regcache, (tdep->ppc_vrsave_regnum - 1),
		       vrregs, offsets->vscr_offset, 4);

      ppc_collect_reg (regcache, tdep->ppc_vrsave_regnum,
		       vrregs, offsets->vrsave_offset, 4);
      return;
    }

  offset = ppc_vrreg_offset (tdep, offsets, regnum);
  if (regnum != tdep->ppc_vrsave_regnum
      && regnum != tdep->ppc_vrsave_regnum - 1)
    ppc_collect_reg (regcache, regnum, vrregs, offset, 16);
  else
    ppc_collect_reg (regcache, regnum,
		    vrregs, offset, 4);
}


/* Read a LEN-byte address from debugged memory address MEMADDR. */

static CORE_ADDR
read_memory_addr (CORE_ADDR memaddr, int len)
{
  return read_memory_unsigned_integer (memaddr, len);
}

static CORE_ADDR
rs6000_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  struct rs6000_framedata frame;
  CORE_ADDR limit_pc, func_addr;

  /* See if we can determine the end of the prologue via the symbol table.
     If so, then return either PC, or the PC after the prologue, whichever
     is greater.  */
  if (find_pc_partial_function (pc, NULL, &func_addr, NULL))
    {
      CORE_ADDR post_prologue_pc = skip_prologue_using_sal (func_addr);
      if (post_prologue_pc != 0)
	return max (pc, post_prologue_pc);
    }

  /* Can't determine prologue from the symbol table, need to examine
     instructions.  */

  /* Find an upper limit on the function prologue using the debug
     information.  If the debug information could not be used to provide
     that bound, then use an arbitrary large number as the upper bound.  */
  limit_pc = skip_prologue_using_sal (pc);
  if (limit_pc == 0)
    limit_pc = pc + 100;          /* Magic.  */

  pc = skip_prologue (gdbarch, pc, limit_pc, &frame);
  return pc;
}

static int
insn_changes_sp_or_jumps (unsigned long insn)
{
  int opcode = (insn >> 26) & 0x03f;
  int sd = (insn >> 21) & 0x01f;
  int a = (insn >> 16) & 0x01f;
  int subcode = (insn >> 1) & 0x3ff;

  /* Changes the stack pointer.  */

  /* NOTE: There are many ways to change the value of a given register.
           The ways below are those used when the register is R1, the SP,
           in a funtion's epilogue.  */

  if (opcode == 31 && subcode == 444 && a == 1)
    return 1;  /* mr R1,Rn */
  if (opcode == 14 && sd == 1)
    return 1;  /* addi R1,Rn,simm */
  if (opcode == 58 && sd == 1)
    return 1;  /* ld R1,ds(Rn) */

  /* Transfers control.  */

  if (opcode == 18)
    return 1;  /* b */
  if (opcode == 16)
    return 1;  /* bc */
  if (opcode == 19 && subcode == 16)
    return 1;  /* bclr */
  if (opcode == 19 && subcode == 528)
    return 1;  /* bcctr */

  return 0;
}

/* Return true if we are in the function's epilogue, i.e. after the
   instruction that destroyed the function's stack frame.

   1) scan forward from the point of execution:
       a) If you find an instruction that modifies the stack pointer
          or transfers control (except a return), execution is not in
          an epilogue, return.
       b) Stop scanning if you find a return instruction or reach the
          end of the function or reach the hard limit for the size of
          an epilogue.
   2) scan backward from the point of execution:
        a) If you find an instruction that modifies the stack pointer,
            execution *is* in an epilogue, return.
        b) Stop scanning if you reach an instruction that transfers
           control or the beginning of the function or reach the hard
           limit for the size of an epilogue.  */

static int
rs6000_in_function_epilogue_p (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  bfd_byte insn_buf[PPC_INSN_SIZE];
  CORE_ADDR scan_pc, func_start, func_end, epilogue_start, epilogue_end;
  unsigned long insn;
  struct frame_info *curfrm;

  /* Find the search limits based on function boundaries and hard limit.  */

  if (!find_pc_partial_function (pc, NULL, &func_start, &func_end))
    return 0;

  epilogue_start = pc - PPC_MAX_EPILOGUE_INSTRUCTIONS * PPC_INSN_SIZE;
  if (epilogue_start < func_start) epilogue_start = func_start;

  epilogue_end = pc + PPC_MAX_EPILOGUE_INSTRUCTIONS * PPC_INSN_SIZE;
  if (epilogue_end > func_end) epilogue_end = func_end;

  curfrm = get_current_frame ();

  /* Scan forward until next 'blr'.  */

  for (scan_pc = pc; scan_pc < epilogue_end; scan_pc += PPC_INSN_SIZE)
    {
      if (!safe_frame_unwind_memory (curfrm, scan_pc, insn_buf, PPC_INSN_SIZE))
        return 0;
      insn = extract_unsigned_integer (insn_buf, PPC_INSN_SIZE);
      if (insn == 0x4e800020)
        break;
      if (insn_changes_sp_or_jumps (insn))
        return 0;
    }

  /* Scan backward until adjustment to stack pointer (R1).  */

  for (scan_pc = pc - PPC_INSN_SIZE;
       scan_pc >= epilogue_start;
       scan_pc -= PPC_INSN_SIZE)
    {
      if (!safe_frame_unwind_memory (curfrm, scan_pc, insn_buf, PPC_INSN_SIZE))
        return 0;
      insn = extract_unsigned_integer (insn_buf, PPC_INSN_SIZE);
      if (insn_changes_sp_or_jumps (insn))
        return 1;
    }

  return 0;
}

/* Get the ith function argument for the current function.  */
static CORE_ADDR
rs6000_fetch_pointer_argument (struct frame_info *frame, int argi, 
			       struct type *type)
{
  return get_frame_register_unsigned (frame, 3 + argi);
}

/* Calculate the destination of a branch/jump.  Return -1 if not a branch.  */

static CORE_ADDR
branch_dest (struct frame_info *frame, int opcode, int instr,
	     CORE_ADDR pc, CORE_ADDR safety)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (get_frame_arch (frame));
  CORE_ADDR dest;
  int immediate;
  int absolute;
  int ext_op;

  absolute = (int) ((instr >> 1) & 1);

  switch (opcode)
    {
    case 18:
      immediate = ((instr & ~3) << 6) >> 6;	/* br unconditional */
      if (absolute)
	dest = immediate;
      else
	dest = pc + immediate;
      break;

    case 16:
      immediate = ((instr & ~3) << 16) >> 16;	/* br conditional */
      if (absolute)
	dest = immediate;
      else
	dest = pc + immediate;
      break;

    case 19:
      ext_op = (instr >> 1) & 0x3ff;

      if (ext_op == 16)		/* br conditional register */
	{
          dest = get_frame_register_unsigned (frame, tdep->ppc_lr_regnum) & ~3;

	  /* If we are about to return from a signal handler, dest is
	     something like 0x3c90.  The current frame is a signal handler
	     caller frame, upon completion of the sigreturn system call
	     execution will return to the saved PC in the frame.  */
	  if (dest < tdep->text_segment_base)
	    dest = read_memory_addr (get_frame_base (frame) + SIG_FRAME_PC_OFFSET,
				     tdep->wordsize);
	}

      else if (ext_op == 528)	/* br cond to count reg */
	{
          dest = get_frame_register_unsigned (frame, tdep->ppc_ctr_regnum) & ~3;

	  /* If we are about to execute a system call, dest is something
	     like 0x22fc or 0x3b00.  Upon completion the system call
	     will return to the address in the link register.  */
	  if (dest < tdep->text_segment_base)
            dest = get_frame_register_unsigned (frame, tdep->ppc_lr_regnum) & ~3;
	}
      else
	return -1;
      break;

    default:
      return -1;
    }
  return (dest < tdep->text_segment_base) ? safety : dest;
}


/* Sequence of bytes for breakpoint instruction.  */

const static unsigned char *
rs6000_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *bp_addr,
			   int *bp_size)
{
  static unsigned char big_breakpoint[] = { 0x7d, 0x82, 0x10, 0x08 };
  static unsigned char little_breakpoint[] = { 0x08, 0x10, 0x82, 0x7d };
  *bp_size = 4;
  if (gdbarch_byte_order (gdbarch) == BFD_ENDIAN_BIG)
    return big_breakpoint;
  else
    return little_breakpoint;
}


/* Instruction masks used during single-stepping of atomic sequences.  */
#define LWARX_MASK 0xfc0007fe
#define LWARX_INSTRUCTION 0x7c000028
#define LDARX_INSTRUCTION 0x7c0000A8
#define STWCX_MASK 0xfc0007ff
#define STWCX_INSTRUCTION 0x7c00012d
#define STDCX_INSTRUCTION 0x7c0001ad
#define BC_MASK 0xfc000000
#define BC_INSTRUCTION 0x40000000

/* Checks for an atomic sequence of instructions beginning with a LWARX/LDARX
   instruction and ending with a STWCX/STDCX instruction.  If such a sequence
   is found, attempt to step through it.  A breakpoint is placed at the end of 
   the sequence.  */

static int 
deal_with_atomic_sequence (struct frame_info *frame)
{
  CORE_ADDR pc = get_frame_pc (frame);
  CORE_ADDR breaks[2] = {-1, -1};
  CORE_ADDR loc = pc;
  CORE_ADDR branch_bp; /* Breakpoint at branch instruction's destination.  */
  CORE_ADDR closing_insn; /* Instruction that closes the atomic sequence.  */
  int insn = read_memory_integer (loc, PPC_INSN_SIZE);
  int insn_count;
  int index;
  int last_breakpoint = 0; /* Defaults to 0 (no breakpoints placed).  */  
  const int atomic_sequence_length = 16; /* Instruction sequence length.  */
  int opcode; /* Branch instruction's OPcode.  */
  int bc_insn_count = 0; /* Conditional branch instruction count.  */

  /* Assume all atomic sequences start with a lwarx/ldarx instruction.  */
  if ((insn & LWARX_MASK) != LWARX_INSTRUCTION
      && (insn & LWARX_MASK) != LDARX_INSTRUCTION)
    return 0;

  /* Assume that no atomic sequence is longer than "atomic_sequence_length" 
     instructions.  */
  for (insn_count = 0; insn_count < atomic_sequence_length; ++insn_count)
    {
      loc += PPC_INSN_SIZE;
      insn = read_memory_integer (loc, PPC_INSN_SIZE);

      /* Assume that there is at most one conditional branch in the atomic
         sequence.  If a conditional branch is found, put a breakpoint in 
         its destination address.  */
      if ((insn & BC_MASK) == BC_INSTRUCTION)
        {
          if (bc_insn_count >= 1)
            return 0; /* More than one conditional branch found, fallback 
                         to the standard single-step code.  */
          
          opcode = insn >> 26;
          branch_bp = branch_dest (frame, opcode, insn, pc, breaks[0]);
          
          if (branch_bp != -1)
            {
              breaks[1] = branch_bp;
              bc_insn_count++;
              last_breakpoint++;
            }
        }

      if ((insn & STWCX_MASK) == STWCX_INSTRUCTION
          || (insn & STWCX_MASK) == STDCX_INSTRUCTION)
        break;
    }

  /* Assume that the atomic sequence ends with a stwcx/stdcx instruction.  */
  if ((insn & STWCX_MASK) != STWCX_INSTRUCTION
      && (insn & STWCX_MASK) != STDCX_INSTRUCTION)
    return 0;

  closing_insn = loc;
  loc += PPC_INSN_SIZE;
  insn = read_memory_integer (loc, PPC_INSN_SIZE);

  /* Insert a breakpoint right after the end of the atomic sequence.  */
  breaks[0] = loc;

  /* Check for duplicated breakpoints.  Check also for a breakpoint
     placed (branch instruction's destination) at the stwcx/stdcx 
     instruction, this resets the reservation and take us back to the 
     lwarx/ldarx instruction at the beginning of the atomic sequence.  */
  if (last_breakpoint && ((breaks[1] == breaks[0]) 
      || (breaks[1] == closing_insn)))
    last_breakpoint = 0;

  /* Effectively inserts the breakpoints.  */
  for (index = 0; index <= last_breakpoint; index++)
    insert_single_step_breakpoint (breaks[index]);

  return 1;
}

/* AIX does not support PT_STEP.  Simulate it.  */

int
rs6000_software_single_step (struct frame_info *frame)
{
  CORE_ADDR dummy;
  int breakp_sz;
  const gdb_byte *breakp
    = rs6000_breakpoint_from_pc (get_frame_arch (frame), &dummy, &breakp_sz);
  int ii, insn;
  CORE_ADDR loc;
  CORE_ADDR breaks[2];
  int opcode;

  loc = get_frame_pc (frame);

  insn = read_memory_integer (loc, 4);

  if (deal_with_atomic_sequence (frame))
    return 1;
  
  breaks[0] = loc + breakp_sz;
  opcode = insn >> 26;
  breaks[1] = branch_dest (frame, opcode, insn, loc, breaks[0]);

  /* Don't put two breakpoints on the same address. */
  if (breaks[1] == breaks[0])
    breaks[1] = -1;

  for (ii = 0; ii < 2; ++ii)
    {
      /* ignore invalid breakpoint. */
      if (breaks[ii] == -1)
	continue;
      insert_single_step_breakpoint (breaks[ii]);
    }

  errno = 0;			/* FIXME, don't ignore errors! */
  /* What errors?  {read,write}_memory call error().  */
  return 1;
}


#define SIGNED_SHORT(x) 						\
  ((sizeof (short) == 2)						\
   ? ((int)(short)(x))							\
   : ((int)((((x) & 0xffff) ^ 0x8000) - 0x8000)))

#define GET_SRC_REG(x) (((x) >> 21) & 0x1f)

/* Limit the number of skipped non-prologue instructions, as the examining
   of the prologue is expensive.  */
static int max_skip_non_prologue_insns = 10;

/* Return nonzero if the given instruction OP can be part of the prologue
   of a function and saves a parameter on the stack.  FRAMEP should be
   set if one of the previous instructions in the function has set the
   Frame Pointer.  */

static int
store_param_on_stack_p (unsigned long op, int framep, int *r0_contains_arg)
{
  /* Move parameters from argument registers to temporary register.  */
  if ((op & 0xfc0007fe) == 0x7c000378)         /* mr(.)  Rx,Ry */
    {
      /* Rx must be scratch register r0.  */
      const int rx_regno = (op >> 16) & 31;
      /* Ry: Only r3 - r10 are used for parameter passing.  */
      const int ry_regno = GET_SRC_REG (op);

      if (rx_regno == 0 && ry_regno >= 3 && ry_regno <= 10)
        {
          *r0_contains_arg = 1;
          return 1;
        }
      else
        return 0;
    }

  /* Save a General Purpose Register on stack.  */

  if ((op & 0xfc1f0003) == 0xf8010000 ||       /* std  Rx,NUM(r1) */
      (op & 0xfc1f0000) == 0xd8010000)         /* stfd Rx,NUM(r1) */
    {
      /* Rx: Only r3 - r10 are used for parameter passing.  */
      const int rx_regno = GET_SRC_REG (op);

      return (rx_regno >= 3 && rx_regno <= 10);
    }
           
  /* Save a General Purpose Register on stack via the Frame Pointer.  */

  if (framep &&
      ((op & 0xfc1f0000) == 0x901f0000 ||     /* st rx,NUM(r31) */
       (op & 0xfc1f0000) == 0x981f0000 ||     /* stb Rx,NUM(r31) */
       (op & 0xfc1f0000) == 0xd81f0000))      /* stfd Rx,NUM(r31) */
    {
      /* Rx: Usually, only r3 - r10 are used for parameter passing.
         However, the compiler sometimes uses r0 to hold an argument.  */
      const int rx_regno = GET_SRC_REG (op);

      return ((rx_regno >= 3 && rx_regno <= 10)
              || (rx_regno == 0 && *r0_contains_arg));
    }

  if ((op & 0xfc1f0000) == 0xfc010000)         /* frsp, fp?,NUM(r1) */
    {
      /* Only f2 - f8 are used for parameter passing.  */
      const int src_regno = GET_SRC_REG (op);

      return (src_regno >= 2 && src_regno <= 8);
    }

  if (framep && ((op & 0xfc1f0000) == 0xfc1f0000))  /* frsp, fp?,NUM(r31) */
    {
      /* Only f2 - f8 are used for parameter passing.  */
      const int src_regno = GET_SRC_REG (op);

      return (src_regno >= 2 && src_regno <= 8);
    }

  /* Not an insn that saves a parameter on stack.  */
  return 0;
}

/* Assuming that INSN is a "bl" instruction located at PC, return
   nonzero if the destination of the branch is a "blrl" instruction.
   
   This sequence is sometimes found in certain function prologues.
   It allows the function to load the LR register with a value that
   they can use to access PIC data using PC-relative offsets.  */

static int
bl_to_blrl_insn_p (CORE_ADDR pc, int insn)
{
  CORE_ADDR dest;
  int immediate;
  int absolute;
  int dest_insn;

  absolute = (int) ((insn >> 1) & 1);
  immediate = ((insn & ~3) << 6) >> 6;
  if (absolute)
    dest = immediate;
  else
    dest = pc + immediate;

  dest_insn = read_memory_integer (dest, 4);
  if ((dest_insn & 0xfc00ffff) == 0x4c000021) /* blrl */
    return 1;

  return 0;
}

/* return pc value after skipping a function prologue and also return
   information about a function frame.

   in struct rs6000_framedata fdata:
   - frameless is TRUE, if function does not have a frame.
   - nosavedpc is TRUE, if function does not save %pc value in its frame.
   - offset is the initial size of this stack frame --- the amount by
   which we decrement the sp to allocate the frame.
   - saved_gpr is the number of the first saved gpr.
   - saved_fpr is the number of the first saved fpr.
   - saved_vr is the number of the first saved vr.
   - saved_ev is the number of the first saved ev.
   - alloca_reg is the number of the register used for alloca() handling.
   Otherwise -1.
   - gpr_offset is the offset of the first saved gpr from the previous frame.
   - fpr_offset is the offset of the first saved fpr from the previous frame.
   - vr_offset is the offset of the first saved vr from the previous frame.
   - ev_offset is the offset of the first saved ev from the previous frame.
   - lr_offset is the offset of the saved lr
   - cr_offset is the offset of the saved cr
   - vrsave_offset is the offset of the saved vrsave register
 */

static CORE_ADDR
skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc, CORE_ADDR lim_pc,
	       struct rs6000_framedata *fdata)
{
  CORE_ADDR orig_pc = pc;
  CORE_ADDR last_prologue_pc = pc;
  CORE_ADDR li_found_pc = 0;
  gdb_byte buf[4];
  unsigned long op;
  long offset = 0;
  long vr_saved_offset = 0;
  int lr_reg = -1;
  int cr_reg = -1;
  int vr_reg = -1;
  int ev_reg = -1;
  long ev_offset = 0;
  int vrsave_reg = -1;
  int reg;
  int framep = 0;
  int minimal_toc_loaded = 0;
  int prev_insn_was_prologue_insn = 1;
  int num_skip_non_prologue_insns = 0;
  int r0_contains_arg = 0;
  const struct bfd_arch_info *arch_info = gdbarch_bfd_arch_info (gdbarch);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  memset (fdata, 0, sizeof (struct rs6000_framedata));
  fdata->saved_gpr = -1;
  fdata->saved_fpr = -1;
  fdata->saved_vr = -1;
  fdata->saved_ev = -1;
  fdata->alloca_reg = -1;
  fdata->frameless = 1;
  fdata->nosavedpc = 1;

  for (;; pc += 4)
    {
      /* Sometimes it isn't clear if an instruction is a prologue
         instruction or not.  When we encounter one of these ambiguous
	 cases, we'll set prev_insn_was_prologue_insn to 0 (false).
	 Otherwise, we'll assume that it really is a prologue instruction. */
      if (prev_insn_was_prologue_insn)
	last_prologue_pc = pc;

      /* Stop scanning if we've hit the limit.  */
      if (pc >= lim_pc)
	break;

      prev_insn_was_prologue_insn = 1;

      /* Fetch the instruction and convert it to an integer.  */
      if (target_read_memory (pc, buf, 4))
	break;
      op = extract_unsigned_integer (buf, 4);

      if ((op & 0xfc1fffff) == 0x7c0802a6)
	{			/* mflr Rx */
	  /* Since shared library / PIC code, which needs to get its
	     address at runtime, can appear to save more than one link
	     register vis:

	     *INDENT-OFF*
	     stwu r1,-304(r1)
	     mflr r3
	     bl 0xff570d0 (blrl)
	     stw r30,296(r1)
	     mflr r30
	     stw r31,300(r1)
	     stw r3,308(r1);
	     ...
	     *INDENT-ON*

	     remember just the first one, but skip over additional
	     ones.  */
	  if (lr_reg == -1)
	    lr_reg = (op & 0x03e00000);
          if (lr_reg == 0)
            r0_contains_arg = 0;
	  continue;
	}
      else if ((op & 0xfc1fffff) == 0x7c000026)
	{			/* mfcr Rx */
	  cr_reg = (op & 0x03e00000);
          if (cr_reg == 0)
            r0_contains_arg = 0;
	  continue;

	}
      else if ((op & 0xfc1f0000) == 0xd8010000)
	{			/* stfd Rx,NUM(r1) */
	  reg = GET_SRC_REG (op);
	  if (fdata->saved_fpr == -1 || fdata->saved_fpr > reg)
	    {
	      fdata->saved_fpr = reg;
	      fdata->fpr_offset = SIGNED_SHORT (op) + offset;
	    }
	  continue;

	}
      else if (((op & 0xfc1f0000) == 0xbc010000) ||	/* stm Rx, NUM(r1) */
	       (((op & 0xfc1f0000) == 0x90010000 ||	/* st rx,NUM(r1) */
		 (op & 0xfc1f0003) == 0xf8010000) &&	/* std rx,NUM(r1) */
		(op & 0x03e00000) >= 0x01a00000))	/* rx >= r13 */
	{

	  reg = GET_SRC_REG (op);
	  if (fdata->saved_gpr == -1 || fdata->saved_gpr > reg)
	    {
	      fdata->saved_gpr = reg;
	      if ((op & 0xfc1f0003) == 0xf8010000)
		op &= ~3UL;
	      fdata->gpr_offset = SIGNED_SHORT (op) + offset;
	    }
	  continue;

	}
      else if ((op & 0xffff0000) == 0x60000000)
        {
	  /* nop */
	  /* Allow nops in the prologue, but do not consider them to
	     be part of the prologue unless followed by other prologue
	     instructions. */
	  prev_insn_was_prologue_insn = 0;
	  continue;

	}
      else if ((op & 0xffff0000) == 0x3c000000)
	{			/* addis 0,0,NUM, used
				   for >= 32k frames */
	  fdata->offset = (op & 0x0000ffff) << 16;
	  fdata->frameless = 0;
          r0_contains_arg = 0;
	  continue;

	}
      else if ((op & 0xffff0000) == 0x60000000)
	{			/* ori 0,0,NUM, 2nd ha
				   lf of >= 32k frames */
	  fdata->offset |= (op & 0x0000ffff);
	  fdata->frameless = 0;
          r0_contains_arg = 0;
	  continue;

	}
      else if (lr_reg >= 0 &&
	       /* std Rx, NUM(r1) || stdu Rx, NUM(r1) */
	       (((op & 0xffff0000) == (lr_reg | 0xf8010000)) ||
		/* stw Rx, NUM(r1) */
		((op & 0xffff0000) == (lr_reg | 0x90010000)) ||
		/* stwu Rx, NUM(r1) */
		((op & 0xffff0000) == (lr_reg | 0x94010000))))
	{	/* where Rx == lr */
	  fdata->lr_offset = offset;
	  fdata->nosavedpc = 0;
	  /* Invalidate lr_reg, but don't set it to -1.
	     That would mean that it had never been set.  */
	  lr_reg = -2;
	  if ((op & 0xfc000003) == 0xf8000000 ||	/* std */
	      (op & 0xfc000000) == 0x90000000)		/* stw */
	    {
	      /* Does not update r1, so add displacement to lr_offset.  */
	      fdata->lr_offset += SIGNED_SHORT (op);
	    }
	  continue;

	}
      else if (cr_reg >= 0 &&
	       /* std Rx, NUM(r1) || stdu Rx, NUM(r1) */
	       (((op & 0xffff0000) == (cr_reg | 0xf8010000)) ||
		/* stw Rx, NUM(r1) */
		((op & 0xffff0000) == (cr_reg | 0x90010000)) ||
		/* stwu Rx, NUM(r1) */
		((op & 0xffff0000) == (cr_reg | 0x94010000))))
	{	/* where Rx == cr */
	  fdata->cr_offset = offset;
	  /* Invalidate cr_reg, but don't set it to -1.
	     That would mean that it had never been set.  */
	  cr_reg = -2;
	  if ((op & 0xfc000003) == 0xf8000000 ||
	      (op & 0xfc000000) == 0x90000000)
	    {
	      /* Does not update r1, so add displacement to cr_offset.  */
	      fdata->cr_offset += SIGNED_SHORT (op);
	    }
	  continue;

	}
      else if ((op & 0xfe80ffff) == 0x42800005 && lr_reg != -1)
	{
	  /* bcl 20,xx,.+4 is used to get the current PC, with or without
	     prediction bits.  If the LR has already been saved, we can
	     skip it.  */
	  continue;
	}
      else if (op == 0x48000005)
	{			/* bl .+4 used in 
				   -mrelocatable */
	  continue;

	}
      else if (op == 0x48000004)
	{			/* b .+4 (xlc) */
	  break;

	}
      else if ((op & 0xffff0000) == 0x3fc00000 ||  /* addis 30,0,foo@ha, used
						      in V.4 -mminimal-toc */
	       (op & 0xffff0000) == 0x3bde0000)
	{			/* addi 30,30,foo@l */
	  continue;

	}
      else if ((op & 0xfc000001) == 0x48000001)
	{			/* bl foo, 
				   to save fprs??? */

	  fdata->frameless = 0;

	  /* If the return address has already been saved, we can skip
	     calls to blrl (for PIC).  */
          if (lr_reg != -1 && bl_to_blrl_insn_p (pc, op))
	    continue;

	  /* Don't skip over the subroutine call if it is not within
	     the first three instructions of the prologue and either
	     we have no line table information or the line info tells
	     us that the subroutine call is not part of the line
	     associated with the prologue.  */
	  if ((pc - orig_pc) > 8)
	    {
	      struct symtab_and_line prologue_sal = find_pc_line (orig_pc, 0);
	      struct symtab_and_line this_sal = find_pc_line (pc, 0);

	      if ((prologue_sal.line == 0) || (prologue_sal.line != this_sal.line))
		break;
	    }

	  op = read_memory_integer (pc + 4, 4);

	  /* At this point, make sure this is not a trampoline
	     function (a function that simply calls another functions,
	     and nothing else).  If the next is not a nop, this branch
	     was part of the function prologue. */

	  if (op == 0x4def7b82 || op == 0)	/* crorc 15, 15, 15 */
	    break;		/* don't skip over 
				   this branch */
	  continue;

	}
      /* update stack pointer */
      else if ((op & 0xfc1f0000) == 0x94010000)
	{		/* stu rX,NUM(r1) ||  stwu rX,NUM(r1) */
	  fdata->frameless = 0;
	  fdata->offset = SIGNED_SHORT (op);
	  offset = fdata->offset;
	  continue;
	}
      else if ((op & 0xfc1f016a) == 0x7c01016e)
	{			/* stwux rX,r1,rY */
	  /* no way to figure out what r1 is going to be */
	  fdata->frameless = 0;
	  offset = fdata->offset;
	  continue;
	}
      else if ((op & 0xfc1f0003) == 0xf8010001)
	{			/* stdu rX,NUM(r1) */
	  fdata->frameless = 0;
	  fdata->offset = SIGNED_SHORT (op & ~3UL);
	  offset = fdata->offset;
	  continue;
	}
      else if ((op & 0xfc1f016a) == 0x7c01016a)
	{			/* stdux rX,r1,rY */
	  /* no way to figure out what r1 is going to be */
	  fdata->frameless = 0;
	  offset = fdata->offset;
	  continue;
	}
      else if ((op & 0xffff0000) == 0x38210000)
 	{			/* addi r1,r1,SIMM */
 	  fdata->frameless = 0;
 	  fdata->offset += SIGNED_SHORT (op);
 	  offset = fdata->offset;
 	  continue;
 	}
      /* Load up minimal toc pointer.  Do not treat an epilogue restore
	 of r31 as a minimal TOC load.  */
      else if (((op >> 22) == 0x20f	||	/* l r31,... or l r30,... */
	       (op >> 22) == 0x3af)		/* ld r31,... or ld r30,... */
	       && !framep
	       && !minimal_toc_loaded)
	{
	  minimal_toc_loaded = 1;
	  continue;

	  /* move parameters from argument registers to local variable
             registers */
 	}
      else if ((op & 0xfc0007fe) == 0x7c000378 &&	/* mr(.)  Rx,Ry */
               (((op >> 21) & 31) >= 3) &&              /* R3 >= Ry >= R10 */
               (((op >> 21) & 31) <= 10) &&
               ((long) ((op >> 16) & 31) >= fdata->saved_gpr)) /* Rx: local var reg */
	{
	  continue;

	  /* store parameters in stack */
	}
      /* Move parameters from argument registers to temporary register.  */
      else if (store_param_on_stack_p (op, framep, &r0_contains_arg))
        {
	  continue;

	  /* Set up frame pointer */
	}
      else if (op == 0x603f0000	/* oril r31, r1, 0x0 */
	       || op == 0x7c3f0b78)
	{			/* mr r31, r1 */
	  fdata->frameless = 0;
	  framep = 1;
	  fdata->alloca_reg = (tdep->ppc_gp0_regnum + 31);
	  continue;

	  /* Another way to set up the frame pointer.  */
	}
      else if ((op & 0xfc1fffff) == 0x38010000)
	{			/* addi rX, r1, 0x0 */
	  fdata->frameless = 0;
	  framep = 1;
	  fdata->alloca_reg = (tdep->ppc_gp0_regnum
			       + ((op & ~0x38010000) >> 21));
	  continue;
	}
      /* AltiVec related instructions.  */
      /* Store the vrsave register (spr 256) in another register for
	 later manipulation, or load a register into the vrsave
	 register.  2 instructions are used: mfvrsave and
	 mtvrsave.  They are shorthand notation for mfspr Rn, SPR256
	 and mtspr SPR256, Rn.  */
      /* mfspr Rn SPR256 == 011111 nnnnn 0000001000 01010100110
	 mtspr SPR256 Rn == 011111 nnnnn 0000001000 01110100110  */
      else if ((op & 0xfc1fffff) == 0x7c0042a6)    /* mfvrsave Rn */
	{
          vrsave_reg = GET_SRC_REG (op);
	  continue;
	}
      else if ((op & 0xfc1fffff) == 0x7c0043a6)     /* mtvrsave Rn */
        {
          continue;
        }
      /* Store the register where vrsave was saved to onto the stack:
         rS is the register where vrsave was stored in a previous
	 instruction.  */
      /* 100100 sssss 00001 dddddddd dddddddd */
      else if ((op & 0xfc1f0000) == 0x90010000)     /* stw rS, d(r1) */
        {
          if (vrsave_reg == GET_SRC_REG (op))
	    {
	      fdata->vrsave_offset = SIGNED_SHORT (op) + offset;
	      vrsave_reg = -1;
	    }
          continue;
        }
      /* Compute the new value of vrsave, by modifying the register
         where vrsave was saved to.  */
      else if (((op & 0xfc000000) == 0x64000000)    /* oris Ra, Rs, UIMM */
	       || ((op & 0xfc000000) == 0x60000000))/* ori Ra, Rs, UIMM */
	{
	  continue;
	}
      /* li r0, SIMM (short for addi r0, 0, SIMM).  This is the first
	 in a pair of insns to save the vector registers on the
	 stack.  */
      /* 001110 00000 00000 iiii iiii iiii iiii  */
      /* 001110 01110 00000 iiii iiii iiii iiii  */
      else if ((op & 0xffff0000) == 0x38000000         /* li r0, SIMM */
               || (op & 0xffff0000) == 0x39c00000)     /* li r14, SIMM */
	{
          if ((op & 0xffff0000) == 0x38000000)
            r0_contains_arg = 0;
	  li_found_pc = pc;
	  vr_saved_offset = SIGNED_SHORT (op);

          /* This insn by itself is not part of the prologue, unless
             if part of the pair of insns mentioned above. So do not
             record this insn as part of the prologue yet.  */
          prev_insn_was_prologue_insn = 0;
	}
      /* Store vector register S at (r31+r0) aligned to 16 bytes.  */      
      /* 011111 sssss 11111 00000 00111001110 */
      else if ((op & 0xfc1fffff) == 0x7c1f01ce)   /* stvx Vs, R31, R0 */
        {
	  if (pc == (li_found_pc + 4))
	    {
	      vr_reg = GET_SRC_REG (op);
	      /* If this is the first vector reg to be saved, or if
		 it has a lower number than others previously seen,
		 reupdate the frame info.  */
	      if (fdata->saved_vr == -1 || fdata->saved_vr > vr_reg)
		{
		  fdata->saved_vr = vr_reg;
		  fdata->vr_offset = vr_saved_offset + offset;
		}
	      vr_saved_offset = -1;
	      vr_reg = -1;
	      li_found_pc = 0;
	    }
	}
      /* End AltiVec related instructions.  */

      /* Start BookE related instructions.  */
      /* Store gen register S at (r31+uimm).
         Any register less than r13 is volatile, so we don't care.  */
      /* 000100 sssss 11111 iiiii 01100100001 */
      else if (arch_info->mach == bfd_mach_ppc_e500
	       && (op & 0xfc1f07ff) == 0x101f0321)    /* evstdd Rs,uimm(R31) */
	{
          if ((op & 0x03e00000) >= 0x01a00000)	/* Rs >= r13 */
	    {
              unsigned int imm;
	      ev_reg = GET_SRC_REG (op);
              imm = (op >> 11) & 0x1f;
	      ev_offset = imm * 8;
	      /* If this is the first vector reg to be saved, or if
		 it has a lower number than others previously seen,
		 reupdate the frame info.  */
	      if (fdata->saved_ev == -1 || fdata->saved_ev > ev_reg)
		{
		  fdata->saved_ev = ev_reg;
		  fdata->ev_offset = ev_offset + offset;
		}
	    }
          continue;
        }
      /* Store gen register rS at (r1+rB).  */
      /* 000100 sssss 00001 bbbbb 01100100000 */
      else if (arch_info->mach == bfd_mach_ppc_e500
	       && (op & 0xffe007ff) == 0x13e00320)     /* evstddx RS,R1,Rb */
	{
          if (pc == (li_found_pc + 4))
            {
              ev_reg = GET_SRC_REG (op);
	      /* If this is the first vector reg to be saved, or if
                 it has a lower number than others previously seen,
                 reupdate the frame info.  */
              /* We know the contents of rB from the previous instruction.  */
	      if (fdata->saved_ev == -1 || fdata->saved_ev > ev_reg)
		{
                  fdata->saved_ev = ev_reg;
                  fdata->ev_offset = vr_saved_offset + offset;
		}
	      vr_saved_offset = -1;
	      ev_reg = -1;
	      li_found_pc = 0;
            }
          continue;
        }
      /* Store gen register r31 at (rA+uimm).  */
      /* 000100 11111 aaaaa iiiii 01100100001 */
      else if (arch_info->mach == bfd_mach_ppc_e500
	       && (op & 0xffe007ff) == 0x13e00321)   /* evstdd R31,Ra,UIMM */
        {
          /* Wwe know that the source register is 31 already, but
             it can't hurt to compute it.  */
	  ev_reg = GET_SRC_REG (op);
          ev_offset = ((op >> 11) & 0x1f) * 8;
	  /* If this is the first vector reg to be saved, or if
	     it has a lower number than others previously seen,
	     reupdate the frame info.  */
	  if (fdata->saved_ev == -1 || fdata->saved_ev > ev_reg)
	    {
	      fdata->saved_ev = ev_reg;
	      fdata->ev_offset = ev_offset + offset;
	    }

	  continue;
      	}
      /* Store gen register S at (r31+r0).
         Store param on stack when offset from SP bigger than 4 bytes.  */
      /* 000100 sssss 11111 00000 01100100000 */
      else if (arch_info->mach == bfd_mach_ppc_e500
	       && (op & 0xfc1fffff) == 0x101f0320)     /* evstddx Rs,R31,R0 */
	{
          if (pc == (li_found_pc + 4))
            {
              if ((op & 0x03e00000) >= 0x01a00000)
		{
		  ev_reg = GET_SRC_REG (op);
		  /* If this is the first vector reg to be saved, or if
		     it has a lower number than others previously seen,
		     reupdate the frame info.  */
                  /* We know the contents of r0 from the previous
                     instruction.  */
		  if (fdata->saved_ev == -1 || fdata->saved_ev > ev_reg)
		    {
		      fdata->saved_ev = ev_reg;
		      fdata->ev_offset = vr_saved_offset + offset;
		    }
		  ev_reg = -1;
		}
	      vr_saved_offset = -1;
	      li_found_pc = 0;
	      continue;
            }
	}
      /* End BookE related instructions.  */

      else
	{
	  /* Not a recognized prologue instruction.
	     Handle optimizer code motions into the prologue by continuing
	     the search if we have no valid frame yet or if the return
	     address is not yet saved in the frame.  */
	  if (fdata->frameless == 0 && fdata->nosavedpc == 0)
	    break;

	  if (op == 0x4e800020		/* blr */
	      || op == 0x4e800420)	/* bctr */
	    /* Do not scan past epilogue in frameless functions or
	       trampolines.  */
	    break;
	  if ((op & 0xf4000000) == 0x40000000) /* bxx */
	    /* Never skip branches.  */
	    break;

	  if (num_skip_non_prologue_insns++ > max_skip_non_prologue_insns)
	    /* Do not scan too many insns, scanning insns is expensive with
	       remote targets.  */
	    break;

	  /* Continue scanning.  */
	  prev_insn_was_prologue_insn = 0;
	  continue;
	}
    }

#if 0
/* I have problems with skipping over __main() that I need to address
 * sometime. Previously, I used to use misc_function_vector which
 * didn't work as well as I wanted to be.  -MGO */

  /* If the first thing after skipping a prolog is a branch to a function,
     this might be a call to an initializer in main(), introduced by gcc2.
     We'd like to skip over it as well.  Fortunately, xlc does some extra
     work before calling a function right after a prologue, thus we can
     single out such gcc2 behaviour.  */


  if ((op & 0xfc000001) == 0x48000001)
    {				/* bl foo, an initializer function? */
      op = read_memory_integer (pc + 4, 4);

      if (op == 0x4def7b82)
	{			/* cror 0xf, 0xf, 0xf (nop) */

	  /* Check and see if we are in main.  If so, skip over this
	     initializer function as well.  */

	  tmp = find_pc_misc_function (pc);
	  if (tmp >= 0
	      && strcmp (misc_function_vector[tmp].name, main_name ()) == 0)
	    return pc + 8;
	}
    }
#endif /* 0 */

  fdata->offset = -fdata->offset;
  return last_prologue_pc;
}


/*************************************************************************
  Support for creating pushing a dummy frame into the stack, and popping
  frames, etc. 
*************************************************************************/


/* All the ABI's require 16 byte alignment.  */
static CORE_ADDR
rs6000_frame_align (struct gdbarch *gdbarch, CORE_ADDR addr)
{
  return (addr & -16);
}

/* Pass the arguments in either registers, or in the stack. In RS/6000,
   the first eight words of the argument list (that might be less than
   eight parameters if some parameters occupy more than one word) are
   passed in r3..r10 registers.  float and double parameters are
   passed in fpr's, in addition to that.  Rest of the parameters if any
   are passed in user stack.  There might be cases in which half of the
   parameter is copied into registers, the other half is pushed into
   stack.

   Stack must be aligned on 64-bit boundaries when synthesizing
   function calls.

   If the function is returning a structure, then the return address is passed
   in r3, then the first 7 words of the parameters can be passed in registers,
   starting from r4.  */

static CORE_ADDR
rs6000_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
			struct regcache *regcache, CORE_ADDR bp_addr,
			int nargs, struct value **args, CORE_ADDR sp,
			int struct_return, CORE_ADDR struct_addr)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int ii;
  int len = 0;
  int argno;			/* current argument number */
  int argbytes;			/* current argument byte */
  gdb_byte tmp_buffer[50];
  int f_argno = 0;		/* current floating point argno */
  int wordsize = gdbarch_tdep (gdbarch)->wordsize;
  CORE_ADDR func_addr = find_function_addr (function, NULL);

  struct value *arg = 0;
  struct type *type;

  ULONGEST saved_sp;

  /* The calling convention this function implements assumes the
     processor has floating-point registers.  We shouldn't be using it
     on PPC variants that lack them.  */
  gdb_assert (ppc_floating_point_unit_p (gdbarch));

  /* The first eight words of ther arguments are passed in registers.
     Copy them appropriately.  */
  ii = 0;

  /* If the function is returning a `struct', then the first word
     (which will be passed in r3) is used for struct return address.
     In that case we should advance one word and start from r4
     register to copy parameters.  */
  if (struct_return)
    {
      regcache_raw_write_unsigned (regcache, tdep->ppc_gp0_regnum + 3,
				   struct_addr);
      ii++;
    }

/* 
   effectively indirect call... gcc does...

   return_val example( float, int);

   eabi: 
   float in fp0, int in r3
   offset of stack on overflow 8/16
   for varargs, must go by type.
   power open:
   float in r3&r4, int in r5
   offset of stack on overflow different 
   both: 
   return in r3 or f0.  If no float, must study how gcc emulates floats;
   pay attention to arg promotion.  
   User may have to cast\args to handle promotion correctly 
   since gdb won't know if prototype supplied or not.
 */

  for (argno = 0, argbytes = 0; argno < nargs && ii < 8; ++ii)
    {
      int reg_size = register_size (gdbarch, ii + 3);

      arg = args[argno];
      type = check_typedef (value_type (arg));
      len = TYPE_LENGTH (type);

      if (TYPE_CODE (type) == TYPE_CODE_FLT)
	{

	  /* Floating point arguments are passed in fpr's, as well as gpr's.
	     There are 13 fpr's reserved for passing parameters. At this point
	     there is no way we would run out of them.  */

	  gdb_assert (len <= 8);

	  regcache_cooked_write (regcache,
	                         tdep->ppc_fp0_regnum + 1 + f_argno,
	                         value_contents (arg));
	  ++f_argno;
	}

      if (len > reg_size)
	{

	  /* Argument takes more than one register.  */
	  while (argbytes < len)
	    {
	      gdb_byte word[MAX_REGISTER_SIZE];
	      memset (word, 0, reg_size);
	      memcpy (word,
		      ((char *) value_contents (arg)) + argbytes,
		      (len - argbytes) > reg_size
		        ? reg_size : len - argbytes);
	      regcache_cooked_write (regcache,
	                            tdep->ppc_gp0_regnum + 3 + ii,
				    word);
	      ++ii, argbytes += reg_size;

	      if (ii >= 8)
		goto ran_out_of_registers_for_arguments;
	    }
	  argbytes = 0;
	  --ii;
	}
      else
	{
	  /* Argument can fit in one register.  No problem.  */
	  int adj = gdbarch_byte_order (gdbarch)
		    == BFD_ENDIAN_BIG ? reg_size - len : 0;
	  gdb_byte word[MAX_REGISTER_SIZE];

	  memset (word, 0, reg_size);
	  memcpy (word, value_contents (arg), len);
	  regcache_cooked_write (regcache, tdep->ppc_gp0_regnum + 3 +ii, word);
	}
      ++argno;
    }

ran_out_of_registers_for_arguments:

  regcache_cooked_read_unsigned (regcache,
				 gdbarch_sp_regnum (gdbarch),
				 &saved_sp);

  /* Location for 8 parameters are always reserved.  */
  sp -= wordsize * 8;

  /* Another six words for back chain, TOC register, link register, etc.  */
  sp -= wordsize * 6;

  /* Stack pointer must be quadword aligned.  */
  sp &= -16;

  /* If there are more arguments, allocate space for them in 
     the stack, then push them starting from the ninth one.  */

  if ((argno < nargs) || argbytes)
    {
      int space = 0, jj;

      if (argbytes)
	{
	  space += ((len - argbytes + 3) & -4);
	  jj = argno + 1;
	}
      else
	jj = argno;

      for (; jj < nargs; ++jj)
	{
	  struct value *val = args[jj];
	  space += ((TYPE_LENGTH (value_type (val))) + 3) & -4;
	}

      /* Add location required for the rest of the parameters.  */
      space = (space + 15) & -16;
      sp -= space;

      /* This is another instance we need to be concerned about
         securing our stack space. If we write anything underneath %sp
         (r1), we might conflict with the kernel who thinks he is free
         to use this area.  So, update %sp first before doing anything
         else.  */

      regcache_raw_write_signed (regcache,
				 gdbarch_sp_regnum (gdbarch), sp);

      /* If the last argument copied into the registers didn't fit there 
         completely, push the rest of it into stack.  */

      if (argbytes)
	{
	  write_memory (sp + 24 + (ii * 4),
			value_contents (arg) + argbytes,
			len - argbytes);
	  ++argno;
	  ii += ((len - argbytes + 3) & -4) / 4;
	}

      /* Push the rest of the arguments into stack.  */
      for (; argno < nargs; ++argno)
	{

	  arg = args[argno];
	  type = check_typedef (value_type (arg));
	  len = TYPE_LENGTH (type);


	  /* Float types should be passed in fpr's, as well as in the
             stack.  */
	  if (TYPE_CODE (type) == TYPE_CODE_FLT && f_argno < 13)
	    {

	      gdb_assert (len <= 8);

	      regcache_cooked_write (regcache,
				     tdep->ppc_fp0_regnum + 1 + f_argno,
				     value_contents (arg));
	      ++f_argno;
	    }

	  write_memory (sp + 24 + (ii * 4), value_contents (arg), len);
	  ii += ((len + 3) & -4) / 4;
	}
    }

  /* Set the stack pointer.  According to the ABI, the SP is meant to
     be set _before_ the corresponding stack space is used.  On AIX,
     this even applies when the target has been completely stopped!
     Not doing this can lead to conflicts with the kernel which thinks
     that it still has control over this not-yet-allocated stack
     region.  */
  regcache_raw_write_signed (regcache, gdbarch_sp_regnum (gdbarch), sp);

  /* Set back chain properly.  */
  store_unsigned_integer (tmp_buffer, wordsize, saved_sp);
  write_memory (sp, tmp_buffer, wordsize);

  /* Point the inferior function call's return address at the dummy's
     breakpoint.  */
  regcache_raw_write_signed (regcache, tdep->ppc_lr_regnum, bp_addr);

  /* Set the TOC register, get the value from the objfile reader
     which, in turn, gets it from the VMAP table.  */
  if (rs6000_find_toc_address_hook != NULL)
    {
      CORE_ADDR tocvalue = (*rs6000_find_toc_address_hook) (func_addr);
      regcache_raw_write_signed (regcache, tdep->ppc_toc_regnum, tocvalue);
    }

  target_store_registers (regcache, -1);
  return sp;
}

static enum return_value_convention
rs6000_return_value (struct gdbarch *gdbarch, struct type *valtype,
		     struct regcache *regcache, gdb_byte *readbuf,
		     const gdb_byte *writebuf)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  gdb_byte buf[8];

  /* The calling convention this function implements assumes the
     processor has floating-point registers.  We shouldn't be using it
     on PowerPC variants that lack them.  */
  gdb_assert (ppc_floating_point_unit_p (gdbarch));

  /* AltiVec extension: Functions that declare a vector data type as a
     return value place that return value in VR2.  */
  if (TYPE_CODE (valtype) == TYPE_CODE_ARRAY && TYPE_VECTOR (valtype)
      && TYPE_LENGTH (valtype) == 16)
    {
      if (readbuf)
	regcache_cooked_read (regcache, tdep->ppc_vr0_regnum + 2, readbuf);
      if (writebuf)
	regcache_cooked_write (regcache, tdep->ppc_vr0_regnum + 2, writebuf);

      return RETURN_VALUE_REGISTER_CONVENTION;
    }

  /* If the called subprogram returns an aggregate, there exists an
     implicit first argument, whose value is the address of a caller-
     allocated buffer into which the callee is assumed to store its
     return value. All explicit parameters are appropriately
     relabeled.  */
  if (TYPE_CODE (valtype) == TYPE_CODE_STRUCT
      || TYPE_CODE (valtype) == TYPE_CODE_UNION
      || TYPE_CODE (valtype) == TYPE_CODE_ARRAY)
    return RETURN_VALUE_STRUCT_CONVENTION;

  /* Scalar floating-point values are returned in FPR1 for float or
     double, and in FPR1:FPR2 for quadword precision.  Fortran
     complex*8 and complex*16 are returned in FPR1:FPR2, and
     complex*32 is returned in FPR1:FPR4.  */
  if (TYPE_CODE (valtype) == TYPE_CODE_FLT
      && (TYPE_LENGTH (valtype) == 4 || TYPE_LENGTH (valtype) == 8))
    {
      struct type *regtype = register_type (gdbarch, tdep->ppc_fp0_regnum);
      gdb_byte regval[8];

      /* FIXME: kettenis/2007-01-01: Add support for quadword
	 precision and complex.  */

      if (readbuf)
	{
	  regcache_cooked_read (regcache, tdep->ppc_fp0_regnum + 1, regval);
	  convert_typed_floating (regval, regtype, readbuf, valtype);
	}
      if (writebuf)
	{
	  convert_typed_floating (writebuf, valtype, regval, regtype);
	  regcache_cooked_write (regcache, tdep->ppc_fp0_regnum + 1, regval);
	}

      return RETURN_VALUE_REGISTER_CONVENTION;
  }

  /* Values of the types int, long, short, pointer, and char (length
     is less than or equal to four bytes), as well as bit values of
     lengths less than or equal to 32 bits, must be returned right
     justified in GPR3 with signed values sign extended and unsigned
     values zero extended, as necessary.  */
  if (TYPE_LENGTH (valtype) <= tdep->wordsize)
    {
      if (readbuf)
	{
	  ULONGEST regval;

	  /* For reading we don't have to worry about sign extension.  */
	  regcache_cooked_read_unsigned (regcache, tdep->ppc_gp0_regnum + 3,
					 &regval);
	  store_unsigned_integer (readbuf, TYPE_LENGTH (valtype), regval);
	}
      if (writebuf)
	{
	  /* For writing, use unpack_long since that should handle any
	     required sign extension.  */
	  regcache_cooked_write_unsigned (regcache, tdep->ppc_gp0_regnum + 3,
					  unpack_long (valtype, writebuf));
	}

      return RETURN_VALUE_REGISTER_CONVENTION;
    }

  /* Eight-byte non-floating-point scalar values must be returned in
     GPR3:GPR4.  */

  if (TYPE_LENGTH (valtype) == 8)
    {
      gdb_assert (TYPE_CODE (valtype) != TYPE_CODE_FLT);
      gdb_assert (tdep->wordsize == 4);

      if (readbuf)
	{
	  gdb_byte regval[8];

	  regcache_cooked_read (regcache, tdep->ppc_gp0_regnum + 3, regval);
	  regcache_cooked_read (regcache, tdep->ppc_gp0_regnum + 4,
				regval + 4);
	  memcpy (readbuf, regval, 8);
	}
      if (writebuf)
	{
	  regcache_cooked_write (regcache, tdep->ppc_gp0_regnum + 3, writebuf);
	  regcache_cooked_write (regcache, tdep->ppc_gp0_regnum + 4,
				 writebuf + 4);
	}

      return RETURN_VALUE_REGISTER_CONVENTION;
    }

  return RETURN_VALUE_STRUCT_CONVENTION;
}

/* Return whether handle_inferior_event() should proceed through code
   starting at PC in function NAME when stepping.

   The AIX -bbigtoc linker option generates functions @FIX0, @FIX1, etc. to
   handle memory references that are too distant to fit in instructions
   generated by the compiler.  For example, if 'foo' in the following
   instruction:

     lwz r9,foo(r2)

   is greater than 32767, the linker might replace the lwz with a branch to
   somewhere in @FIX1 that does the load in 2 instructions and then branches
   back to where execution should continue.

   GDB should silently step over @FIX code, just like AIX dbx does.
   Unfortunately, the linker uses the "b" instruction for the
   branches, meaning that the link register doesn't get set.
   Therefore, GDB's usual step_over_function () mechanism won't work.

   Instead, use the gdbarch_skip_trampoline_code and
   gdbarch_skip_trampoline_code hooks in handle_inferior_event() to skip past
   @FIX code.  */

int
rs6000_in_solib_return_trampoline (CORE_ADDR pc, char *name)
{
  return name && !strncmp (name, "@FIX", 4);
}

/* Skip code that the user doesn't want to see when stepping:

   1. Indirect function calls use a piece of trampoline code to do context
   switching, i.e. to set the new TOC table.  Skip such code if we are on
   its first instruction (as when we have single-stepped to here).

   2. Skip shared library trampoline code (which is different from
   indirect function call trampolines).

   3. Skip bigtoc fixup code.

   Result is desired PC to step until, or NULL if we are not in
   code that should be skipped.  */

CORE_ADDR
rs6000_skip_trampoline_code (struct frame_info *frame, CORE_ADDR pc)
{
  unsigned int ii, op;
  int rel;
  CORE_ADDR solib_target_pc;
  struct minimal_symbol *msymbol;

  static unsigned trampoline_code[] =
  {
    0x800b0000,			/*     l   r0,0x0(r11)  */
    0x90410014,			/*    st   r2,0x14(r1)  */
    0x7c0903a6,			/* mtctr   r0           */
    0x804b0004,			/*     l   r2,0x4(r11)  */
    0x816b0008,			/*     l  r11,0x8(r11)  */
    0x4e800420,			/*  bctr                */
    0x4e800020,			/*    br                */
    0
  };

  /* Check for bigtoc fixup code.  */
  msymbol = lookup_minimal_symbol_by_pc (pc);
  if (msymbol 
      && rs6000_in_solib_return_trampoline (pc, 
					    DEPRECATED_SYMBOL_NAME (msymbol)))
    {
      /* Double-check that the third instruction from PC is relative "b".  */
      op = read_memory_integer (pc + 8, 4);
      if ((op & 0xfc000003) == 0x48000000)
	{
	  /* Extract bits 6-29 as a signed 24-bit relative word address and
	     add it to the containing PC.  */
	  rel = ((int)(op << 6) >> 6);
	  return pc + 8 + rel;
	}
    }

  /* If pc is in a shared library trampoline, return its target.  */
  solib_target_pc = find_solib_trampoline_target (frame, pc);
  if (solib_target_pc)
    return solib_target_pc;

  for (ii = 0; trampoline_code[ii]; ++ii)
    {
      op = read_memory_integer (pc + (ii * 4), 4);
      if (op != trampoline_code[ii])
	return 0;
    }
  ii = get_frame_register_unsigned (frame, 11);	/* r11 holds destination addr   */
  pc = read_memory_addr (ii,
			 gdbarch_tdep (get_frame_arch (frame))->wordsize); /* (r11) value */
  return pc;
}

/* ISA-specific vector types.  */

static struct type *
rs6000_builtin_type_vec64 (struct gdbarch *gdbarch)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (!tdep->ppc_builtin_type_vec64)
    {
      /* The type we're building is this: */
#if 0
      union __gdb_builtin_type_vec64
	{
	  int64_t uint64;
	  float v2_float[2];
	  int32_t v2_int32[2];
	  int16_t v4_int16[4];
	  int8_t v8_int8[8];
	};
#endif

      struct type *t;

      t = init_composite_type ("__ppc_builtin_type_vec64", TYPE_CODE_UNION);
      append_composite_type_field (t, "uint64", builtin_type_int64);
      append_composite_type_field (t, "v2_float",
				   init_vector_type (builtin_type_float, 2));
      append_composite_type_field (t, "v2_int32",
				   init_vector_type (builtin_type_int32, 2));
      append_composite_type_field (t, "v4_int16",
				   init_vector_type (builtin_type_int16, 4));
      append_composite_type_field (t, "v8_int8",
				   init_vector_type (builtin_type_int8, 8));

      TYPE_FLAGS (t) |= TYPE_FLAG_VECTOR;
      TYPE_NAME (t) = "ppc_builtin_type_vec64";
      tdep->ppc_builtin_type_vec64 = t;
    }

  return tdep->ppc_builtin_type_vec64;
}

/* Return the size of register REG when words are WORDSIZE bytes long.  If REG
   isn't available with that word size, return 0.  */

static int
regsize (const struct reg *reg, int wordsize)
{
  return wordsize == 8 ? reg->sz64 : reg->sz32;
}

/* Return the name of register number REGNO, or the empty string if it
   is an anonymous register.  */

static const char *
rs6000_register_name (struct gdbarch *gdbarch, int regno)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  /* The upper half "registers" have names in the XML description,
     but we present only the low GPRs and the full 64-bit registers
     to the user.  */
  if (tdep->ppc_ev0_upper_regnum >= 0
      && tdep->ppc_ev0_upper_regnum <= regno
      && regno < tdep->ppc_ev0_upper_regnum + ppc_num_gprs)
    return "";

  /* Check if the SPE pseudo registers are available.  */
  if (IS_SPE_PSEUDOREG (tdep, regno))
    {
      static const char *const spe_regnames[] = {
	"ev0", "ev1", "ev2", "ev3", "ev4", "ev5", "ev6", "ev7",
	"ev8", "ev9", "ev10", "ev11", "ev12", "ev13", "ev14", "ev15",
	"ev16", "ev17", "ev18", "ev19", "ev20", "ev21", "ev22", "ev23",
	"ev24", "ev25", "ev26", "ev27", "ev28", "ev29", "ev30", "ev31",
      };
      return spe_regnames[regno - tdep->ppc_ev0_regnum];
    }

  /* Check if the decimal128 pseudo-registers are available.  */
  if (IS_DFP_PSEUDOREG (tdep, regno))
    {
      static const char *const dfp128_regnames[] = {
	"dl0", "dl1", "dl2", "dl3",
	"dl4", "dl5", "dl6", "dl7",
	"dl8", "dl9", "dl10", "dl11",
	"dl12", "dl13", "dl14", "dl15"
      };
      return dfp128_regnames[regno - tdep->ppc_dl0_regnum];
    }

  return tdesc_register_name (gdbarch, regno);
}

/* Return the GDB type object for the "standard" data type of data in
   register N.  */

static struct type *
rs6000_pseudo_register_type (struct gdbarch *gdbarch, int regnum)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  /* These are the only pseudo-registers we support.  */
  gdb_assert (IS_SPE_PSEUDOREG (tdep, regnum)
	      || IS_DFP_PSEUDOREG (tdep, regnum));

  /* These are the e500 pseudo-registers.  */
  if (IS_SPE_PSEUDOREG (tdep, regnum))
    return rs6000_builtin_type_vec64 (gdbarch);
  else
    /* Could only be the ppc decimal128 pseudo-registers.  */
    return builtin_type (gdbarch)->builtin_declong;
}

/* Is REGNUM a member of REGGROUP?  */
static int
rs6000_pseudo_register_reggroup_p (struct gdbarch *gdbarch, int regnum,
				   struct reggroup *group)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  /* These are the only pseudo-registers we support.  */
  gdb_assert (IS_SPE_PSEUDOREG (tdep, regnum)
	      || IS_DFP_PSEUDOREG (tdep, regnum));

  /* These are the e500 pseudo-registers.  */
  if (IS_SPE_PSEUDOREG (tdep, regnum))
    return group == all_reggroup || group == vector_reggroup;
  else
    /* Could only be the ppc decimal128 pseudo-registers.  */
    return group == all_reggroup || group == float_reggroup;
}

/* The register format for RS/6000 floating point registers is always
   double, we need a conversion if the memory format is float.  */

static int
rs6000_convert_register_p (struct gdbarch *gdbarch, int regnum,
			   struct type *type)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  return (tdep->ppc_fp0_regnum >= 0
	  && regnum >= tdep->ppc_fp0_regnum
	  && regnum < tdep->ppc_fp0_regnum + ppc_num_fprs
	  && TYPE_CODE (type) == TYPE_CODE_FLT
	  && TYPE_LENGTH (type) != TYPE_LENGTH (builtin_type_double));
}

static void
rs6000_register_to_value (struct frame_info *frame,
                          int regnum,
                          struct type *type,
                          gdb_byte *to)
{
  gdb_byte from[MAX_REGISTER_SIZE];
  
  gdb_assert (TYPE_CODE (type) == TYPE_CODE_FLT);

  get_frame_register (frame, regnum, from);
  convert_typed_floating (from, builtin_type_double, to, type);
}

static void
rs6000_value_to_register (struct frame_info *frame,
                          int regnum,
                          struct type *type,
                          const gdb_byte *from)
{
  gdb_byte to[MAX_REGISTER_SIZE];

  gdb_assert (TYPE_CODE (type) == TYPE_CODE_FLT);

  convert_typed_floating (from, type, to, builtin_type_double);
  put_frame_register (frame, regnum, to);
}

/* Move SPE vector register values between a 64-bit buffer and the two
   32-bit raw register halves in a regcache.  This function handles
   both splitting a 64-bit value into two 32-bit halves, and joining
   two halves into a whole 64-bit value, depending on the function
   passed as the MOVE argument.

   EV_REG must be the number of an SPE evN vector register --- a
   pseudoregister.  REGCACHE must be a regcache, and BUFFER must be a
   64-bit buffer.

   Call MOVE once for each 32-bit half of that register, passing
   REGCACHE, the number of the raw register corresponding to that
   half, and the address of the appropriate half of BUFFER.

   For example, passing 'regcache_raw_read' as the MOVE function will
   fill BUFFER with the full 64-bit contents of EV_REG.  Or, passing
   'regcache_raw_supply' will supply the contents of BUFFER to the
   appropriate pair of raw registers in REGCACHE.

   You may need to cast away some 'const' qualifiers when passing
   MOVE, since this function can't tell at compile-time which of
   REGCACHE or BUFFER is acting as the source of the data.  If C had
   co-variant type qualifiers, ...  */
static void
e500_move_ev_register (void (*move) (struct regcache *regcache,
                                     int regnum, gdb_byte *buf),
                       struct regcache *regcache, int ev_reg,
                       gdb_byte *buffer)
{
  struct gdbarch *arch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (arch); 
  int reg_index;
  gdb_byte *byte_buffer = buffer;

  gdb_assert (IS_SPE_PSEUDOREG (tdep, ev_reg));

  reg_index = ev_reg - tdep->ppc_ev0_regnum;

  if (gdbarch_byte_order (arch) == BFD_ENDIAN_BIG)
    {
      move (regcache, tdep->ppc_ev0_upper_regnum + reg_index, byte_buffer);
      move (regcache, tdep->ppc_gp0_regnum + reg_index, byte_buffer + 4);
    }
  else
    {
      move (regcache, tdep->ppc_gp0_regnum + reg_index, byte_buffer);
      move (regcache, tdep->ppc_ev0_upper_regnum + reg_index, byte_buffer + 4);
    }
}

static void
e500_pseudo_register_read (struct gdbarch *gdbarch, struct regcache *regcache,
			   int reg_nr, gdb_byte *buffer)
{
  e500_move_ev_register (regcache_raw_read, regcache, reg_nr, buffer);
}

static void
e500_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache,
			    int reg_nr, const gdb_byte *buffer)
{
  e500_move_ev_register ((void (*) (struct regcache *, int, gdb_byte *))
			 regcache_raw_write,
			 regcache, reg_nr, (gdb_byte *) buffer);
}

/* Read method for PPC pseudo-registers. Currently this is handling the
   16 decimal128 registers that map into 16 pairs of FP registers.  */
static void
ppc_pseudo_register_read (struct gdbarch *gdbarch, struct regcache *regcache,
			   int reg_nr, gdb_byte *buffer)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int reg_index = reg_nr - tdep->ppc_dl0_regnum;

  if (gdbarch_byte_order (gdbarch) == BFD_ENDIAN_BIG)
    {
      /* Read two FP registers to form a whole dl register.  */
      regcache_raw_read (regcache, tdep->ppc_fp0_regnum +
			 2 * reg_index, buffer);
      regcache_raw_read (regcache, tdep->ppc_fp0_regnum +
			 2 * reg_index + 1, buffer + 8);
    }
  else
    {
      regcache_raw_read (regcache, tdep->ppc_fp0_regnum +
			 2 * reg_index + 1, buffer + 8);
      regcache_raw_read (regcache, tdep->ppc_fp0_regnum +
			 2 * reg_index, buffer);
    }
}

/* Write method for PPC pseudo-registers. Currently this is handling the
   16 decimal128 registers that map into 16 pairs of FP registers.  */
static void
ppc_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache,
			    int reg_nr, const gdb_byte *buffer)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int reg_index = reg_nr - tdep->ppc_dl0_regnum;

  if (gdbarch_byte_order (gdbarch) == BFD_ENDIAN_BIG)
    {
      /* Write each half of the dl register into a separate
      FP register.  */
      regcache_raw_write (regcache, tdep->ppc_fp0_regnum +
			  2 * reg_index, buffer);
      regcache_raw_write (regcache, tdep->ppc_fp0_regnum +
			  2 * reg_index + 1, buffer + 8);
    }
  else
    {
      regcache_raw_write (regcache, tdep->ppc_fp0_regnum +
			  2 * reg_index + 1, buffer + 8);
      regcache_raw_write (regcache, tdep->ppc_fp0_regnum +
			  2 * reg_index, buffer);
    }
}

static void
rs6000_pseudo_register_read (struct gdbarch *gdbarch, struct regcache *regcache,
			     int reg_nr, gdb_byte *buffer)
{
  struct gdbarch *regcache_arch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch); 

  gdb_assert (regcache_arch == gdbarch);

  if (IS_SPE_PSEUDOREG (tdep, reg_nr))
    e500_pseudo_register_read (gdbarch, regcache, reg_nr, buffer);
  else if (IS_DFP_PSEUDOREG (tdep, reg_nr))
    ppc_pseudo_register_read (gdbarch, regcache, reg_nr, buffer);
  else
    internal_error (__FILE__, __LINE__,
		    _("rs6000_pseudo_register_read: "
		    "called on unexpected register '%s' (%d)"),
		    gdbarch_register_name (gdbarch, reg_nr), reg_nr);
}

static void
rs6000_pseudo_register_write (struct gdbarch *gdbarch,
			      struct regcache *regcache,
			      int reg_nr, const gdb_byte *buffer)
{
  struct gdbarch *regcache_arch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch); 

  gdb_assert (regcache_arch == gdbarch);

  if (IS_SPE_PSEUDOREG (tdep, reg_nr))
    e500_pseudo_register_write (gdbarch, regcache, reg_nr, buffer);
  else if (IS_DFP_PSEUDOREG (tdep, reg_nr))
    ppc_pseudo_register_write (gdbarch, regcache, reg_nr, buffer);
  else
    internal_error (__FILE__, __LINE__,
		    _("rs6000_pseudo_register_write: "
		    "called on unexpected register '%s' (%d)"),
		    gdbarch_register_name (gdbarch, reg_nr), reg_nr);
}

/* Convert a DBX STABS register number to a GDB register number.  */
static int
rs6000_stab_reg_to_regnum (struct gdbarch *gdbarch, int num)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (0 <= num && num <= 31)
    return tdep->ppc_gp0_regnum + num;
  else if (32 <= num && num <= 63)
    /* FIXME: jimb/2004-05-05: What should we do when the debug info
       specifies registers the architecture doesn't have?  Our
       callers don't check the value we return.  */
    return tdep->ppc_fp0_regnum + (num - 32);
  else if (77 <= num && num <= 108)
    return tdep->ppc_vr0_regnum + (num - 77);
  else if (1200 <= num && num < 1200 + 32)
    return tdep->ppc_ev0_regnum + (num - 1200);
  else
    switch (num)
      {
      case 64: 
        return tdep->ppc_mq_regnum;
      case 65:
        return tdep->ppc_lr_regnum;
      case 66: 
        return tdep->ppc_ctr_regnum;
      case 76: 
        return tdep->ppc_xer_regnum;
      case 109:
        return tdep->ppc_vrsave_regnum;
      case 110:
        return tdep->ppc_vrsave_regnum - 1; /* vscr */
      case 111:
        return tdep->ppc_acc_regnum;
      case 112:
        return tdep->ppc_spefscr_regnum;
      default: 
        return num;
      }
}


/* Convert a Dwarf 2 register number to a GDB register number.  */
static int
rs6000_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int num)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (0 <= num && num <= 31)
    return tdep->ppc_gp0_regnum + num;
  else if (32 <= num && num <= 63)
    /* FIXME: jimb/2004-05-05: What should we do when the debug info
       specifies registers the architecture doesn't have?  Our
       callers don't check the value we return.  */
    return tdep->ppc_fp0_regnum + (num - 32);
  else if (1124 <= num && num < 1124 + 32)
    return tdep->ppc_vr0_regnum + (num - 1124);
  else if (1200 <= num && num < 1200 + 32)
    return tdep->ppc_ev0_regnum + (num - 1200);
  else
    switch (num)
      {
      case 64:
	return tdep->ppc_cr_regnum;
      case 67:
        return tdep->ppc_vrsave_regnum - 1; /* vscr */
      case 99:
        return tdep->ppc_acc_regnum;
      case 100:
        return tdep->ppc_mq_regnum;
      case 101:
        return tdep->ppc_xer_regnum;
      case 108:
        return tdep->ppc_lr_regnum;
      case 109:
        return tdep->ppc_ctr_regnum;
      case 356:
        return tdep->ppc_vrsave_regnum;
      case 612:
        return tdep->ppc_spefscr_regnum;
      default:
        return num;
      }
}

/* Translate a .eh_frame register to DWARF register, or adjust a
   .debug_frame register.  */

static int
rs6000_adjust_frame_regnum (struct gdbarch *gdbarch, int num, int eh_frame_p)
{
  /* GCC releases before 3.4 use GCC internal register numbering in
     .debug_frame (and .debug_info, et cetera).  The numbering is
     different from the standard SysV numbering for everything except
     for GPRs and FPRs.  We can not detect this problem in most cases
     - to get accurate debug info for variables living in lr, ctr, v0,
     et cetera, use a newer version of GCC.  But we must detect
     one important case - lr is in column 65 in .debug_frame output,
     instead of 108.

     GCC 3.4, and the "hammer" branch, have a related problem.  They
     record lr register saves in .debug_frame as 108, but still record
     the return column as 65.  We fix that up too.

     We can do this because 65 is assigned to fpsr, and GCC never
     generates debug info referring to it.  To add support for
     handwritten debug info that restores fpsr, we would need to add a
     producer version check to this.  */
  if (!eh_frame_p)
    {
      if (num == 65)
	return 108;
      else
	return num;
    }

  /* .eh_frame is GCC specific.  For binary compatibility, it uses GCC
     internal register numbering; translate that to the standard DWARF2
     register numbering.  */
  if (0 <= num && num <= 63)	/* r0-r31,fp0-fp31 */
    return num;
  else if (68 <= num && num <= 75) /* cr0-cr8 */
    return num - 68 + 86;
  else if (77 <= num && num <= 108) /* vr0-vr31 */
    return num - 77 + 1124;
  else
    switch (num)
      {
      case 64: /* mq */
	return 100;
      case 65: /* lr */
	return 108;
      case 66: /* ctr */
	return 109;
      case 76: /* xer */
	return 101;
      case 109: /* vrsave */
	return 356;
      case 110: /* vscr */
	return 67;
      case 111: /* spe_acc */
	return 99;
      case 112: /* spefscr */
	return 612;
      default:
	return num;
      }
}

/* Support for CONVERT_FROM_FUNC_PTR_ADDR (ARCH, ADDR, TARG).

   Usually a function pointer's representation is simply the address
   of the function. On the RS/6000 however, a function pointer is
   represented by a pointer to an OPD entry. This OPD entry contains
   three words, the first word is the address of the function, the
   second word is the TOC pointer (r2), and the third word is the
   static chain value.  Throughout GDB it is currently assumed that a
   function pointer contains the address of the function, which is not
   easy to fix.  In addition, the conversion of a function address to
   a function pointer would require allocation of an OPD entry in the
   inferior's memory space, with all its drawbacks.  To be able to
   call C++ virtual methods in the inferior (which are called via
   function pointers), find_function_addr uses this function to get the
   function address from a function pointer.  */

/* Return real function address if ADDR (a function pointer) is in the data
   space and is therefore a special function pointer.  */

static CORE_ADDR
rs6000_convert_from_func_ptr_addr (struct gdbarch *gdbarch,
				   CORE_ADDR addr,
				   struct target_ops *targ)
{
  struct obj_section *s;

  s = find_pc_section (addr);
  if (s && s->the_bfd_section->flags & SEC_CODE)
    return addr;

  /* ADDR is in the data space, so it's a special function pointer. */
  return read_memory_addr (addr, gdbarch_tdep (gdbarch)->wordsize);
}


/* Handling the various POWER/PowerPC variants.  */

/* Information about a particular processor variant.  */

struct variant
  {
    /* Name of this variant.  */
    char *name;

    /* English description of the variant.  */
    char *description;

    /* bfd_arch_info.arch corresponding to variant.  */
    enum bfd_architecture arch;

    /* bfd_arch_info.mach corresponding to variant.  */
    unsigned long mach;

    /* Target description for this variant.  */
    struct target_desc **tdesc;
  };

static struct variant variants[] =
{
  {"powerpc", "PowerPC user-level", bfd_arch_powerpc,
   bfd_mach_ppc, &tdesc_powerpc_32},
  {"power", "POWER user-level", bfd_arch_rs6000,
   bfd_mach_rs6k, &tdesc_rs6000},
  {"403", "IBM PowerPC 403", bfd_arch_powerpc,
   bfd_mach_ppc_403, &tdesc_powerpc_403},
  {"601", "Motorola PowerPC 601", bfd_arch_powerpc,
   bfd_mach_ppc_601, &tdesc_powerpc_601},
  {"602", "Motorola PowerPC 602", bfd_arch_powerpc,
   bfd_mach_ppc_602, &tdesc_powerpc_602},
  {"603", "Motorola/IBM PowerPC 603 or 603e", bfd_arch_powerpc,
   bfd_mach_ppc_603, &tdesc_powerpc_603},
  {"604", "Motorola PowerPC 604 or 604e", bfd_arch_powerpc,
   604, &tdesc_powerpc_604},
  {"403GC", "IBM PowerPC 403GC", bfd_arch_powerpc,
   bfd_mach_ppc_403gc, &tdesc_powerpc_403gc},
  {"505", "Motorola PowerPC 505", bfd_arch_powerpc,
   bfd_mach_ppc_505, &tdesc_powerpc_505},
  {"860", "Motorola PowerPC 860 or 850", bfd_arch_powerpc,
   bfd_mach_ppc_860, &tdesc_powerpc_860},
  {"750", "Motorola/IBM PowerPC 750 or 740", bfd_arch_powerpc,
   bfd_mach_ppc_750, &tdesc_powerpc_750},
  {"7400", "Motorola/IBM PowerPC 7400 (G4)", bfd_arch_powerpc,
   bfd_mach_ppc_7400, &tdesc_powerpc_7400},
  {"e500", "Motorola PowerPC e500", bfd_arch_powerpc,
   bfd_mach_ppc_e500, &tdesc_powerpc_e500},

  /* 64-bit */
  {"powerpc64", "PowerPC 64-bit user-level", bfd_arch_powerpc,
   bfd_mach_ppc64, &tdesc_powerpc_64},
  {"620", "Motorola PowerPC 620", bfd_arch_powerpc,
   bfd_mach_ppc_620, &tdesc_powerpc_64},
  {"630", "Motorola PowerPC 630", bfd_arch_powerpc,
   bfd_mach_ppc_630, &tdesc_powerpc_64},
  {"a35", "PowerPC A35", bfd_arch_powerpc,
   bfd_mach_ppc_a35, &tdesc_powerpc_64},
  {"rs64ii", "PowerPC rs64ii", bfd_arch_powerpc,
   bfd_mach_ppc_rs64ii, &tdesc_powerpc_64},
  {"rs64iii", "PowerPC rs64iii", bfd_arch_powerpc,
   bfd_mach_ppc_rs64iii, &tdesc_powerpc_64},

  /* FIXME: I haven't checked the register sets of the following.  */
  {"rs1", "IBM POWER RS1", bfd_arch_rs6000,
   bfd_mach_rs6k_rs1, &tdesc_rs6000},
  {"rsc", "IBM POWER RSC", bfd_arch_rs6000,
   bfd_mach_rs6k_rsc, &tdesc_rs6000},
  {"rs2", "IBM POWER RS2", bfd_arch_rs6000,
   bfd_mach_rs6k_rs2, &tdesc_rs6000},

  {0, 0, 0, 0, 0}
};

/* Return the variant corresponding to architecture ARCH and machine number
   MACH.  If no such variant exists, return null.  */

static const struct variant *
find_variant_by_arch (enum bfd_architecture arch, unsigned long mach)
{
  const struct variant *v;

  for (v = variants; v->name; v++)
    if (arch == v->arch && mach == v->mach)
      return v;

  return NULL;
}

static int
gdb_print_insn_powerpc (bfd_vma memaddr, disassemble_info *info)
{
  if (!info->disassembler_options)
    info->disassembler_options = "any";

  if (info->endian == BFD_ENDIAN_BIG)
    return print_insn_big_powerpc (memaddr, info);
  else
    return print_insn_little_powerpc (memaddr, info);
}

static CORE_ADDR
rs6000_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_unwind_register_unsigned (next_frame,
					 gdbarch_pc_regnum (gdbarch));
}

static struct frame_id
rs6000_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_id_build (frame_unwind_register_unsigned
			 (next_frame, gdbarch_sp_regnum (gdbarch)),
			frame_pc_unwind (next_frame));
}

struct rs6000_frame_cache
{
  CORE_ADDR base;
  CORE_ADDR initial_sp;
  struct trad_frame_saved_reg *saved_regs;
};

static struct rs6000_frame_cache *
rs6000_frame_cache (struct frame_info *next_frame, void **this_cache)
{
  struct rs6000_frame_cache *cache;
  struct gdbarch *gdbarch = get_frame_arch (next_frame);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  struct rs6000_framedata fdata;
  int wordsize = tdep->wordsize;
  CORE_ADDR func, pc;

  if ((*this_cache) != NULL)
    return (*this_cache);
  cache = FRAME_OBSTACK_ZALLOC (struct rs6000_frame_cache);
  (*this_cache) = cache;
  cache->saved_regs = trad_frame_alloc_saved_regs (next_frame);

  func = frame_func_unwind (next_frame, NORMAL_FRAME);
  pc = frame_pc_unwind (next_frame);
  skip_prologue (gdbarch, func, pc, &fdata);

  /* Figure out the parent's stack pointer.  */

  /* NOTE: cagney/2002-04-14: The ->frame points to the inner-most
     address of the current frame.  Things might be easier if the
     ->frame pointed to the outer-most address of the frame.  In
     the mean time, the address of the prev frame is used as the
     base address of this frame.  */
  cache->base = frame_unwind_register_unsigned
		(next_frame, gdbarch_sp_regnum (gdbarch));

  /* If the function appears to be frameless, check a couple of likely
     indicators that we have simply failed to find the frame setup.
     Two common cases of this are missing symbols (i.e.
     frame_func_unwind returns the wrong address or 0), and assembly
     stubs which have a fast exit path but set up a frame on the slow
     path.

     If the LR appears to return to this function, then presume that
     we have an ABI compliant frame that we failed to find.  */
  if (fdata.frameless && fdata.lr_offset == 0)
    {
      CORE_ADDR saved_lr;
      int make_frame = 0;

      saved_lr = frame_unwind_register_unsigned (next_frame,
						 tdep->ppc_lr_regnum);
      if (func == 0 && saved_lr == pc)
	make_frame = 1;
      else if (func != 0)
	{
	  CORE_ADDR saved_func = get_pc_function_start (saved_lr);
	  if (func == saved_func)
	    make_frame = 1;
	}

      if (make_frame)
	{
	  fdata.frameless = 0;
	  fdata.lr_offset = tdep->lr_frame_offset;
	}
    }

  if (!fdata.frameless)
    /* Frameless really means stackless.  */
    cache->base = read_memory_addr (cache->base, wordsize);

  trad_frame_set_value (cache->saved_regs,
			gdbarch_sp_regnum (gdbarch), cache->base);

  /* if != -1, fdata.saved_fpr is the smallest number of saved_fpr.
     All fpr's from saved_fpr to fp31 are saved.  */

  if (fdata.saved_fpr >= 0)
    {
      int i;
      CORE_ADDR fpr_addr = cache->base + fdata.fpr_offset;

      /* If skip_prologue says floating-point registers were saved,
         but the current architecture has no floating-point registers,
         then that's strange.  But we have no indices to even record
         the addresses under, so we just ignore it.  */
      if (ppc_floating_point_unit_p (gdbarch))
        for (i = fdata.saved_fpr; i < ppc_num_fprs; i++)
          {
            cache->saved_regs[tdep->ppc_fp0_regnum + i].addr = fpr_addr;
            fpr_addr += 8;
          }
    }

  /* if != -1, fdata.saved_gpr is the smallest number of saved_gpr.
     All gpr's from saved_gpr to gpr31 are saved.  */

  if (fdata.saved_gpr >= 0)
    {
      int i;
      CORE_ADDR gpr_addr = cache->base + fdata.gpr_offset;
      for (i = fdata.saved_gpr; i < ppc_num_gprs; i++)
	{
	  cache->saved_regs[tdep->ppc_gp0_regnum + i].addr = gpr_addr;
	  gpr_addr += wordsize;
	}
    }

  /* if != -1, fdata.saved_vr is the smallest number of saved_vr.
     All vr's from saved_vr to vr31 are saved.  */
  if (tdep->ppc_vr0_regnum != -1 && tdep->ppc_vrsave_regnum != -1)
    {
      if (fdata.saved_vr >= 0)
	{
	  int i;
	  CORE_ADDR vr_addr = cache->base + fdata.vr_offset;
	  for (i = fdata.saved_vr; i < 32; i++)
	    {
	      cache->saved_regs[tdep->ppc_vr0_regnum + i].addr = vr_addr;
	      vr_addr += register_size (gdbarch, tdep->ppc_vr0_regnum);
	    }
	}
    }

  /* if != -1, fdata.saved_ev is the smallest number of saved_ev.
     All vr's from saved_ev to ev31 are saved. ????? */
  if (tdep->ppc_ev0_regnum != -1)
    {
      if (fdata.saved_ev >= 0)
	{
	  int i;
	  CORE_ADDR ev_addr = cache->base + fdata.ev_offset;
	  for (i = fdata.saved_ev; i < ppc_num_gprs; i++)
	    {
	      cache->saved_regs[tdep->ppc_ev0_regnum + i].addr = ev_addr;
              cache->saved_regs[tdep->ppc_gp0_regnum + i].addr = ev_addr + 4;
	      ev_addr += register_size (gdbarch, tdep->ppc_ev0_regnum);
            }
	}
    }

  /* If != 0, fdata.cr_offset is the offset from the frame that
     holds the CR.  */
  if (fdata.cr_offset != 0)
    cache->saved_regs[tdep->ppc_cr_regnum].addr = cache->base + fdata.cr_offset;

  /* If != 0, fdata.lr_offset is the offset from the frame that
     holds the LR.  */
  if (fdata.lr_offset != 0)
    cache->saved_regs[tdep->ppc_lr_regnum].addr = cache->base + fdata.lr_offset;
  /* The PC is found in the link register.  */
  cache->saved_regs[gdbarch_pc_regnum (gdbarch)] =
    cache->saved_regs[tdep->ppc_lr_regnum];

  /* If != 0, fdata.vrsave_offset is the offset from the frame that
     holds the VRSAVE.  */
  if (fdata.vrsave_offset != 0)
    cache->saved_regs[tdep->ppc_vrsave_regnum].addr = cache->base + fdata.vrsave_offset;

  if (fdata.alloca_reg < 0)
    /* If no alloca register used, then fi->frame is the value of the
       %sp for this frame, and it is good enough.  */
    cache->initial_sp = frame_unwind_register_unsigned
			(next_frame, gdbarch_sp_regnum (gdbarch));
  else
    cache->initial_sp = frame_unwind_register_unsigned (next_frame,
							fdata.alloca_reg);

  return cache;
}

static void
rs6000_frame_this_id (struct frame_info *next_frame, void **this_cache,
		      struct frame_id *this_id)
{
  struct rs6000_frame_cache *info = rs6000_frame_cache (next_frame,
							this_cache);
  (*this_id) = frame_id_build (info->base,
			       frame_func_unwind (next_frame, NORMAL_FRAME));
}

static void
rs6000_frame_prev_register (struct frame_info *next_frame,
				 void **this_cache,
				 int regnum, int *optimizedp,
				 enum lval_type *lvalp, CORE_ADDR *addrp,
				 int *realnump, gdb_byte *valuep)
{
  struct rs6000_frame_cache *info = rs6000_frame_cache (next_frame,
							this_cache);
  trad_frame_get_prev_register (next_frame, info->saved_regs, regnum,
				optimizedp, lvalp, addrp, realnump, valuep);
}

static const struct frame_unwind rs6000_frame_unwind =
{
  NORMAL_FRAME,
  rs6000_frame_this_id,
  rs6000_frame_prev_register
};

static const struct frame_unwind *
rs6000_frame_sniffer (struct frame_info *next_frame)
{
  return &rs6000_frame_unwind;
}



static CORE_ADDR
rs6000_frame_base_address (struct frame_info *next_frame,
				void **this_cache)
{
  struct rs6000_frame_cache *info = rs6000_frame_cache (next_frame,
							this_cache);
  return info->initial_sp;
}

static const struct frame_base rs6000_frame_base = {
  &rs6000_frame_unwind,
  rs6000_frame_base_address,
  rs6000_frame_base_address,
  rs6000_frame_base_address
};

static const struct frame_base *
rs6000_frame_base_sniffer (struct frame_info *next_frame)
{
  return &rs6000_frame_base;
}

/* DWARF-2 frame support.  Used to handle the detection of
  clobbered registers during function calls.  */

static void
ppc_dwarf2_frame_init_reg (struct gdbarch *gdbarch, int regnum,
			    struct dwarf2_frame_state_reg *reg,
			    struct frame_info *next_frame)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  /* PPC32 and PPC64 ABI's are the same regarding volatile and
     non-volatile registers.  We will use the same code for both.  */

  /* Call-saved GP registers.  */
  if ((regnum >= tdep->ppc_gp0_regnum + 14
      && regnum <= tdep->ppc_gp0_regnum + 31)
      || (regnum == tdep->ppc_gp0_regnum + 1))
    reg->how = DWARF2_FRAME_REG_SAME_VALUE;

  /* Call-clobbered GP registers.  */
  if ((regnum >= tdep->ppc_gp0_regnum + 3
      && regnum <= tdep->ppc_gp0_regnum + 12)
      || (regnum == tdep->ppc_gp0_regnum))
    reg->how = DWARF2_FRAME_REG_UNDEFINED;

  /* Deal with FP registers, if supported.  */
  if (tdep->ppc_fp0_regnum >= 0)
    {
      /* Call-saved FP registers.  */
      if ((regnum >= tdep->ppc_fp0_regnum + 14
	  && regnum <= tdep->ppc_fp0_regnum + 31))
	reg->how = DWARF2_FRAME_REG_SAME_VALUE;

      /* Call-clobbered FP registers.  */
      if ((regnum >= tdep->ppc_fp0_regnum
	  && regnum <= tdep->ppc_fp0_regnum + 13))
	reg->how = DWARF2_FRAME_REG_UNDEFINED;
    }

  /* Deal with ALTIVEC registers, if supported.  */
  if (tdep->ppc_vr0_regnum > 0 && tdep->ppc_vrsave_regnum > 0)
    {
      /* Call-saved Altivec registers.  */
      if ((regnum >= tdep->ppc_vr0_regnum + 20
	  && regnum <= tdep->ppc_vr0_regnum + 31)
	  || regnum == tdep->ppc_vrsave_regnum)
	reg->how = DWARF2_FRAME_REG_SAME_VALUE;

      /* Call-clobbered Altivec registers.  */
      if ((regnum >= tdep->ppc_vr0_regnum
	  && regnum <= tdep->ppc_vr0_regnum + 19))
	reg->how = DWARF2_FRAME_REG_UNDEFINED;
    }

  /* Handle PC register and Stack Pointer correctly.  */
  if (regnum == gdbarch_pc_regnum (gdbarch))
    reg->how = DWARF2_FRAME_REG_RA;
  else if (regnum == gdbarch_sp_regnum (gdbarch))
    reg->how = DWARF2_FRAME_REG_CFA;
}


/* Initialize the current architecture based on INFO.  If possible, re-use an
   architecture from ARCHES, which is a list of architectures already created
   during this debugging session.

   Called e.g. at program startup, when reading a core file, and when reading
   a binary file.  */

static struct gdbarch *
rs6000_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;
  int wordsize, from_xcoff_exec, from_elf_exec;
  enum bfd_architecture arch;
  unsigned long mach;
  bfd abfd;
  int sysv_abi;
  asection *sect;
  enum auto_boolean soft_float_flag = powerpc_soft_float_global;
  int soft_float;
  enum powerpc_vector_abi vector_abi = powerpc_vector_abi_global;
  int have_fpu = 1, have_spe = 0, have_mq = 0, have_altivec = 0, have_dfp = 0;
  int tdesc_wordsize = -1;
  const struct target_desc *tdesc = info.target_desc;
  struct tdesc_arch_data *tdesc_data = NULL;
  int num_pseudoregs = 0;

  from_xcoff_exec = info.abfd && info.abfd->format == bfd_object &&
    bfd_get_flavour (info.abfd) == bfd_target_xcoff_flavour;

  from_elf_exec = info.abfd && info.abfd->format == bfd_object &&
    bfd_get_flavour (info.abfd) == bfd_target_elf_flavour;

  sysv_abi = info.abfd && bfd_get_flavour (info.abfd) == bfd_target_elf_flavour;

  /* Check word size.  If INFO is from a binary file, infer it from
     that, else choose a likely default.  */
  if (from_xcoff_exec)
    {
      if (bfd_xcoff_is_xcoff64 (info.abfd))
	wordsize = 8;
      else
	wordsize = 4;
    }
  else if (from_elf_exec)
    {
      if (elf_elfheader (info.abfd)->e_ident[EI_CLASS] == ELFCLASS64)
	wordsize = 8;
      else
	wordsize = 4;
    }
  else if (tdesc_has_registers (tdesc))
    wordsize = -1;
  else
    {
      if (info.bfd_arch_info != NULL && info.bfd_arch_info->bits_per_word != 0)
	wordsize = info.bfd_arch_info->bits_per_word /
	  info.bfd_arch_info->bits_per_byte;
      else
	wordsize = 4;
    }

  if (!from_xcoff_exec)
    {
      arch = info.bfd_arch_info->arch;
      mach = info.bfd_arch_info->mach;
    }
  else
    {
      arch = bfd_arch_powerpc;
      bfd_default_set_arch_mach (&abfd, arch, 0);
      info.bfd_arch_info = bfd_get_arch_info (&abfd);
      mach = info.bfd_arch_info->mach;
    }

  /* For e500 executables, the apuinfo section is of help here.  Such
     section contains the identifier and revision number of each
     Application-specific Processing Unit that is present on the
     chip.  The content of the section is determined by the assembler
     which looks at each instruction and determines which unit (and
     which version of it) can execute it. In our case we just look for
     the existance of the section.  */

  if (info.abfd)
    {
      sect = bfd_get_section_by_name (info.abfd, ".PPC.EMB.apuinfo");
      if (sect)
	{
	  arch = info.bfd_arch_info->arch;
	  mach = bfd_mach_ppc_e500;
	  bfd_default_set_arch_mach (&abfd, arch, mach);
	  info.bfd_arch_info = bfd_get_arch_info (&abfd);
	}
    }

  /* Find a default target description which describes our register
     layout, if we do not already have one.  */
  if (! tdesc_has_registers (tdesc))
    {
      const struct variant *v;

      /* Choose variant.  */
      v = find_variant_by_arch (arch, mach);
      if (!v)
	return NULL;

      tdesc = *v->tdesc;
    }

  gdb_assert (tdesc_has_registers (tdesc));

  /* Check any target description for validity.  */
  if (tdesc_has_registers (tdesc))
    {
      static const char *const gprs[] = {
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
      };
      static const char *const segment_regs[] = {
	"sr0", "sr1", "sr2", "sr3", "sr4", "sr5", "sr6", "sr7",
	"sr8", "sr9", "sr10", "sr11", "sr12", "sr13", "sr14", "sr15"
      };
      const struct tdesc_feature *feature;
      int i, valid_p;
      static const char *const msr_names[] = { "msr", "ps" };
      static const char *const cr_names[] = { "cr", "cnd" };
      static const char *const ctr_names[] = { "ctr", "cnt" };

      feature = tdesc_find_feature (tdesc,
				    "org.gnu.gdb.power.core");
      if (feature == NULL)
	return NULL;

      tdesc_data = tdesc_data_alloc ();

      valid_p = 1;
      for (i = 0; i < ppc_num_gprs; i++)
	valid_p &= tdesc_numbered_register (feature, tdesc_data, i, gprs[i]);
      valid_p &= tdesc_numbered_register (feature, tdesc_data, PPC_PC_REGNUM,
					  "pc");
      valid_p &= tdesc_numbered_register (feature, tdesc_data, PPC_LR_REGNUM,
					  "lr");
      valid_p &= tdesc_numbered_register (feature, tdesc_data, PPC_XER_REGNUM,
					  "xer");

      /* Allow alternate names for these registers, to accomodate GDB's
	 historic naming.  */
      valid_p &= tdesc_numbered_register_choices (feature, tdesc_data,
						  PPC_MSR_REGNUM, msr_names);
      valid_p &= tdesc_numbered_register_choices (feature, tdesc_data,
						  PPC_CR_REGNUM, cr_names);
      valid_p &= tdesc_numbered_register_choices (feature, tdesc_data,
						  PPC_CTR_REGNUM, ctr_names);

      if (!valid_p)
	{
	  tdesc_data_cleanup (tdesc_data);
	  return NULL;
	}

      have_mq = tdesc_numbered_register (feature, tdesc_data, PPC_MQ_REGNUM,
					 "mq");

      tdesc_wordsize = tdesc_register_size (feature, "pc") / 8;
      if (wordsize == -1)
	wordsize = tdesc_wordsize;

      feature = tdesc_find_feature (tdesc,
				    "org.gnu.gdb.power.fpu");
      if (feature != NULL)
	{
	  static const char *const fprs[] = {
	    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
	    "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",
	    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
	    "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
	  };
	  valid_p = 1;
	  for (i = 0; i < ppc_num_fprs; i++)
	    valid_p &= tdesc_numbered_register (feature, tdesc_data,
						PPC_F0_REGNUM + i, fprs[i]);
	  valid_p &= tdesc_numbered_register (feature, tdesc_data,
					      PPC_FPSCR_REGNUM, "fpscr");

	  if (!valid_p)
	    {
	      tdesc_data_cleanup (tdesc_data);
	      return NULL;
	    }
	  have_fpu = 1;
	}
      else
	have_fpu = 0;

      /* The DFP pseudo-registers will be available when there are floating
         point registers.  */
      have_dfp = have_fpu;

      feature = tdesc_find_feature (tdesc,
				    "org.gnu.gdb.power.altivec");
      if (feature != NULL)
	{
	  static const char *const vector_regs[] = {
	    "vr0", "vr1", "vr2", "vr3", "vr4", "vr5", "vr6", "vr7",
	    "vr8", "vr9", "vr10", "vr11", "vr12", "vr13", "vr14", "vr15",
	    "vr16", "vr17", "vr18", "vr19", "vr20", "vr21", "vr22", "vr23",
	    "vr24", "vr25", "vr26", "vr27", "vr28", "vr29", "vr30", "vr31"
	  };

	  valid_p = 1;
	  for (i = 0; i < ppc_num_gprs; i++)
	    valid_p &= tdesc_numbered_register (feature, tdesc_data,
						PPC_VR0_REGNUM + i,
						vector_regs[i]);
	  valid_p &= tdesc_numbered_register (feature, tdesc_data,
					      PPC_VSCR_REGNUM, "vscr");
	  valid_p &= tdesc_numbered_register (feature, tdesc_data,
					      PPC_VRSAVE_REGNUM, "vrsave");

	  if (have_spe || !valid_p)
	    {
	      tdesc_data_cleanup (tdesc_data);
	      return NULL;
	    }
	  have_altivec = 1;
	}
      else
	have_altivec = 0;

      /* On machines supporting the SPE APU, the general-purpose registers
	 are 64 bits long.  There are SIMD vector instructions to treat them
	 as pairs of floats, but the rest of the instruction set treats them
	 as 32-bit registers, and only operates on their lower halves.

	 In the GDB regcache, we treat their high and low halves as separate
	 registers.  The low halves we present as the general-purpose
	 registers, and then we have pseudo-registers that stitch together
	 the upper and lower halves and present them as pseudo-registers.

	 Thus, the target description is expected to supply the upper
	 halves separately.  */

      feature = tdesc_find_feature (tdesc,
				    "org.gnu.gdb.power.spe");
      if (feature != NULL)
	{
	  static const char *const upper_spe[] = {
	    "ev0h", "ev1h", "ev2h", "ev3h",
	    "ev4h", "ev5h", "ev6h", "ev7h",
	    "ev8h", "ev9h", "ev10h", "ev11h",
	    "ev12h", "ev13h", "ev14h", "ev15h",
	    "ev16h", "ev17h", "ev18h", "ev19h",
	    "ev20h", "ev21h", "ev22h", "ev23h",
	    "ev24h", "ev25h", "ev26h", "ev27h",
	    "ev28h", "ev29h", "ev30h", "ev31h"
	  };

	  valid_p = 1;
	  for (i = 0; i < ppc_num_gprs; i++)
	    valid_p &= tdesc_numbered_register (feature, tdesc_data,
						PPC_SPE_UPPER_GP0_REGNUM + i,
						upper_spe[i]);
	  valid_p &= tdesc_numbered_register (feature, tdesc_data,
					      PPC_SPE_ACC_REGNUM, "acc");
	  valid_p &= tdesc_numbered_register (feature, tdesc_data,
					      PPC_SPE_FSCR_REGNUM, "spefscr");

	  if (have_mq || have_fpu || !valid_p)
	    {
	      tdesc_data_cleanup (tdesc_data);
	      return NULL;
	    }
	  have_spe = 1;
	}
      else
	have_spe = 0;
    }

  /* If we have a 64-bit binary on a 32-bit target, complain.  Also
     complain for a 32-bit binary on a 64-bit target; we do not yet
     support that.  For instance, the 32-bit ABI routines expect
     32-bit GPRs.

     As long as there isn't an explicit target description, we'll
     choose one based on the BFD architecture and get a word size
     matching the binary (probably powerpc:common or
     powerpc:common64).  So there is only trouble if a 64-bit target
     supplies a 64-bit description while debugging a 32-bit
     binary.  */
  if (tdesc_wordsize != -1 && tdesc_wordsize != wordsize)
    {
      tdesc_data_cleanup (tdesc_data);
      return NULL;
    }

#ifdef HAVE_ELF
  if (soft_float_flag == AUTO_BOOLEAN_AUTO && from_elf_exec)
    {
      switch (bfd_elf_get_obj_attr_int (info.abfd, OBJ_ATTR_GNU,
					Tag_GNU_Power_ABI_FP))
	{
	case 1:
	  soft_float_flag = AUTO_BOOLEAN_FALSE;
	  break;
	case 2:
	  soft_float_flag = AUTO_BOOLEAN_TRUE;
	  break;
	default:
	  break;
	}
    }

  if (vector_abi == POWERPC_VEC_AUTO && from_elf_exec)
    {
      switch (bfd_elf_get_obj_attr_int (info.abfd, OBJ_ATTR_GNU,
					Tag_GNU_Power_ABI_Vector))
	{
	case 1:
	  vector_abi = POWERPC_VEC_GENERIC;
	  break;
	case 2:
	  vector_abi = POWERPC_VEC_ALTIVEC;
	  break;
	case 3:
	  vector_abi = POWERPC_VEC_SPE;
	  break;
	default:
	  break;
	}
    }
#endif

  if (soft_float_flag == AUTO_BOOLEAN_TRUE)
    soft_float = 1;
  else if (soft_float_flag == AUTO_BOOLEAN_FALSE)
    soft_float = 0;
  else
    soft_float = !have_fpu;

  /* If we have a hard float binary or setting but no floating point
     registers, downgrade to soft float anyway.  We're still somewhat
     useful in this scenario.  */
  if (!soft_float && !have_fpu)
    soft_float = 1;

  /* Similarly for vector registers.  */
  if (vector_abi == POWERPC_VEC_ALTIVEC && !have_altivec)
    vector_abi = POWERPC_VEC_GENERIC;

  if (vector_abi == POWERPC_VEC_SPE && !have_spe)
    vector_abi = POWERPC_VEC_GENERIC;

  if (vector_abi == POWERPC_VEC_AUTO)
    {
      if (have_altivec)
	vector_abi = POWERPC_VEC_ALTIVEC;
      else if (have_spe)
	vector_abi = POWERPC_VEC_SPE;
      else
	vector_abi = POWERPC_VEC_GENERIC;
    }

  /* Do not limit the vector ABI based on available hardware, since we
     do not yet know what hardware we'll decide we have.  Yuck!  FIXME!  */

  /* Find a candidate among extant architectures.  */
  for (arches = gdbarch_list_lookup_by_info (arches, &info);
       arches != NULL;
       arches = gdbarch_list_lookup_by_info (arches->next, &info))
    {
      /* Word size in the various PowerPC bfd_arch_info structs isn't
         meaningful, because 64-bit CPUs can run in 32-bit mode.  So, perform
         separate word size check.  */
      tdep = gdbarch_tdep (arches->gdbarch);
      if (tdep && tdep->soft_float != soft_float)
	continue;
      if (tdep && tdep->vector_abi != vector_abi)
	continue;
      if (tdep && tdep->wordsize == wordsize)
	{
	  if (tdesc_data != NULL)
	    tdesc_data_cleanup (tdesc_data);
	  return arches->gdbarch;
	}
    }

  /* None found, create a new architecture from INFO, whose bfd_arch_info
     validity depends on the source:
       - executable		useless
       - rs6000_host_arch()	good
       - core file		good
       - "set arch"		trust blindly
       - GDB startup		useless but harmless */

  tdep = XCALLOC (1, struct gdbarch_tdep);
  tdep->wordsize = wordsize;
  tdep->soft_float = soft_float;
  tdep->vector_abi = vector_abi;

  gdbarch = gdbarch_alloc (&info, tdep);

  tdep->ppc_gp0_regnum = PPC_R0_REGNUM;
  tdep->ppc_toc_regnum = PPC_R0_REGNUM + 2;
  tdep->ppc_ps_regnum = PPC_MSR_REGNUM;
  tdep->ppc_cr_regnum = PPC_CR_REGNUM;
  tdep->ppc_lr_regnum = PPC_LR_REGNUM;
  tdep->ppc_ctr_regnum = PPC_CTR_REGNUM;
  tdep->ppc_xer_regnum = PPC_XER_REGNUM;
  tdep->ppc_mq_regnum = have_mq ? PPC_MQ_REGNUM : -1;

  tdep->ppc_fp0_regnum = have_fpu ? PPC_F0_REGNUM : -1;
  tdep->ppc_fpscr_regnum = have_fpu ? PPC_FPSCR_REGNUM : -1;
  tdep->ppc_vr0_regnum = have_altivec ? PPC_VR0_REGNUM : -1;
  tdep->ppc_vrsave_regnum = have_altivec ? PPC_VRSAVE_REGNUM : -1;
  tdep->ppc_ev0_upper_regnum = have_spe ? PPC_SPE_UPPER_GP0_REGNUM : -1;
  tdep->ppc_acc_regnum = have_spe ? PPC_SPE_ACC_REGNUM : -1;
  tdep->ppc_spefscr_regnum = have_spe ? PPC_SPE_FSCR_REGNUM : -1;

  set_gdbarch_pc_regnum (gdbarch, PPC_PC_REGNUM);
  set_gdbarch_sp_regnum (gdbarch, PPC_R0_REGNUM + 1);
  set_gdbarch_deprecated_fp_regnum (gdbarch, PPC_R0_REGNUM + 1);
  set_gdbarch_fp0_regnum (gdbarch, tdep->ppc_fp0_regnum);
  set_gdbarch_register_sim_regno (gdbarch, rs6000_register_sim_regno);

  /* The XML specification for PowerPC sensibly calls the MSR "msr".
     GDB traditionally called it "ps", though, so let GDB add an
     alias.  */
  set_gdbarch_ps_regnum (gdbarch, tdep->ppc_ps_regnum);

  if (sysv_abi && wordsize == 8)
    set_gdbarch_return_value (gdbarch, ppc64_sysv_abi_return_value);
  else if (sysv_abi && wordsize == 4)
    set_gdbarch_return_value (gdbarch, ppc_sysv_abi_return_value);
  else
    set_gdbarch_return_value (gdbarch, rs6000_return_value);

  /* Set lr_frame_offset.  */
  if (wordsize == 8)
    tdep->lr_frame_offset = 16;
  else if (sysv_abi)
    tdep->lr_frame_offset = 4;
  else
    tdep->lr_frame_offset = 8;

  if (have_spe || have_dfp)
    {
      set_gdbarch_pseudo_register_read (gdbarch, rs6000_pseudo_register_read);
      set_gdbarch_pseudo_register_write (gdbarch, rs6000_pseudo_register_write);
    }

  set_gdbarch_have_nonsteppable_watchpoint (gdbarch, 1);

  /* Select instruction printer.  */
  if (arch == bfd_arch_rs6000)
    set_gdbarch_print_insn (gdbarch, print_insn_rs6000);
  else
    set_gdbarch_print_insn (gdbarch, gdb_print_insn_powerpc);

  set_gdbarch_num_regs (gdbarch, PPC_NUM_REGS);

  if (have_spe)
    num_pseudoregs += 32;
  if (have_dfp)
    num_pseudoregs += 16;

  set_gdbarch_num_pseudo_regs (gdbarch, num_pseudoregs);

  set_gdbarch_ptr_bit (gdbarch, wordsize * TARGET_CHAR_BIT);
  set_gdbarch_short_bit (gdbarch, 2 * TARGET_CHAR_BIT);
  set_gdbarch_int_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_long_bit (gdbarch, wordsize * TARGET_CHAR_BIT);
  set_gdbarch_long_long_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_float_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_double_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  if (sysv_abi)
    set_gdbarch_long_double_bit (gdbarch, 16 * TARGET_CHAR_BIT);
  else
    set_gdbarch_long_double_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_char_signed (gdbarch, 0);

  set_gdbarch_frame_align (gdbarch, rs6000_frame_align);
  if (sysv_abi && wordsize == 8)
    /* PPC64 SYSV.  */
    set_gdbarch_frame_red_zone_size (gdbarch, 288);
  else if (!sysv_abi && wordsize == 4)
    /* PowerOpen / AIX 32 bit.  The saved area or red zone consists of
       19 4 byte GPRS + 18 8 byte FPRs giving a total of 220 bytes.
       Problem is, 220 isn't frame (16 byte) aligned.  Round it up to
       224.  */
    set_gdbarch_frame_red_zone_size (gdbarch, 224);

  set_gdbarch_convert_register_p (gdbarch, rs6000_convert_register_p);
  set_gdbarch_register_to_value (gdbarch, rs6000_register_to_value);
  set_gdbarch_value_to_register (gdbarch, rs6000_value_to_register);

  set_gdbarch_stab_reg_to_regnum (gdbarch, rs6000_stab_reg_to_regnum);
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, rs6000_dwarf2_reg_to_regnum);

  if (sysv_abi && wordsize == 4)
    set_gdbarch_push_dummy_call (gdbarch, ppc_sysv_abi_push_dummy_call);
  else if (sysv_abi && wordsize == 8)
    set_gdbarch_push_dummy_call (gdbarch, ppc64_sysv_abi_push_dummy_call);
  else
    set_gdbarch_push_dummy_call (gdbarch, rs6000_push_dummy_call);

  set_gdbarch_skip_prologue (gdbarch, rs6000_skip_prologue);
  set_gdbarch_in_function_epilogue_p (gdbarch, rs6000_in_function_epilogue_p);

  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_breakpoint_from_pc (gdbarch, rs6000_breakpoint_from_pc);

  /* The value of symbols of type N_SO and N_FUN maybe null when
     it shouldn't be. */
  set_gdbarch_sofun_address_maybe_missing (gdbarch, 1);

  /* Handles single stepping of atomic sequences.  */
  set_gdbarch_software_single_step (gdbarch, deal_with_atomic_sequence);
  
  /* Handle the 64-bit SVR4 minimal-symbol convention of using "FN"
     for the descriptor and ".FN" for the entry-point -- a user
     specifying "break FN" will unexpectedly end up with a breakpoint
     on the descriptor and not the function.  This architecture method
     transforms any breakpoints on descriptors into breakpoints on the
     corresponding entry point.  */
  if (sysv_abi && wordsize == 8)
    set_gdbarch_adjust_breakpoint_address (gdbarch, ppc64_sysv_abi_adjust_breakpoint_address);

  /* Not sure on this. FIXMEmgo */
  set_gdbarch_frame_args_skip (gdbarch, 8);

  if (!sysv_abi)
    {
      /* Handle RS/6000 function pointers (which are really function
         descriptors).  */
      set_gdbarch_convert_from_func_ptr_addr (gdbarch,
	rs6000_convert_from_func_ptr_addr);
    }

  /* Helpers for function argument information.  */
  set_gdbarch_fetch_pointer_argument (gdbarch, rs6000_fetch_pointer_argument);

  /* Trampoline.  */
  set_gdbarch_in_solib_return_trampoline
    (gdbarch, rs6000_in_solib_return_trampoline);
  set_gdbarch_skip_trampoline_code (gdbarch, rs6000_skip_trampoline_code);

  /* Hook in the DWARF CFI frame unwinder.  */
  frame_unwind_append_sniffer (gdbarch, dwarf2_frame_sniffer);
  dwarf2_frame_set_adjust_regnum (gdbarch, rs6000_adjust_frame_regnum);

  /* Frame handling.  */
  dwarf2_frame_set_init_reg (gdbarch, ppc_dwarf2_frame_init_reg);

  /* Hook in ABI-specific overrides, if they have been registered.  */
  gdbarch_init_osabi (info, gdbarch);

  switch (info.osabi)
    {
    case GDB_OSABI_LINUX:
    case GDB_OSABI_NETBSD_AOUT:
    case GDB_OSABI_NETBSD_ELF:
    case GDB_OSABI_UNKNOWN:
      set_gdbarch_unwind_pc (gdbarch, rs6000_unwind_pc);
      frame_unwind_append_sniffer (gdbarch, rs6000_frame_sniffer);
      set_gdbarch_unwind_dummy_id (gdbarch, rs6000_unwind_dummy_id);
      frame_base_append_sniffer (gdbarch, rs6000_frame_base_sniffer);
      break;
    default:
      set_gdbarch_believe_pcc_promotion (gdbarch, 1);

      set_gdbarch_unwind_pc (gdbarch, rs6000_unwind_pc);
      frame_unwind_append_sniffer (gdbarch, rs6000_frame_sniffer);
      set_gdbarch_unwind_dummy_id (gdbarch, rs6000_unwind_dummy_id);
      frame_base_append_sniffer (gdbarch, rs6000_frame_base_sniffer);
    }

  set_tdesc_pseudo_register_type (gdbarch, rs6000_pseudo_register_type);
  set_tdesc_pseudo_register_reggroup_p (gdbarch,
					rs6000_pseudo_register_reggroup_p);
  tdesc_use_registers (gdbarch, tdesc, tdesc_data);

  /* Override the normal target description method to make the SPE upper
     halves anonymous.  */
  set_gdbarch_register_name (gdbarch, rs6000_register_name);

  /* Recording the numbering of pseudo registers.  */
  tdep->ppc_ev0_regnum = have_spe ? gdbarch_num_regs (gdbarch) : -1;

  /* Set the register number for _Decimal128 pseudo-registers.  */
  tdep->ppc_dl0_regnum = have_dfp? gdbarch_num_regs (gdbarch) : -1;

  if (have_dfp && have_spe)
    /* Put the _Decimal128 pseudo-registers after the SPE registers.  */
    tdep->ppc_dl0_regnum += 32;

  return gdbarch;
}

static void
rs6000_dump_tdep (struct gdbarch *gdbarch, struct ui_file *file)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (tdep == NULL)
    return;

  /* FIXME: Dump gdbarch_tdep.  */
}

/* PowerPC-specific commands.  */

static void
set_powerpc_command (char *args, int from_tty)
{
  printf_unfiltered (_("\
\"set powerpc\" must be followed by an appropriate subcommand.\n"));
  help_list (setpowerpccmdlist, "set powerpc ", all_commands, gdb_stdout);
}

static void
show_powerpc_command (char *args, int from_tty)
{
  cmd_show_list (showpowerpccmdlist, from_tty, "");
}

static void
powerpc_set_soft_float (char *args, int from_tty,
			struct cmd_list_element *c)
{
  struct gdbarch_info info;

  /* Update the architecture.  */
  gdbarch_info_init (&info);
  if (!gdbarch_update_p (info))
    internal_error (__FILE__, __LINE__, "could not update architecture");
}

static void
powerpc_set_vector_abi (char *args, int from_tty,
			struct cmd_list_element *c)
{
  struct gdbarch_info info;
  enum powerpc_vector_abi vector_abi;

  for (vector_abi = POWERPC_VEC_AUTO;
       vector_abi != POWERPC_VEC_LAST;
       vector_abi++)
    if (strcmp (powerpc_vector_abi_string,
		powerpc_vector_strings[vector_abi]) == 0)
      {
	powerpc_vector_abi_global = vector_abi;
	break;
      }

  if (vector_abi == POWERPC_VEC_LAST)
    internal_error (__FILE__, __LINE__, _("Invalid vector ABI accepted: %s."),
		    powerpc_vector_abi_string);

  /* Update the architecture.  */
  gdbarch_info_init (&info);
  if (!gdbarch_update_p (info))
    internal_error (__FILE__, __LINE__, "could not update architecture");
}

/* Initialization code.  */

extern initialize_file_ftype _initialize_rs6000_tdep; /* -Wmissing-prototypes */

void
_initialize_rs6000_tdep (void)
{
  gdbarch_register (bfd_arch_rs6000, rs6000_gdbarch_init, rs6000_dump_tdep);
  gdbarch_register (bfd_arch_powerpc, rs6000_gdbarch_init, rs6000_dump_tdep);

  /* Initialize the standard target descriptions.  */
  initialize_tdesc_powerpc_32 ();
  initialize_tdesc_powerpc_403 ();
  initialize_tdesc_powerpc_403gc ();
  initialize_tdesc_powerpc_505 ();
  initialize_tdesc_powerpc_601 ();
  initialize_tdesc_powerpc_602 ();
  initialize_tdesc_powerpc_603 ();
  initialize_tdesc_powerpc_604 ();
  initialize_tdesc_powerpc_64 ();
  initialize_tdesc_powerpc_7400 ();
  initialize_tdesc_powerpc_750 ();
  initialize_tdesc_powerpc_860 ();
  initialize_tdesc_powerpc_e500 ();
  initialize_tdesc_rs6000 ();

  /* Add root prefix command for all "set powerpc"/"show powerpc"
     commands.  */
  add_prefix_cmd ("powerpc", no_class, set_powerpc_command,
		  _("Various PowerPC-specific commands."),
		  &setpowerpccmdlist, "set powerpc ", 0, &setlist);

  add_prefix_cmd ("powerpc", no_class, show_powerpc_command,
		  _("Various PowerPC-specific commands."),
		  &showpowerpccmdlist, "show powerpc ", 0, &showlist);

  /* Add a command to allow the user to force the ABI.  */
  add_setshow_auto_boolean_cmd ("soft-float", class_support,
				&powerpc_soft_float_global,
				_("Set whether to use a soft-float ABI."),
				_("Show whether to use a soft-float ABI."),
				NULL,
				powerpc_set_soft_float, NULL,
				&setpowerpccmdlist, &showpowerpccmdlist);

  add_setshow_enum_cmd ("vector-abi", class_support, powerpc_vector_strings,
			&powerpc_vector_abi_string,
			_("Set the vector ABI."),
			_("Show the vector ABI."),
			NULL, powerpc_set_vector_abi, NULL,
			&setpowerpccmdlist, &showpowerpccmdlist);
}
