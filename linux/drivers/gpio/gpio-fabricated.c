// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2021 Digi International Inc.
 *
 */
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#define FABR_NGPIOS_MAX		(8 * sizeof_field(struct fabr, dir))

struct fabr_gpio {
	struct gpio_desc *input;
	struct gpio_desc *output;
	struct gpio_desc *pullup;
	int dir_in;
	int irq;
	int virq;
};

struct fabr {
	struct gpio_chip gpio_chip;
	struct fabr_gpio *gpios;
	unsigned int ngpios;
	unsigned long dir;
	struct irq_domain *domain;
};

static int fabr_gpio_get_dir(struct gpio_chip *gc, unsigned int offset)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];

	dev_dbg(gc->parent, "%s: dir=%d", __func__, gpio->dir_in);

	return gpio->dir_in;
}

static int fabr_gpio_dir_in(struct gpio_chip *gc, unsigned int offset)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];

	/* Return if input-mode is not supported */
	if (!gpio->input)
		return -EINVAL;

	/* If this is an input-only pin, just return with OK */
	if (!gpio->output)
		return 0;

	gpio->dir_in = 1;
	return gpiod_direction_output(gpio->output, 1);
}

static int fabr_gpio_dir_out(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];

	/* Return if output-mode is not supported */
	if (!gpio->output)
		return -EINVAL;

	gpio->dir_in = 0;
	return gpiod_direction_output(gpio->output, val);
}

static int fabr_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];
	struct gpio_desc *pin;

	/* If output-only pin, return the output pin's state */
	if (gpio->input)
		pin = gpio->input;
	else
		pin = gpio->output;

	return gpiod_get_value_cansleep(pin);
}

static void fabr_gpio_set(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];

	/* If input-only pin, return with error */
	if (!gpio->output)
		return;

	gpiod_set_value_cansleep(gpio->output, val);
}

static int fabr_gpio_set_config(struct gpio_chip *gc, unsigned int offset,
				unsigned long config)
{
	struct fabr *chip = gpiochip_get_data(gc);
	struct fabr_gpio *gpio = &chip->gpios[offset];
	unsigned long param = pinconf_to_config_param(config);

	if (!gpio->pullup || (param != PIN_CONFIG_BIAS_DISABLE &&
			      param != PIN_CONFIG_BIAS_PULL_UP))
		return -ENOTSUPP;

	gpiod_set_value_cansleep(gpio->pullup,
				 param == PIN_CONFIG_BIAS_PULL_UP);

	return 0;
}

static void fabr_gpio_irq_ack(struct irq_data *data)
{
}

static void fabr_gpio_irq_mask(struct irq_data *data)
{
	struct fabr *chip = irq_data_get_irq_chip_data(data);
	struct fabr_gpio *gpio = &chip->gpios[data->hwirq];

	disable_irq(gpio->irq);
}

static void fabr_gpio_irq_unmask(struct irq_data *data)
{
	struct fabr *chip = irq_data_get_irq_chip_data(data);
	struct fabr_gpio *gpio = &chip->gpios[data->hwirq];

	enable_irq(gpio->irq);
}

static int fabr_gpio_irq_set_type(struct irq_data *data, unsigned int type)
{
	struct fabr *chip = irq_data_get_irq_chip_data(data);
	struct fabr_gpio *gpio = &chip->gpios[data->hwirq];

	return irq_set_irq_type(gpio->irq, type);
}

static struct irq_chip fabr_gpio_irqchip = {
	.name = "fabr_gpio",
	.irq_ack = fabr_gpio_irq_ack,
	.irq_mask = fabr_gpio_irq_mask,
	.irq_unmask = fabr_gpio_irq_unmask,
	.irq_set_type = fabr_gpio_irq_set_type
};

static irqreturn_t fabr_gpio_irq_handler(int irq, void *dev)
{
	struct fabr *chip = dev;
	int i;
	bool found = false;

	for (i = 0; i < chip->ngpios; i++) {
		struct fabr_gpio *gpio = &chip->gpios[i];

		if (gpio->irq == irq) {
			found = true;
			generic_handle_irq(gpio->virq);
			break;
		}
	}

	if (!found)
		dev_warn(chip->gpio_chip.parent, "No GPIO found for IRQ %d\n",
			 irq);

	return IRQ_HANDLED;
}

