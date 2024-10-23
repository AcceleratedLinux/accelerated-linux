// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright 2023 Digi International Inc.
 *
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/mcu_tx40.h>
#include <linux/mfd/core.h>
#include <linux/mfd/mcu-tx40/core.h>
#include <linux/module.h>
#include <linux/of.h>

#define TRANSACTION_TIMEOUT_MSEC	50

static const struct resource mcu_tx40_keys_resources[] = {
	DEFINE_RES_IRQ_NAMED(MCU_TX40_IRQ_IGN_PWDN, "IRQ_IGN_PWDN"),
	DEFINE_RES_IRQ_NAMED(MCU_TX40_IRQ_VIN_LOW, "IRQ_VIN_LOW"),
	DEFINE_RES_IRQ_NAMED(MCU_TX40_IRQ_VIN_CRITICAL, "IRQ_VIN_CRITICAL"),
};

static const struct mfd_cell mcu_tx40_subdevs[] = {
	{
	 .name = "mcu-tx40-led",
	},
	{
	 .name = "mcu-tx40-keys",
	 .resources = mcu_tx40_keys_resources,
	 .num_resources = ARRAY_SIZE(mcu_tx40_keys_resources),
	},
	{
	 .name = "mcu-tx40-poweroff",
	},
	{
	 .name = "mcu-tx40-battery",
	},
	{
	 .name = "mcu-tx40-wdt",
	},
};

struct mcu_tx40_err_type {
	const char	*str;
	int		errno;
};

static const struct mcu_tx40_err_type mcu_tx40_err_types[] = {
	{
		.str   = "Success",
		.errno = 0,
	},
	{
		.str   = "Invalid length",
		.errno = -EBADMSG,
	},
	{
		.str   = "Invalid command",
		.errno = -EOPNOTSUPP,
	},
	{
		.str   = "Invalid magic",
		.errno = -EINVAL,
	},
	{
		.str   = "Invalid value",
		.errno = -EINVAL,
	},
	{
		.str   = "Overrun",
		.errno = -E2BIG,
	},
	{
		.str   = "Invalid address",
		.errno = -EFAULT,
	},
	{
		.str   = "Invalid checksum",
		.errno = -EINVAL,
	}
};
static_assert(ARRAY_SIZE(mcu_tx40_err_types) == MCU_STATUS_LAST);

static const struct mcu_tx40_err_type mcu_tx40_err_type_unknown = {
	.str   = "Unknown error",
	.errno = -EPROTO,
};

static const struct mcu_tx40_err_type *mcu_tx40_status2str(u8 status)
{
	if (status >= ARRAY_SIZE(mcu_tx40_err_types))
		return &mcu_tx40_err_type_unknown;

	return &mcu_tx40_err_types[status];
}

static int mcu_tx40_transaction_unlocked(struct mcu_tx40 *mcu,
					 struct mcu_tx_pkt *tx,
					 unsigned int tx_len,
					 struct mcu_rx_pkt *rx,
					 unsigned int rx_len)
{
	struct i2c_client *client = to_i2c_client(mcu->dev);
	int ret;
	unsigned long timeout;

	/* Set packet length */
	tx->length = tx_len;

	ret = i2c_master_send(client, (const char *)tx, tx_len);
	if (ret != tx_len) {
		dev_err(mcu->dev, "couldn't send command (%d)\n", ret);
		ret = -EIO;
		goto fail;
	}

	timeout = jiffies + msecs_to_jiffies(TRANSACTION_TIMEOUT_MSEC);
	usleep_range(1000, 2000);

	do {
		ret = i2c_master_recv(client, (char *)rx, rx_len);
		if (ret == -ENXIO)
			/* NAK, try again */
			continue;

		/* Min. response length is response byte + status byte */
		if (ret < 2) {
			dev_err(mcu->dev, "couldn't read response (%d)\n", ret);
			ret = -EIO;
			goto fail;
		}

		if (ret != rx->length) {
			dev_err(mcu->dev,
				"invalid response length (%d != %d)\n", ret,
				rx->length);
			ret = -EIO;
			goto fail;
		}

		if (rx->status == MCU_STATUS_OK) {
			break;
		} else {
			const struct mcu_tx40_err_type *err;

			err = mcu_tx40_status2str(rx->status);

			dev_err(mcu->dev, "failed with '%s'\n", err->str);
			ret = err->errno;

			goto fail;
		}
	} while (time_before(jiffies, timeout));

	if (ret >= 0) {
		ret = 0;
	} else {
		ret = -EIO;
		dev_err(mcu->dev, "no reply\n");
	}

fail:
	return ret;
}

