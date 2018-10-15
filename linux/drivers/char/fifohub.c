/***************************************************************************
 *
 *   fifohub.c  -- Text FIFO Hub driver:                                   
 *                 for the broadcasting of the text frames                 
 *                 with specified timeout for listeners                    
 *
 *   Copyright (C) 2008 Arcturus Networks Inc.
 *                 by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 ***************************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#define FIFO_HUB_NR_DEVS	2
#define MAX_BUF_SIZE		4096	/* need to be power of 2 ??? */
#define FRAME_SEPARATOR		0xFE
#define BUF_SEPARATOR		0xFF

struct fifo_hub_dev {
	int index;
	int buf_size;
	int frame_size;
	int timeout;
	volatile int offset;
	wait_queue_head_t wait;
	struct semaphore sem;
	rwlock_t lock;
	char *buf;
	unsigned char state;
};

static int fifo_hub_major = 240;
static int fifo_hub_minor = 0;
static unsigned int fifo_hub_timeout = 20000;	/* default timeout in msec ~ 20 sec */
static const char fifo_hub_name[] = "fifohub";
static const char fifo_hub_version[] = "v.1.0 (c) 2008 Arcturus Networks Inc.";

static dev_t fifo_hub_rdev;
static struct cdev fifo_hub_cdev;
static struct fifo_hub_dev fifo_hub[FIFO_HUB_NR_DEVS];
static int fifo_hub_nr_devs = FIFO_HUB_NR_DEVS;

static void fifo_hub_flush_buffer(char *buf, int len, char ch)
{
	int i = 0;
	for (; i < len; i++)
		buf[i] = ch;
}

static int fifo_hub_state_mamager(struct fifo_hub_dev *dev)
{
	int ret = 1;

	switch (dev->buf[dev->offset + 1]) {
	case 'h':
		if (dev->state == '0')
			ret = 0;
		else
			dev->state = '0';
		break;
	case 'd':
		if (dev->state == '1')
			ret = 0;
		else
			dev->state = '1';
		break;
	}

	return ret;
}

static int fifo_hub_ioctl(struct inode *inode, struct file *file,
			  unsigned int cmd, unsigned long arg)
{
	struct fifo_hub_dev *dev = (struct fifo_hub_dev *)file->private_data;
	unsigned long flags;
	unsigned int ret = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	write_lock_irqsave(&dev->lock, flags);

	switch (cmd) {

	case 1:		/* set listeners timeout ( arg in msecs ) */
		dev->timeout = msecs_to_jiffies(arg);
		break;

	case 2:		/* Disable timeout at all (will wait only for event) */
		dev->timeout = MAX_SCHEDULE_TIMEOUT;
		break;

	case 3:		/* flush & reset fifos */
		fifo_hub_flush_buffer(dev->buf, dev->buf_size, 0);
		file->f_pos = dev->offset = 0;
		break;

	case 4:		/* return internal device state */
		ret = dev->state;
		break;

	case 5:		/* force internal device state */
		dev->state = (unsigned char)arg;
		break;
	}

	write_unlock_irqrestore(&dev->lock, flags);
	up(&dev->sem);

	return ret;
}

static ssize_t fifo_hub_read(struct file *file, char *buf, size_t size,
			     loff_t * offset)
{
	struct fifo_hub_dev *dev = (struct fifo_hub_dev *)file->private_data;
	ssize_t len;
	volatile unsigned char tmp;

	if (*offset == dev->offset)
		if (wait_event_interruptible_timeout
		    (dev->wait, *offset != dev->offset, dev->timeout) == 0)
			return -EAGAIN;

	for (len = 0;; len++) {
		tmp = dev->buf[*offset + len];

		if (tmp == FRAME_SEPARATOR)
			break;

		if (len >= dev->frame_size) {
			*offset = dev->offset;
			return -ENOMEM;
		}
		if (tmp == BUF_SEPARATOR) {
			*offset = 0;
			len = -1;
		}
	}

	if (copy_to_user(buf, (char *)&dev->buf[*offset], len))
		return -EFAULT;

	*offset += (len + 1);

	return len;
}

