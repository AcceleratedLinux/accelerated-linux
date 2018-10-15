/***************************************************************************
 *   Copyright (c) 2008 Arcturus Networks Inc.
 *                 by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 *   mtdsingle.c - creates single mtd partition on the fly by `insmod`
 *                 `rmmod` will remove this partition
 *                 May be useful to update a non-standart image
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 ***************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#define MIN_SECTOR_SIZE 0x20000

static unsigned int fbase = 0;
static unsigned int fsize = 0;
static unsigned int fbus = 2;	/* 1-8bit 2-16bit 4-32bit */

static unsigned int pbase = 0;	/* default - start of the flash    */
static unsigned int psize = 0;	/* default - all flash             */
static char *pname = "TEMP";

static struct mtd_info *flash_mtd;
static struct map_info flash_map = { name:"uFlash" };
static struct mtd_partition partition;

static int __init mtdsingle_init(void)
{
	if (fsize < MIN_SECTOR_SIZE) {
		printk(KERN_ERR "%s: Flash Size (fsize=xxx) MUST be set.\n",
		       __FILE__);
		printk(KERN_INFO
		       "%s: Arguments: fbase, fsize, fbus, pbase, psize, bname. (See file for details)\n",
		       __FILE__);
		return -1;
	}

	flash_map.phys = fbase;
	flash_map.bankwidth = fbus;
	flash_map.size = fsize;
	flash_map.virt = ioremap(fbase, fsize);

	if ((flash_map.phys != 0) && (flash_map.virt == 0)) {
		printk(KERN_ERR "%s: Failed to ioremap\n", __FILE__);
		return -EIO;
	}
	simple_map_init(&flash_map);

	flash_mtd = do_map_probe("cfi_probe", &flash_map);

	if (flash_mtd != NULL) {
		partition.offset = pbase - (unsigned int)flash_map.virt;
		partition.size = psize;
		partition.name = (char *)pname;
		flash_mtd->owner = THIS_MODULE;
		mtd_device_register(flash_mtd, &partition, 1);
	} else
		iounmap(flash_map.virt);

	return 0;
}

static void __exit mtdsingle_exit(void)
{
	mtd_device_unregister(flash_mtd);
}

module_init(mtdsingle_init);
module_exit(mtdsingle_exit);

module_param(fbase, uint, 0);
module_param(fsize, uint, 0);
module_param(fbus, uint, 0);

MODULE_PARM_DESC(fbase, "Flash Base Address");
MODULE_PARM_DESC(fsize, "Flash Size");
MODULE_PARM_DESC(fbus, "Flash bus width (1, 2 or 4 8bit words)");

module_param(pbase, uint, 0);
module_param(psize, uint, 0);
module_param(pname, charp, 0);

MODULE_PARM_DESC(pbase, "Partition Base Address");
MODULE_PARM_DESC(psize, "Partition Size");
MODULE_PARM_DESC(pname, "Partition Name");

MODULE_DESCRIPTION
    ("Partition creation module for the non-standart update image");
MODULE_AUTHOR("Oleksandr Zhadan <www.ArcturusNetworks.com>");
MODULE_LICENSE("GPL");
