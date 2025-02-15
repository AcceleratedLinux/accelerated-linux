/*
 *  Atheros AR71xx SoC platform devices
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros 2.6.15 BSP
 *  Parts of this file are based on Atheros 2.6.31 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/irq.h>

#include "common.h"
#include "dev-eth.h"

unsigned char ath79_mac_base[ETH_ALEN] __initdata;

static struct resource ath79_mdio0_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}
};

static struct ag71xx_mdio_platform_data ath79_mdio0_data;

struct platform_device ath79_mdio0_device = {
	.name		= "ag71xx-mdio",
	.id		= 0,
	.resource	= ath79_mdio0_resources,
	.num_resources	= ARRAY_SIZE(ath79_mdio0_resources),
	.dev = {
		.platform_data = &ath79_mdio0_data,
	},
};

static struct resource ath79_mdio1_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}
};

static struct ag71xx_mdio_platform_data ath79_mdio1_data;

struct platform_device ath79_mdio1_device = {
	.name		= "ag71xx-mdio",
	.id		= 1,
	.resource	= ath79_mdio1_resources,
	.num_resources	= ARRAY_SIZE(ath79_mdio1_resources),
	.dev = {
		.platform_data = &ath79_mdio1_data,
	},
};

static void ath79_set_pll(u32 cfg_reg, u32 pll_reg, u32 pll_val, u32 shift)
{
	void __iomem *base;
	u32 t;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	t = __raw_readl(base + cfg_reg);
	t &= ~(3 << shift);
	t |=  (2 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	__raw_writel(pll_val, base + pll_reg);

	t |= (3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	t &= ~(3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	printk(KERN_DEBUG "ar71xx: pll_reg %#x: %#x\n",
		(unsigned int)(base + pll_reg), __raw_readl(base + pll_reg));

	iounmap(base);
}

static void __init ath79_mii_ctrl_set_if(unsigned int reg,
					  unsigned int mii_if)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(AR71XX_MII_CTRL_IF_MASK);
	t |= (mii_if & AR71XX_MII_CTRL_IF_MASK);
	__raw_writel(t, base + reg);

	iounmap(base);
}

static void ath79_mii_ctrl_set_speed(unsigned int reg, unsigned int speed)
{
	void __iomem *base;
	unsigned int mii_speed;
	u32 t;

	switch (speed) {
	case SPEED_10:
		mii_speed =  AR71XX_MII_CTRL_SPEED_10;
		break;
	case SPEED_100:
		mii_speed =  AR71XX_MII_CTRL_SPEED_100;
		break;
	case SPEED_1000:
		mii_speed =  AR71XX_MII_CTRL_SPEED_1000;
		break;
	default:
		BUG();
	}

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(AR71XX_MII_CTRL_SPEED_MASK << AR71XX_MII_CTRL_SPEED_SHIFT);
	t |= mii_speed  << AR71XX_MII_CTRL_SPEED_SHIFT;
	__raw_writel(t, base + reg);

	iounmap(base);
}

void __init ath79_register_mdio(unsigned int id, u32 phy_mask)
{
	struct platform_device *mdio_dev;
	struct ag71xx_mdio_platform_data *mdio_data;
	unsigned int max_id;

	if (ath79_soc == ATH79_SOC_AR9341 ||
	    ath79_soc == ATH79_SOC_AR9342 ||
	    ath79_soc == ATH79_SOC_AR9344)
		max_id = 1;
	else
		max_id = 0;

	if (id > max_id) {
		printk(KERN_ERR "ar71xx: invalid MDIO id %u\n", id);
		return;
	}

	switch (ath79_soc) {
	case ATH79_SOC_AR7241:
	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		mdio_dev = &ath79_mdio1_device;
		mdio_data = &ath79_mdio1_data;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
		if (id == 0) {
			mdio_dev = &ath79_mdio0_device;
			mdio_data = &ath79_mdio0_data;
		} else {
			mdio_dev = &ath79_mdio1_device;
			mdio_data = &ath79_mdio1_data;
		}
		break;

	case ATH79_SOC_AR7242:
		ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG,
			       AR7242_PLL_REG_ETH0_INT_CLOCK, 0x62000000,
			       AR71XX_ETH0_PLL_SHIFT);
		/* fall through */
	default:
		mdio_dev = &ath79_mdio0_device;
		mdio_data = &ath79_mdio0_data;
		break;
	}

	mdio_data->phy_mask = phy_mask;

	switch (ath79_soc) {
	case ATH79_SOC_AR7240:
	case ATH79_SOC_AR7241:
	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		mdio_data->is_ar7240 = 1;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
		if (id == 1)
			mdio_data->is_ar7240 = 1;
		break;

	default:
		break;
	}

	platform_device_register(mdio_dev);
}