static ssize_t fifo_hub_write(struct file *file, const char *buf, size_t size,
			      loff_t * offset)
{
	struct fifo_hub_dev *dev = (struct fifo_hub_dev *)file->private_data;
	unsigned long flags;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (size >= dev->frame_size)
		return -ENOMEM;

	if ((dev->offset + size + 2) >= dev->buf_size) {
		int i = 0;
		for (; (dev->offset + i) < dev->buf_size; i++)
			dev->buf[dev->offset + i] = BUF_SEPARATOR;
		dev->offset = 0;
	}

	if (copy_from_user((char *)&dev->buf[dev->offset + 1], buf, size)) {
		up(&dev->sem);
		return -EFAULT;
	}

	fifo_hub_state_mamager(dev);

	write_lock_irqsave(&dev->lock, flags);
	dev->buf[dev->offset] = dev->state;
	dev->buf[dev->offset + size + 1] = FRAME_SEPARATOR;
	dev->offset += (size + 2);
	*offset = dev->offset;	/* to avoid self-frame reading */
	write_unlock_irqrestore(&dev->lock, flags);

	up(&dev->sem);

	if (waitqueue_active(&dev->wait))
		wake_up_interruptible(&dev->wait);

	return size;
}

static int fifo_hub_open(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	struct fifo_hub_dev *dev;

	file->private_data = &fifo_hub[minor];

	dev = (struct fifo_hub_dev *)file->private_data;

	if (!dev->buf) {
		dev->buf = kmalloc(dev->buf_size, GFP_KERNEL);
		if (!dev->buf)
			return -ENOMEM;
#if defined(CONFIG_UNCACHED_REGION_MASK)
		dev->buf =
		    (char *)((u32) dev->buf | CONFIG_UNCACHED_REGION_MASK);
#endif
		fifo_hub_flush_buffer(dev->buf, dev->buf_size, BUF_SEPARATOR);
	}
	file->f_pos = dev->offset;

	return 0;
}

static struct file_operations fifo_hub_fops = {
	.owner = THIS_MODULE,
	.open = fifo_hub_open,
	.ioctl = fifo_hub_ioctl,
	.read = fifo_hub_read,
	.write = fifo_hub_write,
};

static int fifo_hub_init_module(void)
{
	int i, ret;

	printk(KERN_INFO "%s: %s\n", fifo_hub_name, fifo_hub_version);

	if (fifo_hub_major) {
		fifo_hub_rdev = MKDEV(fifo_hub_major, fifo_hub_minor);
		ret =
		    register_chrdev_region(fifo_hub_rdev, fifo_hub_nr_devs,
					   fifo_hub_name);
	} else {
		ret =
		    alloc_chrdev_region(&fifo_hub_rdev, fifo_hub_minor,
					fifo_hub_nr_devs, fifo_hub_name);
		fifo_hub_major = MAJOR(fifo_hub_rdev);
	}

	if (ret < 0) {
		printk(KERN_WARNING "%s: Can't get major %d\n", fifo_hub_name,
		       fifo_hub_major);
		return ret;
	}

	cdev_init(&fifo_hub_cdev, &fifo_hub_fops);
	fifo_hub_cdev.owner = THIS_MODULE;
	kobject_set_name(&fifo_hub_cdev.kobj, "%s%d", fifo_hub_name,
			 fifo_hub_rdev);

	ret = cdev_add(&fifo_hub_cdev, fifo_hub_rdev, fifo_hub_nr_devs);
	if (ret < 0) {
		printk(KERN_ERR "%s: error %d adding %s%d\n", fifo_hub_name,
		       ret, fifo_hub_name, fifo_hub_rdev);
		unregister_chrdev_region(fifo_hub_rdev, fifo_hub_nr_devs);
		return ret;
	}

	for (i = 0; i < fifo_hub_nr_devs; i++) {
		fifo_hub[i].index = i;
		fifo_hub[i].buf_size = MAX_BUF_SIZE;
		fifo_hub[i].frame_size = (MAX_BUF_SIZE >> 2);
		fifo_hub[i].timeout = msecs_to_jiffies(fifo_hub_timeout);
		fifo_hub[i].offset = 0;
		init_waitqueue_head(&fifo_hub[i].wait);
		init_MUTEX(&fifo_hub[i].sem);
		fifo_hub[i].buf = NULL;
		fifo_hub[i].state = 0;
	}

	return 0;
}

static void fifo_hub_exit_module(void)
{
	printk(KERN_DEBUG "%s: exit\n", fifo_hub_name);
	cdev_del(&fifo_hub_cdev);
	unregister_chrdev_region(fifo_hub_rdev, fifo_hub_nr_devs);
}

module_init(fifo_hub_init_module);
module_exit(fifo_hub_exit_module);

MODULE_DESCRIPTION("Text FIFO Hub kernel module");
MODULE_AUTHOR("Oleksandr Zhadan (www.ArcturusNetworks.com)");
MODULE_LICENSE("GPL");
