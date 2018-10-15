/****************************************************************************/

/*
 *	uclinux.c -- generic memory mapped MTD driver for uclinux
 *
 *	(C) Copyright 2002, Greg Ungerer (gerg@snapgear.com)
 *
 *      License: GPL
 */

/****************************************************************************/

#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/root_dev.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <asm/sections.h>

/****************************************************************************/

#if defined(CONFIG_MTD_UCLINUX_EBSS)
#define MAP_NAME	"ram"
#define CONFIG_MTD_UCLINUX_ADDRESS __bss_stop
#elif defined(CONFIG_MTD_UCLINUX_RAM)
#define MAP_NAME	"ram"
#elif defined(CONFIG_MTD_UCLINUX_ROM)
#define MAP_NAME	"rom"
#else
#error "Unknown uClinux map type"
#endif

/****************************************************************************/

/*
 * Blackfin uses uclinux_ram_map during startup, so it must not be static.
 * Provide a dummy declaration to make sparse happy.
 */
extern struct map_info uclinux_ram_map;

struct map_info uclinux_ram_map = {
	.name = MAP_NAME,
	.size = 0,
};

static unsigned long physaddr = -1;
module_param(physaddr, ulong, S_IRUGO);

static struct mtd_info *uclinux_ram_mtdinfo;

/****************************************************************************/

static const struct mtd_partition uclinux_romfs[] = {
	{ .name = "ROMfs" }
};

#define	NUM_PARTITIONS	ARRAY_SIZE(uclinux_romfs)

/****************************************************************************/

static int uclinux_point(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, void **virt, resource_size_t *phys)
{
	struct map_info *map = mtd->priv;
	*virt = map->virt + from;
	if (phys)
		*phys = map->phys + from;
	*retlen = len;
	return(0);
}

/****************************************************************************/

static int __init uclinux_mtd_init(void)
{
	struct mtd_info *mtd;
	struct map_info *mapp;

	mapp = &uclinux_ram_map;

	if (physaddr == -1)
		mapp->phys = (resource_size_t) CONFIG_MTD_UCLINUX_ADDRESS;
	else
		mapp->phys = physaddr;

	if (!mapp->size)
		mapp->size = PAGE_ALIGN(ntohl(*((unsigned long *)(mapp->phys + 8))));
	mapp->bankwidth = 4;

	printk("uclinux[mtd]: probe address=0x%x size=0x%x\n",
	       	(int) mapp->phys, (int) mapp->size);

	/*
	 * The filesystem is guaranteed to be in direct mapped memory. It is
	 * directly following the kernels own bss region. Following the same
	 * mechanism used by architectures setting up traditional initrds we
	 * use phys_to_virt to get the virtual address of its start.
	 */
	mapp->virt = phys_to_virt(mapp->phys);

	if (mapp->virt == 0) {
		printk("uclinux[mtd]: no virtual mapping?\n");
		return(-EIO);
	}

	simple_map_init(mapp);

	mtd = do_map_probe("map_" MAP_NAME, mapp);
	if (!mtd) {
		printk("uclinux[mtd]: failed to find a mapping?\n");
		return(-ENXIO);
	}

	mtd->owner = THIS_MODULE;
	mtd->_point = uclinux_point;
	mtd->priv = mapp;

	printk("uclinux[mtd]: set %s to be root filesystem\n",
	     	uclinux_romfs[0].name);
	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);

	uclinux_ram_mtdinfo = mtd;
	mtd_device_register(mtd, uclinux_romfs, NUM_PARTITIONS);

	return(0);
}
device_initcall(uclinux_mtd_init);

/****************************************************************************/
