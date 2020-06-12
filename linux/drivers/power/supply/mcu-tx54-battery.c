// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2019 Digi International Inc.
 */
#include <linux/interrupt.h>
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/reboot.h>

#define DEV_NAME	"mcu-tx54-battery"

struct mcu_tx54_battery {
	struct device		*dev;
	struct mcu_tx54		*mcu;
	struct power_supply	*battery;
};

static enum power_supply_property mcu_tx54_battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_ONLINE,
};

/* ADC: 10-bit, Vfs=3.346V, divider=11 */
#define ADC_TO_MV(_raw)		(((_raw) * 11 * 3346 + 512) / 1023)
#define MV_TO_ADC(_v)		(((_v) * 1023 + (11 * 3346 / 2)) / 11 / 3346)

/* Voltage threshold min. value (raw) */
#define VOLTAGE_THRESHOLD_MIN		222
/* Voltage threshold max. value (raw) */
#define VOLTAGE_THRESHOLD_MAX		1023
/* Voltage threshold min. value (uV) */
#define VOLTAGE_THRESHOLD_MIN_IN_UV	\
	(ADC_TO_MV(VOLTAGE_THRESHOLD_MIN) * 1000)
/* Voltage threshold max. value (uV) */
#define VOLTAGE_THRESHOLD_MAX_IN_UV	\
	(ADC_TO_MV(VOLTAGE_THRESHOLD_MAX) * 1000)

static int mcu_tx54_battery_get_prop(struct power_supply *psy,
				     enum power_supply_property psp,
				     union power_supply_propval *val)
{
	struct mcu_tx54_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		put_unaligned(MCU_CMD_GET_STATUS, &tx_pkt.cmd);
		ret = mcu_tx54_transaction(info->mcu, &tx_pkt, &rx_pkt,
					   get_status);
		if (ret < 0) {
			dev_err(info->dev, "Get status command failed (%d)\n",
				ret);
			return ret;
		}
		val->intval =
		    ADC_TO_MV(get_unaligned(&rx_pkt.get_status.battery)) * 1000;
		break;

	case POWER_SUPPLY_PROP_ONLINE:
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static const struct power_supply_desc mcu_tx54_battery_desc = {
	.name		= DEV_NAME,
	.type		= POWER_SUPPLY_TYPE_BATTERY,
	.properties	= mcu_tx54_battery_props,
	.num_properties	= ARRAY_SIZE(mcu_tx54_battery_props),
	.get_property	= mcu_tx54_battery_get_prop,
};

static ssize_t voltage_stup_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx54_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	unsigned int raw;

	put_unaligned(MCU_CMD_GET_VOLTAGE_THRES_STUP, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   get_voltage_thres_stup);
	if (ret < 0) {
		dev_err(info->dev,
			"Couldn't get startup voltage threshold (%d)\n", ret);
		return ret;
	}

	raw = get_unaligned(&rx_pkt.get_voltage_thres_stup.value);

	return scnprintf(buf, PAGE_SIZE, "%d\n", ADC_TO_MV(raw) * 1000);
}