int mcu_tx40_transaction_(struct mcu_tx40 *mcu, struct mcu_tx_pkt *tx,
			  unsigned int tx_len, struct mcu_rx_pkt *rx,
			  unsigned int rx_len)
{
	int ret;

	ret = mutex_lock_interruptible(&mcu->lock);
	if (ret)
		return ret;

	ret = mcu_tx40_transaction_unlocked(mcu, tx, tx_len, rx, rx_len);

	mutex_unlock(&mcu->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(mcu_tx40_transaction_);

static int mcu_tx40_get_fw_version(struct mcu_tx40 *mcu, u16 *version)
{
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	int ret;

	put_unaligned(MCU_CMD_GET_VERSION, &tx_pkt.cmd);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_version);
	if (ret) {
		dev_err(mcu->dev, "Couldn't get version\n");
		return ret;
	}

	*version = rx_pkt.get_version.version;

	return 0;
}

static int mcu_tx40_get_events(struct mcu_tx40 *mcu, u16 *events)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_GET_EVENTS, &tx_pkt.cmd);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_events);
	if (ret) {
		dev_err(mcu->dev, "Couldn't get events\n");
		return ret;
	}

	*events = get_unaligned(&rx_pkt.get_events.events);

	return 0;
}

static int mcu_tx40_clear_events(struct mcu_tx40 *mcu)
{
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	put_unaligned(MCU_CMD_CLEAR_EVENTS, &tx_pkt.cmd);
	put_unaligned(0xFFFF, &tx_pkt.clear_events.events);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, clear_events);
	if (ret) {
		dev_err(mcu->dev, "Couldn't clear events\n");
		return ret;
	}

	return 0;
}

static void mcu_tx40_irq_lock(struct irq_data *data)
{
	struct mcu_tx40 *mcu = irq_data_get_irq_chip_data(data);

	mutex_lock(&mcu->irq_lock);
}

static void mcu_tx40_irq_sync_unlock(struct irq_data *data)
{
	struct mcu_tx40 *mcu = irq_data_get_irq_chip_data(data);

	mutex_unlock(&mcu->irq_lock);
}

static void mcu_tx40_irq_enable(struct irq_data *data)
{
	struct mcu_tx40 *mcu = irq_data_get_irq_chip_data(data);

	mcu->irq_en |= BIT(data->hwirq);
}

static void mcu_tx40_irq_disable(struct irq_data *data)
{
	struct mcu_tx40 *mcu = irq_data_get_irq_chip_data(data);

	mcu->irq_en &= ~(BIT(data->hwirq));
}

static struct irq_chip mcu_tx40_irq_chip = {
	.name			= "mcu-tx40",
	.irq_enable		= mcu_tx40_irq_enable,
	.irq_disable		= mcu_tx40_irq_disable,
	.irq_bus_lock		= mcu_tx40_irq_lock,
	.irq_bus_sync_unlock	= mcu_tx40_irq_sync_unlock,
};

static irqreturn_t mcu_tx40_irq_thread(int irq, void *data)
{
	struct mcu_tx40 *mcu = data;
	int ret;
	u16 events;
	bool handled = false;

	/* Read event */
	ret = mcu_tx40_get_events(mcu, &events);
	if (ret < 0)
		return IRQ_NONE;

	if (events & MCU_EVENT_TEST_EVENT) {
		dev_info(mcu->dev, "TEST EVENT\n");
		events &= ~MCU_EVENT_TEST_EVENT;
		handled = true;
	}

	while (events) {
		int i = __ffs(events);

		if (mcu->irq_en & BIT(i)) {
			handle_nested_irq(irq_find_mapping(mcu->irq_domain, i));
			handled = true;
		}

		events &= ~(1 << i);
	}

	mcu_tx40_clear_events(mcu);

	return handled ? IRQ_HANDLED : IRQ_NONE;
}

static int mcu_tx40_irq_map(struct irq_domain *d, unsigned int irq,
			    irq_hw_number_t hw)
{
	struct mcu_tx40 *mcu = d->host_data;

	irq_set_chip_data(irq, mcu);
	irq_set_chip_and_handler(irq, &mcu_tx40_irq_chip, handle_edge_irq);
	irq_set_nested_thread(irq, 1);
	irq_set_parent(irq, mcu->irq);
	irq_set_noprobe(irq);

	return 0;
}

