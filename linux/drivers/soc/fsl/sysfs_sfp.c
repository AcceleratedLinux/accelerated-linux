/*
 * Layerscape On-Chip SFP driver
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/of.h>

#define SFP_INGR 0x20 /* Instruction reg */
#define SFP_VERS 0x38 /* Version reg */
#define SFP_INGR_ERR 0x100
#define SFP_INGR_INST_READ 0x1 /* Reload mirror regs */
#define SFP_INGR_INST_PROG 0x2 /* Burn mirror to SFP */
#define SFP_INGR_INST_STATUS (SFP_INGR_ERR | SFP_INGR_INST_READ | SFP_INGR_INST_PROG)
#define SFP_SVHESR 0x24 /* hamming status reg */
#define SFP_SFPCR 0x28 /* Configuration reg */
#define SFP_SFPCR_PPM (0xffff) /* program pulse width */
#define SFP_BASE 0x1E80000
#define SFP_MIRROR_BASE (sfp_base + 0x200)
#define SFP_NUM_REGS (sizeof(sfp_reg_desc) / sizeof(sfp_reg_desc[0]))

static void __iomem *sfp_base;
static struct resource sfp_res;
struct kobject *sfp_kobj;
struct kobj_attribute *sfp_kattr;
struct attribute_group sfp_attr_group;

static char *sfp_reg_desc[] = {
	"OSPR",
	"OSPR1",
	"DCVR0",
	"DCVR1",
	"DRVR0",
	"DRVR1",
	"FSWPR",
	"FUIDR0",
	"FUIDR1",
	"ISBCCR",
	"FSPFR0",
	"FSPFR1",
	"FSPFR2",
	"OTPMKR0",
	"OTPMKR1",
	"OTPMKR2",
	"OTPMKR3",
	"OTPMKR4",
	"OTPMKR5",
	"OTPMKR6",
	"OTPMKR7",
	"SRKHR0",
	"SRKHR1",
	"SRKHR2",
	"SRKHR3",
	"SRKHR4",
	"SRKHR5",
	"SRKHR6",
	"SRKHR7",
	"OUIDR0",
	"OUIDR1",
	"OUIDR2",
	"OUIDR3",
	"OUIDR4",
};

static void (*sfp_write)(u32 value, volatile void __iomem *addr);
static u32 (*sfp_read)(const volatile void __iomem *addr);

static void sfp_write_be(u32 value, volatile void __iomem *addr)
{
	iowrite32be(value, addr);
}

static u32 sfp_read_be(const volatile void __iomem *addr)
{
	return ioread32be(addr);
}

static void sfp_write_le(u32 value, volatile void __iomem *addr)
{
	iowrite32(value, addr);
}

static u32 sfp_read_le(const volatile void __iomem *addr)
{
	return ioread32(addr);
}

#if 0
static int sfp_set_timing(void)
{
	u32 clk_rate = 0;
	u32 timing = 0;
	u32 cur_timing;
	struct clk *clk;

	clk = clk_get(NULL, "cg-pll0-div4");
	if (IS_ERR(clk)) {
		pr_err("SFP: failed to get clk: %ld\n", PTR_ERR(clk));
		return PTR_ERR(clk);
	}
	clk_rate = clk_get_rate(clk);
	/*
	 * The optimal value for PPW is calculated as the SFP module input clock
	 * frequency (in MHz) * 12 where the SFP module input clock is platform
	 * clock/4.
	 */
	timing = clk_rate/1000000 * 12;

	cur_timing = sfp_read(sfp_base + SFP_SFPCR);
	pr_err("SFP: calculated timing: 0x%x, cur timing: 0x%x\n", timing, cur_timing);

	if (cur_timing != timing)
		sfp_write(timing, sfp_base + SFP_SFPCR);

	return 0;
}
#endif

static int sfp_wait(void)
{
	u32 reg;

	/* Don’t poll the INGR as a method of determining if fuse programming has
	 * completed. Wait the worst case amount of time before lowering
	 * TA_PROG_SFP voltage.
	 * For 28nm products (all Trust 2.1 devices), this time is:
	 * (12 us prog/bit) x (4096 bits) = 49.15ms
	 * We will wait double that value just to be safe */
	msleep(100);
	reg = sfp_read(sfp_base + SFP_INGR);
	if (reg == 0)
		return 0;

	pr_err("SFP: timeout! - status: 0x%x\n", reg);
	reg = sfp_read(sfp_base + SFP_SVHESR);
	pr_err("SFP: VHESR: 0x%x\n", reg);

	return -ETIMEDOUT;
}

/* A write of any value to save, will try and blow the SFP bits */
static ssize_t save_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	u32 value;

	/* verify hamming status for OTPMK */
	value = sfp_read(sfp_base + SFP_SVHESR);
	if (value != 0) {
		pr_err("SFP: error in hamming status: 0x%x\n", value);
		return -EINVAL;
	}

	sfp_write(SFP_INGR_INST_PROG, sfp_base + SFP_INGR);
	ret = sfp_wait();

	return ret ? ret : count;
}

static ssize_t svhesr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	u32 value;

	/* Display hamming status for OTPMK */
	value = sfp_read(sfp_base + SFP_SVHESR);
	pr_err("SFP: hamming status: 0x%x\n", value);

	return scnprintf(buf, PAGE_SIZE, "0x%08x\n", value);
}