struct ath79_eth_pll_data ath79_eth0_pll_data;
struct ath79_eth_pll_data ath79_eth1_pll_data;

static u32 ath79_get_eth_pll(unsigned int mac, int speed)
{
	struct ath79_eth_pll_data *pll_data;
	u32 pll_val;

	switch (mac) {
	case 0:
		pll_data = &ath79_eth0_pll_data;
		break;
	case 1:
		pll_data = &ath79_eth1_pll_data;
		break;
	default:
		BUG();
	}

	switch (speed) {
	case SPEED_10:
		pll_val = pll_data->pll_10;
		break;
	case SPEED_100:
		pll_val = pll_data->pll_100;
		break;
	case SPEED_1000:
		pll_val = pll_data->pll_1000;
		break;
	default:
		BUG();
	}

	return pll_val;
}

static void ath79_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);

	ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH0_INT_CLOCK,
			val, AR71XX_ETH0_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII0_CTRL, speed);
}

static void ath79_set_speed_ge1(int speed)
{
	u32 val = ath79_get_eth_pll(1, speed);

	ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH1_INT_CLOCK,
			 val, AR71XX_ETH1_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII1_CTRL, speed);
}

static void ar7242_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);
	void __iomem *base;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR7242_PLL_REG_ETH0_INT_CLOCK);
	iounmap(base);
}

static void ar91xx_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);

	ath79_set_pll(AR913X_PLL_REG_ETH_CONFIG, AR913X_PLL_REG_ETH0_INT_CLOCK,
			 val, AR913X_ETH0_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII0_CTRL, speed);
}

static void ar91xx_set_speed_ge1(int speed)
{
	u32 val = ath79_get_eth_pll(1, speed);

	ath79_set_pll(AR913X_PLL_REG_ETH_CONFIG, AR913X_PLL_REG_ETH1_INT_CLOCK,
			 val, AR913X_ETH1_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII1_CTRL, speed);
}

static void ar934x_set_speed_ge0(int speed)
{
	void __iomem *base;
	u32 val = ath79_get_eth_pll(0, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR934X_PLL_ETH_XMII_CONTROL_REG);
	iounmap(base);
}

static void ath79_set_speed_dummy(int speed)
{
}

static void ath79_ddr_no_flush(void)
{
}

static void ath79_ddr_flush_ge0(void)
{
	ath79_ddr_wb_flush(AR71XX_DDR_REG_FLUSH_GE0);
}

static void ath79_ddr_flush_ge1(void)
{
	ath79_ddr_wb_flush(AR71XX_DDR_REG_FLUSH_GE1);
}

static void ar724x_ddr_flush_ge0(void)
{
	ath79_ddr_wb_flush(AR724X_DDR_REG_FLUSH_GE0);
}

static void ar724x_ddr_flush_ge1(void)
{
	ath79_ddr_wb_flush(AR724X_DDR_REG_FLUSH_GE1);
}

static void ar91xx_ddr_flush_ge0(void)
{
	ath79_ddr_wb_flush(AR913X_DDR_REG_FLUSH_GE0);
}

static void ar91xx_ddr_flush_ge1(void)
{
	ath79_ddr_wb_flush(AR913X_DDR_REG_FLUSH_GE1);
}

static void ar933x_ddr_flush_ge0(void)
{
	ath79_ddr_wb_flush(AR933X_DDR_REG_FLUSH_GE0);
}

static void ar933x_ddr_flush_ge1(void)
{
	ath79_ddr_wb_flush(AR933X_DDR_REG_FLUSH_GE1);
}

#define ATH79_CPU_IRQ_GE0	(MIPS_CPU_IRQ_BASE + 4)
#define ATH79_CPU_IRQ_GE1	(MIPS_CPU_IRQ_BASE + 5)

static struct resource ath79_eth0_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= ATH79_CPU_IRQ_GE0,
		.end	= ATH79_CPU_IRQ_GE0,
	},
};

struct ag71xx_platform_data ath79_eth0_data = {
	.reset_bit	= AR71XX_RESET_GE0_MAC,
};