static int fabr_gpio_irq_map(struct irq_domain *domain, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	struct fabr *chip = domain->host_data;
	struct fabr_gpio *gpio = &chip->gpios[hwirq];
	int ret;

	if (gpio->irq < 0)
		return -ENXIO;

	irq_set_status_flags(gpio->irq, IRQ_NOAUTOEN);
	ret = request_irq(gpio->irq, fabr_gpio_irq_handler, 0,
			  dev_name(chip->gpio_chip.parent), chip);
	if (ret) {
		dev_err(chip->gpio_chip.parent,
			"Couldn't request underlying IRQ %d\n", gpio->irq);
		return ret;
	}

	gpio->virq = irq;

	irq_set_chip_data(irq, chip);
	irq_set_chip_and_handler(irq, &fabr_gpio_irqchip, handle_simple_irq);
	irq_set_noprobe(irq);

	return 0;
}

static void fabr_gpio_irq_unmap(struct irq_domain *domain, unsigned int irq)
{
	struct fabr *chip = domain->host_data;
	struct fabr_gpio *gpio = NULL;
	int i;

	for (i = 0; i < chip->ngpios; i++) {
		if (chip->gpios[i].virq == irq) {
			gpio = &chip->gpios[i];
			break;
		}
	}

	WARN_ON(gpio == NULL);

	if (gpio) {
		free_irq(gpio->irq, chip);
		gpio->irq = -1;
		gpio->virq = -1;
	}

	irq_set_chip_and_handler(irq, NULL, NULL);
	irq_set_chip_data(irq, NULL);
}

static const struct irq_domain_ops fabr_gpio_irq_domain_ops = {
	.map = fabr_gpio_irq_map,
	.unmap = fabr_gpio_irq_unmap,
};

static int fabr_gpio_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct fabr *fabr = gpiochip_get_data(gc);

	if (offset >= gc->ngpio)
		return -ENXIO;

	return irq_create_mapping(fabr->domain, offset);
}

static const struct of_device_id fabr_gpio_dt_ids[] = {
	{ .compatible = "fabricated-gpio" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, fabr_gpio_dt_ids);

static int fabr_parse_dt(struct platform_device *pdev, struct fabr *chip)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev_of_node(dev), *child;
	unsigned int index = 0;

	chip->ngpios = of_get_child_count(np);
	if (chip->ngpios == 0 || chip->ngpios > FABR_NGPIOS_MAX) {
		dev_err(dev, "Number of GPIOs should be >0 && <=%d",
			FABR_NGPIOS_MAX);

		return -EINVAL;
	}

	chip->gpios = devm_kcalloc(dev, chip->ngpios,
				   sizeof(*chip->gpios), GFP_KERNEL);
	if (!chip->gpios)
		return -ENOMEM;

	for_each_available_child_of_node (np, child) {
		struct fabr_gpio *gpio = &chip->gpios[index++];
		char label[80];
		enum gpiod_flags flags;

		gpio->dir_in = -1;
		gpio->irq = -1;
		gpio->virq = -1;

		flags = of_property_read_bool(child, "disable-pullup") ?
				      GPIOD_OUT_LOW :
				      GPIOD_OUT_HIGH;

		snprintf(label, sizeof(label), "%s_%s_pullup", dev_name(dev),
			 of_node_full_name(child));
		gpio->pullup = devm_gpiod_get_from_of_node(
			dev, child, "pullup-gpio", 0, flags, label);
		if (IS_ERR(gpio->pullup)) {
			if (PTR_ERR(gpio->pullup) != -ENOENT) {
				dev_err(dev, "Failed to request %s gpio",
					label);
				return PTR_ERR(gpio->pullup);
			}

			gpio->pullup = NULL;
		}

		snprintf(label, sizeof(label), "%s_%s_input", dev_name(dev),
			 of_node_full_name(child));
		gpio->input = devm_gpiod_get_from_of_node(
			dev, child, "input-gpio", 0, GPIOD_IN, label);
		if (IS_ERR(gpio->input)) {
			if (PTR_ERR(gpio->input) != -ENOENT) {
				dev_err(dev, "Failed to request %s gpio",
					label);
				return PTR_ERR(gpio->input);
			}

			gpio->input = NULL;
		}

		snprintf(label, sizeof(label), "%s_%s_output", dev_name(dev),
			 of_node_full_name(child));
		gpio->output = devm_gpiod_get_from_of_node(
			dev, child, "output-gpio", 0, GPIOD_OUT_HIGH, label);
		if (IS_ERR(gpio->output)) {
			if (PTR_ERR(gpio->output) != -ENOENT) {
				dev_err(dev, "Failed to request %s gpio",
					label);
				return PTR_ERR(gpio->output);
			}

			gpio->output = NULL;
		}

		if (!gpio->input) {
			if (!gpio->output) {
				dev_err(dev,
					"need to define either an input or output pin for %s_%s",
					dev_name(dev),
					of_node_full_name(child));
				return -EINVAL;
			}

			gpio->dir_in = 0;
		} else {
			gpio->dir_in = 1;
			gpio->irq = gpiod_to_irq(gpio->input);
		}
	}

	return 0;
}

