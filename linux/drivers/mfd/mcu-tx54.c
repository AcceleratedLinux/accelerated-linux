// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2019 Digi International Inc.
 *
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mfd/core.h>
#include <linux/mfd/mcu-tx54/core.h>
#include <linux/of.h>

static const struct resource mcu_tx54_keys_resources[] = {
	DEFINE_RES_IRQ_NAMED(MCU_TX54_IRQ_PWR_BUTTON, "IRQ_PWR_BTN"),
	DEFINE_RES_IRQ_NAMED(MCU_TX54_IRQ_IGN_PWDN, "IRQ_IGN_PWDN"),
};

static const struct resource mcu_tx54_battery_resources[] = {
	DEFINE_RES_IRQ_NAMED(MCU_TX54_IRQ_PWR_FAIL, "IRQ_PWR_FAIL"),
};

static const struct mfd_cell mcu_tx54_subdevs[] = {
	{.name = "mcu-tx54-led",},
	{.name = "mcu-tx54-poweroff",},
	{
	 .name = "mcu-tx54-keys",
	 .resources = mcu_tx54_keys_resources,
	 .num_resources = ARRAY_SIZE(mcu_tx54_keys_resources),
	},
	{
	 .name = "mcu-tx54-battery",
	 .resources = mcu_tx54_battery_resources,
	 .num_resources = ARRAY_SIZE(mcu_tx54_battery_resources),
	},
};

struct mcu_tx54_err_type {
	const char	*str;
	int		errno;
};

static const struct mcu_tx54_err_type mcu_tx54_err_types[] = {
	{
		.str   = "I2C error",
		.errno = -EIO,
	},
	{
		.str   = "Success",
		.errno = 0,
	},
	{
		.str   = "Busy",
		.errno = -EBUSY,
	},
	{
		.str   = "Invalid packet",
		.errno = -EBADMSG,
	},
	{
		.str   = "Parameter error",
		.errno = -EINVAL,
	},
	{
		.str   = "Command not found",
		.errno = -EOPNOTSUPP,
	},
};
static const struct mcu_tx54_err_type mcu_tx54_err_type_unknown = {
	.str   = "Unknown error",
	.errno = -EPROTO,
};

static const struct mcu_tx54_err_type *mcu_tx54_status2str(uint8_t status)
{
	if (status >= ARRAY_SIZE(mcu_tx54_err_types))
		return &mcu_tx54_err_type_unknown;

	return &mcu_tx54_err_types[status];
}

int mcu_tx54_transaction_(struct mcu_tx54 *mcu, struct mcu_tx_pkt *tx,
			  unsigned int tx_len, struct mcu_rx_pkt *rx,
			  unsigned int rx_len)
{
	struct i2c_client *client = to_i2c_client(mcu->dev);
	int retry = 10;
	int ret;

	/*
	 * If we are in power-off-only mode, don't allow any other command go
	 * out
	 */
	if (mcu->pwroff_only) {
		typeof(tx->cmd) cmd = get_unaligned(&tx->cmd);

		if (cmd != MCU_CMD_SET_POWER) {
			dev_dbg(mcu->dev, "only power off command allowed\n");
			return -EPERM;
		}
	}

	/* Set packet length */
	tx->len = tx_len;

	ret = mutex_lock_interruptible(&mcu->lock);
	if (ret)
		return ret;

	ret = i2c_master_send(client, (const char *)tx, tx_len);
	if (ret != tx_len) {
		dev_err(mcu->dev, "couldn't send command (%d)\n", ret);
		ret = -EIO;
		goto fail;
	}

	usleep_range(1000, 2000);

	while (1) {
		ret = i2c_master_recv(client, (char *)rx, rx_len);
		if (ret != rx_len) {
			dev_err(mcu->dev, "couldn't read response (%d)\n", ret);
			ret = -EIO;
			goto fail;
		}

		if (rx->status == MCU_STATUS_SUCCESS)
			break;
		else if (rx->status == MCU_STATUS_BUSY) {
			/* Busy */
			if (retry-- > 0) {
				dev_dbg(mcu->dev, "busy, trying again...\n");
				usleep_range(10000, 12000);
			} else {
				dev_err(mcu->dev, "failed with 'busy'\n");
				ret = -EBUSY;
				goto fail;
			}
		} else {
			const struct mcu_tx54_err_type *err;

			/* ERROR */
			err = mcu_tx54_status2str(rx->status);

			dev_err(mcu->dev, "failed with '%s'", err->str);
			ret = err->errno;

			goto fail;
		}
	}

	ret = 0;
fail:
	mutex_unlock(&mcu->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(mcu_tx54_transaction_);

static int mcu_tx54_get_fw_version(struct mcu_tx54 *mcu, char *version)
{
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int ret;

	put_unaligned(MCU_CMD_GET_VERSION, &tx_pkt.cmd);

	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, get_version);
	if (ret < 0) {
		dev_err(mcu->dev, "Couldn't get version\n");
		return ret;
	}

	snprintf(version, MCU_VERSION_LEN, "%s", rx_pkt.get_version.version);

	return 0;
}

static int mcu_tx54_get_events(struct mcu_tx54 *mcu,
			       union mcu_rx_get_events *events)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_EVENTS, &tx_pkt.cmd);

	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, get_events);
	if (ret < 0) {
		dev_err(mcu->dev, "Couldn't get events\n");
		return ret;
	}

	memcpy(events, &rx_pkt.get_events, sizeof(*events));

	return 0;
}