struct platform_device ath79_eth0_device = {
	.name		= "ag71xx",
	.id		= 0,
	.resource	= ath79_eth0_resources,
	.num_resources	= ARRAY_SIZE(ath79_eth0_resources),
	.dev = {
		.platform_data = &ath79_eth0_data,
	},
};

static struct resource ath79_eth1_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= ATH79_CPU_IRQ_GE1,
		.end	= ATH79_CPU_IRQ_GE1,
	},
};

struct ag71xx_platform_data ath79_eth1_data = {
	.reset_bit	= AR71XX_RESET_GE1_MAC,
};

struct platform_device ath79_eth1_device = {
	.name		= "ag71xx",
	.id		= 1,
	.resource	= ath79_eth1_resources,
	.num_resources	= ARRAY_SIZE(ath79_eth1_resources),
	.dev = {
		.platform_data = &ath79_eth1_data,
	},
};

struct ag71xx_switch_platform_data ath79_switch_data;

#define AR71XX_PLL_VAL_1000	0x00110000
#define AR71XX_PLL_VAL_100	0x00001099
#define AR71XX_PLL_VAL_10	0x00991099

#define AR724X_PLL_VAL_1000	0x00110000
#define AR724X_PLL_VAL_100	0x00001099
#define AR724X_PLL_VAL_10	0x00991099

#define AR7242_PLL_VAL_1000	0x16000000
#define AR7242_PLL_VAL_100	0x00000101
#define AR7242_PLL_VAL_10	0x00001616

#define AR913X_PLL_VAL_1000	0x1a000000
#define AR913X_PLL_VAL_100	0x13000a44
#define AR913X_PLL_VAL_10	0x00441099

#define AR933X_PLL_VAL_1000	0x00110000
#define AR933X_PLL_VAL_100	0x00001099
#define AR933X_PLL_VAL_10	0x00991099

#define AR934X_PLL_VAL_1000	0x16000000
#define AR934X_PLL_VAL_100	0x00000101
#define AR934X_PLL_VAL_10	0x00001616

static void __init ath79_init_eth_pll_data(unsigned int id)
{
	struct ath79_eth_pll_data *pll_data;
	u32 pll_10, pll_100, pll_1000;

	switch (id) {
	case 0:
		pll_data = &ath79_eth0_pll_data;
		break;
	case 1:
		pll_data = &ath79_eth1_pll_data;
		break;
	default:
		BUG();
	}

	switch (ath79_soc) {
	case ATH79_SOC_AR7130:
	case ATH79_SOC_AR7141:
	case ATH79_SOC_AR7161:
		pll_10 = AR71XX_PLL_VAL_10;
		pll_100 = AR71XX_PLL_VAL_100;
		pll_1000 = AR71XX_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR7240:
	case ATH79_SOC_AR7241:
		pll_10 = AR724X_PLL_VAL_10;
		pll_100 = AR724X_PLL_VAL_100;
		pll_1000 = AR724X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR7242:
		pll_10 = AR7242_PLL_VAL_10;
		pll_100 = AR7242_PLL_VAL_100;
		pll_1000 = AR7242_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9130:
	case ATH79_SOC_AR9132:
		pll_10 = AR913X_PLL_VAL_10;
		pll_100 = AR913X_PLL_VAL_100;
		pll_1000 = AR913X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		pll_10 = AR933X_PLL_VAL_10;
		pll_100 = AR933X_PLL_VAL_100;
		pll_1000 = AR933X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
		pll_10 = AR934X_PLL_VAL_10;
		pll_100 = AR934X_PLL_VAL_100;
		pll_1000 = AR934X_PLL_VAL_1000;
		break;

	default:
		BUG();
	}

	if (!pll_data->pll_10)
		pll_data->pll_10 = pll_10;

	if (!pll_data->pll_100)
		pll_data->pll_100 = pll_100;

	if (!pll_data->pll_1000)
		pll_data->pll_1000 = pll_1000;
}

static int __init ath79_setup_phy_if_mode(unsigned int id,
					   struct ag71xx_platform_data *pdata)
{
	unsigned int mii_if;

