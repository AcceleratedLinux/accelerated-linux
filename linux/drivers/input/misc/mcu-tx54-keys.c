// SPDX-License-Identifier: GPL-2.0
/*
 *  Power keys (ignition, power fail and power down) driver for TX54 MCU
 *
 *  Copyright 2019 Digi International Inc.
 */

#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define DEV_NAME		"mcu-tx54-keys"

enum mcu_tx54_key_index {
	MCU_TX54_KEY_PWR_BTN = 0,
	MCU_TX54_KEY_IGN_PWDN,

	MCU_TX54_KEY_MAX
};

const char *key_irq_names[] = {
	"IRQ_PWR_BTN",
	"IRQ_IGN_PWDN",
};

struct mcu_tx54_key_info {
	struct mcu_tx54_keys		*keys;
	unsigned int			keycode;
	enum mcu_tx54_key_index		index;
};

struct mcu_tx54_keys {
	struct input_dev		*input_dev;
	struct device			*dev;
	struct mcu_tx54			*mcu;
	struct mcu_tx54_key_info	key[MCU_TX54_KEY_MAX];
};

static irqreturn_t mcu_tx54_keys_irq_handler_thread(int irq, void *data)
{
	struct mcu_tx54_key_info *info = data;
	struct mcu_tx54_keys *keys = info->keys;

	switch (info->index) {
	case MCU_TX54_KEY_PWR_BTN:
		dev_info(keys->dev, "Received power button event\n");
		break;

	case MCU_TX54_KEY_IGN_PWDN:
		dev_info(keys->dev, "Received ignition power down event\n");
		break;

	default:
		break;
	}

	/* Clear before set to ensure the event is generated */
	input_report_key(info->keys->input_dev, info->keycode, 0);
	input_report_key(info->keys->input_dev, info->keycode, 1);
	input_sync(info->keys->input_dev);

	return IRQ_HANDLED;
}

static int mcu_tx54_keys_get_inputs(struct mcu_tx54_keys *keys,
				    struct mcu_inputs *inputs)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_STATUS, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt, get_status);
	if (ret < 0) {
		dev_err(keys->dev, "Get status command failed (%d)\n", ret);
		return ret;
	}

	*inputs = rx_pkt.get_status.inputs;

	return 0;
}

static ssize_t ignition_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_inputs inputs;

	ret = mcu_tx54_keys_get_inputs(keys, &inputs);
	if (ret)
		return ret;

	return sprintf(buf, "%d\n", !!inputs.ign_sense);
}

static ssize_t pwr_button_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_inputs inputs;

	ret = mcu_tx54_keys_get_inputs(keys, &inputs);
	if (ret)
		return ret;

	return sprintf(buf, "%d\n", !!inputs.wake_btn);
}

static const char str_pwr_button_lock_short[] = "short";
static const char str_pwr_button_lock_long[]  = "long";

static ssize_t pwr_button_lock_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	union mcu_rx_get_pwr_b_lock *p = &rx_pkt.get_pwr_button_lock;
	bool append = false;

	put_unaligned(MCU_CMD_GET_PWR_BUTTON_LOCK, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   get_pwr_button_lock);
	if (ret < 0) {
		dev_err(keys->dev,
			"Get power button lock command failed (%d)\n", ret);
		return ret;
	}

	buf[0] = 0;

	if (p->short_press_disabled) {
		strcat(buf, str_pwr_button_lock_short);
		append = true;
	}

	if (p->long_press_disabled) {
		if (append)
			strcat(buf, " ");
		strcat(buf, str_pwr_button_lock_long);
	}

	strcat(buf, "\n");

	return strlen(buf);
}

static ssize_t pwr_button_lock_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	union mcu_tx_set_pwr_b_lock val;
	char *p;

	val.raw = 0;

	while ((p = strsep((char **)&buf, " \n")) != NULL && *p != 0) {
		if (strcmp(p, str_pwr_button_lock_short) == 0)
			val.short_press_disabled = 1;
		else if (strcmp(p, str_pwr_button_lock_long) == 0)
			val.long_press_disabled = 1;
		else {
			dev_err(dev, "Invalid flag '%s'\n", p);
			return -EINVAL;
		}
	}

	put_unaligned(MCU_CMD_SET_PWR_BUTTON_LOCK, &tx_pkt.cmd);
	put_unaligned(val.raw, &tx_pkt.set_pwr_button_lock.raw);

	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   set_pwr_button_lock);
	if (ret < 0) {
		dev_err(keys->dev,
			"Couldn't set power button lock (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t ign_delay_on_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_IGN_PWR_ON_DELAY, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   get_ign_pwr_on_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Get ignition ON delay command failed (%d)\n", ret);
		return ret;
	}

	return sprintf(buf, "%u\n",
		       get_unaligned(&rx_pkt.get_ign_pwr_on_delay.value));
}

static ssize_t ign_delay_on_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < 0 || val > MCU_IGN_PWR_ON_DELAY_MAX) {
		dev_err(keys->dev, "Invalid ignition ON delay (%ld) [0 - %d]\n",
			val, MCU_IGN_PWR_ON_DELAY_MAX);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_IGN_PWR_ON_DELAY, &tx_pkt.cmd);
	put_unaligned(val, &tx_pkt.set_ign_pwr_on_delay.param);

	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   set_ign_pwr_on_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Couldn't set ignition ON delay (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t ign_delay_off_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_IGN_PWR_OFF_DELAY, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   get_ign_pwr_off_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Get ignition OFF delay command failed (%d)\n", ret);
		return ret;
	}

	return sprintf(buf, "%u\n",
		       get_unaligned(&rx_pkt.get_ign_pwr_off_delay.value));
}