static void mcu_tx54_irq_lock(struct irq_data *data)
{
	struct mcu_tx54 *mcu = irq_data_get_irq_chip_data(data);

	mutex_lock(&mcu->irq_lock);
}

static void mcu_tx54_irq_sync_unlock(struct irq_data *data)
{
	struct mcu_tx54 *mcu = irq_data_get_irq_chip_data(data);

	mutex_unlock(&mcu->irq_lock);
}

static void mcu_tx54_irq_enable(struct irq_data *data)
{
	struct mcu_tx54 *mcu = irq_data_get_irq_chip_data(data);

	mcu->irq_en |= BIT(data->hwirq);
}

static void mcu_tx54_irq_disable(struct irq_data *data)
{
	struct mcu_tx54 *mcu = irq_data_get_irq_chip_data(data);

	mcu->irq_en &= ~(BIT(data->hwirq));
}

static struct irq_chip mcu_tx54_irq_chip = {
	.name			= "mcu-tx54",
	.irq_enable		= mcu_tx54_irq_enable,
	.irq_disable		= mcu_tx54_irq_disable,
	.irq_bus_lock		= mcu_tx54_irq_lock,
	.irq_bus_sync_unlock	= mcu_tx54_irq_sync_unlock,
};

static irqreturn_t mcu_tx54_irq_thread(int irq, void *data)
{
	struct mcu_tx54 *mcu = data;
	int ret;
	union mcu_rx_get_events events;
	uint32_t e;
	bool handled = false;

	/* Read event */
	ret = mcu_tx54_get_events(mcu, &events);
	if (ret < 0)
		return IRQ_NONE;

	e = events.events;
	while (e) {
		int i = __ffs(e);

		if (mcu->irq_en & BIT(i)) {
			handle_nested_irq(irq_find_mapping(mcu->irq_domain, i));
			handled = true;
		}

		e &= ~(1 << i);
	}

	return handled ? IRQ_HANDLED : IRQ_NONE;
}

static int mcu_tx54_irq_map(struct irq_domain *d, unsigned int irq,
			    irq_hw_number_t hw)
{
	struct mcu_tx54 *mcu = d->host_data;

	irq_set_chip_data(irq, mcu);
	irq_set_chip_and_handler(irq, &mcu_tx54_irq_chip, handle_edge_irq);
	irq_set_nested_thread(irq, 1);
	irq_set_parent(irq, mcu->irq);
	irq_set_noprobe(irq);

	return 0;
}

static const struct irq_domain_ops mcu_tx54_irq_domain_ops = {
	.map = mcu_tx54_irq_map,
	.xlate	= irq_domain_xlate_onecell,
};

