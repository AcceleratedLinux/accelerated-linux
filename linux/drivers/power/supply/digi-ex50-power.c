// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2024 Digi International Inc.
 */
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>

#define DEV_NAME	"ex50-power"

struct digi_ex50_power {
	struct device		*dev;
	struct power_supply	*poe;
	struct power_supply	*dc;

	struct gpio_desc *poe_present_gpio;
	struct gpio_desc *poe_mec_gpio;
	struct gpio_desc *dc_present_gpio;

	int dc_irqn;
	int poe_irqn;
	int mec_irqn;

	volatile u64 last_rising;
	volatile unsigned poe_class;
};

static irqreturn_t state_irq_handler_thread(int irq, struct digi_ex50_power *dev_id) {
	/* Calling power_supply_changed allows udev to perform actions when we
	 * detect changes. */
	if (irq == dev_id->dc_irqn) {
		power_supply_changed(dev_id->dc);
	} else {
		if (!gpiod_get_value(dev_id->poe_present_gpio)) {
			/* Reset PoE class on disconnection */
			dev_id->poe_class = 0;
		}
		power_supply_changed(dev_id->poe);
	}

	return IRQ_HANDLED;
}

static irqreturn_t mec_irq_handler(int irq, struct digi_ex50_power *dev_id) {
	int ret = IRQ_HANDLED;

	u64 ts = ktime_get();

	if (gpiod_get_value(dev_id->poe_mec_gpio)) {
		dev_id->last_rising = ts;
	} else {
		unsigned pw_ns = (unsigned)(ts - dev_id->last_rising);
		unsigned poe_class;

		if (pw_ns < 1000000) {
			/* Note: To support detecting PoE++ and above, we would
			 * need to check multiple pulse widths in a row. */
			poe_class = 4;
		} else {
			poe_class = 0;
		}

		if (dev_id->poe_class != poe_class) {
			dev_id->poe_class = poe_class;
			/* Notify that the available current has changed */
			ret = IRQ_WAKE_THREAD;
		}
	}

	return ret;
}


#define EX50_SUPPLY_V  12
/* Max currents are measured in uA */
#define POE_CURRENT_AF (12950000 / EX50_SUPPLY_V)
#define POE_CURRENT_AT (25500000 / EX50_SUPPLY_V)



static int digi_ex50_power_poe_get_prop(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
	struct digi_ex50_power *info = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		if (gpiod_get_value_cansleep(info->poe_present_gpio)) {
			if (info->poe_class == 4) {
				val->intval = POE_CURRENT_AT;
			} else {
				val->intval = POE_CURRENT_AF;
			}
		} else {
			val->intval = 0;
		}
		break;

	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = (gpiod_get_value_cansleep(info->poe_present_gpio) != 0);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static enum power_supply_property digi_ex50_power_poe_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX
};

static const struct power_supply_desc digi_ex50_power_poe_desc = {
	.name		= "poe",
	.type		= POWER_SUPPLY_TYPE_MAINS,
	.properties	= digi_ex50_power_poe_props,
	.num_properties	= ARRAY_SIZE(digi_ex50_power_poe_props),
	.get_property	= digi_ex50_power_poe_get_prop,
};


