/*
 * pcf2116.c -- driver for PCF2116 simple LCD controller
 *
 * Copyright (C) 2012, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>

static inline int pcf2116_i2c_read(struct i2c_client *client)
{
	return i2c_smbus_read_byte(client);
}

static inline int pcf2116_i2c_write(struct i2c_client *client, unsigned int reg,
	unsigned int val)
{
	return i2c_smbus_write_byte_data(client, reg, val);
}

static ssize_t pcf2116_sysfs_data_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
	size_t i;

	for (i = 0; i < count; i++, buf++)
		pcf2116_i2c_write(client, 0x40, *buf);

	return i;
}

static ssize_t pcf2116_sysfs_cmd_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
	size_t i;

	for (i = 0; i < count; i++, buf++)
		pcf2116_i2c_write(client, 0, *buf);

	return i;
}

static ssize_t pcf2116_sysfs_data_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	pcf2116_i2c_write(client, 0x60, 0);
	*buf = pcf2116_i2c_read(client);
	return 1;
}

static DEVICE_ATTR(data, S_IWUSR, NULL, pcf2116_sysfs_data_write);
static DEVICE_ATTR(cmd, S_IWUSR, NULL, pcf2116_sysfs_cmd_write);

static struct attribute *pcf2116_sysfs_atrs[] = {
        &dev_attr_data.attr,
        &dev_attr_cmd.attr,
        NULL,
};

static struct attribute_group pcf2116_sysfs_group = {
        .name = "pcf2116",
        .attrs = pcf2116_sysfs_atrs,
};

static int pcf2116_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	int rc;

	rc = i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA);
	if (!rc)
		return -EIO;

	rc = sysfs_create_group(&client->dev.kobj, &pcf2116_sysfs_group);
	if (rc)
		return rc;
		
	return 0;
}

static int pcf2116_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &pcf2116_sysfs_group);
	return 0;
}

static const struct i2c_device_id pcf2116_i2c_id[] = {
	{ "pcf2116", 0 },
	{ }
};

static struct i2c_driver pcf2116_i2c_driver = {
	.driver = {
		.name = "pcf2116",
		.owner = THIS_MODULE,
	},
	.probe = pcf2116_probe,
	.remove = pcf2116_remove,
	.id_table = pcf2116_i2c_id,
};

module_i2c_driver(pcf2116_i2c_driver);

MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PCF2116 simple LCD controller");