static int mcu_tx54_irq_init(struct mcu_tx54 *mcu, unsigned int irq)
{
	int ret;

	mutex_init(&mcu->irq_lock);

	mcu->irq = irq;
	mcu->irq_en = 0;

	mcu->irq_domain = irq_domain_add_linear(mcu->dev->of_node,
		MCU_TX54_IRQ_NR, &mcu_tx54_irq_domain_ops, mcu);
	if (!mcu->irq_domain) {
		dev_err(mcu->dev, "Couldn't create IRQ domain\n");
		return -ENOMEM;
	}

	ret = devm_request_threaded_irq(mcu->dev, irq, NULL,
		mcu_tx54_irq_thread, IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
		"mcu-tx54", mcu);
	if (ret) {
		dev_err(mcu->dev, "Unable to request IRQ\n");
		return ret;
	}

	return 0;
}

/* sysfs attributes */
static ssize_t fw_version_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(dev);
	int ret;

	ret = mcu_tx54_get_fw_version(mcu, buf);
	if (ret < 0)
		return ret;

	return strlen(buf);
}

static DEVICE_ATTR_RO(fw_version);

static ssize_t wake_src_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	enum mcu_wakeup_src wake_src;
	static const char * const wake_src_str[] = {
		"none",		/* MCU_WAKEUP_SRC_NONE */
		"ignition",	/* MCU_WAKEUP_SRC_IGNITION */
		"button",	/* MCU_WAKEUP_SRC_BUTTON */
		"acc",		/* MCU_WAKEUP_SRC_ACC */
		"rtc",		/* MCU_WAKEUP_SRC_RTC */

		"Unknown"	/* MCU_WAKEUP_SRC_MAX */
	};

	put_unaligned(MCU_CMD_GET_WAKEUP_SRC, &tx_pkt.cmd);
	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, get_wakeup_src);
	if (ret < 0) {
		dev_err(dev, "Get wakeup source command failed (%d)\n", ret);
		return ret;
	}

	wake_src = get_unaligned(&rx_pkt.get_wakeup_src.wakeup_src);
	if (wake_src >= MCU_WAKEUP_SRC_MAX)
		wake_src = MCU_WAKEUP_SRC_MAX;

	return scnprintf(buf, PAGE_SIZE, "%s\n", wake_src_str[wake_src]);
}

static DEVICE_ATTR_RO(wake_src);

static ssize_t mcu_int_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	long val;

	if (kstrtol(buf, 10, &val) < 0)
		return -EINVAL;

	put_unaligned(MCU_CMD_SET_MCU_INT, &tx_pkt.cmd);
	put_unaligned(!!val, &tx_pkt.set_mcu_int.param);

	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, set_mcu_int);
	if (ret < 0) {
		dev_err(dev, "Couldn't set interrupt pin (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_WO(mcu_int);

static const char str_enable[] = "enable";

static ssize_t bloader_mode_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_enable, sizeof(str_enable) - 1) != 0) {
		dev_err(dev,
			"Should write 'enable' to enter bootloader-mode\n");
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_TO_BLOADER, &tx_pkt.cmd);
	put_unaligned(MCU_BLOADER_MAGIC_KEY, &tx_pkt.to_bloader.param);

	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, to_bloader);
	if (ret < 0) {
		dev_err(dev, "Couldn't set bootloader-mode (%d)\n", ret);
		return ret;
	}

	/*
	 * Disable interrupts. In the bootloader, the IRQ line is high, so the
	 * level-sensitive interrupt while be triggered endlessly. We don't need
	 * it anyway, as the MCU won't be able to go back to application-mode
	 * until a reset
	 */
	disable_irq(mcu->irq);

	mcu->pwroff_only = true;

	return count;
}

static DEVICE_ATTR_WO(bloader_mode);

