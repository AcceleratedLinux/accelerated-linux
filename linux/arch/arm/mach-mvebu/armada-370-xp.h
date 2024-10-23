/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Generic definitions for Marvell Armada_370_XP SoCs
 *
 * Copyright (C) 2012 Marvell
 *
 * Lior Amsalem <alior@marvell.com>
 * Gregory CLEMENT <gregory.clement@free-electrons.com>
 * Thomas Petazzoni <thomas.petazzoni@free-electrons.com>
 */

#ifndef __MACH_ARMADA_370_XP_H
#define __MACH_ARMADA_370_XP_H

/* For legacy drivers (like ocf/cssa crypto) */
#define ARMADA_370_XP_REGS_PHYS_BASE	0xd0000000
#define ARMADA_370_XP_REGS_VIRT_BASE	IOMEM(0xfec00000)
#define ARMADA_370_XP_REGS_SIZE		SZ_1M
#define ARMADA_380_REGS_PHYS_BASE	0xf1000000
#define ARMADA_380_REGS_VIRT_BASE	IOMEM(0xfec00000)
#define ARMADA_380_REGS_SIZE		SZ_1M

#ifdef CONFIG_SMP
void armada_xp_secondary_startup(void);
extern const struct smp_operations armada_xp_smp_ops;
#endif

#endif /* __MACH_ARMADA_370_XP_H */
