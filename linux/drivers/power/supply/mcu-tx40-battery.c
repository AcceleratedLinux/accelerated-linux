// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2023 Digi International Inc.
 */
#include <linux/mfd/mcu-tx40/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/reboot.h>

#define DEV_NAME	"mcu-tx40-battery"

struct mcu_tx40_battery {
	struct device		*dev;
	struct mcu_tx40		*mcu;
	struct power_supply	*battery;
};

static enum power_supply_property mcu_tx40_battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_ONLINE,
};

#define VIN_MIN_MV		6000
#define VIN_MAX_MV		35000
#define VIN_MIN_HYSTERESIS_MV	100

static int mcu_tx40_battery_get_prop(struct power_supply *psy,
				     enum power_supply_property psp,
				     union power_supply_propval *val)
{
	struct mcu_tx40_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		put_unaligned(MCU_CMD_GET_IO_INFO, &tx_pkt.cmd);
		ret = mcu_tx40_transaction(info->mcu, &tx_pkt, &rx_pkt,
					   get_io_info);
		if (ret) {
			dev_err(info->dev, "Get IO info command failed (%d)\n",
				ret);
			return ret;
		}
		val->intval = get_unaligned(&rx_pkt.get_io_info.vin) * 1000;
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

static const struct power_supply_desc mcu_tx40_battery_desc = {
	.name		= DEV_NAME,
	.type		= POWER_SUPPLY_TYPE_BATTERY,
	.properties	= mcu_tx40_battery_props,
	.num_properties	= ARRAY_SIZE(mcu_tx40_battery_props),
	.get_property	= mcu_tx40_battery_get_prop,
};

static ssize_t voltage_thresholds_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx40_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	char end;
	int stup, shdn;

	ret = sscanf(buf, "%d %d%c", &stup, &shdn, &end);
	if (ret != 3) {
		dev_err(info->dev,
			"Missing parameter, should be '<Vstup> <Vshdn>'\n");
		return -EINVAL;
	}
	if (end != '\n') {
		dev_err(info->dev, "Extra parameters\n");
		return -EINVAL;
	}

	stup /= 1000;
	shdn /= 1000;

	if (stup < VIN_MIN_MV || stup > VIN_MAX_MV ||
	    shdn < VIN_MIN_MV || shdn > VIN_MAX_MV ||
	    (stup - shdn) < VIN_MIN_HYSTERESIS_MV) {
		dev_err(info->dev,
			"Invalid startup (%d) or shutdown (%d) voltage (range: [%d - %d], hysteresis: %d)\n",
			stup, shdn, VIN_MIN_MV, VIN_MAX_MV,
			VIN_MIN_HYSTERESIS_MV);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_VIN_STUP_SHDN, &tx_pkt.cmd);
	put_unaligned(stup, &tx_pkt.set_vin_stup_shdn.stup_mv);
	put_unaligned(shdn, &tx_pkt.set_vin_stup_shdn.shdn_mv);

	ret = mcu_tx40_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   set_vin_stup_shdn);
	if (ret) {
		dev_err(info->dev,
			"Couldn't set voltage thresholds (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t voltage_thresholds_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct mcu_tx40_battery *info = power_supply_get_drvdata(psy);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	unsigned int stup, shdn;

	put_unaligned(MCU_CMD_GET_VIN_STUP_SHDN, &tx_pkt.cmd);
	ret = mcu_tx40_transaction(info->mcu, &tx_pkt, &rx_pkt,
				   get_vin_stup_shdn);
	if (ret < 0) {
		dev_err(info->dev,
			"Couldn't get voltage thresholds (%d)\n", ret);
		return ret;
	}

	stup = get_unaligned(&rx_pkt.get_vin_stup_shdn.stup_mv);
	shdn = get_unaligned(&rx_pkt.get_vin_stup_shdn.shdn_mv);

	return scnprintf(buf, PAGE_SIZE, "%u %u\n", stup * 1000, shdn * 1000);
}

static DEVICE_ATTR_RW(voltage_thresholds);

static struct attribute *mcu_tx40_battery_sysfs_attrs[] = {
	&dev_attr_voltage_thresholds.attr,
	NULL,
};

ATTRIBUTE_GROUPS(mcu_tx40_battery_sysfs);

static int mcu_tx40_battery_probe(struct platform_device *pdev)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct mcu_tx40_battery *info;
	struct device *dev = &pdev->dev;
	struct power_supply_config psy_cfg = {};
	int ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->mcu = mcu;
	info->dev = dev;

	psy_cfg.attr_grp = mcu_tx40_battery_sysfs_groups;
	psy_cfg.drv_data = info;

	info->battery = devm_power_supply_register(dev, &mcu_tx40_battery_desc,
						   &psy_cfg);
	if (IS_ERR(info->battery)) {
		ret = PTR_ERR(info->battery);
		dev_err(dev, "Failed to register power supply: %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, info);

	return 0;
}

static const struct of_device_id mcu_tx40_battery_dt_ids[] = {
	{ .compatible = "digi,mcu-tx40-battery", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx40_battery_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = mcu_tx40_battery_dt_ids,
	},
	.probe = mcu_tx40_battery_probe,
};

static int mcu_tx40_battery_init(void)
{
	return platform_driver_register(&mcu_tx40_battery_driver);
}
subsys_initcall(mcu_tx40_battery_init);

static void mcu_tx40_battery_exit(void)
{
	platform_driver_unregister(&mcu_tx40_battery_driver);
}
module_exit(mcu_tx40_battery_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power supply driver for MCU of TX40");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mcu-tx40-battery");
