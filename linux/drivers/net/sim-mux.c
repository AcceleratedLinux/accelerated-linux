// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2024 Digi International Inc.
 */
#include "linux/dev_printk.h"
#include "linux/device.h"
#include "linux/gfp_types.h"
#include "linux/kstrtox.h"
#include "linux/property.h"
#include <linux/module.h>
#include <linux/of.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

#define DEV_NAME	"sim-mux"

struct sim_mux {
	struct device *dev;

	struct gpio_desc  *sim_select_gpio;
	struct gpio_descs *sim_present_gpios;

	struct attribute_group *attr_group;

	unsigned int sim_count;
};

struct sim_mux_attr {
	struct device_attribute dev_attr;
	unsigned int sim_idx;
};

static ssize_t sim_mux_attr_count_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct sim_mux *info = dev_get_drvdata(dev);
	if (info == NULL) {
		return -EIO;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", info->sim_count);
}
static DEVICE_ATTR(count, S_IRUGO, sim_mux_attr_count_show, NULL);


static ssize_t sim_mux_attr_select_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	struct sim_mux *info = dev_get_drvdata(dev);
	if (info == NULL) {
		return -EIO;
	}

	int sel = gpiod_get_value_cansleep(info->sim_select_gpio);

	return snprintf(buf, PAGE_SIZE, "%d\n", sel);
}

static ssize_t sim_mux_attr_select_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct sim_mux *info = dev_get_drvdata(dev);
	if (info == NULL) {
		return -EIO;
	}

	u8 val;
	if (kstrtou8(buf, 10, &val) ||
	    (val >= info->sim_count)) {
		return -ERANGE;
	}

	gpiod_set_value_cansleep(info->sim_select_gpio, val);

	return count;
}
static DEVICE_ATTR(select, S_IRUGO | S_IWUSR, sim_mux_attr_select_show,
		   sim_mux_attr_select_store);

static ssize_t sim_mux_attr_present_show(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	struct sim_mux *info = dev_get_drvdata(dev);
	if (info == NULL) {
		return -EIO;
	}

	struct sim_mux_attr *mux_attr = container_of(attr, struct sim_mux_attr, dev_attr);

	int present = gpiod_get_value_cansleep(info->sim_present_gpios->desc[mux_attr->sim_idx]);

	return snprintf(buf, PAGE_SIZE, "%d\n", present);
}

static int sim_mux_setup_sysfs(struct sim_mux *info)
{
	struct device *dev = info->dev;

	int ret;
	int num_present = 0;

	if (info->sim_present_gpios) {
		num_present = info->sim_present_gpios->ndescs;
	}

	info->attr_group = devm_kzalloc(dev, sizeof(*info->attr_group), GFP_KERNEL);
	if (!info->attr_group) {
		return -ENOMEM;
	}

	struct attribute **attrs;
	attrs = devm_kcalloc(dev, num_present + 3, sizeof(*attrs), GFP_KERNEL);
	if (!attrs) {
		return -ENOMEM;
	}

	unsigned int offset = 0;
	attrs[offset++] = &dev_attr_count.attr;
	attrs[offset++] = &dev_attr_select.attr;

	for (unsigned int i = 0; i < num_present; i++) {
		struct sim_mux_attr *attr = devm_kzalloc(dev, sizeof(*attr),
							 GFP_KERNEL);
		if (!attr) {
			return -ENOMEM;
		}

		struct device_attribute *dev_attr = &attr->dev_attr;

		sysfs_attr_init(&dev_attr->attr);
		dev_attr->attr.name = devm_kasprintf(dev, GFP_KERNEL,
						     "sim%u_present", i);
		dev_attr->attr.mode = S_IRUGO;
		dev_attr->show = sim_mux_attr_present_show;

		attr->sim_idx = i;

		attrs[offset++] = &dev_attr->attr;
	}

	info->attr_group->attrs = attrs;

	ret = sysfs_create_group(&dev->kobj, info->attr_group);
	if (ret) {
		return dev_err_probe(dev, ret, "sysfs group creation failed\n");
	}

	return 0;
}

static int sim_mux_probe(struct platform_device *pdev)
{
	struct sim_mux *info;
	struct device *dev = &pdev->dev;

	int ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (!info) {
		return -ENOMEM;
	}

	info->dev = dev;

	dev->platform_data = info;

	u32 val;
	ret = device_property_read_u32(dev, "sim-count", &val);
	if (ret) {
		return dev_err_probe(dev, ret,
				     "Failed to get sim-count value\n");
	}
	if (val != 2) {
		dev_err(dev, "Only dual sim multiplexers are currently supported, requested %u\n", val);
		return -ERANGE;
	}
	info->sim_count = (u8)val;

	/* Get GPIO pins from device-tree */

	info->sim_select_gpio = devm_gpiod_get(dev, "sim-select", GPIOD_OUT_LOW);
	if (IS_ERR(info->sim_select_gpio)) {
		return dev_err_probe(dev, PTR_ERR(info->sim_select_gpio),
				     "Failed to register sim-select GPIO\n");
	}

	int present_pins = gpiod_count(dev, "sim-present");
	if ((present_pins > 0) && (present_pins != info->sim_count)) {
		dev_err(dev,
			"sim-present GPIOs present but not equal to sim-count: %d",
			present_pins);
		return -ERANGE;
	}
	info->sim_present_gpios = devm_gpiod_get_array_optional(dev, "sim-present",
								GPIOD_IN);
	if (IS_ERR(info->sim_select_gpio)) {
		return dev_err_probe(dev, PTR_ERR(info->sim_select_gpio),
				     "Failed to register sim-present GPIOs\n");
	}

	ret = sim_mux_setup_sysfs(info);
	if (ret) {
		return ret;
	}

	platform_set_drvdata(pdev, info);

	return 0;
}

static int sim_mux_remove(struct platform_device *pdev)
{
	struct sim_mux *info = dev_get_drvdata(&pdev->dev);

	sysfs_remove_group(&pdev->dev.kobj, info->attr_group);

	return 0;
}

static const struct of_device_id sim_mux_dt_ids[] = {
	{ .compatible = "sim-mux", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sim_mux_dt_ids);

static struct platform_driver sim_mux_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = sim_mux_dt_ids,
	},
	.probe  = sim_mux_probe,
	.remove = sim_mux_remove
};
module_platform_driver(sim_mux_driver);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("SIM multiplexer driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sim-mux");