static const struct irq_domain_ops mcu_tx40_irq_domain_ops = {
	.map = mcu_tx40_irq_map,
	.xlate	= irq_domain_xlate_onecell,
};

static int mcu_tx40_irq_init(struct mcu_tx40 *mcu, unsigned int irq)
{
	int ret;

	mutex_init(&mcu->irq_lock);

	mcu->irq = irq;
	mcu->irq_en = 0;

	mcu->irq_domain = irq_domain_add_linear(mcu->dev->of_node,
		MCU_TX40_IRQ_NR, &mcu_tx40_irq_domain_ops, mcu);
	if (!mcu->irq_domain) {
		dev_err(mcu->dev, "Couldn't create IRQ domain\n");
		return -ENOMEM;
	}

	/*
	 * Level-triggering is not supported on LS1028A. The MCU will
	 * periodically create an interrupt pulse if event is not cleared.
	 * Don't enable it yet
	 */
	irq_set_status_flags(irq, IRQ_NOAUTOEN);
	ret = devm_request_threaded_irq(mcu->dev, irq, NULL,
		mcu_tx40_irq_thread, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"mcu-tx40", mcu);
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
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	u16 version;

	ret = mcu_tx40_get_fw_version(mcu, &version);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%u.%u", version >> 8, version & 0xFF);
}

static DEVICE_ATTR_RO(fw_version);

static ssize_t bloader_version_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	u16 version;

	put_unaligned(MCU_CMD_GET_BLOADER_VERSION, &tx_pkt.cmd);
	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_bloader_version);
	if (ret < 0) {
		dev_err(dev, "Get bootloader version command failed (%d)\n",
			ret);
		return ret;
	}

	version = get_unaligned(&rx_pkt.get_bloader_version.version);

	return sprintf(buf, "%u.%u", version >> 8, version & 0xFF);
}

static DEVICE_ATTR_RO(bloader_version);

static ssize_t wake_src_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	enum mcu_wakeup_src wake_src;
	static const char * const wake_src_str[] = {
		"none",		/* MCU_WAKEUP_SRC_NONE */
		"ignition",	/* MCU_WAKEUP_SRC_IGNITION */
		"rtc",		/* MCU_WAKEUP_SRC_RTC */
		"acc",		/* MCU_WAKEUP_SRC_ACCELEROMETER */
		"soft_reboot",	/* MCU_WAKEUP_SRC_SOFT_REBOOT */
		"watchdog",	/* MCU_WAKEUP_SRC_WDT */

		"unknown"	/* MCU_WAKEUP_SRC_MAX */
	};

	put_unaligned(MCU_CMD_GET_WAKEUP_SRC, &tx_pkt.cmd);
	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_wakeup_src);
	if (ret < 0) {
		dev_err(dev, "Get wakeup source command failed (%d)\n", ret);
		return ret;
	}

	wake_src = get_unaligned(&rx_pkt.get_wakeup_src.src);
	if (wake_src >= MCU_WAKEUP_SRC_MAX)
		wake_src = MCU_WAKEUP_SRC_MAX;

	return scnprintf(buf, PAGE_SIZE, "%s\n", wake_src_str[wake_src]);
}

static DEVICE_ATTR_RO(wake_src);

static ssize_t test_event_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	static const char str_trigger[] = "trigger";
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_trigger, sizeof(str_trigger) - 1) != 0) {
		dev_err(dev, "Should write '%s' to trigger test event\n",
			str_trigger);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_TEST_EVENT, &tx_pkt.cmd);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, test_event);
	if (ret < 0) {
		dev_err(dev, "Couldn't trigger test event (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_WO(test_event);

static const char str_enable[] = "enable";
static const char empty[] = "";

static ssize_t bloader_mode_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_enable, sizeof(str_enable) - 1) != 0) {
		dev_err(dev, "Should write '%s' to enter bootloader-mode\n",
			str_enable);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_TO_BLOADER, &tx_pkt.cmd);
	put_unaligned(MCU_MAGIC_TO_BLOADER, &tx_pkt.to_bloader.magic);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, to_bloader);
	if (ret < 0) {
		dev_err(dev, "Couldn't set bootloader-mode (%d)\n", ret);
		return ret;
	}

	disable_irq(mcu->irq);

	return count;
}

static DEVICE_ATTR_WO(bloader_mode);

