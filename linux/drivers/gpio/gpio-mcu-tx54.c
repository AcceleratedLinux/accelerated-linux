// SPDX-License-Identifier: GPL-2.0
/*
 *  Ignition GPIO driver for TX54 MCU
 *  (ignition sense should be disabled to prevent board powering off)
 *
 *  Copyright 2023 Digi International Inc.
 */
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define DEV_NAME		"mcu-tx54-gpio"

enum {
	MCU_TX54_GPIO_IGNITION,
	MCU_TX54_GPIO_POWER_BUTTON,

	MCU_TX54_NGPIO
};

struct mcu_tx54_gpio_info {
	struct mcu_tx54_gpio *gpio;
	unsigned int index;
	uint8_t debounce_10msec;
};

#define MCU_TX54_IRQ_CFG_ENABLED	BIT(4)
#define MCU_TX54_IRQ_CFG_CHANGED	BIT(5)

struct mcu_tx54_gpio {
	struct mcu_tx54 *mcu;
	struct gpio_chip gc;
	struct mcu_tx54_gpio_info pin[MCU_TX54_NGPIO];
	struct irq_chip irqchip;
	struct mutex irq_lock;
	uint8_t irq_cfg[MCU_TX54_NGPIO];
};

static const char *mcu_tx54_gpio_names[MCU_TX54_NGPIO] = {
	"ignition", "pwr_button"
};

static const char *mcu_tx54_gpio_irq_names[] = {
	MCU_TX54_IRQ_NAME_GPIO_IGN,
	MCU_TX54_IRQ_NAME_GPIO_BUTTON,
};

static int mcu_tx54_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	struct mcu_inputs *inputs;

	put_unaligned(MCU_CMD_GET_STATUS, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(gpio->mcu, &tx_pkt, &rx_pkt, get_status);
	if (ret < 0) {
		dev_err(gc->parent, "Get status command failed (%d)\n", ret);
		return ret;
	}

	inputs = &rx_pkt.get_status.inputs;

	switch (offset) {
	case MCU_TX54_GPIO_IGNITION:
		return inputs->ign_sense;
	case MCU_TX54_GPIO_POWER_BUTTON:
		return inputs->wake_btn;
	default:
		return -EINVAL;
	}
}

static int mcu_tx54_gpio_set_debounce(struct mcu_tx54_gpio *gpio,
				      struct device *dev, unsigned int offset,
				      u32 debounce)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	uint8_t debounce_ign, debounce_button;

	if (offset >= MCU_TX54_NGPIO)
		return -EINVAL;

	/*
	 * MCU is using 10-sec granularity.
	 * - the first 10msec is somewhere in the range of ]0-10]msec
	 * - max. is 2550msec
	 * - 0 is disabled
	 */
	debounce = DIV_ROUND_UP(debounce, 10000);
	if (debounce > 0xFF) {
		dev_info(dev,
			 "Setting the maximum debounce time (2550msec) for MCU GPIO %d\n",
			 offset);

		debounce = 0xFF;
	}
	/* Don't allow less than 20msec to make sure debouncing is effective */
	if (debounce == 1)
		debounce++;

	switch (offset) {
	case MCU_TX54_GPIO_IGNITION:
		debounce_ign = debounce;
		debounce_button =
			gpio->pin[MCU_TX54_GPIO_POWER_BUTTON].debounce_10msec;
		break;
	case MCU_TX54_GPIO_POWER_BUTTON:
		debounce_button = debounce;
		debounce_ign =
			gpio->pin[MCU_TX54_GPIO_IGNITION].debounce_10msec;
		break;
	}

	put_unaligned(MCU_CMD_SET_GPIO_INT_DEBOUNCE, &tx_pkt.cmd);
	tx_pkt.set_gpio_int_debounce.ign_10msec = debounce_ign;
	tx_pkt.set_gpio_int_debounce.button_10msec = debounce_button;

	ret = mcu_tx54_transaction(gpio->mcu, &tx_pkt, &rx_pkt,
				   set_gpio_int_debounce);
	if (ret < 0) {
		dev_err(dev, "Couldn't set GPIO interrupt debounce time (%d)\n",
			ret);
		return ret;
	}

	return 0;
}