static ssize_t ign_delay_off_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < 0 || val > MCU_IGN_PWR_OFF_DELAY_MAX) {
		dev_err(keys->dev,
			"Invalid ignition OFF delay (%ld) [0 - %d]\n", val,
			MCU_IGN_PWR_OFF_DELAY_MAX);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_IGN_PWR_OFF_DELAY, &tx_pkt.cmd);
	put_unaligned(val, &tx_pkt.set_ign_pwr_off_delay.param);

	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   set_ign_pwr_off_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Couldn't set ignition OFF delay (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t temp_ign_delay_off_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_TEMP_IGN_PWR_OFF_DELAY, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   get_ign_pwr_off_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Get temp ignition OFF delay command failed (%d)\n", ret);
		return ret;
	}

	return sprintf(buf, "%u\n",
			   get_unaligned(&rx_pkt.get_ign_pwr_off_delay.value));
}

static ssize_t temp_ign_delay_off_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct mcu_tx54_keys *keys = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	if (val < 0 || val > MCU_IGN_PWR_OFF_DELAY_MAX) {
		dev_err(keys->dev,
			"Invalid temp ignition OFF delay (%ld) [0 - %d]\n", val,
			MCU_IGN_PWR_OFF_DELAY_MAX);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_TEMP_IGN_PWR_OFF_DELAY, &tx_pkt.cmd);
	put_unaligned(val, &tx_pkt.set_temp_ign_pwr_off_delay.param);

	ret = mcu_tx54_transaction(keys->mcu, &tx_pkt, &rx_pkt,
				   set_temp_ign_pwr_off_delay);
	if (ret < 0) {
		dev_err(keys->dev,
			"Couldn't set temp ignition OFF delay (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_RO(ignition);
static DEVICE_ATTR_RO(pwr_button);
static DEVICE_ATTR_RW(pwr_button_lock);
static DEVICE_ATTR_RW(ign_delay_on);
static DEVICE_ATTR_RW(ign_delay_off);
static DEVICE_ATTR_RW(temp_ign_delay_off);

static struct attribute *mcu_tx54_keys_attrs[] = {
	&dev_attr_ignition.attr,
	&dev_attr_pwr_button.attr,
	&dev_attr_pwr_button_lock.attr,
	&dev_attr_ign_delay_on.attr,
	&dev_attr_ign_delay_off.attr,
	&dev_attr_temp_ign_delay_off.attr,
	NULL
};

static struct attribute_group mcu_tx54_keys_attr_group = {
	.attrs = mcu_tx54_keys_attrs,
};

static int mcu_tx54_keys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mcu_tx54 *mcu = dev_get_drvdata(pdev->dev.parent);
	struct mcu_tx54_keys *keys;
	struct input_dev *input_dev;
	int ret;
	int i;

	keys = devm_kzalloc(dev, sizeof(*keys), GFP_KERNEL);
	if (!keys)
		return -ENOMEM;

	keys->dev = dev;
	keys->mcu = mcu;

	keys->input_dev = input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(dev, "Input device allocation failed\n");
		return -ENOMEM;
	}

	input_dev->name = DEV_NAME;
	input_dev->id.bustype = BUS_HOST;

	for (i = 0; i < MCU_TX54_KEY_MAX; i++) {
		int irq;
		struct mcu_tx54_key_info *key = &keys->key[i];

		irq = platform_get_irq_byname(pdev, key_irq_names[i]);
		if (irq < 0) {
			dev_err(dev, "No IRQ (%s) defined\n", key_irq_names[i]);
			ret = -EINVAL;
			goto free_input_dev;
		}

		key->keycode = KEY_POWER;
		key->keys = keys;

		ret = devm_request_threaded_irq(dev, irq, NULL,
			mcu_tx54_keys_irq_handler_thread,
			IRQF_ONESHOT | IRQF_TRIGGER_HIGH, DEV_NAME, key);
		if (ret) {
			dev_err(keys->dev, "Failed to request IRQ: %d: %d\n",
				irq, ret);
			goto free_input_dev;
		}

		input_set_capability(input_dev, EV_KEY, key->keycode);
	}

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(dev, "Couldn't register input device (%d)\n", ret);
		goto free_input_dev;
	}

	ret = sysfs_create_group(&dev->kobj, &mcu_tx54_keys_attr_group);
	if (ret) {
		dev_err(dev, "Failed to create sysfs entries (%d)\n", ret);
		goto unreg_input_dev;
	}

	platform_set_drvdata(pdev, keys);

	return 0;

unreg_input_dev:
	input_unregister_device(input_dev);
free_input_dev:
	input_free_device(input_dev);

	return ret;
}

static int mcu_tx54_keys_remove(struct platform_device *pdev)
{
	struct mcu_tx54_keys *keys = platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj, &mcu_tx54_keys_attr_group);
	input_unregister_device(keys->input_dev);

	return 0;
}

static const struct of_device_id mcu_tx54_keys_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54-keys", },
	{ /* sentinel */ }
};

static struct platform_driver mcu_tx54_keys_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = mcu_tx54_keys_dt_ids,
	},
	.probe = mcu_tx54_keys_probe,
	.remove = mcu_tx54_keys_remove,
};

static int mcu_tx54_keys_init(void)
{
	return platform_driver_register(&mcu_tx54_keys_driver);
}
subsys_initcall(mcu_tx54_keys_init);

static void mcu_tx54_keys_exit(void)
{
	platform_driver_unregister(&mcu_tx54_keys_driver);
}
module_exit(mcu_tx54_keys_exit);

MODULE_AUTHOR("Digi International Inc.");
MODULE_DESCRIPTION("Power keys driver for MCU of TX54");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DEV_NAME);