static int fabr_gpio_probe(struct platform_device *pdev)
{
	struct fabr *chip;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	ret = fabr_parse_dt(pdev, chip);
	if (ret)
		return ret;

	chip->gpio_chip.owner = THIS_MODULE;
	chip->gpio_chip.ngpio = chip->ngpios;
	chip->gpio_chip.parent = &pdev->dev;
	chip->gpio_chip.get_direction = fabr_gpio_get_dir;
	chip->gpio_chip.direction_input = fabr_gpio_dir_in;
	chip->gpio_chip.direction_output = fabr_gpio_dir_out;
	chip->gpio_chip.get = fabr_gpio_get;
	chip->gpio_chip.set = fabr_gpio_set;
	chip->gpio_chip.set_config = fabr_gpio_set_config;
	chip->gpio_chip.label = dev_name(&pdev->dev);
	chip->gpio_chip.base = -1;
	/*
	 * TODO: could do it more clever, like checking all physical GPIOs'
	 * can_sleep property
	 */
	chip->gpio_chip.can_sleep = true;

	chip->domain =
		irq_domain_add_linear(dev_of_node(&pdev->dev), chip->ngpios,
				      &fabr_gpio_irq_domain_ops, chip);
	if (!chip->domain) {
		dev_err(&pdev->dev, "Couldn't add IRQ domain\n");
		return -EINVAL;
	}
	chip->gpio_chip.to_irq = fabr_gpio_to_irq;

	platform_set_drvdata(pdev, chip);

	ret = gpiochip_add_data(&chip->gpio_chip, chip);
	if (ret) {
		dev_err(&pdev->dev, "failed to register gpiochip\n");
		irq_domain_remove(chip->domain);
		return ret;
	}

	dev_info(&pdev->dev, "Fabricated GPIO driver registered\n");
	return 0;
}

static int fabr_gpio_remove(struct platform_device *pdev)
{
	struct fabr *chip = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < chip->ngpios; i++) {
		int irq = irq_find_mapping(chip->domain, i);

		if (irq)
			irq_dispose_mapping(irq);
	}
	irq_domain_remove(chip->domain);
	gpiochip_remove(&chip->gpio_chip);

	return 0;
}

static struct platform_driver fabr_gpio_driver = {
	.driver = {
		.name = "fabr-gpio",
		.of_match_table = of_match_ptr(fabr_gpio_dt_ids),
	},
	.probe = fabr_gpio_probe,
	.remove = fabr_gpio_remove,
};
module_platform_driver(fabr_gpio_driver);

MODULE_AUTHOR("Digi International Inc, "
	      "Robert Hodaszi <robert.hodaszi@digi.com>");
MODULE_DESCRIPTION("Fabricated GPIO");
MODULE_LICENSE("GPL");
