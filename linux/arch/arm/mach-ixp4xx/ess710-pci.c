/*
 * arch/arm/mach-ixp4xx/ess710-pci.c 
 *
 * ESS710 board-level PCI initialization
 * Copyright (C) 2004 SnapGear - A CyberGuard Company
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/mach/pci.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>

#define IRQ_ESS710_PCI_INTA	IRQ_IXP4XX_GPIO6
#define IRQ_ESS710_PCI_INTB	IRQ_IXP4XX_GPIO7
#define IRQ_ESS710_PCI_INTC	IRQ_IXP4XX_GPIO8

void __init ess710_pci_preinit(void)
{
	printk("PCI: reset bus...\n");
	gpio_line_set(13, 0);
	gpio_line_config(13, IXP4XX_GPIO_OUT);
	gpio_line_set(13, 0);
	mdelay(50);
	gpio_line_set(13, 1);
	mdelay(50);

	gpio_line_config(6, IXP4XX_GPIO_IN);
	irq_set_irq_type(IRQ_IXP4XX_GPIO6, IRQ_TYPE_LEVEL_LOW); /* INTA */
	gpio_line_config(7, IXP4XX_GPIO_IN);
	irq_set_irq_type(IRQ_IXP4XX_GPIO7, IRQ_TYPE_LEVEL_LOW); /* INTB */
	gpio_line_config(8, IXP4XX_GPIO_IN);
	irq_set_irq_type(IRQ_IXP4XX_GPIO8, IRQ_TYPE_LEVEL_LOW); /* INTC */

	ixp4xx_pci_preinit();
}

static int __init ess710_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 16)
		return IRQ_ESS710_PCI_INTA;
	else if (slot == 15)
		return IRQ_ESS710_PCI_INTB;
	else if (slot == 14)
		return IRQ_ESS710_PCI_INTC;
	else if (slot == 13)
		return IRQ_ESS710_PCI_INTC;
	return -1;
}

struct hw_pci ess710_pci __initdata = {
	.nr_controllers = 1,
	.ops		= &ixp4xx_ops,
	.preinit	= ess710_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= ess710_map_irq,
};

int __init ess710_pci_init(void)
{
	if (machine_is_ess710())
		pci_common_init(&ess710_pci);
	return 0;
}

subsys_initcall(ess710_pci_init);