static ssize_t factory_default_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcu_tx54 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_enable, sizeof(str_enable) - 1) != 0) {
		dev_err(dev,
			"Should write 'enable' to restore factory defaults\n");
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_FACTORY_DEFAULTS, &tx_pkt.cmd);
	put_unaligned(MCU_FACTORY_DEF_MAGIC_KEY,
		      &tx_pkt.set_factory_defaults.param);

	ret = mcu_tx54_transaction(mcu, &tx_pkt, &rx_pkt, set_factory_defaults);
	if (ret < 0) {
		dev_err(dev, "Couldn't restore factory defaults (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_WO(factory_default);

static struct attribute *mcu_tx54_sysfs_entries[] = {
	&dev_attr_fw_version.attr,
	&dev_attr_wake_src.attr,
	&dev_attr_mcu_int.attr,
	&dev_attr_bloader_mode.attr,
	&dev_attr_factory_default.attr,
	NULL,
};

static struct attribute_group mcu_tx54_attr_group = {
	.name	= NULL,			/* put in device directory */
	.attrs	= mcu_tx54_sysfs_entries,
};

static int mcu_tx54_device_init(struct mcu_tx54 *mcu)
{
	int ret;

	ret = devm_mfd_add_devices(mcu->dev, PLATFORM_DEVID_NONE,
				   mcu_tx54_subdevs,
				   ARRAY_SIZE(mcu_tx54_subdevs), NULL, 0,
				   mcu->irq_domain);
	if (ret) {
		dev_err(mcu->dev, "Couldn't add mfd devices (%d)\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&mcu->dev->kobj, &mcu_tx54_attr_group);
	if (ret) {
		dev_err(mcu->dev, "Couldn't create sysfs entries (%d)\n", ret);
		return ret;
	}

	return 0;
}

static void mcu_tx54_device_exit(struct mcu_tx54 *mcu)
{
	sysfs_remove_group(&mcu->dev->kobj, &mcu_tx54_attr_group);
}

static int mcu_tx54_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	int ret;
	struct mcu_tx54 *mcu;
	int retry;
	char fw_version[MCU_VERSION_LEN];

	mcu = devm_kzalloc(&i2c->dev, sizeof(struct mcu_tx54), GFP_KERNEL);
	if (mcu == NULL)
		return -ENOMEM;

	mcu->dev = &i2c->dev;

	if (!i2c->irq) {
		dev_err(&i2c->dev, "IRQ number not specified\n");
		return -EINVAL;
	}

	ret = mcu_tx54_irq_init(mcu, i2c->irq);
	if (ret)
		return ret;

	mutex_init(&mcu->lock);

	i2c_set_clientdata(i2c, mcu);

	/* Read out FW version */
	retry = 3;
	do {
		ret = mcu_tx54_get_fw_version(mcu, fw_version);
	} while (ret && --retry > 0);

	if (ret < 0) {
		dev_err(&i2c->dev,
			"Communication error on device probing. Disabling interrupts\n");
		disable_irq(mcu->irq);

		/* Load the driver in power-off only mode anyway, otherwise if
		 * MCU stuck in the bootloader, we won't be able to power off
		 * the board */
		mcu->pwroff_only = true;
	}

	return mcu_tx54_device_init(mcu);
}

static int mcu_tx54_i2c_remove(struct i2c_client *i2c)
{
	struct mcu_tx54 *mcu = i2c_get_clientdata(i2c);
	int i;
	unsigned int irq;

	mcu_tx54_device_exit(mcu);

	for (i = 0; i < MCU_TX54_IRQ_NR; i++) {
		irq = irq_find_mapping(mcu->irq_domain, i);
		if (irq)
			irq_dispose_mapping(irq);
	}

	irq_domain_remove(mcu->irq_domain);
	mcu->irq_domain = NULL;

	return 0;
}

static const struct i2c_device_id mcu_tx54_i2c_id[] = {
	{"mcu_tx54", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, mcu_tx54_i2c_id);

static const struct of_device_id mcu_tx54_dt_ids[] = {
	{ .compatible = "digi,mcu-tx54", },
	{ }
};
MODULE_DEVICE_TABLE(of, mcu_tx54_dt_ids);

static struct i2c_driver mcu_tx54_i2c_driver = {
	.driver = {
		.name = "mcu_tx54",
		.of_match_table = of_match_ptr(mcu_tx54_dt_ids),
	},
	.probe    = mcu_tx54_i2c_probe,
	.remove   = mcu_tx54_i2c_remove,
	.id_table = mcu_tx54_i2c_id,
};

module_i2c_driver(mcu_tx54_i2c_driver);

MODULE_AUTHOR("Digi International Inc");
MODULE_DESCRIPTION("MCU driver for TX54");
MODULE_LICENSE("GPL v2");
