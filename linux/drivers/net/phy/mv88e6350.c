/*
 * mv88e6350.c -- specific support for Marvell 88e6350 switch
 *
 * (C) Copyright 2016, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>

#define SMI_CMD		0
#define SMI_DATA	1

#define OP_BUSY		(1 << 15)
#define OP_SMI22	(1 << 12)
#define OP_READ		(2 << 10)
#define OP_WRITE	(1 << 10)

#define TIMEOUT		1000

/*
 * Wait for the switch chip to be ready (not busy).
 */
static int mv88e6350_cmdwait(struct mii_bus *bus, int mii)
{
	int i;

	for (i = 0; i < TIMEOUT; i++) {
		if ((mdiobus_read(bus, mii, SMI_CMD) & OP_BUSY) == 0)
			return 0;
	}
	return -EIO;
}

static int mv88e6350_read(struct mii_bus *bus, unsigned int mii,
			  unsigned int dev, unsigned int reg)
{
	int cmd, rc;

	if ((mii >= 32) || (dev >= 32) || (reg >= 32))
		return 0xffff;

	rc = mv88e6350_cmdwait(bus, mii);
	if (rc)
		return rc;

	cmd = reg | (dev << 5) | OP_SMI22 | OP_READ | OP_BUSY;
	mdiobus_write(bus, mii, SMI_CMD, cmd);

	rc = mv88e6350_cmdwait(bus, mii);
	if (rc)
		return rc;

	return mdiobus_read(bus, mii, SMI_DATA);
}

static int mv88e6350_write(struct mii_bus *bus, unsigned int mii,
			    unsigned int dev, unsigned int reg, u16 v)
{
	int cmd, rc;

	if ((mii >= 32) || (dev >= 32) || (reg >= 32))
		return 0;

	rc = mv88e6350_cmdwait(bus, mii);
	if (rc)
		return rc;

	cmd = reg | (dev << 5) | OP_SMI22 | OP_WRITE | OP_BUSY;
	mdiobus_write(bus, mii, SMI_DATA, v);
	mdiobus_write(bus, mii, SMI_CMD, cmd);
	return mv88e6350_cmdwait(bus, mii);
}

static struct mii_bus *mv88e6350_bus;
static unsigned int mv88e6350_mii;
static unsigned int mv88e6350_addr;
static unsigned int mv88e6350_reg;

static ssize_t mv88e6350_addr_show(struct device *dev,
			           struct device_attribute *attr,
				   char *buf)
{
	sprintf(buf, "%d\n", mv88e6350_addr);
	return strlen(buf) + 1;
}

static ssize_t mv88e6350_addr_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t size)
{
	char *after;

	mv88e6350_addr = simple_strtoul(buf, &after, 10);
	return size;
}

static ssize_t mv88e6350_reg_show(struct device *dev,
			          struct device_attribute *attr,
				  char *buf)
{
	sprintf(buf, "%d\n", mv88e6350_reg);
	return strlen(buf) + 1;
}

static ssize_t mv88e6350_reg_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	char *after;

	mv88e6350_reg = simple_strtoul(buf, &after, 10);
	return size;
}

static ssize_t mv88e6350_data_show(struct device *dev,
			           struct device_attribute *attr,
				   char *buf)
{
	int v;

	v = mv88e6350_read(mv88e6350_bus, mv88e6350_mii, mv88e6350_addr, mv88e6350_reg);
	if (v < 0)
		return v;

	sprintf(buf, "%d\n", v);
	return strlen(buf) + 1;
}

static ssize_t mv88e6350_data_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t size)
{
	unsigned int v;
	char *after;
	int rc;

	v = simple_strtoul(buf, &after, 10);
	rc = mv88e6350_write(mv88e6350_bus, mv88e6350_mii, mv88e6350_addr, mv88e6350_reg, v);
	return (rc) ? rc : size;
}

static DEVICE_ATTR(addr, 0600, mv88e6350_addr_show, mv88e6350_addr_store);
static DEVICE_ATTR(reg, 0600, mv88e6350_reg_show, mv88e6350_reg_store);
static DEVICE_ATTR(data, 0600, mv88e6350_data_show, mv88e6350_data_store);

/*
 * The probe relies on the devicetree setup for DSA switches.
 * This should be done as per standard DSA setup, this is meant to be
 * completely 100% compatible with that.
 */
static int mv88e6350_probe(struct mdio_device *mdiodev)
{
	struct device *dev = &mdiodev->dev;
	struct device_node *np = dev->of_node;
	int rc;

	if (np == NULL)
		return -ENODEV;

	mv88e6350_bus = mdiodev->bus;
	mv88e6350_mii = mdiodev->addr;

	rc = device_create_file(dev, &dev_attr_addr);
	rc = device_create_file(dev, &dev_attr_reg);
	rc = device_create_file(dev, &dev_attr_data);
	return 0;
}

static void mv88e6350_remove(struct mdio_device *mdiodev)
{
	struct device *dev = &mdiodev->dev;
	device_remove_file(dev, &dev_attr_addr);
	device_remove_file(dev, &dev_attr_reg);
	device_remove_file(dev, &dev_attr_data);
}

static const struct of_device_id dsa_of_match_table[] = {
	{ .compatible = "marvell,mv88e6085", },
	{ .compatible = "marvell,mv88e6190", },
	{}
};
MODULE_DEVICE_TABLE(of, mv88e6350_of_match_table);

static struct mdio_driver mv88e6350_driver = {
	.probe = mv88e6350_probe,
	.remove = mv88e6350_remove,
	.mdiodrv.driver = {
		.name   = "mv88e6350",
		.of_match_table = dsa_of_match_table,
	},
};

static int __init mv88e6350_init_module(void)
{
        return mdio_driver_register(&mv88e6350_driver);
}
module_init(mv88e6350_init_module);

static void __exit mv88e6350_cleanup_module(void)
{
        mdio_driver_unregister(&mv88e6350_driver);
}
module_exit(mv88e6350_cleanup_module);