	switch (id) {
	case 0:
		switch (ath79_soc) {
		case ATH79_SOC_AR7130:
		case ATH79_SOC_AR7141:
		case ATH79_SOC_AR7161:
		case ATH79_SOC_AR9130:
		case ATH79_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
				mii_if = AR71XX_MII0_CTRL_IF_MII;
				break;
			case PHY_INTERFACE_MODE_GMII:
				mii_if = AR71XX_MII0_CTRL_IF_GMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = AR71XX_MII0_CTRL_IF_RGMII;
				break;
			case PHY_INTERFACE_MODE_RMII:
				mii_if = AR71XX_MII0_CTRL_IF_RMII;
				break;
			default:
				return -EINVAL;
			}
			ath79_mii_ctrl_set_if(AR71XX_MII_REG_MII0_CTRL, mii_if);
			break;

		case ATH79_SOC_AR7240:
		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_MII;
			break;

		case ATH79_SOC_AR7242:
			/* FIXME */

		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_RMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	case 1:
		switch (ath79_soc) {
		case ATH79_SOC_AR7130:
		case ATH79_SOC_AR7141:
		case ATH79_SOC_AR7161:
		case ATH79_SOC_AR9130:
		case ATH79_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_RMII:
				mii_if = AR71XX_MII1_CTRL_IF_RMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = AR71XX_MII1_CTRL_IF_RGMII;
				break;
			default:
				return -EINVAL;
			}
			ath79_mii_ctrl_set_if(AR71XX_MII_REG_MII1_CTRL, mii_if);
			break;

		case ATH79_SOC_AR7240:
		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_GMII;
			break;

		case ATH79_SOC_AR7242:
			/* FIXME */

		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	}

	return 0;
}

