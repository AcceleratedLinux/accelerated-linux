/*
 * gpio-amdfch.c -- AMD Fusion Controller Hub GPIO driver
 *
 * Copyright 2015, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/io.h>

/*
 * Register set offsets within the AcpiMmioAddr region.
 */
#define GPIO_BASE	0x1500
#define GPIO_SIZE	0x300
#define IOMUX_BASE	0x0d00
#define IOMUX_SIZE	0x100

/*
 * Bit definitions for the GPIO setup registers.
 */
#define GPIO_OUTENABLE	0x00800000
#define GPIO_OUT	0x00400000
#define GPIO_IN		0x00010000


struct amdfch_gpio {
	struct gpio_chip	chip;
	void __iomem		*mapbase;
	u32			base;
};

/*
 * Mapping from "FT3b Package Name" (GPIO number) to MUX register number.
 * The AMD documented tables for the IOMUX and GPIO registers are _very_
 * confusing. The table below distills out the vital info, so we can map
 * from the hardware GPIO pin name to iomux, function and gpio address
 * register offsets.
 */
struct amdfch_gpiomuxfunc {
	u8	iomux;
	u8	func;
	u16	gpiox;
};

#define	MAXGPIO	228

static struct amdfch_gpiomuxfunc amdfch_gpio2mux[MAXGPIO] = {
	/* GPIO33 */   [33] = { 0x5a, 0x00, 0x0168, },

	/* GPIO45 */   [45] = { 0x5d, 0x02, 0x0174, },

	/* GPIO49 */   [49] = { 0x40, 0x02, 0x0100, },
	/* GPIO50 */   [50] = { 0x41, 0x02, 0x0104, },
	/* GPIO51 */   [51] = { 0x42, 0x02, 0x0108, },

	/* GPIO55 */   [55] = { 0x43, 0x03, 0x010c, },

	/* GPIO59 */   [59] = { 0x46, 0x03, 0x0118, },
	/* GPIO64 */   [64] = { 0x47, 0x02, 0x011c, },
	/* GPIO66 */   [66] = { 0x5b, 0x01, 0x016c, },

	/* GPIO73 */   [73] = { 0x5f, 0x00, 0x017c, },
	/* GPIO74 */   [74] = { 0x60, 0x00, 0x0180, },
	/* GPIO75 */   [75] = { 0x5e, 0x00, 0x0178, },
	/* GPIO76 */   [76] = { 0x65, 0x00, 0x0194, },
	/* GPIO77 */   [77] = { 0x61, 0x00, 0x0184, },
	/* GPIO78 */   [78] = { 0x62, 0x00, 0x0188, },
	/* GPIO79 */   [79] = { 0x63, 0x00, 0x018c, },
	/* GPIO80 */   [80] = { 0x64, 0x00, 0x0190, },

	/* GPIO167 */ [167] = { 0x1a, 0x01, 0x0068, },
	/* GPIO168 */ [168] = { 0x1b, 0x01, 0x006c, },
	/* GPIO169 */ [169] = { 0x1c, 0x01, 0x0070, },
	/* GPIO170 */ [170] = { 0x1d, 0x01, 0x0074, },
};

static int amdfch_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct amdfch_gpio *gpp = container_of(chip, struct amdfch_gpio, chip);
	u8 iomux = amdfch_gpio2mux[offset].iomux;
	u8 func = amdfch_gpio2mux[offset].func;

	if (iomux == 0)
		return -ENODEV;

	writeb(func, gpp->mapbase + IOMUX_BASE + iomux);
	return 0;
}

static void amdfch_gpio_free(struct gpio_chip *chip, unsigned offset)
{
}

static void amdfch_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct amdfch_gpio *gpp = container_of(chip, struct amdfch_gpio, chip);
	u16 gpiox = amdfch_gpio2mux[offset].gpiox;
	u32 v;

	v = readl(gpp->mapbase + GPIO_BASE + gpiox);
	if (value)
		v |= GPIO_OUT;
	else
		v &= ~GPIO_OUT;
	writel(v, gpp->mapbase + GPIO_BASE + gpiox);
}

static int amdfch_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct amdfch_gpio *gpp = container_of(chip, struct amdfch_gpio, chip);
	u16 gpiox = amdfch_gpio2mux[offset].gpiox;
	u32 v;

	v = readl(gpp->mapbase + GPIO_BASE + gpiox);
	return (v & GPIO_IN) ? 1 : 0;
}