static struct kobj_attribute save_attr = __ATTR(save,
                                                S_IRUGO | S_IWUSR,
                                                NULL,
                                                save_store);

static struct kobj_attribute svhesr_attr = __ATTR(SVHESR,
                                                S_IRUGO,
                                                svhesr_show,
                                                NULL);


static ssize_t sfp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	unsigned int index = (attr - sfp_kattr) * sizeof(u32);
	u32 value = 0;

	pr_err("SFP: Showing reg 0x%x\n", index);
	value = sfp_read(SFP_MIRROR_BASE + index);

	return scnprintf(buf, PAGE_SIZE, "0x%08x\n", value);
}

#if 0
static int load_mirror(void)
{
	sfp_write(SFP_INGR_INST_READ, sfp_base + SFP_INGR);
	return sfp_wait();
}
#endif

static ssize_t sfp_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int index = (attr - sfp_kattr) * sizeof(u32);
	u32 value;

	if (!sscanf(buf, "0x%x", &value))
		return -EINVAL;
	pr_err("SFP: Storing 0x%08x to reg 0x%x\n", value, index);

	sfp_write(value, SFP_MIRROR_BASE + index);

	/* verify hamming status for OTPMK */
	value = sfp_read(sfp_base + SFP_SVHESR);
	pr_err("SFP: hamming status: 0x%x\n", value);

	return count;
}


static int sfp_init(void)
{
	struct attribute **attrs = 0;
	int i;
	int ret;
	u32 version;
	struct device_node *np;

	sfp_res.name = "SFP mirror register memory space";
	sfp_res.start = SFP_BASE;
	sfp_res.end = sfp_res.start + SZ_4K;
	ret = request_resource(&iomem_resource, &sfp_res);
	if (ret != 0) {
		pr_err("SFP: couldn't request_resource: %d\n", ret);
		goto err;
	}

	sfp_base = ioremap(SFP_BASE, SZ_4K);
	if (sfp_base == NULL) {
		ret = -EIO;
		pr_err("SFP: failed to ioremap resource\n");
		goto err;
	}

	/*
	 * Platforms, compatible with ls1021a-sfp are using big-endian,
	 * ls1028a-sfp are using little-endian IO
	 */
	np = of_find_compatible_node(NULL, NULL, "fsl,ls1021a-sfp");
	if (np) {
		sfp_write = sfp_write_be;
		sfp_read = sfp_read_be;
	} else {
		np = of_find_compatible_node(NULL, NULL, "fsl,ls1028a-sfp");
		if (!np) {
			ret = -ENODEV;
			pr_err("SFP: failed to find compatible node\n");
			goto err;
		}

		sfp_write = sfp_write_le;
		sfp_read = sfp_read_le;
	}
	of_node_put(np);

	/* The last one is NULL, which is used to detect the end */
	attrs = kzalloc((SFP_NUM_REGS + 4) * sizeof(*attrs), GFP_KERNEL);
	sfp_kattr = kzalloc((SFP_NUM_REGS + 1) * sizeof(*sfp_kattr), GFP_KERNEL);
	if (!attrs || !sfp_kattr) {
		pr_err("SFP: Falled to alloc attrs\n");
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < SFP_NUM_REGS; i++) {
		sysfs_attr_init(&sfp_kattr[i].attr);
		sfp_kattr[i].attr.name = sfp_reg_desc[i];
		sfp_kattr[i].attr.mode = 0600;
		sfp_kattr[i].store = sfp_store;
		sfp_kattr[i].show = sfp_show;
		attrs[i] = &sfp_kattr[i].attr;
	}
	// Last attr to tell SFP to burn
	attrs[i] = &save_attr.attr;
	attrs[i+1] = &svhesr_attr.attr;
	attrs[i+2] = NULL;
	sfp_attr_group.attrs = attrs;

	sfp_kobj = kobject_create_and_add("sfp", NULL);
	if (!sfp_kobj) {
		pr_err("SFP: failed to add kobject\n");
		ret = -ENOMEM;
		goto err;
	}

	ret = sysfs_create_group(sfp_kobj, &sfp_attr_group);
	if (ret) {
		pr_err("SFP: failed to create sysfs group: %d\n", ret);
		goto err;
	}

#if 0
	load_mirror();
#endif
	version = sfp_read(sfp_base + SFP_VERS);
	pr_info("SFP: version: 0x%08x\n", version);

	ret = sfp_read(sfp_base + SFP_SFPCR);
	pr_err("SFP: cur timing: 0x%x\n", ret);

	return 0;

err:
	if (attrs)
		kfree(attrs);

	if (sfp_kattr)
		kfree(sfp_kattr);

	release_resource(&sfp_res);
	kobject_put(sfp_kobj);

	return ret;
}

static void sfp_exit(void)
{
	release_resource(&sfp_res);
	sysfs_remove_group(sfp_kobj, &sfp_attr_group);

	if (sfp_attr_group.attrs)
		kfree(sfp_attr_group.attrs);

	if (sfp_kattr)
		kfree(sfp_kattr);


	kobject_put(sfp_kobj);
}

module_init(sfp_init);
module_exit(sfp_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seth Bollinger <support@digi.com>");
MODULE_DESCRIPTION("Layerscape SFP driver");