static int ath79_eth_instance __initdata;
void __init ath79_register_eth(unsigned int id)
{
	struct platform_device *pdev;
	struct ag71xx_platform_data *pdata;
	int err;

	if (id > 1) {
		printk(KERN_ERR "ar71xx: invalid ethernet id %d\n", id);
		return;
	}

	ath79_init_eth_pll_data(id);

	if (id == 0)
		pdev = &ath79_eth0_device;
	else
		pdev = &ath79_eth1_device;

	pdata = pdev->dev.platform_data;

	err = ath79_setup_phy_if_mode(id, pdata);
	if (err) {
		printk(KERN_ERR
		       "ar71xx: invalid PHY interface mode for GE%u\n", id);
		return;
	}

	switch (ath79_soc) {
	case ATH79_SOC_AR7130:
		if (id == 0) {
			pdata->ddr_flush = ath79_ddr_flush_ge0;
			pdata->set_speed = ath79_set_speed_ge0;
		} else {
			pdata->ddr_flush = ath79_ddr_flush_ge1;
			pdata->set_speed = ath79_set_speed_ge1;
		}
		break;

	case ATH79_SOC_AR7141:
	case ATH79_SOC_AR7161:
		if (id == 0) {
			pdata->ddr_flush = ath79_ddr_flush_ge0;
			pdata->set_speed = ath79_set_speed_ge0;
		} else {
			pdata->ddr_flush = ath79_ddr_flush_ge1;
			pdata->set_speed = ath79_set_speed_ge1;
		}
		pdata->has_gbit = 1;
		break;

	case ATH79_SOC_AR7242:
		if (id == 0) {
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO |
					    AR71XX_RESET_GE0_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge0;
			pdata->set_speed = ar7242_set_speed_ge0;
		} else {
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO |
					    AR71XX_RESET_GE1_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge1;
			pdata->set_speed = ath79_set_speed_dummy;
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case ATH79_SOC_AR7241:
		if (id == 0)
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO;
		else
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO;
		/* fall through */
	case ATH79_SOC_AR7240:
		if (id == 0) {
			pdata->reset_bit |= AR71XX_RESET_GE0_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge0;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit |= AR71XX_RESET_GE1_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge1;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ath79_switch_data;

			ath79_switch_data.phy_poll_mask |= BIT(4);
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		if (ath79_soc == ATH79_SOC_AR7240)
			pdata->is_ar7240 = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case ATH79_SOC_AR9130:
		if (id == 0) {
			pdata->ddr_flush = ar91xx_ddr_flush_ge0;
			pdata->set_speed = ar91xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar91xx_ddr_flush_ge1;
			pdata->set_speed = ar91xx_set_speed_ge1;
		}
		pdata->is_ar91xx = 1;
		break;

	case ATH79_SOC_AR9132:
		if (id == 0) {
			pdata->ddr_flush = ar91xx_ddr_flush_ge0;
			pdata->set_speed = ar91xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar91xx_ddr_flush_ge1;
			pdata->set_speed = ar91xx_set_speed_ge1;
		}
		pdata->is_ar91xx = 1;
		pdata->has_gbit = 1;
		break;

	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		if (id == 0) {
			pdata->reset_bit = AR933X_RESET_GE0_MAC |
					   AR933X_RESET_GE0_MDIO;
			pdata->ddr_flush = ar933x_ddr_flush_ge0;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit = AR933X_RESET_GE1_MAC |
					   AR933X_RESET_GE1_MDIO;
			pdata->ddr_flush = ar933x_ddr_flush_ge1;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ath79_switch_data;

			ath79_switch_data.phy_poll_mask |= BIT(4);
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
		if (id == 0) {
			pdata->reset_bit = AR934X_RESET_GE0_MAC |
					   AR934X_RESET_GE0_MDIO;
			pdata->set_speed = ar934x_set_speed_ge0;
		} else {
			pdata->reset_bit = AR934X_RESET_GE1_MAC |
					   AR934X_RESET_GE1_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->switch_data = &ath79_switch_data;

			/* reset the built-in switch */
			ath79_device_reset_set(AR934X_RESET_ETH_SWITCH);
			ath79_device_reset_clear(AR934X_RESET_ETH_SWITCH);
		}

		pdata->ddr_flush = ath79_ddr_no_flush;
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	default:
		BUG();
	}

	switch (pdata->phy_if_mode) {
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RGMII:
		if (!pdata->has_gbit) {
			printk(KERN_ERR "ar71xx: no gbit available on eth%d\n",
					id);
			return;
		}
		/* fallthrough */
	default:
		break;
	}

	if (!is_valid_ether_addr(pdata->mac_addr)) {
		eth_random_addr(pdata->mac_addr);
		printk(KERN_DEBUG
			"ar71xx: using random MAC address for eth%d\n",
			ath79_eth_instance);
	}

	if (pdata->mii_bus_dev == NULL) {
		switch (ath79_soc) {
		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
			if (id == 0)
				pdata->mii_bus_dev = &ath79_mdio0_device.dev;
			else
				pdata->mii_bus_dev = &ath79_mdio1_device.dev;
			break;

		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
			pdata->mii_bus_dev = &ath79_mdio1_device.dev;
			break;

		default:
			pdata->mii_bus_dev = &ath79_mdio0_device.dev;
			break;
		}
	}

	/* Reset the device */
	ath79_device_reset_set(pdata->reset_bit);
	mdelay(100);

	ath79_device_reset_clear(pdata->reset_bit);
	mdelay(100);

	platform_device_register(pdev);
	ath79_eth_instance++;
}

void __init ath79_set_mac_base(unsigned char *mac)
{
	memcpy(ath79_mac_base, mac, ETH_ALEN);
}

void __init ath79_parse_mac_addr(char *mac_str)
{
	u8 tmp[ETH_ALEN];
	int t;

	t = sscanf(mac_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
			&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);

	if (t != ETH_ALEN)
		t = sscanf(mac_str, "%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx",
			&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);

	if (t == ETH_ALEN)
		ath79_set_mac_base(tmp);
	else
		printk(KERN_DEBUG "ar71xx: failed to parse mac address "
				"\"%s\"\n", mac_str);
}

static int __init ath79_ethaddr_setup(char *str)
{
	ath79_parse_mac_addr(str);
	return 1;
}
__setup("ethaddr=", ath79_ethaddr_setup);

static int __init ath79_kmac_setup(char *str)
{
	ath79_parse_mac_addr(str);
	return 1;
}
__setup("kmac=", ath79_kmac_setup);

void __init ath79_init_mac(unsigned char *dst, const unsigned char *src,
			    int offset)
{
	int t;

	if (!is_valid_ether_addr(src)) {
		memset(dst, '\0', ETH_ALEN);
		return;
	}

	t = (((u32) src[3]) << 16) + (((u32) src[4]) << 8) + ((u32) src[5]);
	t += offset;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = (t >> 16) & 0xff;
	dst[4] = (t >> 8) & 0xff;
	dst[5] = t & 0xff;
}

void __init ath79_init_local_mac(unsigned char *dst, const unsigned char *src)
{
	int i;

	if (!is_valid_ether_addr(src)) {
		memset(dst, '\0', ETH_ALEN);
		return;
	}

	for (i = 0; i < ETH_ALEN; i++)
		dst[i] = src[i];
	dst[0] |= 0x02;
}
