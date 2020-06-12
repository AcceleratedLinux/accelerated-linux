// SPDX-License-Identifier: GPL-2.0
/******************************************************************************
 *
 * Copyright (c) 2019 Digi International Inc.
 * All rights reserved.
 *
 *****************************************************************************/

#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/memblock.h>
#include <uapi/linux/stat.h> /* S_IRUSR, S_IWUSR  */

/* When adding new values, please start at next and then set next so it doesn't
 * trample your memory */
#define BOOT_ATTEMPT_OFFSET 0x0
#define PMEM_OFFSET_NEXT    0x4

static struct kobject *kobj;
static void __iomem *mem;
static struct resource persistent_mem_io;
extern char *saved_command_line;

static ssize_t boot_attempts_show(struct kobject *kobj,
                                  struct kobj_attribute *attr,
                                  char *buf)
{
	u32 ba;
	ssize_t ret;

	ba = readl(mem + BOOT_ATTEMPT_OFFSET);
	ret = snprintf(buf, 10, "%u", ba);

	return ret;
}

static ssize_t boot_attempts_store(struct  kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf,
                                   size_t cnt)
{
	/* There's no reason to do anything other than reset from userland */
	writel(0, mem + BOOT_ATTEMPT_OFFSET);
	return 1;
}

static struct kobj_attribute boot_attempts_attr = __ATTR(boot_attempts,
                                                         S_IRUGO | S_IWUSR,
                                                         boot_attempts_show,
                                                         boot_attempts_store);

static struct attribute *pmem_attrs[] = {
	&boot_attempts_attr.attr,
	NULL,
};

static struct attribute_group pmem_attr_group = {
	.attrs = pmem_attrs,
};

static int __init pmem_init(void)
{
	int ret;

	if (strstr(saved_command_line, "mem") == NULL)
	{
		pr_err("Error: need to reserve 4 KiB in memory for kernel (see u-boot CONFIG_PRAM)\n");
		return -EINVAL;
	}

	persistent_mem_io.name = "Warm boot persistent memory space";
	persistent_mem_io.start = memblock_end_of_DRAM();
	persistent_mem_io.end = persistent_mem_io.start + SZ_4K;
	pr_debug("start: 0x%llx, end: 0x%llx\n",
		 (unsigned long long)persistent_mem_io.start,
		 (unsigned long long)persistent_mem_io.end);

	ret = request_resource(&iomem_resource, &persistent_mem_io);
	if (ret != 0)
	{
		pr_err("Error: couldn't request_resource: %d\n", ret);
		return ret;
	}
	mem = ioremap(memblock_end_of_DRAM(), SZ_4K);
	/*pr_info("remap: 0x%x\n", mem);
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_NONE, 32, 4, mem, SZ_4K, 0);*/

	kobj = kobject_create_and_add("persistent_mem", kernel_kobj);
	if (!kobj)
	{
		ret = -ENOMEM;
		goto err;
	}

	ret = sysfs_create_group(kobj, &pmem_attr_group);
	if (ret)
		goto err;

	return 0;

err:
	if (mem)
		iounmap(mem);

	release_resource(&persistent_mem_io);
	kobject_put(kobj);

	return ret;
}

static void pmem_exit(void)
{
	iounmap(mem);
	release_resource(&persistent_mem_io);
	kobject_put(kobj);
}

module_init(pmem_init);
module_exit(pmem_exit);
MODULE_LICENSE("GPL");
