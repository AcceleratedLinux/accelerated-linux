// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2023 Digi International Inc.
 */
#include <linux/leds.h>
#include <linux/mfd/mcu-tx40/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct mcu_tx40_led {
	struct led_classdev	cdev;
	struct mcu_tx40		*mcu;
};

static int mcu_tx40_led_set_delays(struct led_classdev *cdev, u16 delay_on,
				   u16 delay_off)
{
	struct mcu_tx40_led *led = container_of(cdev, struct mcu_tx40_led,
						cdev);
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_SET_PWR_LED, &tx_pkt.cmd);
	put_unaligned(delay_on, &tx_pkt.set_pwr_led.delay_on);
	put_unaligned(delay_off, &tx_pkt.set_pwr_led.delay_off);

	return mcu_tx40_transaction(led->mcu, &tx_pkt, &rx_pkt, set_pwr_led);
}

static int mcu_tx40_led_set_blink(struct led_classdev *cdev,
				  unsigned long *delay_on,
				  unsigned long *delay_off)
{
	int ret;

	if (*delay_on == 0 && *delay_off == 0) {
		*delay_on = *delay_off = 500;
	} else {
		*delay_on = min((unsigned long)U16_MAX, *delay_on);
		*delay_off = min((unsigned long)U16_MAX, *delay_off);
	}

	ret = mcu_tx40_led_set_delays(cdev, *delay_on, *delay_off);
	if (ret < 0)
		dev_err(cdev->dev, "Failed to set LED blink\n");

	return ret;
}

static int mcu_tx40_led_set(struct led_classdev *cdev,
			    enum led_brightness value)
{
	int ret;
	u16 delay_off = 0, delay_on = 0;

	if (value)
		delay_on = 1;
	else
		delay_off = 1;

	ret = mcu_tx40_led_set_delays(cdev, delay_on, delay_off);
	if (ret < 0)
		dev_err(cdev->dev, "Failed to set LED\n");

	return ret;
}

static enum led_brightness mcu_tx40_led_get(struct led_classdev *cdev)
{
	struct mcu_tx40_led *led = container_of(cdev, struct mcu_tx40_led,
						cdev);
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int ret;
	unsigned int delay_off;

	put_unaligned(MCU_CMD_GET_PWR_LED, &tx_pkt.cmd);

	ret = mcu_tx40_transaction(led->mcu, &tx_pkt, &rx_pkt, get_pwr_led);
	if (ret < 0) {
		dev_err(cdev->dev, "Failed to get LED\n");
		return ret;
	}

	delay_off = get_unaligned(&rx_pkt.get_pwr_led.delay_off);

	return delay_off ? LED_OFF : LED_FULL;
}

static int mcu_tx40_leds_probe(struct platform_device *pdev)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct device *mcu_dev = mcu->dev;
	struct mcu_tx40_led *led;
	struct device_node *np;
	const char *state;
	int ret;

	if (!mcu_dev)
		return -EPROBE_DEFER;

	led = devm_kcalloc(&pdev->dev, 1, sizeof(*led), GFP_KERNEL);
	if (!led)
		return -ENOMEM;

	/* Return if node does not exist or if it is disabled */
	np = of_find_compatible_node(mcu_dev->of_node, NULL,
					"digi,mcu-tx40-led");
	if (!np)
		return -ENODEV;

	if (!of_device_is_available(np))
		return -ENODEV;

	led->mcu = mcu;

	led->cdev.name = of_get_property(np, "label", NULL) ? : np->name;
	led->cdev.default_trigger = of_get_property(np, "linux,default-trigger",
						    NULL);
	led->cdev.brightness_set_blocking = mcu_tx40_led_set;
	led->cdev.brightness_get = mcu_tx40_led_get;
	led->cdev.max_brightness = LED_FULL;
	led->cdev.blink_set = mcu_tx40_led_set_blink;

	platform_set_drvdata(pdev, led);

	state = of_get_property(np, "default-state", NULL);
	if (state) {
		if (!strcmp(state, "on")) {
			led->cdev.brightness = LED_FULL;
		} else if (!strcmp(state, "keep")) {
			dev_warn(mcu_dev,
				 "Invalid default-state 'keep' for LED\n");
			led->cdev.brightness = LED_OFF;
		} else {
			led->cdev.brightness = LED_OFF;
		}
	} else
		led->cdev.brightness = LED_OFF;

	ret = mcu_tx40_led_set(&led->cdev, led->cdev.brightness);
	if (ret < 0)
		return ret;

	ret = devm_led_classdev_register(&pdev->dev, &led->cdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register LED: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id mcu_tx40_leds_dt_ids[] = {
	{ .compatible = "digi,mcu-tx40-led", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx40_leds_driver = {
	.probe = mcu_tx40_leds_probe,
	.driver = {
		.name	= "mcu-tx40-led",
		.of_match_table = mcu_tx40_leds_dt_ids,
	},
};

static int mcu_tx40_leds_init(void)
{
	return platform_driver_register(&mcu_tx40_leds_driver);
}
subsys_initcall(mcu_tx40_leds_init);

static void mcu_tx40_leds_exit(void)
{
	platform_driver_unregister(&mcu_tx40_leds_driver);
}
module_exit(mcu_tx40_leds_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power LED support for MCU of TX40");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mcu-tx40-led");
