// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2019 Digi International Inc.
 */
#include <linux/leds.h>
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct mcu_tx54_led {
	struct led_classdev	cdev;
	struct mcu_tx54		*mcu;
};

static int mcu_tx54_led_set_mode(struct led_classdev *cdev,
				 enum mcu_led_mode mode)
{
	struct mcu_tx54_led *led = container_of(cdev, struct mcu_tx54_led,
						cdev);
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_SET_PWR_LED, &tx_pkt.cmd);
	put_unaligned(mode, &tx_pkt.set_pwr_led.param);

	return mcu_tx54_transaction(led->mcu, &tx_pkt, &rx_pkt, set_pwr_led);
}

static int mcu_tx54_led_set(struct led_classdev *cdev,
			    enum led_brightness value)
{
	int ret;
	enum mcu_led_mode mode = (value == LED_OFF) ?
			MCU_LED_MODE_OFF : MCU_LED_MODE_ON;

	ret = mcu_tx54_led_set_mode(cdev, mode);
	if (ret < 0)
		dev_err(cdev->dev, "Couldn't set LED\n");

	return ret;
}

static int mcu_tx54_led_set_blink(struct led_classdev *cdev,
				  unsigned long *delay_on,
				  unsigned long *delay_off)
{
	int ret;
	enum mcu_led_mode mode;

	if (*delay_on == 0) {
		if (*delay_off == 0) {
			/* Set to blinking */
			mode = MCU_LED_MODE_BLINKING_FAST;
			*delay_off = 500;
			*delay_on = 500;
		} else
			mode = MCU_LED_MODE_OFF;
	} else if (*delay_off == 0)
		mode = MCU_LED_MODE_ON;
	else {
		/*
		 * Figure out which mode is closest. Not supporting the distinct
		 * delay_on and delay_off values, so check only one of them
		 */
		if (*delay_on >= 1000)
			mode = MCU_LED_MODE_BLINKING_SLOW;
		else if (*delay_on >= 500)
			mode = MCU_LED_MODE_BLINKING_FAST;
		else
			mode = MCU_LED_MODE_BLINKING_FASTER;
	}

	ret = mcu_tx54_led_set_mode(cdev, mode);
	if (ret < 0)
		dev_err(cdev->dev, "Couldn't set LED blink\n");

	return ret;
}

static int mcu_tx54_leds_probe(struct platform_device *pdev)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct device *mcu_dev = mcu->dev;
	struct mcu_tx54_led *led;
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
					"digi,mcu-tx54-led");
	if (!np)
		return -ENODEV;

	if (!of_device_is_available(np))
		return -ENODEV;

	led->mcu = mcu;

	led->cdev.name = of_get_property(np, "label", NULL) ? : np->name;
	led->cdev.default_trigger = of_get_property(np, "linux,default-trigger",
						    NULL);
	led->cdev.brightness_set_blocking = mcu_tx54_led_set;
	led->cdev.max_brightness = LED_FULL;
	led->cdev.blink_set = mcu_tx54_led_set_blink;

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

	ret = mcu_tx54_led_set(&led->cdev, led->cdev.brightness);
	if (ret < 0)
		return ret;

	ret = devm_led_classdev_register(&pdev->dev, &led->cdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to register LED: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id mcu_tx54_leds_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54-led", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx54_leds_driver = {
	.probe = mcu_tx54_leds_probe,
	//.remove = mcu_tx54_leds_remove,
	.driver = {
		.name	= "mcu-tx54-led",
		.of_match_table = mcu_tx54_leds_dt_ids,
	},
};

static int mcu_tx54_leds_init(void)
{
	return platform_driver_register(&mcu_tx54_leds_driver);
}
subsys_initcall(mcu_tx54_leds_init);

static void mcu_tx54_leds_exit(void)
{
	platform_driver_unregister(&mcu_tx54_leds_driver);
}
module_exit(mcu_tx54_leds_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power LED support for MCU of TX54");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mcu-tx54-led");