static ssize_t factory_default_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_enable, sizeof(str_enable) - 1) != 0) {
		dev_err(dev, "Should write '%s' to restore factory defaults\n",
			str_enable);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_FACTORY_RESET, &tx_pkt.cmd);
	put_unaligned(MCU_FACTORY_RESET_MAGIC, &tx_pkt.factory_reset.magic);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, factory_reset);
	if (ret < 0) {
		dev_err(dev, "Couldn't restore factory defaults (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_WO(factory_default);

static ssize_t ios_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	struct mcu_pkt_io_info *info;

	put_unaligned(MCU_CMD_GET_IO_INFO, &tx_pkt.cmd);
	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_io_info);
	if (ret < 0) {
		dev_err(dev, "Get IO info command failed (%d)\n", ret);
		return ret;
	}

	info = &rx_pkt.get_io_info;

	return scnprintf(buf, PAGE_SIZE, "%s%s%s%s%s\n",
			 info->rtc_int ? "RTC_INT " : empty,
			 info->acc_int ? "ACC_INT " : empty,
			 info->ign_sense ? "IGN " : empty,
			 info->mcu_int_state ? "MCU_INT " : empty,
			 info->wd_trig ? "WD_TRIG " : empty);
}

static DEVICE_ATTR_RO(ios);

static ssize_t cold_reboot_flag_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;

	if (strncmp(buf, str_enable, sizeof(str_enable) - 1) != 0) {
		dev_err(dev,
			"Should write '%s' to enter set cold reboot flag\n",
			str_enable);
		return -EINVAL;
	}

	put_unaligned(MCU_CMD_SET_COLD_REBOOT, &tx_pkt.cmd);
	put_unaligned(MCU_MAGIC_SET_COLD_REBOOT, &tx_pkt.set_cold_reboot.magic);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, set_cold_reboot);
	if (ret < 0) {
		dev_err(dev, "Couldn't set cold reboot flag (%d)\n", ret);
		return ret;
	}

	return count;
}

static DEVICE_ATTR_WO(cold_reboot_flag);

static ssize_t pwr_led_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	size_t i;

	tx_pkt.set_pwr_led_color.raw = 0;
	for (i = 0; i < count && buf[i] != '\0'; i++) {
		switch (buf[i]) {
		case 'r':
		case 'R':
			tx_pkt.set_pwr_led_color.r = 1;
			break;
		case 'g':
		case 'G':
			tx_pkt.set_pwr_led_color.g = 1;
			break;
		case 'b':
		case 'B':
			tx_pkt.set_pwr_led_color.b = 1;
			break;
		case '\n':
		case ' ':
		case ',':
			break;
		default:
			dev_err(dev, "Invalid color, should be r, g, or b\n");
			return -EINVAL;
		}
	}

	put_unaligned(MCU_CMD_SET_PWR_LED_COLOR, &tx_pkt.cmd);

	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, set_pwr_led_color);
	if (ret < 0) {
		dev_err(dev, "Failed to set power LED color (%d)\n", ret);
		return ret;
	}

	return count;
}

static ssize_t pwr_led_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct mcu_tx40 *mcu = dev_get_drvdata(dev);
	int ret;
	struct mcu_tx_pkt tx_pkt;
	struct mcu_rx_pkt rx_pkt;
	union mcu_pkt_color *color;

	put_unaligned(MCU_CMD_GET_PWR_LED_COLOR, &tx_pkt.cmd);
	ret = mcu_tx40_transaction(mcu, &tx_pkt, &rx_pkt, get_pwr_led_color);
	if (ret < 0) {
		dev_err(dev, "Failed to get power LED color (%d)\n", ret);
		return ret;
	}

	color = &rx_pkt.get_pwr_led_color;

	return scnprintf(buf, PAGE_SIZE, "%s%s%s\n",
			 color->r ? "r" : empty,
			 color->g ? "g" : empty,
			 color->b ? "b" : empty);
}

static DEVICE_ATTR_RW(pwr_led);

static struct attribute *mcu_tx40_sysfs_entries[] = {
	&dev_attr_fw_version.attr,
	&dev_attr_bloader_version.attr,
	&dev_attr_wake_src.attr,
	&dev_attr_test_event.attr,
	&dev_attr_bloader_mode.attr,
	&dev_attr_factory_default.attr,
	&dev_attr_ios.attr,
	&dev_attr_cold_reboot_flag.attr,
	&dev_attr_pwr_led.attr,
	NULL,
};

static struct attribute_group mcu_tx40_attr_group = {
	.name	= NULL,			/* put in device directory */
	.attrs	= mcu_tx40_sysfs_entries,
};

