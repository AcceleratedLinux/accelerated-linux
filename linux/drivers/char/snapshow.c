/****************************************************************************/

/*
 *	snapshow.c --  collect and display the ADSL showtime status
 *
 *	(C) Copyright 2009,  Greg Ungerer <gerg@snapgear.com>
 */

/****************************************************************************/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>

/****************************************************************************/

static volatile u8 *showt_cs2;

/****************************************************************************/

static int showt_proc_show(struct seq_file *m, void *v)
{
	int link;

	link = (*showt_cs2 & 0x1) ? 0 : 1;
	seq_printf(m, "%d\n", link);
	return 0;
}

/****************************************************************************/

static int showt_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, showt_proc_show, NULL);
}

/****************************************************************************/

static const struct file_operations showt_proc_fops = {
	.open		= showt_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/****************************************************************************/

static __init int showt_init(void)
{
	printk("SNAPSHOW: SnapGear xDSL SHOWTIME status driver\n");

	/* Configure the CS1 line as 8bit input */
	*IXP4XX_EXP_CS1 = 0xbfff0001;

	showt_cs2 = (volatile u8 *) ioremap(IXP4XX_EXP_BUS_BASE(1), 16);
	if (showt_cs2 == NULL) {
		printk("SHOWTIME: cannot remap io region?\n");
		return -ENODEV;
	}

	if (!proc_create("showtime", 0, NULL, &showt_proc_fops)) {
		printk("SHOWTIME: failed to register proc entry?\n");
		return -ENOMEM;
	}

	return 0;
}

module_init(showt_init);

/****************************************************************************/
