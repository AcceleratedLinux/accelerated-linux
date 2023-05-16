// SPDX-License-Identifier: GPL-2.0-only
/*
 * GPIO driver for Pericom 4PI7C9X795X chip
 *
 * Based on the gpio-exar.c driver
 *
 * Copyright (C) 2021 Seth Bollinger <seth.bollinger@digi.com>
 */

#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/gpio/driver.h>
#include <linux/idr.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include "gpiolib.h"

#define PERICOM_GPIO_CTRL 0xd8

#define PERICOM_GPIO_MODE_MASK   0x0000ff00
#define PERICOM_GPIO_MODE_SHIFT  8
#define PERICOM_GPIO_MODE_OUTPUT 0x1
#define PERICOM_GPIO_MODE_INPUT  0x0

#define PERICOM_GPIO_INPUT_MASK  0x000000ff
#define PERICOM_GPIO_INPUT_SHIFT 0

#define PERICOM_GPIO_OUTPUT_MASK  0x00ff0000
#define PERICOM_GPIO_OUTPUT_SHIFT 16

#define DRIVER_NAME "gpio_pericom"

static DEFINE_IDA(ida_index);

struct pericom_gpio_chip {
	struct gpio_chip gpio_chip;
	struct regmap *regmap;
	int index;
	char name[20];
	struct pci_dev *pcidev;
	struct mutex lock;
};

static int pericom_get_direction(struct gpio_chip *chip, unsigned int offset)
{
	struct pericom_gpio_chip *pericom_gpio = gpiochip_get_data(chip);
	unsigned int reg;

	mutex_lock(&pericom_gpio->lock);
	pci_read_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, &reg);
	mutex_unlock(&pericom_gpio->lock);

	if (reg & (BIT(offset) << PERICOM_GPIO_MODE_SHIFT))
		return GPIO_LINE_DIRECTION_OUT;

	return GPIO_LINE_DIRECTION_IN;
}

static int pericom_get_value(struct gpio_chip *chip, unsigned int offset)
{
	struct pericom_gpio_chip *pericom_gpio = gpiochip_get_data(chip);
	unsigned int reg;
	int dir = pericom_get_direction(chip, offset);
	int shift = PERICOM_GPIO_INPUT_SHIFT;

	mutex_lock(&pericom_gpio->lock);
	pci_read_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, &reg);
	mutex_unlock(&pericom_gpio->lock);

	if (dir == GPIO_LINE_DIRECTION_OUT)
		shift = PERICOM_GPIO_OUTPUT_SHIFT;

	return !!(reg & (BIT(offset) << shift));
}

static void pericom_set_value(struct gpio_chip *chip, unsigned int offset,
			   int value)
{
	struct pericom_gpio_chip *pericom_gpio = gpiochip_get_data(chip);
	unsigned int reg;

	mutex_lock(&pericom_gpio->lock);
	pci_read_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, &reg);

	if (value)
		reg |= (BIT(offset) << PERICOM_GPIO_OUTPUT_SHIFT);
	else
		reg &= ~(BIT(offset) << PERICOM_GPIO_OUTPUT_SHIFT);

	/* remove any input RO bits */
	reg &= ~PERICOM_GPIO_INPUT_MASK;

	pci_write_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, reg);
	mutex_unlock(&pericom_gpio->lock);
}

static int pericom_direction_output(struct gpio_chip *chip, unsigned int offset,
				 int value)
{
	struct pericom_gpio_chip *pericom_gpio = gpiochip_get_data(chip);
	unsigned int reg;

	mutex_lock(&pericom_gpio->lock);
	pci_read_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, &reg);

	reg |= (BIT(offset) << PERICOM_GPIO_MODE_SHIFT);

	if (value)
		reg |= (BIT(offset) << PERICOM_GPIO_OUTPUT_SHIFT);
	else
		reg &= ~(BIT(offset) << PERICOM_GPIO_OUTPUT_SHIFT);

	/* remove any input RO bits */
	reg &= ~PERICOM_GPIO_INPUT_MASK;

	pci_write_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, reg);
	mutex_unlock(&pericom_gpio->lock);

	return 0;
}

static int pericom_direction_input(struct gpio_chip *chip, unsigned int offset)
{
	struct pericom_gpio_chip *pericom_gpio = gpiochip_get_data(chip);
	unsigned int reg;

	mutex_lock(&pericom_gpio->lock);
	pci_read_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, &reg);

	reg &= ~(BIT(offset) << PERICOM_GPIO_MODE_SHIFT | BIT(offset) << PERICOM_GPIO_OUTPUT_SHIFT);

	/* remove any input RO bits */
	reg &= ~PERICOM_GPIO_INPUT_MASK;

	pci_write_config_dword(pericom_gpio->pcidev, PERICOM_GPIO_CTRL, reg);
	mutex_unlock(&pericom_gpio->lock);

	return 0;
}

static void pericom_devm_ida_free(void *data)
{
	struct pericom_gpio_chip *pericom_gpio = data;

	ida_free(&ida_index, pericom_gpio->index);
}

static int gpio_pericom_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pci_dev *pcidev = to_pci_dev(dev->parent);
	struct pericom_gpio_chip *pericom_gpio;
	char node_name[32];
	struct device_node *np;
	u32 ngpios;
	int index, ret;

	ret = device_property_read_u32(dev, "ngpios", &ngpios);
	if (ret)
		return ret;

	pericom_gpio = devm_kzalloc(dev, sizeof(*pericom_gpio), GFP_KERNEL);
	if (!pericom_gpio)
		return -ENOMEM;

	index = ida_alloc(&ida_index, GFP_KERNEL);
	if (index < 0)
		return index;

	/* Need to associate a DT node with the driver */
	snprintf(node_name, sizeof(node_name), "pericom-gpio%d", index);
	np = of_find_node_by_name(NULL, node_name);
	if (!np)
		return -ENODEV;

	ret = devm_add_action_or_reset(dev, pericom_devm_ida_free, pericom_gpio);
	if (ret)
		return ret;

	mutex_init(&pericom_gpio->lock);

	sprintf(pericom_gpio->name, "pericom_gpio%d", index);
	pericom_gpio->gpio_chip.label = pericom_gpio->name;
	pericom_gpio->gpio_chip.of_node = np;
	pericom_gpio->gpio_chip.parent = dev;
	pericom_gpio->gpio_chip.direction_output = pericom_direction_output;
	pericom_gpio->gpio_chip.direction_input = pericom_direction_input;
	pericom_gpio->gpio_chip.get_direction = pericom_get_direction;
	pericom_gpio->gpio_chip.get = pericom_get_value;
	pericom_gpio->gpio_chip.set = pericom_set_value;
	pericom_gpio->gpio_chip.base = -1;
	pericom_gpio->gpio_chip.ngpio = ngpios;
	pericom_gpio->index = index;
	pericom_gpio->pcidev = pcidev;

	ret = devm_gpiochip_add_data(dev, &pericom_gpio->gpio_chip, pericom_gpio);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, pericom_gpio);

	return 0;
}

static struct platform_driver gpio_pericom_driver = {
	.probe	= gpio_pericom_probe,
	.driver	= {
		.name = DRIVER_NAME,
	},
};

module_platform_driver(gpio_pericom_driver);

MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_DESCRIPTION("Pericom GPIO driver");
MODULE_AUTHOR("Seth Bollinger <seth.bollinger@digi.com>");
MODULE_LICENSE("GPL");