static ssize_t voltage_stup_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx54_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < VOLTAGE_THRESHOLD_MIN_IN_UV ||
	    val > VOLTAGE_THRESHOLD_MAX_IN_UV) {
		dev_err(info->dev, "Invalid startup voltage (%ld) [%d - %d]\n",
			val, VOLTAGE_THRESHOLD_MIN_IN_UV,
			VOLTAGE_THRESHOLD_MAX_IN_UV);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_VOLTAGE_THRES_STUP, &tx_pkt.cmd);
	put_unaligned(MV_TO_ADC(val / 1000),
		      &tx_pkt.set_voltage_thres_stup.param);

	ret = mcu_tx54_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   set_voltage_thres_stup);
	if (ret < 0) {
		dev_err(info->dev,
			"Couldn't set startup voltage threshold (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t voltage_shdn_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx54_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	unsigned int raw;

	put_unaligned(MCU_CMD_GET_VOLTAGE_THRES_SHDN, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   get_voltage_thres_shdn);
	if (ret < 0) {
		dev_err(info->dev,
			"Couldn't get shutdown voltage threshold (%d)\n", ret);
		return ret;
	}

	raw = get_unaligned(&rx_pkt.get_voltage_thres_shdn.value);

	return scnprintf(buf, PAGE_SIZE, "%d\n", ADC_TO_MV(raw) * 1000);
}

static ssize_t voltage_shdn_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx54_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < VOLTAGE_THRESHOLD_MIN_IN_UV ||
	    val > VOLTAGE_THRESHOLD_MAX_IN_UV) {
		dev_err(info->dev, "Invalid shutdown voltage (%ld) [%d - %d]\n",
			val, VOLTAGE_THRESHOLD_MIN_IN_UV,
			VOLTAGE_THRESHOLD_MAX_IN_UV);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_VOLTAGE_THRES_SHDN, &tx_pkt.cmd);
	put_unaligned(MV_TO_ADC(val / 1000),
		      &tx_pkt.set_voltage_thres_shdn.param);

	ret = mcu_tx54_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   set_voltage_thres_shdn);
	if (ret < 0) {
		dev_err(info->dev,
			"Couldn't set shutdown voltage threshold (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_RW(voltage_stup);
static DEVICE_ATTR_RW(voltage_shdn);

static struct attribute *mcu_tx54_battery_sysfs_attrs[] = {
	&dev_attr_voltage_stup.attr,
	&dev_attr_voltage_shdn.attr,
	NULL,
};

ATTRIBUTE_GROUPS(mcu_tx54_battery_sysfs);

static irqreturn_t mcu_tx54_battery_irq_handler_thread(int irq, void *data)
{
	struct mcu_tx54_battery *info = data;

	/* Power off the board */
	dev_emerg(info->dev, "Battery is low, powering off...\n");
	orderly_poweroff(true);

	return IRQ_HANDLED;
}

static int mcu_tx54_battery_probe(struct platform_device *pdev)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct mcu_tx54_battery *info;
	struct device *dev = &pdev->dev;
	struct power_supply_config psy_cfg = {};
	int ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	{
		int irq;

		irq = platform_get_irq_byname(pdev, "IRQ_PWR_FAIL");
		if (irq < 0) {
			dev_err(dev, "No IRQ defined\n");
			return -EINVAL;
		}

		ret = devm_request_threaded_irq(dev, irq, NULL,
			mcu_tx54_battery_irq_handler_thread,
			IRQF_ONESHOT | IRQF_TRIGGER_HIGH, DEV_NAME, info);
		if (ret) {
			dev_err(dev, "Failed to request IRQ: %d\n", ret);
			return ret;
		}
	}

	info->mcu = mcu;
	info->dev = dev;

	psy_cfg.attr_grp = mcu_tx54_battery_sysfs_groups;
	psy_cfg.drv_data = info;

	info->battery = devm_power_supply_register(dev, &mcu_tx54_battery_desc,
						   &psy_cfg);
	if (IS_ERR(info->battery)) {
		ret = PTR_ERR(info->battery);
		dev_err(dev, "Failed to register power supply: %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, info);

	return 0;
}

static const struct of_device_id mcu_tx54_battery_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54-battery", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx54_battery_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = mcu_tx54_battery_dt_ids,
	},
	.probe = mcu_tx54_battery_probe,
};

static int mcu_tx54_battery_init(void)
{
	return platform_driver_register(&mcu_tx54_battery_driver);
}
subsys_initcall(mcu_tx54_battery_init);

static void mcu_tx54_battery_exit(void)
{
	platform_driver_unregister(&mcu_tx54_battery_driver);
}
module_exit(mcu_tx54_battery_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power supply driver for MCU of TX54");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mcu-tx54-battery");