static int mcu_tx40_open(struct inode *inode, struct file *filep)
{
	struct miscdevice *misc = filep->private_data;
	struct mcu_tx40 *mcu = container_of(misc, struct mcu_tx40, miscdev);
	int ret;

	ret = mutex_lock_interruptible(&mcu->lock);
	if (ret)
		return ret;

	filep->private_data = mcu;
	filep->f_pos = 0;

	return 0;
}

static int mcu_tx40_release(struct inode *inode, struct file *filep)
{
	struct mcu_tx40 *mcu = filep->private_data;

	mutex_unlock(&mcu->lock);

	return 0;
}

static int validate_write_size(const size_t count)
{
	const int MIN_SIZE = 2;
	/* Header and CRC occupy 4 bytes and the length is a one byte
	   value */
	const int MAX_SIZE = MCU_TX_MAX_SIZE;
	int ret = -EMSGSIZE;

	if (count <= MAX_SIZE && count >= MIN_SIZE)
		ret = 0;

	return ret;

}

static int validate_read_size(const size_t count, const size_t expected_count)
{
	int ret;

	if (count != 0) {
		ret = validate_write_size(count);
		if (ret)
			return ret;

		if (expected_count > count)
			return ret;
	}

	if (expected_count > MCU_RX_MAX_SIZE)
		return ret;

	return 0;
}

static long mcu_tx40_ioctl_exec(struct file *filep, unsigned int nmsgs,
				struct mcu_tx40_msg *msgs)
{
	struct mcu_tx40 *mcu = filep->private_data;
	struct mcu_tx_pkt **tx_bufs = NULL;
	struct mcu_rx_pkt *rx_bufs = NULL;
	int ret;
	int i;

