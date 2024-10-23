// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 */
#include <linux/irqchip.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/imx6q-iomuxc-gpr.h>
#include <linux/of_platform.h>
#include <linux/sys_soc.h>
#include <linux/phy.h>
#include <linux/regmap.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include "common.h"
#include "cpuidle.h"
#include "hardware.h"

/*
 * Fixup for ConnectIT4 WAN port not coming up.
 * The clock enable and tx direction setting for ENET1 (the WAN port of a
 * ConnectIT4) that are part of the IOMUXC_GPR_GPR1 register needs to be set
 * for the kernels iMX6 clocking to detect it and configure it correctly.
 * If these bits are not set then the iMX6 clk driver will incorrectly set
 * the muxing of "enet1_ref" clock and will not configure and enable
 * "enet1_ref_125m" (see drivers/clk/imx/clk-imx6ul.c). Most platforms seem
 * to get this right in the boot loader and nothing is needed from the Linux
 * kernel itself.
 *
 * Note that these bits must be set very early - before the kernels clock
 * driver initializes the system clocks. That is before the usual init_early()
 * or machine_init() hooks are run. But it does have to be after the IO
 * mapping is in place (so something like dt_fixup is too early). The only
 * viable solution is the init_irq hook.
 */
static void __init imx6ul_enet_fixup(void)
{
	if (of_machine_is_compatible("digi,connectit4")) {
		struct regmap *gpr;

		pr_info("ConnectIT4: forcing GPR1_CLK_ENET1_OUTPUT\n");
		gpr = syscon_regmap_lookup_by_compatible("fsl,imx6ul-iomuxc-gpr");
		if (!IS_ERR(gpr))
			regmap_update_bits(gpr, IOMUXC_GPR1,
				IMX6UL_GPR1_ENET_CLK_DIR,
				IMX6UL_GPR1_ENET_CLK_OUTPUT);
		else
			pr_err("ConnectIT4: failed to map fsl,imx6ul-iomuxc-gpr\n");
	}
}

static void __init imx6ul_init_machine(void)
{
	imx_print_silicon_rev(cpu_is_imx6ull() ? "i.MX6ULL" : "i.MX6UL",
		imx_get_soc_revision());

	of_platform_default_populate(NULL, NULL, NULL);
	imx_anatop_init();
	imx6ul_pm_init();
}

static void __init imx6ul_init_irq(void)
{
	imx_init_revision_from_anatop();
	imx_src_init();
	irqchip_init();
	imx6_pm_ccm_init("fsl,imx6ul-ccm");
	imx6ul_enet_fixup();
}

static void __init imx6ul_init_late(void)
{
	imx6sx_cpuidle_init();

	if (IS_ENABLED(CONFIG_ARM_IMX6Q_CPUFREQ))
		platform_device_register_simple("imx6q-cpufreq", -1, NULL, 0);
}

static const char * const imx6ul_dt_compat[] __initconst = {
	"fsl,imx6ul",
	"fsl,imx6ull",
	"fsl,imx6ulz",
	NULL,
};

DT_MACHINE_START(IMX6UL, "Freescale i.MX6 Ultralite (Device Tree)")
	.init_irq	= imx6ul_init_irq,
	.init_machine	= imx6ul_init_machine,
	.init_late	= imx6ul_init_late,
	.dt_compat	= imx6ul_dt_compat,
MACHINE_END
