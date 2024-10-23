// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Pin controller for NXP QorIQ LS1046A.
 *
 * Copyright (c) 2024 Digi International, Inc. All rights reserved.
 * Author: David Leonard <David.Leonard@digi.com>
 */
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/sys_soc.h>

struct ls1046a_pinctrl_pdata {
	struct pinctrl_dev *pctl_dev;
	void __iomem *cr0mem;
	bool big_endian;
	u32 ssc;
};

/*
 *       L4           M4            M3           N3
 *  i2c  IIC3_SCL     IIC3_SDA      IIC4_SCL     IIC4_SDA
 *  gpio GPIO_4[10]   GPIO_4[11]    GPIO_4[12]   GPIO_4[13]
 *  evt  EVT_B[5]     EVT_B[6]      EVT_B[7]     EVT_B[8]
 *  usb  USB2_DRVVBUS USB2_PWRFAULT USB3_DRVVBUS USB3_PWRFAULT
 *  ftm  FTM8_CH0     FTM8_CH1      FTM3_FAULT   FTM_EXT3CLK
 */

/* SCFG_RCWPMUXCR0 pinmux control register */
#define SCFG_RCWPMUXCR0			0x0157040c
#define RCWPMUXCR0_FIELD(shift, func)	((u32)(func) << (29-(shift)))
#define RCWPMUXCR0_MASK(shift)		RCWPMUXCR0_FIELD(shift, RCWPMUXCR0_FUNC_MASK)
#define RCWPMUXCR0_IIC3_SCL_SHIFT	17
#define RCWPMUXCR0_IIC3_SDA_SHIFT	21
#define RCWPMUXCR0_IIC4_SCL_SHIFT	25
#define RCWPMUXCR0_IIC4_SDA_SHIFT	29
#define RCWPMUXCR0_FUNC_I2C		0	/* 0b000 */
#define RCWPMUXCR0_FUNC_GPIO		1	/* 0b001 */
#define RCWPMUXCR0_FUNC_EVT		2	/* 0b010 */
#define RCWPMUXCR0_FUNC_USB		3	/* 0b011 */
#define RCWPMUXCR0_FUNC_FTM		5	/* 0b101 */
#define RCWPMUXCR0_FUNC_CLK		6	/* 0b110 */
#define RCWPMUXCR0_FUNC_MASK		7	/* 0b111 */

#define PIN_L4 0	/* IIC3_SCL */
#define PIN_M4 1	/* IIC3_SDA */
#define PIN_M3 2	/* IIC4_SCL */
#define PIN_N3 3	/* IIC4_SDA */

const struct pinctrl_pin_desc ls1046a_pins[] = {
	PINCTRL_PIN(PIN_L4, "L4"), /* IIC3_SCL/GPIO_4[10]/EVT_B[5]/USB2_DRVVBUS/FTM8_CH0 */
	PINCTRL_PIN(PIN_M4, "M4"), /* IIC3_SDA/GPIO_4[11]/EVT_B[6]/USB2_PWRFAULT/FTM8_CH1 */
	PINCTRL_PIN(PIN_M3, "M3"), /* IIC4_SCL/GPIO_4[12]/EVT_B[7]/USB3_DRVVBUS/FTM3_FAULT */
	PINCTRL_PIN(PIN_N3, "N3"), /* IIC4_SDA/GPIO_4[13]/EVT_B[8]/USB3_PWRFAULT/FTM_EXT3CLK */
};

/* Each pin is its own group */
static const char * const ls1046a_groups[] = { "L4", "M4", "M3", "N3" };

static int ls1046a_get_groups_count(struct pinctrl_dev *pcdev)
{
	return ARRAY_SIZE(ls1046a_pins);
}

static const char *ls1046a_get_group_name(struct pinctrl_dev *pcdev,
	unsigned int selector)
{
	return ls1046a_pins[selector].name;
}

static int ls1046a_get_group_pins(struct pinctrl_dev *pcdev,
	unsigned int selector, const unsigned int **pins, unsigned int *npins)
{
	*pins = &ls1046a_pins[selector].number;
	*npins = 1;
	return 0;
}

static const struct pinctrl_ops ls1046a_pinctrl_ops = {
	.get_groups_count = ls1046a_get_groups_count,
	.get_group_name = ls1046a_get_group_name,
	.get_group_pins = ls1046a_get_group_pins,
	.dt_node_to_map = pinconf_generic_dt_node_to_map_group,
	.dt_free_map = pinconf_generic_dt_free_map,
};

/* Every pin has the same set of functions */
#define FUNC_i2c	0
#define FUNC_gpio	1
#define FUNC_evt	2
#define FUNC_usb	3
#define FUNC_ftm	4

