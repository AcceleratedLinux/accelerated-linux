// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2023 Digi International Inc.
 *
 */

#include <linux/gpio/consumer.h>
#include <linux/mfd/mcu-tx40/core.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/watchdog.h>

/* default timeout in seconds */
#define DEFAULT_TIMEOUT		60

#define MODULE_NAME		"mcu-tx40-wdt"

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started. Default: "
		 __MODULE_STRING(WATCHDOG_NOWAYOUT));

struct mcu_tx40_wdt {
	struct watchdog_device wdd;
	struct mcu_tx40 *mcu;
	struct gpio_desc *gpio;
	bool gpio_state;
};

static int mcu_tx40_wdt_get(struct mcu_tx40_wdt *wdt, uint8_t *timeout)
{
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int ret;

	tx_pkt.cmd = MCU_CMD_GET_WDT_TIMEOUT;

	ret = mcu_tx40_transaction(wdt->mcu, &tx_pkt, &rx_pkt, get_wdt_timeout);
	if (ret < 0) {
		dev_err(wdt->mcu->dev, "Failed to get watchdog timeout\n");
		return ret;
	}

	*timeout = rx_pkt.get_wdt_timeout.timeout;

	return 0;
}

static int mcu_tx40_wdt_set(struct mcu_tx40_wdt *wdt, uint8_t timeout)
{
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int ret;

	tx_pkt.cmd = MCU_CMD_SET_WDT_TIMEOUT;
	tx_pkt.set_wdt_timeout.timeout = timeout;

	ret = mcu_tx40_transaction(wdt->mcu, &tx_pkt, &rx_pkt, set_wdt_timeout);
	if (ret < 0) {
		dev_err(wdt->mcu->dev, "Failed to set watchdog timeout (%d)\n",
			ret);
		return ret;
	}

	return 0;
}

static int mcu_tx40_wdt_start(struct watchdog_device *wdd)
{
	struct mcu_tx40_wdt *wdt = watchdog_get_drvdata(wdd);
	int ret;

	wdt->gpio_state = 1;
	gpiod_direction_output(wdt->gpio, wdt->gpio_state);

	ret = mcu_tx40_wdt_set(wdt, wdd->timeout);
	if (ret)
		dev_err(wdt->mcu->dev, "Failed to start watchdog\n");

	return ret;
}

static int mcu_tx40_wdt_stop(struct watchdog_device *wdd)
{
	struct mcu_tx40_wdt *wdt = watchdog_get_drvdata(wdd);
	int ret;

	gpiod_direction_input(wdt->gpio);

	ret = mcu_tx40_wdt_set(wdt, 0);
	if (ret)
		dev_err(wdt->mcu->dev, "Failed to stop watchdog\n");

	return ret;
}

static int mcu_tx40_wdt_set_timeout(struct watchdog_device *wdd,
				    unsigned int timeout)
{
	struct mcu_tx40_wdt *wdt = watchdog_get_drvdata(wdd);

	wdd->timeout = timeout;

	return mcu_tx40_wdt_set(wdt, timeout);
}

static int mcu_tx40_wdt_ping(struct watchdog_device *wdd)
{
	struct mcu_tx40_wdt *wdt = watchdog_get_drvdata(wdd);

	wdt->gpio_state = !wdt->gpio_state;
	gpiod_set_value_cansleep(wdt->gpio, wdt->gpio_state);

	return 0;
}

static const struct watchdog_ops mcu_tx40_wdt_ops = {
	.owner		= THIS_MODULE,
	.start		= mcu_tx40_wdt_start,
	.stop		= mcu_tx40_wdt_stop,
	.ping		= mcu_tx40_wdt_ping,
	.set_timeout	= mcu_tx40_wdt_set_timeout,
};

static const struct watchdog_info mcu_tx40_wdt_info = {
	.options = WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity = MODULE_NAME,
};

static const struct of_device_id mcu_tx40_compatible_id_table[] = {
	{ .compatible = "digi,mcu-tx40-wdt", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mcu_tx40_compatible_id_table);

static int mcu_tx40_wdt_probe(struct platform_device *pdev)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct mcu_tx40_wdt *wdt;
	struct device_node *np = NULL;
	int ret;
	uint8_t timeout;

	if (!mcu->dev)
		return -EPROBE_DEFER;

	wdt = devm_kcalloc(&pdev->dev, 1, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;

	/* Return if node does not exist or if it is disabled */
	np = of_find_compatible_node(mcu->dev->of_node, NULL,
					"digi,mcu-tx40-wdt");
	if (!np || !of_device_is_available(np)) {
		ret = -ENODEV;
		goto err;
	}

	pdev->dev.of_node = np;

	wdt->gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_HIGH);
	if (IS_ERR(wdt->gpio)) {
		dev_err(&pdev->dev, "failed to request GPIO");
		ret = PTR_ERR(wdt->gpio);
		goto err;
	}

	wdt->mcu = mcu;
	wdt->wdd.info = &mcu_tx40_wdt_info;
	wdt->wdd.ops = &mcu_tx40_wdt_ops;
	wdt->wdd.min_timeout = 2;
	wdt->wdd.max_timeout = U8_MAX;
	wdt->wdd.parent = &pdev->dev;

	watchdog_set_nowayout(&wdt->wdd, nowayout);
	watchdog_set_drvdata(&wdt->wdd, wdt);
	watchdog_stop_on_unregister(&wdt->wdd);

	/*
	 * If 'timeout-sec' devicetree property is specified, use that.
	 * Otherwise, use DEFAULT_TIMEOUT
	 */
	wdt->wdd.timeout = DEFAULT_TIMEOUT;
	watchdog_init_timeout(&wdt->wdd, 0, &pdev->dev);
	mcu_tx40_wdt_set_timeout(&wdt->wdd, wdt->wdd.timeout);

	/*
	 * If HW is already running, enable/reset the wdt and set the running
	 * bit to tell the wdt subsystem
	 */
	ret = mcu_tx40_wdt_get(wdt, &timeout);
	if (ret) {
		dev_err(&pdev->dev,
		 	"communication failed. WDT not supported?\n");
		return ret;
	}
	if (timeout != 0) {
		mcu_tx40_wdt_start(&wdt->wdd);
		set_bit(WDOG_HW_RUNNING, &wdt->wdd.status);
	}

	ret = watchdog_register_device(&wdt->wdd);
	if (ret) {
		dev_err(&pdev->dev, "registration failed\n");
		goto err;
	}

	dev_info(&pdev->dev, "registered\n");

	return 0;

err:
	if (np)
		of_node_put(np);

	return ret;
}

static struct platform_driver mcu_tx40_wdt_driver = {
	.probe = mcu_tx40_wdt_probe,
	.driver = {
		.name =	MODULE_NAME,
		.of_match_table = mcu_tx40_compatible_id_table,
	},
};
module_platform_driver(mcu_tx40_wdt_driver);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("TX40 MCU watchdog driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" MODULE_NAME);