static int digi_ex50_power_dc_get_prop(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct digi_ex50_power *info = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = (gpiod_get_value_cansleep(info->dc_present_gpio) != 0);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static enum power_supply_property digi_ex50_power_dc_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static const struct power_supply_desc digi_ex50_power_dc_desc = {
	.name		= "dc",
	.type		= POWER_SUPPLY_TYPE_MAINS,
	.properties	= digi_ex50_power_dc_props,
	.num_properties	= ARRAY_SIZE(digi_ex50_power_dc_props),
	.get_property	= digi_ex50_power_dc_get_prop,
};



static int digi_ex50_power_probe(struct platform_device *pdev)
{
	struct digi_ex50_power *info;
	struct device *dev = &pdev->dev;
	struct power_supply_config psy_cfg = {};
	int ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (!info) {
		return -ENOMEM;
	}

	info->dev = dev;

	psy_cfg.drv_data   = info;
	dev->platform_data = info;

	info->poe = devm_power_supply_register(dev, &digi_ex50_power_poe_desc,
					       &psy_cfg);
	if (IS_ERR(info->poe)) {
		return dev_err_probe(dev, PTR_ERR(info->poe),
				     "Failed to register power supply for PoE\n");
	}

	info->dc = devm_power_supply_register(dev, &digi_ex50_power_dc_desc,
					      &psy_cfg);
	if (IS_ERR(info->dc)) {
		return dev_err_probe(dev, PTR_ERR(info->dc),
				     "Failed to register power supply for DC\n");
	}

	/* Get GPIO pins from device-tree */

	info->poe_present_gpio = devm_gpiod_get(dev, "poe-present", GPIOD_IN);
	if (IS_ERR(info->poe_present_gpio)) {
		return dev_err_probe(dev, PTR_ERR(info->poe_present_gpio),
				     "Failed to register poe-present GPIO\n");
	}
	info->poe_mec_gpio = devm_gpiod_get(dev, "poe-mec", GPIOD_IN);
	if (IS_ERR(info->poe_mec_gpio)) {
		return dev_err_probe(dev, PTR_ERR(info->poe_mec_gpio),
				     "Failed to register poe-mec GPIO\n");
	}
	info->dc_present_gpio = devm_gpiod_get(dev, "dc-present", GPIOD_IN);
	if (IS_ERR(info->dc_present_gpio)) {
		return dev_err_probe(dev, PTR_ERR(info->dc_present_gpio),
				     "Failed to register dc-present GPIO\n");
	}

	info->dc_irqn = gpiod_to_irq(info->dc_present_gpio);
	if (info->dc_irqn < 0) {
		return dev_err_probe(dev, info->dc_irqn,
				     "Failed to get dc-present IRQ number\n");
	}
	info->poe_irqn = gpiod_to_irq(info->poe_present_gpio);
	if (info->poe_irqn < 0) {
		return dev_err_probe(dev, info->poe_irqn,
				     "Failed to get poe-present IRQ number\n");
	}
	info->mec_irqn = gpiod_to_irq(info->poe_mec_gpio);
	if (info->mec_irqn < 0) {
		return dev_err_probe(dev, info->mec_irqn,
				     "Failed to get poe-mec IRQ number\n");
	}

	ret = devm_request_threaded_irq(dev, info->dc_irqn, NULL,
					(void *)state_irq_handler_thread,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					DEV_NAME, info);
	if (ret) {
		return dev_err_probe(dev, ret,
				     "Failed to register dc-present IRQ handler\n");
	}
	ret = devm_request_threaded_irq(dev, info->poe_irqn, NULL,
					(void *)state_irq_handler_thread,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					DEV_NAME, info);
	if (ret) {
		return dev_err_probe(dev, ret,
				     "Failed to register poe-present IRQ handler\n");
	}
	ret = devm_request_threaded_irq(dev, info->mec_irqn,
					(void *)mec_irq_handler,
					(void *)state_irq_handler_thread,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
					DEV_NAME, info);
	if (ret) {
		return dev_err_probe(dev, ret,
				     "Failed to register poe-mec IRQ handler\n");
	}

	platform_set_drvdata(pdev, info);

	return 0;
}

static const struct of_device_id digi_ex50_power_dt_ids[] = {
	{ .compatible = "digi,ex50-power", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, digi_ex50_power_dt_ids);

static struct platform_driver digi_ex50_power_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = digi_ex50_power_dt_ids,
	},
	.probe = digi_ex50_power_probe
};
module_platform_driver(digi_ex50_power_driver);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power supply driver for EX50");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:ex50-power");
