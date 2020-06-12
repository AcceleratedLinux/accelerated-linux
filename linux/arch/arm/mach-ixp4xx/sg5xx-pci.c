/*
 * arch/arch/mach-ixp4xx/sg5xx-pci.c
 *
 * PCI setup routines for Cyberguard/SnapGear SG5XX family boards
 *
 * Copyright (C) 2005-2008 SnapGear Inc
 * Copyright (C) 2004 MontaVista Softwrae, Inc.
 *
 * Maintainer: Cyberguard Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/pci.h>
#include "irqs.h"

void __init sg5xx_pci_preinit(void)
{
	irq_set_irq_type(IRQ_IXP4XX_GPIO8, IRQ_TYPE_LEVEL_LOW);
	ixp4xx_pci_preinit();
}

static int __init sg5xx_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 12 || slot == 14)
		return IRQ_IXP4XX_GPIO8;
	return -1;
}

struct hw_pci sg5xx_pci __initdata = {
	.nr_controllers = 1,
	.ops		= &ixp4xx_ops,
	.preinit	= sg5xx_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= sg5xx_map_irq,
};

int __init sg5xx_pci_init(void)
{
	if (machine_is_sg560usb() || machine_is_sg560adsl() ||
	    machine_is_sg565() || machine_is_sg8100())
		pci_common_init(&sg5xx_pci);
	return 0;
}

subsys_initcall(sg5xx_pci_init);
