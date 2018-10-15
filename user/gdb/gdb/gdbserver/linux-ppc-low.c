/* GNU/Linux/PowerPC specific low level interface, for the remote server for
   GDB.
   Copyright (C) 1995, 1996, 1998, 1999, 2000, 2001, 2002, 2005, 2007, 2008
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

#include "server.h"
#include "linux-low.h"

#include <asm/ptrace.h>

#define ppc_num_regs 71

/* Currently, don't check/send MQ.  */
static int ppc_regmap[] =
 {PT_R0 * 4,     PT_R1 * 4,     PT_R2 * 4,     PT_R3 * 4,
  PT_R4 * 4,     PT_R5 * 4,     PT_R6 * 4,     PT_R7 * 4,
  PT_R8 * 4,     PT_R9 * 4,     PT_R10 * 4,    PT_R11 * 4,
  PT_R12 * 4,    PT_R13 * 4,    PT_R14 * 4,    PT_R15 * 4,
  PT_R16 * 4,    PT_R17 * 4,    PT_R18 * 4,    PT_R19 * 4,
  PT_R20 * 4,    PT_R21 * 4,    PT_R22 * 4,    PT_R23 * 4,
  PT_R24 * 4,    PT_R25 * 4,    PT_R26 * 4,    PT_R27 * 4,
  PT_R28 * 4,    PT_R29 * 4,    PT_R30 * 4,    PT_R31 * 4,
#ifdef __SPE__
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
#else
  PT_FPR0*4,     PT_FPR0*4 + 8, PT_FPR0*4+16,  PT_FPR0*4+24,
  PT_FPR0*4+32,  PT_FPR0*4+40,  PT_FPR0*4+48,  PT_FPR0*4+56,
  PT_FPR0*4+64,  PT_FPR0*4+72,  PT_FPR0*4+80,  PT_FPR0*4+88,
  PT_FPR0*4+96,  PT_FPR0*4+104,  PT_FPR0*4+112,  PT_FPR0*4+120,
  PT_FPR0*4+128, PT_FPR0*4+136,  PT_FPR0*4+144,  PT_FPR0*4+152,
  PT_FPR0*4+160,  PT_FPR0*4+168,  PT_FPR0*4+176,  PT_FPR0*4+184,
  PT_FPR0*4+192,  PT_FPR0*4+200,  PT_FPR0*4+208,  PT_FPR0*4+216,
  PT_FPR0*4+224,  PT_FPR0*4+232,  PT_FPR0*4+240,  PT_FPR0*4+248,
#endif
  PT_NIP * 4,    PT_MSR * 4,    PT_CCR * 4,    PT_LNK * 4,
#ifdef __SPE__
  PT_CTR * 4,    PT_XER * 4,    -1
#else
  PT_CTR * 4,    PT_XER * 4,    PT_FPSCR * 4
#endif
 };

static int
ppc_cannot_store_register (int regno)
{
#ifndef __SPE__
  /* Some kernels do not allow us to store fpscr.  */
  if (regno == find_regno ("fpscr"))
    return 2;
#endif

  return 0;
}

static int
ppc_cannot_fetch_register (int regno)
{
  return 0;
}

static CORE_ADDR
ppc_get_pc (void)
{
  unsigned long pc;

  collect_register_by_name ("pc", &pc);
  return (CORE_ADDR) pc;
}

static void
ppc_set_pc (CORE_ADDR pc)
{
  unsigned long newpc = pc;

  supply_register_by_name ("pc", &newpc);
}

/* Correct in either endianness.  Note that this file is
   for PowerPC only, not PowerPC64.
   This instruction is "twge r2, r2", which GDB uses as a software
   breakpoint.  */
static const unsigned long ppc_breakpoint = 0x7d821008;
#define ppc_breakpoint_len 4

static int
ppc_breakpoint_at (CORE_ADDR where)
{
  unsigned long insn;

  (*the_target->read_memory) (where, (unsigned char *) &insn, 4);
  if (insn == ppc_breakpoint)
    return 1;
  /* If necessary, recognize more trap instructions here.  GDB only uses the
     one.  */
  return 0;
}

