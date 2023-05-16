/*
 *  Copyright 2016, 2017, 2018 Digi International, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>

#define DRIVER_NAME		"awusb24_cpld"
#define AWUSB24_MAX_LEDS	(24*3 + 8*2)  /* 24-port has a lot. */

/* LED bits */
#define AWUSB_RED		0x04
#define AWUSB_GREEN		0x02
#define AWUSB_BLUE		0x01

#define AWUSB_RGB_MASK		(AWUSB_RED|AWUSB_GREEN|AWUSB_BLUE)

/* Registers. */
#define AWUSB24_PWR1		0x30

enum awusb24_id {
	AWUSB24_CPLD,
};
static const struct i2c_device_id awusb24_cpld_id[] = {
	{ DRIVER_NAME,  AWUSB24_CPLD },
	{ }
};

MODULE_DEVICE_TABLE(i2c, awusb24_cpld_id);

#ifdef CONFIG_OF
static const struct of_device_id awusb24_cpld_of_match[] = {
	{ .compatible = "digi,awusb24_cpld", },
	{},
};
MODULE_DEVICE_TABLE(of, awusb24_cpld_of_match);
#endif

extern int awusb24_cpld_register_sysfs(struct i2c_client *client);
extern void awusb24_cpld_unregister_sysfs(struct i2c_client *client);

/*
 * struct awusb24_cpld_chip
 * @cl         : I2C communication for access registers
 * @pdata      : Platform specific data
 * @lock       : Lock for user-space interface
 * @num_leds   : Number of registered LEDs
 * @cfg        : Device specific configuration data
 */
struct awusb24_cpld_chip {
	struct i2c_client *cl;
	struct awusb24_cpld_platform_data *pdata;
	struct mutex lock;	/* lock for user-space interface */
	int num_leds;
	struct awusb24_cpld_device_config *cfg;
};

/*
 * struct awusb24_cpld_config
 * @name       : LED name
 * @default_trigger: Default trigger mode
 * @chan_nr    : LED channel number
 * @reg        : Register of this LED
 * @led        : Bitmask of this LED
 */
struct awusb24_cpld_config {
	const char *name;
	const char *default_trigger;
	u8 chan_nr;
	u8 reg;
	u8 led;
};

/*
 * struct awusb24_cpld_platform_data
 * @led_config        : Configurable led class device
 * @num_channels      : Number of LED channels
 * @label             : Used for naming LEDs
 */
struct awusb24_cpld_platform_data {

	/* LED channel configuration */
	struct awusb24_cpld_config *led_config;
	u8 num_channels;
	const char *label;
};

/*
 * struct awusb24_cpld
 * @chan_nr         : Channel number
 * @cdev            : LED class device
 * @reg             : LED register
 * @led             : LED bit
 * @brightness      : Brightness value
 * @chip            : The awusb24_cpld chip data
 */
struct awusb24_cpld {
	int chan_nr;
	struct led_classdev cdev;
	u8 reg;
	u8 led;
	u8 brightness;
	struct awusb24_cpld_chip *chip;
};

/*
 * struct awusb24_cpld_device_config
 * @max_channel        : Maximum number of channels
 */
struct awusb24_cpld_device_config {
	const int max_channel;
};

static struct awusb24_cpld *cdev_to_awusb24_cpld(struct led_classdev *cdev)
{
	return container_of(cdev, struct awusb24_cpld, cdev);
}

static int awusb24_cpld_set_brightness(struct led_classdev *cdev,
			     enum led_brightness brightness)
{
	struct awusb24_cpld *led = cdev_to_awusb24_cpld(cdev);
	struct awusb24_cpld_chip *chip = led->chip;
	int ret = -EINVAL;
	int led_val;

	mutex_lock(&chip->lock);
	led->brightness = (u8)brightness;

	//dev_info(&chip->cl->dev, "brightness %x[%x]", led->reg, led->led);
	led_val = i2c_smbus_read_byte_data(chip->cl, led->reg);
	//dev_info(&chip->cl->dev, "%s %s", chip->pdata->led_config[led->chan_nr].name, led->brightness ? "on" : "off");
	if (led_val < 0)
	{
		ret = led_val;
		goto exit;
	}

	if (led->brightness == LED_OFF)
		led_val = (led_val & AWUSB_RGB_MASK) & ~led->led;
	else
		led_val = (led_val & AWUSB_RGB_MASK) | led->led;