static int amdfch_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct amdfch_gpio *gpp = container_of(chip, struct amdfch_gpio, chip);
	u16 gpiox = amdfch_gpio2mux[offset].gpiox;
	u32 v;

	v = readl(gpp->mapbase + GPIO_BASE + gpiox);
	v &= ~GPIO_OUTENABLE;
	writel(v, gpp->mapbase + GPIO_BASE + gpiox);
	return 0;
}

static int amdfch_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	struct amdfch_gpio *gpp = container_of(chip, struct amdfch_gpio, chip);
	u16 gpiox = amdfch_gpio2mux[offset].gpiox;
	u32 v;

	v = readl(gpp->mapbase + GPIO_BASE + gpiox);
	v |= GPIO_OUTENABLE;
	writel(v, gpp->mapbase + GPIO_BASE + gpiox);
	return 0;
}

static struct amdfch_gpio gp = {
	.chip = {
		.label		= "AMD FCH GPIO",
		.owner		= THIS_MODULE,
		.base		= 0 /*-1*/,
		.ngpio		= MAXGPIO,
		.request	= amdfch_gpio_request,
		.free		= amdfch_gpio_free,
		.set		= amdfch_gpio_set,
		.get		= amdfch_gpio_get,
		.direction_input = amdfch_gpio_direction_input,
		.direction_output = amdfch_gpio_direction_output,
	},
};

static u8 in8_pm_reg(u8 reg)
{
	outb(reg, 0xcd6);
	return inb(0xcd7);
}

static void out8_pm_reg(u8 reg, u8 v)
{
	outb(reg, 0xcd6);
	outb(v, 0xcd7);
}

static u32 in32_pm_reg(u8 reg)
{
	u32 v = 0;
	int i;

	for (i = 0; i < 4; i++) {
		outb(reg + i, 0xcd6);
		v |= (inb(0xcd7) << (i*8));
	}

	return v;
}

static const struct pci_device_id amdfch_devices[] = {
        { PCI_VDEVICE(AMD, 0x780e), 0 },
        { },
};

/*
 * The GPIO module is not strcitly its own PCI device. So we don't do a
 * PCI probe for device here. We do want to check that we are actually
 * running on AMD FCH based hardware. So we do a PCI bus scan to make sure
 * we are good.
 */
static int __init amdfch_gpio_init(void)
{
	struct pci_device_id const *id = NULL;
	struct pci_dev *pdev = NULL;
	u32 base;
	int rc;

	for_each_pci_dev(pdev) {
		id = pci_match_id(amdfch_devices, pdev);
		if (id)
			break;
	}
	if (id == NULL)
		return -ENODEV;

	if (in8_pm_reg(0xe8)) {
		printk("AMDFCH-GPIO: disabling SD-CARD bus pins\n");
		out8_pm_reg(0xe8, 0);
	}

	base = in32_pm_reg(0x24);
	if (base == 0xffffffff) {
		printk("AMDFCH-GPIO: AcpiMmioAddr not present\n");
		return -ENODEV;
	}
	if (((base & 0x1) == 0) || ((base &  0x2) == 1)) {
		printk("AMDFCH-GPIO: AcpiMmioAddr not enabled\n");
		return -ENODEV;
	}
	base &= 0xffffe000;

#if 0
	/* This currently FAILs because the HPET timer claims the whole range */
	if (!request_region(base + GPIO_BASE, GPIO_SIZE, "AMD FCH GPIO")) {
		printk("AMDFCH-GPIO: failed to request GPIO registers\n");
		return -EBUSY;
	}
	if (!request_region(base + IOMUX_BASE, IOMUX_SIZE, "AMD FCH IOMUX")) {
		printk("AMDFCH-GPIO: failed to request IOMUX registers\n");
		return -EBUSY;
	}
#endif

	gp.base = base;
	gp.mapbase = ioremap(base, 0x2000);
	if (!gp.mapbase) {
		printk("AMDFCH-GPIO: failed to ioremap(0x%08x)\n", base);
		return -ENOMEM;
	}
	printk("AMDFCH-GPIO: GPIO pins mapped at 0x%p\n", gp.mapbase);

	rc = gpiochip_add(&gp.chip);
	if (rc) {
		printk("AMDFCH-GPIO: failed to register GPIO?\n");
		iounmap(gp.mapbase);
		return rc;
	}

	return 0;
}

module_init(amdfch_gpio_init);

MODULE_DESCRIPTION("AMD Fusion Controller Hub GPIO driver");
MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_LICENSE("GPL");