/* Provide only a fill function for the general register set.  ps_lgetregs
   will use this for NPTL support.  */

static void ppc_fill_gregset (void *buf)
{
  int i;

  for (i = 0; i < 32; i++)
    collect_register (i, (char *) buf + ppc_regmap[i]);

  for (i = 64; i < 70; i++)
    collect_register (i, (char *) buf + ppc_regmap[i]);
}

#ifdef __ALTIVEC__

#ifndef PTRACE_GETVRREGS
#define PTRACE_GETVRREGS 18
#define PTRACE_SETVRREGS 19
#endif

#define SIZEOF_VRREGS 33*16+4

static void
ppc_fill_vrregset (void *buf)
{
  int i, base;
  char *regset = buf;

  base = find_regno ("vr0");
  for (i = 0; i < 32; i++)
    collect_register (base + i, &regset[i * 16]);

  collect_register_by_name ("vscr", &regset[32 * 16 + 12]);
  collect_register_by_name ("vrsave", &regset[33 * 16]);
}

static void
ppc_store_vrregset (const void *buf)
{
  int i, base;
  const char *regset = buf;

  base = find_regno ("vr0");
  for (i = 0; i < 32; i++)
    supply_register (base + i, &regset[i * 16]);

  supply_register_by_name ("vscr", &regset[32 * 16 + 12]);
  supply_register_by_name ("vrsave", &regset[33 * 16]);
}

#endif /* __ALTIVEC__ */

#ifdef __SPE__

#ifndef PTRACE_GETEVRREGS
#define PTRACE_GETEVRREGS	20
#define PTRACE_SETEVRREGS	21
#endif

struct gdb_evrregset_t
{
  unsigned long evr[32];
  unsigned long long acc;
  unsigned long spefscr;
};

static void
ppc_fill_evrregset (void *buf)
{
  int i, ev0;
  struct gdb_evrregset_t *regset = buf;

  ev0 = find_regno ("ev0h");
  for (i = 0; i < 32; i++)
    collect_register (ev0 + i, &regset->evr[i]);

  collect_register_by_name ("acc", &regset->acc);
  collect_register_by_name ("spefscr", &regset->spefscr);
}

static void
ppc_store_evrregset (const void *buf)
{
  int i, ev0;
  const struct gdb_evrregset_t *regset = buf;

  ev0 = find_regno ("ev0h");
  for (i = 0; i < 32; i++)
    supply_register (ev0 + i, &regset->evr[i]);

  supply_register_by_name ("acc", &regset->acc);
  supply_register_by_name ("spefscr", &regset->spefscr);
}
#endif /* __SPE__ */

struct regset_info target_regsets[] = {
  /* List the extra register sets before GENERAL_REGS.  That way we will
     fetch them every time, but still fall back to PTRACE_PEEKUSER for the
     general registers.  Some kernels support these, but not the newer
     PPC_PTRACE_GETREGS.  */
#ifdef __ALTIVEC__
  { PTRACE_GETVRREGS, PTRACE_SETVRREGS, SIZEOF_VRREGS, EXTENDED_REGS,
    ppc_fill_vrregset, ppc_store_vrregset },
#endif
#ifdef __SPE__
  { PTRACE_GETEVRREGS, PTRACE_SETEVRREGS, 32 * 4 + 8 + 4, EXTENDED_REGS,
    ppc_fill_evrregset, ppc_store_evrregset },
#endif
  { 0, 0, 0, GENERAL_REGS, ppc_fill_gregset, NULL },
  { 0, 0, -1, -1, NULL, NULL }
};

struct linux_target_ops the_low_target = {
  ppc_num_regs,
  ppc_regmap,
  ppc_cannot_fetch_register,
  ppc_cannot_store_register,
  ppc_get_pc,
  ppc_set_pc,
  (const unsigned char *) &ppc_breakpoint,
  ppc_breakpoint_len,
  NULL,
  0,
  ppc_breakpoint_at,
};