static int mcu_tx54_gpio_set_config(struct gpio_chip *gc, unsigned int offset,
				    unsigned long config)
{
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	u32 debounce;

	if (pinconf_to_config_param(config) != PIN_CONFIG_INPUT_DEBOUNCE)
		return -ENOTSUPP;

	debounce = pinconf_to_config_argument(config);

	return mcu_tx54_gpio_set_debounce(gpio, gc->parent, offset, debounce);
}

static irqreturn_t mcu_tx54_gpio_irq_handler_thread(int irq, void *data)
{
	struct mcu_tx54_gpio_info *pin = data;
	struct mcu_tx54_gpio *gpio = pin->gpio;

	if (pin->index >= MCU_TX54_NGPIO)
		return IRQ_HANDLED;

	handle_nested_irq(irq_find_mapping(gpio->gc.irq.domain, pin->index));

	return IRQ_HANDLED;
}

static void mcu_tx54_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	int pin = irqd_to_hwirq(d);

	gpio->irq_cfg[pin] &= ~MCU_TX54_IRQ_CFG_ENABLED;
	gpio->irq_cfg[pin] |= MCU_TX54_IRQ_CFG_CHANGED;
}

static void mcu_tx54_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	int pin = irqd_to_hwirq(d);

	gpio->irq_cfg[pin] |= MCU_TX54_IRQ_CFG_CHANGED | MCU_TX54_IRQ_CFG_ENABLED;
}

static void mcu_tx54_gpio_irq_bus_lock(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);

	mutex_lock(&gpio->irq_lock);
}

static int mcu_tx54_gpio_set_gpio_int(struct mcu_tx54_gpio *gpio,
				      struct device *dev, uint16_t cfg)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_SET_GPIO_INT_CFG, &tx_pkt.cmd);
	put_unaligned(cfg, &tx_pkt.set_gpio_int_cfg.param);
	ret = mcu_tx54_transaction(gpio->mcu, &tx_pkt, &rx_pkt,
				   set_gpio_int_cfg);
	if (ret < 0) {
		dev_err(dev, "Failed to set IRQ (%d)\n", ret);
		return ret;
	}

	return 0;
}

static void mcu_tx54_gpio_irq_bus_sync_unlock(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	int i;
	bool changed = false;
	uint16_t cfg = 0;

	for (i = 0; i < MCU_TX54_NGPIO; i++) {
		unsigned int type;

		type = ((gpio->irq_cfg[i] & MCU_TX54_IRQ_CFG_ENABLED) != 0) ?
			       gpio->irq_cfg[i] & MCU_GPIO_INT_TYPE_MASK :
			       0;

		cfg |= type << (MCU_GPIO_INT_SHIFT_BUTTON * i);

		if ((gpio->irq_cfg[i] & MCU_TX54_IRQ_CFG_CHANGED)) {
			changed = true;
			gpio->irq_cfg[i] &= ~MCU_TX54_IRQ_CFG_CHANGED;
		}
	}

	if (changed)
		(void)mcu_tx54_gpio_set_gpio_int(gpio, gc->parent, cfg);

	mutex_unlock(&gpio->irq_lock);
}

static int mcu_tx54_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct mcu_tx54_gpio *gpio = gpiochip_get_data(gc);
	int pin = irqd_to_hwirq(d);
	uint8_t mask;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_LEVEL_LOW:
	case IRQ_TYPE_LEVEL_HIGH:
		dev_err(gpio->gc.parent, "IRQ %d: level IRQs are not supported",
			d->irq);
		return -EINVAL;

	case IRQ_TYPE_EDGE_RISING:
		mask = MCU_GPIO_INT_TYPE_RISING;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		mask = MCU_GPIO_INT_TYPE_FALLING;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		mask = MCU_GPIO_INT_TYPE_BOTH_EDGES;
		break;

	default:
		dev_err(gpio->gc.parent, "IRQ %d: invalid IRQ setting %u",
			d->irq, type);
		return -EINVAL;
	}

	gpio->irq_cfg[pin] &= ~MCU_GPIO_INT_TYPE_MASK;
	gpio->irq_cfg[pin] |= mask | MCU_TX54_IRQ_CFG_CHANGED;

	return 0;
}

static int mcu_tx54_gpio_irq_setup(struct platform_device *pdev,
				   struct mcu_tx54_gpio *gpio)
{
	int i;
	int ret;
	struct gpio_irq_chip *girq;
	struct device *dev = &pdev->dev;

	mutex_init(&gpio->irq_lock);