	tx_bufs = kcalloc(nmsgs, sizeof(*tx_bufs), GFP_KERNEL);
	if (tx_bufs == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	rx_bufs = kcalloc(nmsgs, sizeof(*rx_bufs), GFP_KERNEL);
	if (rx_bufs == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < nmsgs; i++) {
		struct mcu_tx40_msg *msg = &msgs[i];

		if (msg->tx == NULL) {
			ret = -EINVAL;
			goto err;
		}

		ret = validate_write_size(msg->tx_len);
		if (ret)
			goto err;

		ret = validate_read_size(msg->rx_len, msgs->expected_rx_len);
		if (ret)
			goto err;

		tx_bufs[i] = kmalloc(msg->tx_len, GFP_KERNEL);
		if (!tx_bufs[i]) {
			ret = -ENOMEM;
			goto err;
		}

		if (copy_from_user(tx_bufs[i], msg->tx, msg->tx_len)) {
			ret = -EFAULT;
			goto err;
		}
	}

	for (i = 0; i < nmsgs; i++) {
		ret = mcu_tx40_transaction_unlocked(mcu, tx_bufs[i],
						   msgs[i].tx_len, &rx_bufs[i],
						   msgs[i].expected_rx_len);
		if (ret != 0)
			goto err;
	}

	for (i = 0; i < nmsgs; i++) {
		struct mcu_rx_pkt *rx = &rx_bufs[i];
		struct mcu_tx40_msg *msg = &msgs[i];

		if (msg->rx == NULL)
			continue;

		if (rx->length == 0) {
			((struct mcu_rx_pkt *)msg->rx)->length = 0;
			continue;
		}

		if (copy_to_user(msg->rx, rx, rx->length)) {
			ret = -EFAULT;
			goto err;
		}
	}

	ret = 0;
err:
	if (tx_bufs) {
		for (i = 0; i < nmsgs; i++)
			kfree(tx_bufs[i]);
		kfree(tx_bufs);
	}
	kfree(rx_bufs);
	kfree(msgs);

	return ret;
}

static long mcu_tx40_ioctl(struct file *filep, unsigned int cmd,
			   unsigned long arg)
{
	switch (cmd) {
	case MCU_TX40_IOCTL_EXEC: {
		struct mcu_tx40_exec_data data;
		struct mcu_tx40_msg *msgs;

		if (copy_from_user(&data,
				   (struct mcu_txx40_exec_data __user *)arg,
				   sizeof(data)))
			return -EFAULT;

		/* Limit the number of allowed messages */
		if (data.nmsgs > MCU_TX40_EXEC_MSGS_MAX)
			return -EINVAL;

		msgs = memdup_user(data.msgs,
			data.nmsgs * sizeof(struct mcu_tx40_msg));
		if (IS_ERR(msgs))
			return PTR_ERR(msgs);

		return mcu_tx40_ioctl_exec(filep, data.nmsgs, msgs);

	}
	default:
		return -EINVAL;
	}
}

static const struct file_operations mcu_tx40_fops = {
	.open = mcu_tx40_open,
	.unlocked_ioctl = mcu_tx40_ioctl,
	.release = mcu_tx40_release,
};

static int mcu_tx40_device_init(struct mcu_tx40 *mcu)
{
	int ret;

	ret = devm_mfd_add_devices(mcu->dev, PLATFORM_DEVID_NONE,
				   mcu_tx40_subdevs,
				   ARRAY_SIZE(mcu_tx40_subdevs), NULL, 0,
				   mcu->irq_domain);
	if (ret) {
		dev_err(mcu->dev, "Couldn't add mfd devices (%d)\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&mcu->dev->kobj, &mcu_tx40_attr_group);
	if (ret) {
		dev_err(mcu->dev, "Couldn't create sysfs entries (%d)\n", ret);
		return ret;
	}

	mcu->miscdev.fops = &mcu_tx40_fops;
	mcu->miscdev.minor = MISC_DYNAMIC_MINOR;

	mcu->miscdev.name = "mcu";
	mcu->miscdev.parent = mcu->dev;

	ret = misc_register(&mcu->miscdev);
	if (ret) {
		sysfs_remove_group(&mcu->dev->kobj, &mcu_tx40_attr_group);
		dev_err(mcu->dev, "Failed to misc_register (%d)\n", ret);
		return ret;
	}

	return 0;
}

static void mcu_tx40_device_exit(struct mcu_tx40 *mcu)
{
	misc_deregister(&mcu->miscdev);
	sysfs_remove_group(&mcu->dev->kobj, &mcu_tx40_attr_group);
}

static int mcu_tx40_i2c_probe(struct i2c_client *i2c)
{
	int ret;
	struct mcu_tx40 *mcu;
	int retry;
	u16 fw_version;

	mcu = devm_kzalloc(&i2c->dev, sizeof(struct mcu_tx40), GFP_KERNEL);
	if (mcu == NULL)
		return -ENOMEM;

	mcu->dev = &i2c->dev;

	if (!i2c->irq) {
		dev_err(&i2c->dev, "IRQ number not specified\n");
		return -EINVAL;
	}

	mutex_init(&mcu->lock);

	i2c_set_clientdata(i2c, mcu);

	ret = mcu_tx40_irq_init(mcu, i2c->irq);
	if (ret)
		return ret;

	/* Read out FW version */
	retry = 3;
	do {
		ret = mcu_tx40_get_fw_version(mcu, &fw_version);
	} while (ret && --retry > 0);

	if (ret < 0) {
		dev_err(&i2c->dev,
			"Communication error on device probing. Disabling interrupts\n");
		irq_domain_remove(mcu->irq_domain);
		return ret;
	}

	ret = mcu_tx40_device_init(mcu);
	if (ret)
		return ret;

	enable_irq(i2c->irq);

	return 0;
}

static void mcu_tx40_i2c_remove(struct i2c_client *i2c)
{
	struct mcu_tx40 *mcu = i2c_get_clientdata(i2c);
	int i;
	unsigned int irq;

	mcu_tx40_device_exit(mcu);

	for (i = 0; i < MCU_TX40_IRQ_NR; i++) {
		irq = irq_find_mapping(mcu->irq_domain, i);
		if (irq)
			irq_dispose_mapping(irq);
	}

	irq_domain_remove(mcu->irq_domain);
	mcu->irq_domain = NULL;
}

static const struct i2c_device_id mcu_tx40_i2c_id[] = {
	{"mcu_tx40", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, mcu_tx40_i2c_id);

static const struct of_device_id mcu_tx40_dt_ids[] = {
	{ .compatible = "digi,mcu-tx40", },
	{ }
};
MODULE_DEVICE_TABLE(of, mcu_tx40_dt_ids);

static struct i2c_driver mcu_tx40_i2c_driver = {
	.driver = {
		.name = "mcu_tx40",
		.of_match_table = of_match_ptr(mcu_tx40_dt_ids),
	},
	.probe    = mcu_tx40_i2c_probe,
	.remove   = mcu_tx40_i2c_remove,
	.id_table = mcu_tx40_i2c_id,
};

module_i2c_driver(mcu_tx40_i2c_driver);

MODULE_AUTHOR("Digi International Inc");
MODULE_DESCRIPTION("MCU driver for TX40");
MODULE_LICENSE("GPL v2");
