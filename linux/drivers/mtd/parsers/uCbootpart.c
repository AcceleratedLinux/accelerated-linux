/*
 * uCbootstrap.c:
 *
 * Retrieve Arcturus Networks uCbootloader partitioning data
 * to create linux partitions in Flash devices
 *
 * For now, cheat. Read 8 _XSA variables until you can't.
 *
 * Ultimately, $(PARTITION_PREFIX)name$(PARTITION_SUFFIX), so to speak,
 * could specify a partition. I think this may be too much rope...
 * We may jettison the configurable prefix and suffix.
 *
 * Copyright (c) 2004 Arcturus Networks Inc. <www.ArcturusNetworks.com>
 *                    by Michael Leslie, David Wu, et al 
 * 
 * Based on redboot.c by David Woodhouse
 * 
 * Change log:
 *     - modified for 2.6 kernel (David Wu)
 *     - search for all possible partitions, even if they
 *       are not sorted by name (Oleksandr Zhadan)
 */

#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/uCbootstrap.h>

extern char *getbenv(char *varname);

/* These two definitions and more should be moved to uCbootstrap.h,
 * so as to lessen the number of places an update will be required
 * to touch:
 */
#define PART_VAR_LENGTH 4	/* example "_0", one hex byte */
#define PART_VAL_LENGTH 64	/* a practical maximum of 20, but
				 * let's accomodate a bit of growth
				 * example: "10020000:3e0000:RW" */

#define MAX_UC_PARTITIONS 32	/* search for a maximum of this many partition
				 * variables from the bootloader. Real maximum
				 * is 8 bits: 256
				 */
#define PARTITION_LETTER(i)	(i<10)?(i+'0'):(i-10+'a')

int parse_ucbootstrap_partitions(struct mtd_info *master,
				 struct map_info *map,
				 struct mtd_partition **pparts)
{
	struct mtd_partition *partition;
	int n_partitions = 0;
	int i, j, items;
	char varname[PART_VAR_LENGTH];
	char valstr[PART_VAL_LENGTH];
	char *val;
	unsigned int base, size;
	char perms[8];
	char *namep;

	/* Ignore the possibility that partitions can be named other
	 * than 0, 1, 2...
	 */

	/* ugly: do a fist pass to determine the number of partitions
	 * defined for mtd_partition malloc */
	for (i = 0; i < MAX_UC_PARTITIONS; i++) {
		sprintf(varname, "_%c", PARTITION_LETTER(i));
		val = getbenv(varname);
		if (val == NULL)
			continue;
		n_partitions++;
	}

	if (!n_partitions) {
		printk(KERN_NOTICE "No uCbootloader partitions detected in %s\n",
		       master->name);
		return (0);
	}

	/* allocate mtd_partition and name memory: */
	partition =
	    kmalloc((PART_VAL_LENGTH +
		     sizeof(struct mtd_partition)) * n_partitions, GFP_KERNEL);
	if (!partition)
		return (-ENOMEM);

	namep = (char *)&partition[n_partitions];
	memset(partition, 0,
	       (PART_VAL_LENGTH + sizeof(struct mtd_partition)) * n_partitions);

	/* now read and parse partition env vars into mtd_partition structs: */
	for (i = 0, j = 0; (i < MAX_UC_PARTITIONS) && (j <= n_partitions); i++) {
		sprintf(varname, "_%c", PARTITION_LETTER(i));
		/* cli(); */
		val = getbenv(varname);
		if (val == NULL)
			continue;
		strncpy(valstr, val, PART_VAL_LENGTH);
		/* sti(); */
		items = sscanf(valstr, "%x:%x:%s", &base, &size, perms);
		if (items < 3)
			continue;
#ifdef CONFIG_MTD_DEBUG
		printk
		    ("uCbootloader partition detected: offset 0x%x, size 0x%x, permissions %s\n",
		     base, size, perms);
#endif

		/* NOTE: CHECK THE SANITY OF PARTITION SPECIFICATIONS */
		/* make sure this partition IS in the map, usefule for multiflashes */
		if (map->phys > base || base >= (map->phys + master->size))
			continue;
		partition[j].size = size;
		partition[j].offset = base - (unsigned int)map->virt;
		partition[j].name = namep;
		sprintf(namep, "%c", PARTITION_LETTER(i));
		namep += strlen(namep) + 1;
		j++;
	}

	*pparts = partition;

	return (n_partitions);
}

EXPORT_SYMBOL(parse_ucbootstrap_partitions);

MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("Arcturus Networks Inc. <www.ArcturusNetworks.com>");
MODULE_DESCRIPTION("Partition parsing interface to uCbootloader");
