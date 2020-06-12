/*
 * arch/arm/mach-ixp4xx/sg8100-pci.c 
 *
 * SG8100 board-level PCI initialization
 * Copyright (C) 2004-2008 SnapGear - A division of Secure Computing
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/pci.h>

#include "irqs.h"

#define IRQ_SG8100_PCI_INTA	IRQ_IXP4XX_GPIO8

void __init sg8100_pci_preinit(void)
{
	gpio_line_config(8, IXP4XX_GPIO_IN);
	irq_set_irq_type(IRQ_IXP4XX_GPIO8, IRQ_TYPE_LEVEL_LOW); /* INTA */
	ixp4xx_pci_preinit();
}

static int __init sg8100_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 16)
		return IRQ_SG8100_PCI_INTA;
	return -1;
}

void sg8100_cardbus_fixup(struct pci_dev *dev)
{
	static int once = 0;
	u32 ba, scr, mfr;
	u16 bcr;
	u8 dc;

	if (once++)
		return;

	/* Leave the cardbus slots in the reset state for now */
	bcr = 0x0340;
	pci_write_config_word(dev, 0x3e, bcr);

	/* Set to use serialized interrupts - the power on default */
	dc = 0x66;
	pci_write_config_byte(dev, 0x92, dc);

	/* Enable MFUNC0 to be interrupt source for slot */
	scr = 0x28449060;
	pci_write_config_dword(dev, 0x80, scr);

	mfr = 0x00001002;
	pci_write_config_dword(dev, 0x8c, mfr);

#if 0
	/* Turn of power to cardbus slot */
	ba = 0;
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &ba);
	if (ba) {
		/* Request power off on cardbus slot */
		writel(0, ba+0x4); /* MASK */
		writel(0, ba+0x10); /* CONTROL */
	}
#endif
}

struct hw_pci sg8100_pci __initdata = {
	.nr_controllers = 1,
	.ops		= &ixp4xx_ops,
	.preinit	= sg8100_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= sg8100_map_irq,
};

int __init sg8100_pci_init(void)
{
	if (machine_is_sg8100())
		pci_common_init(&sg8100_pci);
	return 0;
}

subsys_initcall(sg8100_pci_init);