	gpio->irqchip.name = gpio->gc.label;
	gpio->irqchip.irq_mask = mcu_tx54_gpio_irq_mask;
	gpio->irqchip.irq_unmask = mcu_tx54_gpio_irq_unmask;
	gpio->irqchip.irq_bus_lock = mcu_tx54_gpio_irq_bus_lock;
	gpio->irqchip.irq_bus_sync_unlock = mcu_tx54_gpio_irq_bus_sync_unlock;
	gpio->irqchip.irq_set_type = mcu_tx54_gpio_irq_set_type;

	girq = &gpio->gc.irq;
	girq->chip = &gpio->irqchip;
	girq->parent_handler = NULL;
	girq->num_parents = 0;
	girq->parents = NULL;
	girq->default_type = IRQ_TYPE_NONE;
	girq->handler = handle_edge_irq;
	girq->threaded = true;
	girq->first = 0;

	/* Make sure all interrupts are disabled in MCU */
	ret = mcu_tx54_gpio_set_gpio_int(gpio, dev, 0);
	if (ret)
		dev_err(dev, "Failed to disable GPIO interrupts: %d\n", ret);

	/* And clear out debounce times */
	ret = mcu_tx54_gpio_set_debounce(gpio, dev, 0, 0);
	if (ret)
		dev_err(dev, "Failed to initialize debounce times: %d\n", ret);

	for (i = 0; i < ARRAY_SIZE(mcu_tx54_gpio_irq_names); i++) {
		int irq;
		struct mcu_tx54_gpio_info *pin = &gpio->pin[i];

		irq = platform_get_irq_byname(pdev, mcu_tx54_gpio_irq_names[i]);
		if (irq < 0) {
			dev_err(dev, "No IRQ (%s) defined\n",
				mcu_tx54_gpio_irq_names[i]);
			return -EINVAL;
		}

		pin->index = i;
		pin->gpio = gpio;

		ret = devm_request_threaded_irq(
			dev, irq, NULL, mcu_tx54_gpio_irq_handler_thread,
			IRQF_ONESHOT | IRQF_TRIGGER_HIGH, DEV_NAME, pin);
		if (ret) {
			dev_err(dev, "Failed to request IRQ: %d: %d\n",
				irq, ret);
			return ret;
		}
	}

	return 0;
}

static int mcu_tx54_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mcu_tx54 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct device *mcu_dev = mcu->dev;
	struct mcu_tx54_gpio *gpio;
	int ret;
	struct device_node *np;

	gpio = devm_kzalloc(dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio) {
		dev_err(dev, "Failed to allocate GPIO device\n");
		return -ENOMEM;
	}

	if (!mcu_dev)
		return -EPROBE_DEFER;

	np = of_find_compatible_node(mcu_dev->of_node, NULL,
					"digi,mcu-tx54-gpio");
	if (!np || !of_device_is_available(np))
		return -ENODEV;

	dev->of_node = np;

	gpio->mcu = mcu;

	gpio->gc.label = dev_name(dev);
	gpio->gc.parent = dev;
	gpio->gc.get = mcu_tx54_gpio_get;
	gpio->gc.set_config = mcu_tx54_gpio_set_config;
	gpio->gc.ngpio = MCU_TX54_NGPIO;
	gpio->gc.can_sleep = 1;
	gpio->gc.base = -1;
	gpio->gc.names = mcu_tx54_gpio_names;

	platform_set_drvdata(pdev, gpio);

	ret = mcu_tx54_gpio_irq_setup(pdev, gpio);
	if (ret)
		return ret;

	ret = devm_gpiochip_add_data(dev, &gpio->gc, gpio);
	if (ret < 0) {
		dev_err(dev, "Failed to register gpiochip, %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id mcu_tx54_gpio_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54-gpio", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx54_gpio_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = mcu_tx54_gpio_dt_ids,
	},
	.probe = mcu_tx54_gpio_probe,
};

static int mcu_tx54_gpio_init(void)
{
	return platform_driver_register(&mcu_tx54_gpio_driver);
}
subsys_initcall(mcu_tx54_gpio_init);

static void mcu_tx54_gpio_exit(void)
{
	platform_driver_unregister(&mcu_tx54_gpio_driver);
}
module_exit(mcu_tx54_gpio_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Ignition GPIO driver for MCU of TX54");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DEV_NAME);
