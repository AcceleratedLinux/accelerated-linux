/*
 * drivers/leds/leds-uC53281.c
 *
 * Copyright (c) 2008 Arturus Networks Inc.
 *               by Oleksandr G Zhadan <www.ArcturusNetworks.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 * The Arcturus Networks uC53281-EVM uCsimm board LED driver
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <asm/coldfire.h>
#include <asm/mcfsim.h>

extern void uC53281_gpio_led_pin_set_bn(unsigned char *n, int set);

static void uC53281_led_set(struct led_classdev *led_dev,
			    enum led_brightness value)
{
	uC53281_gpio_led_pin_set_bn((char *)led_dev->name, value);
}

static struct led_classdev uC53281_led2 = {
	.name = "led2",
	.brightness_set = uC53281_led_set,
	.default_trigger = "none",
};

static struct led_classdev uC53281_led3 = {
	.name = "led3",
	.brightness_set = uC53281_led_set,
	.default_trigger = "none",
};

static struct led_classdev uC53281_led4 = {
	.name = "led4",
	.brightness_set = uC53281_led_set,
	.default_trigger = "none",
};

static struct led_classdev uC53281_led5 = {
	.name = "led5",
	.brightness_set = uC53281_led_set,
	.default_trigger = "none",
};

static int uC53281_leds_probe(struct platform_device *pdev)
{
	int ret;

	ret = led_classdev_register(&pdev->dev, &uC53281_led2);
	if (ret)
		goto err_led2;
	ret = led_classdev_register(&pdev->dev, &uC53281_led3);
	if (ret)
		goto err_led3;
	ret = led_classdev_register(&pdev->dev, &uC53281_led4);
	if (ret)
		goto err_led4;
	ret = led_classdev_register(&pdev->dev, &uC53281_led5);
	if (ret)
		goto err_led5;

	return 0;

      err_led5:
	led_classdev_unregister(&uC53281_led4);

      err_led4:
	led_classdev_unregister(&uC53281_led3);

      err_led3:
	led_classdev_unregister(&uC53281_led2);

      err_led2:
	return ret;
}

static int uC53281_leds_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&uC53281_led2);
	led_classdev_unregister(&uC53281_led3);
	led_classdev_unregister(&uC53281_led4);
	led_classdev_unregister(&uC53281_led5);
	return 0;
}

static struct platform_driver uC53281_leds_driver = {
	.driver = {
		   .name = "uC53281:leds",
		   },
	.probe = uC53281_leds_probe,
	.remove = uC53281_leds_remove,
};

static int __init uC53281_leds_init(void)
{
	return platform_driver_register(&uC53281_leds_driver);
}

static void __exit uC53281_leds_exit(void)
{
	platform_driver_unregister(&uC53281_leds_driver);
}

module_init(uC53281_leds_init);
module_exit(uC53281_leds_exit);

MODULE_AUTHOR("Oleksandr Zhadan <www.ArcturusNetworks.com>");
MODULE_DESCRIPTION("LED driver for the Arcturus Networks uC53281-EVM uCsimm board");
MODULE_LICENSE("GPL");