	//dev_info(&chip->cl->dev, "new %d", led_val);
	ret = i2c_smbus_write_byte_data(chip->cl, led->reg, led_val);
exit:
	mutex_unlock(&chip->lock);
	return ret;
}

#define AWUSB24_CPLD_DEV_ATTR_RW(name, show, store)	\
	DEVICE_ATTR(name, S_IRUGO | S_IWUSR, show, store)
#define AWUSB24_CPLD_DEV_ATTR_RO(name, show)		\
	DEVICE_ATTR(name, S_IRUGO, show, NULL)
#define AWUSB24_CPLD_DEV_ATTR_WO(name, store)		\
	DEVICE_ATTR(name, S_IWUSR, NULL, store)

/* Chip specific configurations */
static struct awusb24_cpld_device_config awusb24_cpld_cfg = {
	.max_channel        = AWUSB24_MAX_LEDS,
};

static struct awusb24_cpld_platform_data *awusb24_cpld_of_populate_pdata(struct device *dev,
						      struct device_node *np)
{
	struct device_node *child;
	struct awusb24_cpld_platform_data *pdata;
	struct awusb24_cpld_config *cfg;
	int num_channels;
	int i = 0;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return ERR_PTR(-ENOMEM);

	num_channels = of_get_child_count(np);
	if (num_channels <= 0) {
		dev_err(dev, "no LED channels\n");
		return ERR_PTR(-EINVAL);
	}
	if (num_channels > AWUSB24_MAX_LEDS) {
		dev_err(dev, "too many LED channels %d\n", num_channels);
		return ERR_PTR(-EINVAL);
	}


	cfg = devm_kzalloc(dev, sizeof(*cfg) * num_channels, GFP_KERNEL);
	if (!cfg)
		return ERR_PTR(-ENOMEM);

	pdata->led_config = &cfg[0];
	pdata->num_channels = num_channels;

	for_each_child_of_node(np, child) {
		uint32_t reg;

		cfg[i].chan_nr = i;

		of_property_read_string(child, "name", &cfg[i].name);
		of_property_read_u8(child, "led", &cfg[i].led);
		of_property_read_u32(child, "reg", &reg);
		cfg[i].reg = (uint8_t)reg;
		cfg[i].default_trigger =
			of_get_property(child, "linux,default-trigger", NULL);
		i++;
	}

	of_property_read_string(np, "label", &pdata->label);

	return pdata;
}

static void awusb24_cpld_reset_device(struct awusb24_cpld_chip *chip)
{
}

static int awusb24_cpld_detect_device(struct awusb24_cpld_chip *chip)
{
	int led_val = i2c_smbus_read_byte_data(chip->cl, AWUSB24_PWR1);
	if (led_val < 0)
		return -ENODEV;
	return 0;
}

static void awusb24_cpld_deinit_device(struct awusb24_cpld_chip *chip)
{
}

static int awusb24_cpld_init_device(struct awusb24_cpld_chip *chip)
{
	struct awusb24_cpld_platform_data *pdata;
	struct awusb24_cpld_device_config *cfg;
	struct device *dev = &chip->cl->dev;
	int ret = 0;

	WARN_ON(!chip);

	pdata = chip->pdata;
	cfg = chip->cfg;

	if (!pdata || !cfg)
		return -EINVAL;

	awusb24_cpld_reset_device(chip);

	/* Wait for reset. */
	//usleep_range(10000, 20000);

	ret = awusb24_cpld_detect_device(chip);
	if (ret) {
		dev_err(dev, "device detection err: %d\n", ret);
		goto err;
	}

	return 0;

err:
	awusb24_cpld_deinit_device(chip);
	return ret;
}

static int awusb24_cpld_init_led(struct awusb24_cpld *led,
			struct awusb24_cpld_chip *chip, int chan)
{
	struct awusb24_cpld_platform_data *pdata = chip->pdata;
	struct awusb24_cpld_device_config *cfg = chip->cfg;
	struct device *dev = &chip->cl->dev;
	char name[32];
	int ret;
	int max_channel = cfg->max_channel;

	if (chan >= max_channel) {
		dev_err(dev, "invalid channel: %d / %d\n", chan, max_channel);
		return -EINVAL;
	}

	if (pdata->led_config[chan].reg == 0)
		return 0;

	led->reg = pdata->led_config[chan].reg;
	led->led = pdata->led_config[chan].led;
	led->chan_nr = pdata->led_config[chan].chan_nr;
	led->cdev.default_trigger = pdata->led_config[chan].default_trigger;
	//dev_info(dev, "Added LED %s[%d] at %x:%x", pdata->led_config[chan].name ? : "none", chan, led->reg, led->led);

