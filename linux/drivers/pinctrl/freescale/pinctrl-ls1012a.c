// SPDX-License-Identifier: (GPL-2.0-only OR MIT)
/*
 * Pin controller for NXP QorIQ LS1012A.
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
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <linux/sys_soc.h>

struct ls1012a_pinctrl_pdata {
	struct pinctrl_dev *pctl_dev;
	void __iomem *cr0mem;
	bool big_endian;
	u32 ssc;
};

/* Macros that follow the datasheet's bit numbering scheme */
#define BIT_BE(b)	(0x80000000 >> ((b)%32))
#define BITV_BE(hi, v)	((v) << (31 - ((hi)%32)))
#define BITS_BE(hi, lo)	(((1<<(((hi)-(lo))+1))-1) << (31-((hi)%32)))
#define BIT_LE(b)	(0x00000001 << ((b)%32))
#define BITV_LE(b, v)	((v) << ((b)%32))
#define BITS_LE(lo, hi)	(((1<<(((hi)-(lo))+1))-1) << ((lo)%32))

/* SCFG PMUXCR0 pinmux control register */
#define SCFG_PMUXCR0			0x430
#define QSPI_MUX_OVRD_MASK		BIT_BE(0)	/* [0] */
#define QSPI_MUX_DISABLE		BITV_BE(0, 0)	/*  use RCW */
#define QSPI_MUX_ENABLE			BITV_BE(0, 1)	/*  use PMUXCR0 */
#define QSPI_DATA0_GPIO_OVR_MASK	BIT_BE(1)	/* [1] */
#define QSPI_DATA0_GPIO_SEL_SPI		BITV_BE(1, 0)	/*  DATA0,SCK,CS0 */
#define QSPI_DATA0_GPIO_SEL_GPIO	BITV_BE(1, 1)	/*  GPIO1[4,11,5] */
#define QSPI_DATA1_GPIO_OVR_MASK	BITS_BE(3, 2)	/* [3:2] */
#define QSPI_DATA1_GPIO_SEL_SPI		BITV_BE(3, 0)	/*  DATA1 */
#define QSPI_DATA1_GPIO_SEL_GPIO	BITV_BE(3, 1)	/*  GPIO1[12] */
#define QSPI_IIC2_OVR_MASK		BITS_BE(5, 4)	/* [5:4] */
#define QSPI_IIC2_SEL_GPIO		BITV_BE(5, 0)	/*  GPIO1[13,14] */
#define QSPI_IIC2_SEL_I2C		BITV_BE(5, 1)	/*  SCL,SDA (rev0) */
#define QSPI_IIC2_SEL_SPI		BITV_BE(5, 2)	/*  DATA2,DATA3 */

/* RCW SoC-specific configuration (read-only) */
#define DCFG_RCWSR			0x100
#define SOC_SPEC_CONFIG			416	/* word 13 */
#define DCFG_SSC_REG			(DCFG_RCWSR + SOC_SPEC_CONFIG / 8)
#define SSC_DATA0_GPIO_MASK		BIT_LE(421)	/* 421 */
#define SSC_DATA0_GPIO_SEL_SPI		BITV_LE(421, 0)	/*  DATA0,SCK,CS0 */
#define SSC_DATA0_GPIO_SEL_GPIO		BITV_LE(421, 1)	/*  GPIO1[11,4,5] */
#define SSC_DATA1_GPIO_MASK		BITS_LE(422, 423)/* 422-423 */
#define SSC_DATA1_GPIO_SEL_SPI		BITV_LE(422, 0)	/*  DATA1 */
#define SSC_DATA1_GPIO_SEL_GPIO		BITV_LE(422, 2)	/*  GPIO1[12] */
#define SSC_IIC2_MASK			BITS_LE(424, 425)/* 424-425 */
#define SSC_IIC2_SEL_GPIO		BITV_LE(424, 0)	/*  GPIO1[13,14] */
#define SSC_IIC2_SEL_I2C		BITV_LE(424, 2)	/*  SCL,SDA */
#define SSC_IIC2_SEL_SPI		BITV_LE(424, 1)	/*  DATA2,DATA3 */
#define SSC_IIC2_SEL_GPIO_RESET		BITV_LE(424, 3)	/*  GPIO1[13],RESET_REQ_B*/

