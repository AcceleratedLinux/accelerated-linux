/*
 * uCbootmap.c:
 *
 * Retrieve Arcturus Networks uCbootloader Flash device data
 * to create MTD devices
 *
 * Copyright (c) 2004-2007 Arcturus Networks Inc.
 *               by Michael Leslie, David Wu, et al <www.ArcturusNetworks.com>
 * Based on solutionengine.c by jsiegel
 * 
 * Change log:
 *   - modified for 2.6 kernel (David Wu)
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/uCbootstrap.h>

static int errno;
extern char *getbenv(char *varname);
static _bsc1 (int, setpmask, int, mask);

#define MAX_FLASH_DEVICES 2 /* for uCbootstrap platforms to date */
extern int parse_ucbootstrap_partitions(struct mtd_info *master,
                                        struct map_info *map,
                                        struct mtd_partition **pparts);

static struct mtd_info *flash_mtd[MAX_FLASH_DEVICES];
static struct mtd_partition *parsed_parts;

struct map_info uCflash_map[MAX_FLASH_DEVICES] = { { name:       "Flash 0" }, { name:       "Flash 1" } };

char *env_name[MAX_FLASH_DEVICES] = { "FLASH0", "FLASH1" };
#define ENVARNAME_LENGTH 32 /* local maximum */

static int __init init_uCbootstrap_maps(void)
{
	int n_partitions = 0;
	char varname[ENVARNAME_LENGTH];
	char valstr[ENVARNAME_LENGTH];
	char *val;
	unsigned int window_size, window_addr, bus_width;
	int i;
	int try = 0;
	int found = 0;

	printk("MTD chip mapping driver for uCbootloader\n");

	/* zero pointers: */
	for (i=0;i<MAX_FLASH_DEVICES;i++)
		flash_mtd[i] = 0;

retry:
	for (i=0;i<MAX_FLASH_DEVICES ;i++) {
		valstr[0] = 0; val = NULL;

		if(try == 0) setpmask(FEF_SUPER_READ); /* set supervisory env var read permission */
        	/* FIXME: assume all requested variables exist */

		/* retrieve FLASH[]_BASE */
		sprintf (varname, "%s_BASE", env_name[i]);
		/* cli(); */
		val = getbenv (varname);
		if (val == NULL) {
			/* sti(); */
			continue;
		} else found = 1;
                
		strncpy (valstr, val, ENVARNAME_LENGTH);
		/* sti(); */
		sscanf (valstr, "0x%x", &window_addr);

		/* retrieve FLASH[]_SIZE */
		sprintf (varname, "%s_SIZE", env_name[i]);
		/* cli(); */
		val = getbenv (varname);
		strncpy (valstr, val, ENVARNAME_LENGTH);
		/* sti(); */
		sscanf (valstr, "0x%x", &window_size);

		/* retrieve FLASH[]_BUS_WIDTH */
		sprintf (varname, "%s_BUS_WIDTH", env_name[i]);
		/* cli(); */
		val = getbenv (varname);
		strncpy (valstr, val, ENVARNAME_LENGTH);
		/* sti(); */
		sscanf (valstr, "%d", &bus_width);
		bus_width /= 8; /* bytes from bits */
		
		/* reset supervisory env var read permission */
		if(try == 0) setpmask(FEF_USER_READ  | FEF_USER_WRITE  |
				 FEF_BOOT_READ  | FEF_BOOT_WRITE);
		
#ifdef CONFIG_MTD_DEBUG	
		printk("%s: %s window address = 0x%08x; size = 0x%08x, bus width = %d bytes\n",
			   __FILE__, env_name[i], window_addr, window_size, bus_width);
#endif
		/* First probe at offset 0 */
		uCflash_map[i].phys = window_addr;
		uCflash_map[i].bankwidth = bus_width;
		uCflash_map[i].size     = window_size;
		uCflash_map[i].virt = ioremap(window_addr, window_size);
		
		
		if ( (uCflash_map[i].phys != 0 ) && ( uCflash_map[i].virt == 0 ) ) {
			printk("%s: Failed to ioremap\n", __FILE__);
			return -EIO;
		}

		simple_map_init(&(uCflash_map[i]));

		printk(KERN_DEBUG "%s: Probing for flash chip at 0x%08x:\n", __FILE__, window_addr);
		flash_mtd[i] = do_map_probe("cfi_probe", &(uCflash_map[i]));

		if(flash_mtd[i]) {
			flash_mtd[i]->owner = THIS_MODULE;

#ifdef CONFIG_MTD_UCBOOTSTRAP_PARTS
			n_partitions = parse_ucbootstrap_partitions(flash_mtd[i], &(uCflash_map[i]), &parsed_parts);
			if (n_partitions > 0)
				printk(KERN_INFO "%s: Found uCbootloader partition table.\n", __FILE__);
			else if (n_partitions < 0)
				printk(KERN_ERR "%s: Error looking for uCbootloader partitions.\n", __FILE__);

			if (n_partitions > 0)
				mtd_device_register(flash_mtd[i], parsed_parts, n_partitions);
			else
#endif /* CONFIG_MTD_UCBOOTSTRAP_PARTS */
				add_mtd_device(flash_mtd[i]);
		} else
			iounmap(uCflash_map[i].virt);
	}
	/* reset supervisory env var read permission */
	setpmask(FEF_USER_READ  | FEF_USER_WRITE  |
				 FEF_BOOT_READ  | FEF_BOOT_WRITE);
	if (!found && !try){
		try = 1;
		goto retry;
	}
	if(found) return 0;
	return -1;
}

static void __exit cleanup_uCbootstrap_maps(void)
{
	int i;

#ifdef CONFIG_MTD_UCBOOTSTRAP_PARTS
	if (parsed_parts) {
		for (i=0;i<MAX_FLASH_DEVICES;i++) {
			if (flash_mtd[i])
				mtd_device_unregister(flash_mtd[i]);
			map_destroy(flash_mtd[i]);
		}
	} else
#endif /* CONFIG_MTD_UCBOOTSTRAP_PARTS */
		for (i=0;i<MAX_FLASH_DEVICES;i++) {
			if (flash_mtd[i]) {
				del_mtd_device(flash_mtd[i]);
				map_destroy(flash_mtd[i]);
			}
		}
}

module_init(init_uCbootstrap_maps);
module_exit(cleanup_uCbootstrap_maps);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arcturus Networks Inc. <www.ArcturusNetworks.com>");
MODULE_DESCRIPTION("MTD map driver for platforms running Arcturus Networks uCbootloader");