	if (led->chan_nr >= max_channel) {
		dev_err(dev, "Use channel numbers between 0 and %d\n",
			max_channel - 1);
		return -EINVAL;
	}

	led->cdev.brightness_set_blocking = awusb24_cpld_set_brightness;

	if (pdata->led_config[chan].name) {
		led->cdev.name = pdata->led_config[chan].name;
	} else {
		snprintf(name, sizeof(name), "%s_%d",
			pdata->label ? : chip->cl->name, chan);
		led->cdev.name = name;
	}

	ret = led_classdev_register(dev, &led->cdev);
	if (ret) {
		dev_err(dev, "led register err: %d\n", ret);
		return ret;
	}

	return 0;
}

static void awusb24_cpld_unregister_leds(struct awusb24_cpld *led, struct awusb24_cpld_chip *chip)
{
	int i;
	struct awusb24_cpld *each;

	for (i = 0; i < chip->num_leds; i++) {
		each = led + i;
		led_classdev_unregister(&each->cdev);
	}
}

static int awusb24_cpld_register_leds(struct awusb24_cpld *led, struct awusb24_cpld_chip *chip)
{
	struct awusb24_cpld_platform_data *pdata = chip->pdata;
	int num_channels = pdata->num_channels;
	struct awusb24_cpld *each;
	int ret;
	int i;

	for (i = 0; i < num_channels; i++) {

		/* do not initialize channels that are not connected */
		if (pdata->led_config[i].reg == 0)
			continue;

		each = led + i;
		ret = awusb24_cpld_init_led(each, chip, i);
		if (ret)
			goto err_init_led;

		chip->num_leds++;
		each->chip = chip;
	}

	return 0;

err_init_led:
	awusb24_cpld_unregister_leds(led, chip);
	return ret;
}

static int awusb24_cpld_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int ret;
	struct awusb24_cpld_chip *chip;
	struct awusb24_cpld *led;
	struct awusb24_cpld_platform_data *pdata = dev_get_platdata(&client->dev);
	struct device_node *np = client->dev.of_node;

	if (!pdata) {
		if (np) {
			pdata = awusb24_cpld_of_populate_pdata(&client->dev, np);
			if (IS_ERR(pdata))
				return PTR_ERR(pdata);
		} else {
			dev_err(&client->dev, "no platform data\n");
			return -EINVAL;
		}
	}

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	led = devm_kzalloc(&client->dev,
			sizeof(*led) * pdata->num_channels, GFP_KERNEL);
	if (!led)
		return -ENOMEM;

	chip->cl = client;
	chip->pdata = pdata;
	chip->cfg = &awusb24_cpld_cfg;

	mutex_init(&chip->lock);

	i2c_set_clientdata(client, led);

	ret = awusb24_cpld_init_device(chip);
	if (ret)
		goto err_init;


	ret = awusb24_cpld_register_leds(led, chip);
	if (ret)
		goto err_register_leds;

	dev_info(&client->dev, "Found %d LEDs\n", chip->num_leds);

	ret = awusb24_cpld_register_sysfs(client);
	if (ret) {
		dev_err(&client->dev, "registering sysfs failed\n");
		goto err_register_sysfs;
	}

	return 0;

err_register_sysfs:
	awusb24_cpld_unregister_leds(led, chip);
err_register_leds:
	awusb24_cpld_deinit_device(chip);
err_init:
	return ret;
}

static int awusb24_cpld_remove(struct i2c_client *client)
{
	struct awusb24_cpld *led = i2c_get_clientdata(client);
	struct awusb24_cpld_chip *chip = led->chip;

	awusb24_cpld_unregister_sysfs(chip ? chip->cl : NULL);
	awusb24_cpld_unregister_leds(led, chip);
	awusb24_cpld_deinit_device(chip);

	return 0;
}

static struct i2c_driver awusb24_cpld_driver = {
	.probe		= awusb24_cpld_probe,
	.remove		= awusb24_cpld_remove,
	.id_table	= awusb24_cpld_id,
	.driver		= {
		.name		= DRIVER_NAME,
		.of_match_table = of_match_ptr(awusb24_cpld_of_match),
	},
};

module_i2c_driver(awusb24_cpld_driver);

MODULE_AUTHOR("Digi International, Inc");
MODULE_DESCRIPTION("CPLD driver for Digi AnywhereUSB 8/24");
MODULE_LICENSE("GPL");