#define _PINFUNC(name) \
	[FUNC_##name] = PINCTRL_PINFUNCTION(#name, ls1046a_groups, ARRAY_SIZE(ls1046a_groups))
static const struct pinfunction ls1046a_functions[] = {
	_PINFUNC(i2c),
	_PINFUNC(gpio),
	_PINFUNC(evt),
	_PINFUNC(usb),
	_PINFUNC(ftm),
};

static int ls1046a_get_functions_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(ls1046a_functions);
}

static const char *ls1046a_get_function_name(struct pinctrl_dev *pctldev, unsigned int func)
{
	return ls1046a_functions[func].name;
}

static int ls1046a_get_function_groups(struct pinctrl_dev *pctldev, unsigned int func,
	const char * const **groups,
	unsigned int * const ngroups)
{
	*groups = ls1046a_functions[func].groups;
	*ngroups = ls1046a_functions[func].ngroups;
	return 0;
}

static int ls1046a_set_mux(struct pinctrl_dev *pcdev,
	unsigned int func, unsigned int pin)
{
	struct ls1046a_pinctrl_pdata *pd = pinctrl_dev_get_drvdata(pcdev);
	static const u32 cr0_reg_func[] = {
		[FUNC_i2c] = RCWPMUXCR0_FUNC_I2C,
		[FUNC_gpio] = RCWPMUXCR0_FUNC_GPIO,
		[FUNC_evt] = RCWPMUXCR0_FUNC_EVT,
		[FUNC_usb] = RCWPMUXCR0_FUNC_USB,
		[FUNC_ftm] = RCWPMUXCR0_FUNC_FTM,
	};
	static const unsigned int cr0_pin_shift[] = {
		[PIN_L4] = RCWPMUXCR0_IIC3_SCL_SHIFT,
		[PIN_M4] = RCWPMUXCR0_IIC3_SDA_SHIFT,
		[PIN_M3] = RCWPMUXCR0_IIC4_SCL_SHIFT,
		[PIN_N3] = RCWPMUXCR0_IIC4_SDA_SHIFT,
	};
	u32 cr0;

	if (pd->big_endian)
		cr0 = ioread32be(pd->cr0mem);
	else
		cr0 = ioread32(pd->cr0mem);

	unsigned int pin_shift = cr0_pin_shift[pin];
	u32 reg_func = cr0_reg_func[func];
	u32 newcr0 = (cr0 & ~RCWPMUXCR0_MASK(pin_shift)) |
		RCWPMUXCR0_FIELD(pin_shift, reg_func);

	if (pd->big_endian)
		iowrite32be(newcr0, pd->cr0mem);
	else
		iowrite32(newcr0, pd->cr0mem);
	return 0;
}

static const struct pinmux_ops ls1046a_pinmux_ops = {
	.get_functions_count = ls1046a_get_functions_count,
	.get_function_name = ls1046a_get_function_name,
	.get_function_groups = ls1046a_get_function_groups,
	.set_mux = ls1046a_set_mux,
};

static const struct pinctrl_desc ls1046a_pinctrl_desc = {
	.name = "ls1046a",
	.pins = ls1046a_pins,
	.npins = ARRAY_SIZE(ls1046a_pins),
	.pctlops = &ls1046a_pinctrl_ops,
	.pmxops = &ls1046a_pinmux_ops,
	.owner = THIS_MODULE,
};

static int ls1046a_pinctrl_probe(struct platform_device *pdev)
{
	struct ls1046a_pinctrl_pdata *pd;
	int ret;

	pd = devm_kzalloc(&pdev->dev, sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return -ENOMEM;
	platform_set_drvdata(pdev, pd);

	pd->big_endian = device_is_big_endian(&pdev->dev);

	/* SCFG PMUX0 */
	pd->cr0mem = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(pd->cr0mem))
		return PTR_ERR(pd->cr0mem);
	dev_dbg(&pdev->dev, "scfg pmuxcr0 at %px %s", pd->cr0mem,
		pd->big_endian ? "be" : "le");

	ret = devm_pinctrl_register_and_init(&pdev->dev, &ls1046a_pinctrl_desc,
		pd, &pd->pctl_dev);
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "Failed pinctrl init\n");

	pinctrl_enable(pd->pctl_dev);
	return ret;
}

static const struct of_device_id ls1046a_pinctrl_match_table[] = {
	{ .compatible = "fsl,ls1046a-pinctrl" },
	{ /* sentinel */ }
};

static struct platform_driver ls1046a_pinctrl_driver = {
	.driver = {
		.name = "ls1046a_pinctrl",
		.of_match_table = ls1046a_pinctrl_match_table,
	},
	.probe = ls1046a_pinctrl_probe,
};

builtin_platform_driver(ls1046a_pinctrl_driver)

MODULE_DESCRIPTION("LS1046A pinctrl driver");
MODULE_LICENSE("GPL");