const struct pinctrl_pin_desc ls1012a_pins[] = {
	PINCTRL_PIN(60, "QSPI_A_DATA3/GPIO1_14/IIC2_SDA/RESET_REQ_B"),
	PINCTRL_PIN(61, "QSPI_A_DATA1/GPIO1_12"),
	PINCTRL_PIN(62, "QSPI_A_SCK/GPIO1_04"),
	PINCTRL_PIN(122, "QSPI_A_DATA2/GPIO1_13/IIC2_SCL"),
	PINCTRL_PIN(123, "QSPI_A_DATA0/GPIO1_11"),
	PINCTRL_PIN(124, "QSPI_A_CS0/GPIO1_05"),
};

static const unsigned int qspi_1_grp[] = { 62, 123, 124 };
static const unsigned int qspi_2_grp[] = { 61 };
static const unsigned int qspi_3_grp[] = { 122, 60 };

#define GRP_qspi_1	0	/* SCK,CS0,DATA0 */
#define GRP_qspi_2	1	/* DATA1 */
#define GRP_qspi_3	2	/* DATA2,DATA3 */
#define _GRP_max	3

#define _PINGROUP(name) \
	[GRP_##name] = PINCTRL_PINGROUP(#name "_grp", name##_grp, ARRAY_SIZE(name##_grp))
static const struct pingroup ls1012a_groups[] = {
	_PINGROUP(qspi_1),
	_PINGROUP(qspi_2),
	_PINGROUP(qspi_3),
};


static void pctrl_reg_write(struct ls1012a_pinctrl_pdata *pdata, u32 val,
			    u32 __iomem *reg)
{
	if (pdata->big_endian)
		iowrite32be(val, reg);
	else
		iowrite32(val, reg);
}

static u32 pctrl_reg_read(struct ls1012a_pinctrl_pdata *pdata, u32 __iomem *reg)
{
	return pdata->big_endian ? ioread32be(reg) : ioread32(reg);
}

static int ls1012a_get_groups_count(struct pinctrl_dev *pcdev)
{
	return ARRAY_SIZE(ls1012a_groups);
}

static const char *ls1012a_get_group_name(struct pinctrl_dev *pcdev,
	unsigned int selector)
{
	return ls1012a_groups[selector].name;
}

static int ls1012a_get_group_pins(struct pinctrl_dev *pcdev,
	unsigned int selector, const unsigned int **pins, unsigned int *npins)
{
	*pins = ls1012a_groups[selector].pins;
	*npins = ls1012a_groups[selector].npins;
	return 0;
}

static const struct pinctrl_ops ls1012a_pinctrl_ops = {
	.get_groups_count = ls1012a_get_groups_count,
	.get_group_name = ls1012a_get_group_name,
	.get_group_pins = ls1012a_get_group_pins,
	.dt_node_to_map = pinconf_generic_dt_node_to_map_group,
	.dt_free_map = pinconf_generic_dt_free_map,
};

static const char * const i2c_groups[] = { "qspi_3_grp" };
static const char * const spi_groups[] = { "qspi_1_grp", "qspi_2_grp", "qspi_3_grp" };
static const char * const gpio_groups[] = { "qspi_1_grp", "qspi_2_grp", "qspi_3_grp" };
static const char * const gpio_reset_groups[] = { "qspi_3_grp" };

#define FUNC_i2c	0
#define FUNC_spi	1
#define FUNC_gpio	2
#define FUNC_gpio_reset 3
#define _FUNC_max	4

#define _PINFUNC(name) \
	[FUNC_##name] = PINCTRL_PINFUNCTION(#name, name##_groups, ARRAY_SIZE(name##_groups))
static const struct pinfunction ls1012a_functions[] = {
	_PINFUNC(i2c),
	_PINFUNC(spi),
	_PINFUNC(gpio),
	_PINFUNC(gpio_reset),
};

static int ls1012a_get_functions_count(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(ls1012a_functions);
}

static const char *ls1012a_get_function_name(struct pinctrl_dev *pctldev, unsigned int func)
{
	return ls1012a_functions[func].name;
}

static int ls1012a_get_function_groups(struct pinctrl_dev *pctldev, unsigned int func,
	const char * const **groups,
	unsigned int * const ngroups)
{
	*groups = ls1012a_functions[func].groups;
	*ngroups = ls1012a_functions[func].ngroups;
	return 0;
}

/*
 * LS1012A
 *    Group: qspi_1             qspi_2      qspi_3
 *           ================== =========== =============
 *    Pin:   62    123    124   61          122    60
 *           ----- ------ ----- ----------- ------ ------
 * i2c                                      SCL    SDA    (RCW only)
 * spi       SCK   DATA0
 * spi       SCK   DATA0        DATA1
 * spi       SCK   DATA0        DATA1       DATA2  DATA3
 * gpio      GPIO4 GPIO11 GPIO5
 * gpio                         GPIO12
 * gpio                                     GPIO13 GPIO14
 * gpio_reset                               GPIO13 REQ_B  (RCW only)
 */

static const struct ls1012a_func_mux {
	u32 cr0mask, cr0; /* mux control */
	u32 sscmask, ssc; /* equivalent in RCW */
} ls1012a_func_mux[_FUNC_max][_GRP_max] = {
	[FUNC_i2c] = {
		[GRP_qspi_3] = {
			.sscmask = SSC_IIC2_MASK,
			.ssc =     SSC_IIC2_SEL_I2C,
		},
	},
	[FUNC_spi] = {
		[GRP_qspi_1] = {
			.cr0mask = QSPI_DATA0_GPIO_OVR_MASK,
			.cr0 =     QSPI_DATA0_GPIO_SEL_SPI,
			.sscmask = SSC_DATA0_GPIO_MASK,
			.ssc =     SSC_DATA0_GPIO_SEL_SPI
		},
		[GRP_qspi_2] = {
			.cr0mask = QSPI_DATA1_GPIO_OVR_MASK,
			.cr0 =     QSPI_DATA1_GPIO_SEL_SPI,
			.sscmask = SSC_DATA1_GPIO_MASK,
			.ssc =     SSC_DATA1_GPIO_SEL_SPI,
		},
		[GRP_qspi_3] = {
			.cr0mask = QSPI_IIC2_OVR_MASK,
			.cr0 =     QSPI_IIC2_SEL_SPI,
			.sscmask = SSC_IIC2_MASK,
			.ssc =     SSC_IIC2_SEL_SPI,
		},
	},
	[FUNC_gpio] = {
		[GRP_qspi_1] = {
			.cr0mask = QSPI_DATA0_GPIO_OVR_MASK,
			.cr0 =     QSPI_DATA0_GPIO_SEL_GPIO,
			.sscmask = SSC_DATA0_GPIO_MASK,
			.ssc =     SSC_DATA0_GPIO_SEL_GPIO,
		},
		[GRP_qspi_2] = {
			.cr0mask = QSPI_DATA1_GPIO_OVR_MASK,
			.cr0 =     QSPI_DATA1_GPIO_SEL_GPIO,
			.sscmask = SSC_DATA1_GPIO_MASK,
			.ssc =     SSC_DATA1_GPIO_SEL_GPIO,
		},
		[GRP_qspi_3] = {
			.cr0mask = QSPI_IIC2_OVR_MASK,
			.cr0 =     QSPI_IIC2_SEL_GPIO,
			.sscmask = SSC_IIC2_MASK,
			.ssc =     SSC_IIC2_SEL_GPIO,
		},
	},
	[FUNC_gpio_reset] = {
		[GRP_qspi_3] = {
			.sscmask = SSC_IIC2_MASK,
			.ssc =     SSC_IIC2_SEL_GPIO_RESET,
		},
	},
};

static int ls1012a_set_mux(struct pinctrl_dev *pcdev,
	unsigned int func, unsigned int group)
{
	struct ls1012a_pinctrl_pdata *pd = pinctrl_dev_get_drvdata(pcdev);
	const struct ls1012a_func_mux *fm = &ls1012a_func_mux[func][group];
	u32 cr0 = pctrl_reg_read(pd, pd->cr0mem);

	if (!fm->cr0mask) {
		if ((pd->ssc & fm->sscmask) != fm->ssc)
			return -EOPNOTSUPP;
		cr0 = (cr0 & ~QSPI_MUX_OVRD_MASK) | QSPI_MUX_DISABLE;
	} else {
		cr0 = (cr0 & ~fm->cr0mask) | fm->cr0;
		if ((pd->ssc & fm->sscmask) != fm->ssc)
			cr0 = (cr0 & ~QSPI_MUX_OVRD_MASK) | QSPI_MUX_ENABLE;
	}

	pctrl_reg_write(pd, cr0, pd->cr0mem);

	return 0;
}

static void ls1012a_init_mux(struct ls1012a_pinctrl_pdata *pd)
{
	unsigned int func, group;
	const struct ls1012a_func_mux *fm;
	u32 cr0;

	cr0 = pctrl_reg_read(pd, pd->cr0mem);
	if ((cr0 & QSPI_MUX_OVRD_MASK) == QSPI_MUX_DISABLE) {
		/*
		 * Prepare a disabled MUXCR0 to have a same/similar
		 * configuration as RCW SSC, and leave it disabled.
		 */
		for (func = 0; func < _FUNC_max; func++) {
			for (group = 0; group < _GRP_max; group++) {
				fm = &ls1012a_func_mux[func][group];
				if (fm->sscmask &&
				    fm->ssc == (pd->ssc & fm->sscmask)) {
					cr0 &= ~fm->cr0mask;
					cr0 |= fm->cr0;
				}
			}
		}
		pctrl_reg_write(pd, cr0, pd->cr0mem);
	}
}

static const struct pinmux_ops ls1012a_pinmux_ops = {
	.get_functions_count = ls1012a_get_functions_count,
	.get_function_name = ls1012a_get_function_name,
	.get_function_groups = ls1012a_get_function_groups,
	.set_mux = ls1012a_set_mux,
};

static struct pinctrl_desc ls1012a_pinctrl_desc = {
	.name = "ls1012a",
	.pins = ls1012a_pins,
	.npins = ARRAY_SIZE(ls1012a_pins),
	.pctlops = &ls1012a_pinctrl_ops,
	.pmxops = &ls1012a_pinmux_ops,
	.owner = THIS_MODULE,
};

static int ls1012a_pinctrl_probe(struct platform_device *pdev)
{
	struct ls1012a_pinctrl_pdata *pd;
	int ret;
	u32 dcfg_ssc;
	struct regmap *dcfg_regmap;

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

	/* DCFG RCW SSC */
	dcfg_regmap = syscon_regmap_lookup_by_phandle(
		dev_of_node(&pdev->dev), "dcfg-regmap");
	if (IS_ERR(dcfg_regmap)) {
		dev_err(&pdev->dev, "dcfg regmap: %pe\n", dcfg_regmap);
		return -EINVAL;
	}
	ret = regmap_read(dcfg_regmap, DCFG_SSC_REG, &dcfg_ssc);
	if (ret) {
		dev_err(&pdev->dev, "dcfg-regmap read: %d\n", ret);
		return ret;
	}
	pd->ssc = swab32(dcfg_ssc); /* untwist RCW fields */

	dev_dbg(&pdev->dev, "dcfg ssc = %08x (grp1=%s grp2=%s grp3=%s)\n",
		pd->ssc,
		(pd->ssc & SSC_DATA0_GPIO_MASK) == SSC_DATA0_GPIO_SEL_SPI ? "spi" : "gpio",
		(pd->ssc & SSC_DATA1_GPIO_MASK) == SSC_DATA1_GPIO_SEL_SPI ? "spi"
		: (pd->ssc & SSC_DATA1_GPIO_MASK) == SSC_DATA1_GPIO_SEL_GPIO ? "gpio"
		: (pd->ssc & SSC_DATA1_GPIO_MASK) == 0x80 ? "10" : "11",
		(pd->ssc & SSC_IIC2_MASK) == SSC_IIC2_SEL_GPIO ? "gpio"
		: (pd->ssc & SSC_IIC2_MASK) == SSC_IIC2_SEL_I2C ? "i2c"
		: (pd->ssc & SSC_IIC2_MASK) == SSC_IIC2_SEL_SPI ? "spi"
		: "gpio+reset");

	ls1012a_init_mux(pd);

	ret = devm_pinctrl_register_and_init(&pdev->dev, &ls1012a_pinctrl_desc,
		pd, &pd->pctl_dev);
	if (ret)
		return dev_err_probe(&pdev->dev, ret, "Failed pinctrl init\n");

	pinctrl_enable(pd->pctl_dev);
	return ret;
}

static const struct of_device_id ls1012a_pinctrl_match_table[] = {
	{ .compatible = "fsl,ls1012a-pinctrl" },
	{ /* sentinel */ }
};

static struct platform_driver ls1012a_pinctrl_driver = {
	.driver = {
		.name = "ls1012a_pinctrl",
		.of_match_table = ls1012a_pinctrl_match_table,
	},
	.probe = ls1012a_pinctrl_probe,
};

builtin_platform_driver(ls1012a_pinctrl_driver)

MODULE_DESCRIPTION("LS1012A pinctrl driver");
MODULE_LICENSE("GPL");
