/*
 *  Copyright 2016, 2017, 2018 Digi International, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/i2c.h>
#include <linux/kernel.h>


//-------------------------------------------------------------------------
//
//  global state
//
static struct i2c_client *cpld_sysfs_client;


//-------------------------------------------------------------------------
//
//  internal helper functions
//
static int cpld_get_reg_val(char *buf, int reg)
{
	int rc;
	if (!cpld_sysfs_client) goto err;
	rc = i2c_smbus_read_byte_data(cpld_sysfs_client, reg);
	if (rc < 0) goto err;

	return sprintf(buf, "%02x\n", rc);
err:
	return sprintf(buf, "--\n");
}


//-------------------------------------------------------------------------
//
//  sysfs action functions
//
static ssize_t
cpld_chipid0_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return cpld_get_reg_val(buf, 0x00);
}

static ssize_t
cpld_chipid1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return cpld_get_reg_val(buf, 0x01);
}

static ssize_t
cpld_hwver_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return cpld_get_reg_val(buf, 0x02);
}

static ssize_t
cpld_swver_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return cpld_get_reg_val(buf, 0x03);
}

static ssize_t
cpld_regs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int rc;
	int reg;
	char *bptr = buf;

	//
	//  NOTE: The buf is a single page, so we must be careful not
	//        to exceed 4k.  The implementation below outputs six
	//        bytes per register.  Over 0x70 registers, there is
	//        therefore less than 1k of output added to the buf,
	//        and no page size checks are needed.
	//
	//        IF THE IMPLEMENTATION CHANGES -- the decision to run
	//        without dynamically testing the writes against the
	//        size of the page must be examined.
	//

	for (reg=0x00; reg < 0x70; reg++)
	{
		bptr += sprintf(bptr, "%02x:", reg);
		bptr += cpld_get_reg_val(bptr, reg);
	}
	return bptr - buf;
}

static ssize_t
cpld_regs_set(struct kobject *kobj, struct kobj_attribute *attr,
              const char *buf, size_t count)
{
	int rc;
	int addr;
	int val;

	if (!cpld_sysfs_client) return -ENOMEM;

	rc = sscanf(buf, "%2x%2x", &addr, &val);
	if (rc == 2)
	{
		rc = i2c_smbus_write_byte_data(cpld_sysfs_client, addr, val);
		if (rc < 0) return -EINVAL;
	}

	return count;
}


//-------------------------------------------------------------------------
//
//  sysfs attributes and groups
//
typedef struct kobj_attribute sysfsattr;

static sysfsattr cpld_chipid0 = __ATTR(chipid0, 0444, cpld_chipid0_show, NULL);
static sysfsattr cpld_chipid1 = __ATTR(chipid1, 0444, cpld_chipid1_show, NULL);
static sysfsattr cpld_hwver = __ATTR(hwver, 0444, cpld_hwver_show, NULL);
static sysfsattr cpld_swver = __ATTR(swver, 0444, cpld_swver_show, NULL);
static sysfsattr cpld_regs = __ATTR(regs, 0600, cpld_regs_show, cpld_regs_set);

static struct attribute *cpld_attrs[] = {
	&cpld_chipid0.attr,
	&cpld_chipid1.attr,
	&cpld_hwver.attr,
	&cpld_swver.attr,
	&cpld_regs.attr,
	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};


//-------------------------------------------------------------------------
//
//  "public" interface to sysfs implementation for leds-awusb24 driver
//
int awusb24_cpld_register_sysfs(struct i2c_client *client)
{
	int ret = 0;

	struct device *dev = &client->dev;
	struct kobject *cpld_dir;

	cpld_sysfs_client = client;

	cpld_dir = kobject_create_and_add("cpld", &dev->kobj);
	if (!cpld_dir) return -ENOMEM;

	ret = sysfs_create_group(cpld_dir, &cpld_attr_group);

	return ret;
}

void awusb24_cpld_unregister_sysfs(struct i2c_client *client)
{
}

