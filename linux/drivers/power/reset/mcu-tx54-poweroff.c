// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2019 Digi International Inc.
 *
 */
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct mcu_tx54_poweroff {
	struct device	*dev;
	struct mcu_tx54	*mcu;
};

static struct mcu_tx54_poweroff *mcu_tx54_pm_poweroff;

static void mcu_tx54_pm_power_off(void)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int retry = 3;

	if (!mcu_tx54_pm_poweroff) {
		pr_err("MCU poweroff is not initialized\n");
		goto err;
	}

	put_unaligned(MCU_CMD_SET_POWER, &tx_pkt.cmd);
	put_unaligned(POWER_STATE_OFF, &tx_pkt.set_power.param);

	do {
		ret = mcu_tx54_transaction(mcu_tx54_pm_poweroff->mcu, &tx_pkt,
					   &rx_pkt, set_power);
		if (ret < 0)
			dev_err(mcu_tx54_pm_poweroff->dev,
				"MCU power off command failed with (%d)\n",
				ret);
	} while (--retry > 0);

err:
	/* Fall back to machine_hang(), the MCU will force power us down  */
	return;
}

static const struct of_device_id mcu_tx54_poweroff_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54-poweroff", },
	{ /* sentinel */ }
};

static int mcu_tx54_poweroff_probe(struct platform_device *pdev)
{
	struct mcu_tx54_poweroff *mcu_tx54_poweroff;
	struct mcu_tx54 *mcu = dev_get_drvdata(pdev->dev.parent);

	mcu_tx54_poweroff = devm_kzalloc(&pdev->dev, sizeof(*mcu_tx54_poweroff),
					 GFP_KERNEL);
	if (!mcu_tx54_poweroff)
		return -ENOMEM;

	mcu_tx54_poweroff->dev = &pdev->dev;
	mcu_tx54_poweroff->mcu = mcu;

	mcu_tx54_pm_poweroff = mcu_tx54_poweroff;

	if (!pm_power_off)
		pm_power_off = mcu_tx54_pm_power_off;

	return 0;
}

static int mcu_tx54_poweroff_remove(struct platform_device *pdev)
{
	if (pm_power_off == mcu_tx54_pm_power_off)
		pm_power_off = NULL;

	mcu_tx54_pm_poweroff = NULL;

	return 0;
}

static struct platform_driver mcu_tx54_poweroff_driver = {
	.driver = {
		.name = "mcu-tx54-poweroff",
		.of_match_table = mcu_tx54_poweroff_dt_ids,
	},
	.probe = mcu_tx54_poweroff_probe,
	.remove = mcu_tx54_poweroff_remove,
};

static int mcu_tx54_poweroff_init(void)
{
	return platform_driver_register(&mcu_tx54_poweroff_driver);
}
subsys_initcall(mcu_tx54_poweroff_init);

static void mcu_tx54_poweroff_exit(void)
{
	platform_driver_unregister(&mcu_tx54_poweroff_driver);
}
module_exit(mcu_tx54_poweroff_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power off driver for MCU of TX54");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mcu-tx54-poweroff");
