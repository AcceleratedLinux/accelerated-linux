// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2023 Digi International Inc.
 *
 */
#include <linux/delay.h>
#include <linux/mfd/mcu-tx40/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>

#define DEV_NAME "mcu-tx40-poweroff"

struct mcu_tx40_poweroff {
	struct device	*dev;
	struct mcu_tx40	*mcu;
};

static struct mcu_tx40_poweroff *mcu_tx40_poweroff;

static int mcu_tx40_pm_power_off(struct sys_off_data *data)
{
	kernel_restart("MCU power off");

	return NOTIFY_DONE;
}

static int mcu_tx40_poweroff_reboot_notifier(struct notifier_block *nb,
					     unsigned long action, void *unused)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int retry = 3;

	if (action != SYS_POWER_OFF)
		return NOTIFY_DONE;

	put_unaligned(MCU_CMD_SET_POWEROFF, &tx_pkt.cmd);
	put_unaligned(MCU_MAGIC_SET_POWEROFF, &tx_pkt.set_poweroff.magic);

	do {
		ret = mcu_tx40_transaction(mcu_tx40_poweroff->mcu, &tx_pkt,
					   &rx_pkt, set_poweroff);
		if (ret < 0)
			dev_err(mcu_tx40_poweroff->dev,
				"MCU power off prepare command failed with (%d)\n",
				ret);
	} while (ret && --retry > 0);

	if (ret < 0)
		dev_err(mcu_tx40_poweroff->dev,
			"failed to send poweroff command, will reboot instead\n");

	return NOTIFY_DONE;
}

static struct notifier_block mcu_tx40_poweroff_reboot_nb = {
	.notifier_call = mcu_tx40_poweroff_reboot_notifier,
};

static const struct of_device_id mcu_tx40_poweroff_dt_ids[] = {
	{ .compatible = "digi,mcu-tx40-poweroff", },
	{ /* sentinel */ }
};

static int mcu_tx40_poweroff_probe(struct platform_device *pdev)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(pdev->dev.parent);
	int ret;

	mcu_tx40_poweroff = devm_kzalloc(&pdev->dev, sizeof(*mcu_tx40_poweroff),
					 GFP_KERNEL);
	if (!mcu_tx40_poweroff)
		return -ENOMEM;

	mcu_tx40_poweroff->dev = &pdev->dev;
	mcu_tx40_poweroff->mcu = mcu;

	/*
	 * Reset with TX40 happens according the following:
	 *  1. call CMD_SET_POWEROFF to set power-off flag in MCU
	 *  2. assert RESET_REQ_B pin of the chip to ask for a reset from MCU
	 */
	ret = devm_register_reboot_notifier(&pdev->dev,
					    &mcu_tx40_poweroff_reboot_nb);
	if (ret) {
		dev_err(&pdev->dev,
			"failed to register poweroff reboot notifier\n");
		return ret;
	}

	ret = devm_register_sys_off_handler(&pdev->dev, SYS_OFF_MODE_POWER_OFF,
					    SYS_OFF_PRIO_DEFAULT + 1,
					    mcu_tx40_pm_power_off, NULL);
	if (ret) {
		dev_err(&pdev->dev,
			"failed to register power off handler\n");
		return ret;
	}

	return 0;
}

static struct platform_driver mcu_tx40_poweroff_driver = {
	.driver = {
		.name = "mcu-tx40-poweroff",
		.of_match_table = mcu_tx40_poweroff_dt_ids,
	},
	.probe = mcu_tx40_poweroff_probe,
};

static int mcu_tx40_poweroff_init(void)
{
	return platform_driver_register(&mcu_tx40_poweroff_driver);
}
subsys_initcall(mcu_tx40_poweroff_init);

static void mcu_tx40_poweroff_exit(void)
{
	platform_driver_unregister(&mcu_tx40_poweroff_driver);
}
module_exit(mcu_tx40_poweroff_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power off driver for MCU of TX40");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DEV_NAME);
