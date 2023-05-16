// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2013 John Crispin <john@phrozen.org>
 */

#include <linux/pm.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>

#include <asm/reboot.h>

#include <asm/mach-ralink/ralink_regs.h>

/* Reset Control */
#define SYSC_REG_RESET_CTRL	0x034

#define RSTCTL_RESET_PCI	BIT(26)
#define RSTCTL_RESET_SYSTEM	BIT(0)

static int ralink_assert_device(struct reset_controller_dev *rcdev,
				unsigned long id)
{
	u32 val;

	if (id == 0)
		return -1;

	val = rt_sysc_r32(SYSC_REG_RESET_CTRL);
	val |= BIT(id);
	rt_sysc_w32(val, SYSC_REG_RESET_CTRL);

	return 0;
}

static int ralink_deassert_device(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	u32 val;

	if (id == 0)
		return -1;

	val = rt_sysc_r32(SYSC_REG_RESET_CTRL);
	val &= ~BIT(id);
	rt_sysc_w32(val, SYSC_REG_RESET_CTRL);

	return 0;
}

static int ralink_reset_device(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	ralink_assert_device(rcdev, id);
	return ralink_deassert_device(rcdev, id);
}

static const struct reset_control_ops reset_ops = {
	.reset = ralink_reset_device,
	.assert = ralink_assert_device,
	.deassert = ralink_deassert_device,
};

static struct reset_controller_dev reset_dev = {
	.ops			= &reset_ops,
	.owner			= THIS_MODULE,
	.nr_resets		= 32,
	.of_reset_n_cells	= 1,
};

static int ralink_reset_probe(struct platform_device *pdev)
{
	/*
	 * The reset driver registration was done earlier at "of" platform
	 * init time, so nothing left to do here now. We need this step to
	 * be present though so that the device tree knows that the reset
	 * controller is marked as present and ready.
	 */
	return 0;
}

static const struct of_device_id ralink_reset_dt_ids[] = {
	{ .compatible = "ralink,rt2880-reset", },
	{ /* sentinel */ },
};

static struct platform_driver ralink_reset_driver = {
        .probe  = ralink_reset_probe,
        .driver = {
                .name           = KBUILD_MODNAME,
                .of_match_table = ralink_reset_dt_ids,
        },
};
builtin_platform_driver(ralink_reset_driver);

void ralink_rst_init(void)
{
	reset_dev.of_node = of_find_compatible_node(NULL, NULL,
						"ralink,rt2880-reset");
	if (!reset_dev.of_node)
		pr_err("Failed to find reset controller node");
	else
		reset_controller_register(&reset_dev);
}

static void ralink_restart(char *command)
{
	if (IS_ENABLED(CONFIG_PCI)) {
		rt_sysc_m32(0, RSTCTL_RESET_PCI, SYSC_REG_RESET_CTRL);
		mdelay(50);
	}

	local_irq_disable();
	rt_sysc_w32(RSTCTL_RESET_SYSTEM, SYSC_REG_RESET_CTRL);
	unreachable();
}

static int __init mips_reboot_setup(void)
{
	_machine_restart = ralink_restart;

	return 0;
}

arch_initcall(mips_reboot_setup);
