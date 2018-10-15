/*
 * Support for AcceleratedConcepts 6300 platforms
 * David McCullough <david.mccullough@accelerated.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mv643xx_eth.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include "common.h"
#include "kirkwood.h"
#include "board.h"

#if defined(CONFIG_MACH_6300CX)
static __init int accelerated_6300_cx_fixup(void)
{
	struct device_node *np;

	/* if we are using all of NAND then nothing to do here */
	if (strstr(boot_command_line, "fullnand"))
		return 0;

	pr_info("6300-CX NAND partition fixup\n");

	/*
	 * find the nand flash and then find the 'flash' partition
	 * change its size if needed :-)
	 */
	np = of_find_node_by_name(NULL, "nand");
	if (np) {
		struct device_node *npp;

		for_each_child_of_node(np, npp) {
			struct property *lp, *rp;
			int rlen, llen, mtd_len;

			llen = 0;
			lp = of_find_property(npp, "label", &llen);
			if (!lp)
				continue;

			/* check the label for the MTD names we want */
			mtd_len= 0;
			if (strcmp(lp->value, "flash") == 0)
				mtd_len = 0x07b00000;
			if (strcmp(lp->value, "all") == 0)
				mtd_len = 0x08000000;
			if (mtd_len == 0)
				continue;
			/*
			 * change the second value (segment size) in the
			 * reg property for the partition
			 */
			rlen = 0;
			rp = of_find_property(npp, "reg", &rlen);
			if (rp && (rp->length == 8)) {
				pr_info("6300-CX NAND partition fixed: %s\n",
					(char *) lp->value);
				((u32 *)rp->value)[1] = cpu_to_be32(mtd_len);
			}
		}
		of_node_put(np);
	}
	return 0;
}
early_initcall(accelerated_6300_cx_fixup);
#endif /* CONFIG_6300CX */

#ifndef CONFIG_PCI_MVEBU
/*
 * Power down the PCIe bus if not used.
 * This can potentially help reduce EMI.
 */
static __init int accelerated_6300_power_down_pci(void)
{
	void __iomem *rp;
	u32 v1, v2;

	rp = ioremap(0xd0000000, 0x00100000);
	if (rp) {
		v1 = readl(rp + 0x18220);
		v2 = v1 & ~0x00000226;
		writel(v2, rp + 0x18220);
		v2 = readl(rp + 0x18220);
		printk("6300: disable PCIe clocks (PMCGCR was=0x%08x now=0x%08x)\n", v1, v2);
		iounmap(rp);
	} else {
		printk("6300: FAILED to change PMCGCR\n");
	}
	return 0;
}
early_initcall(accelerated_6300_power_down_pci);
#endif
