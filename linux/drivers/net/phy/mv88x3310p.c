/* MV88X3310P PHY drivers.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "linux/phy.h"
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/ethtool.h>
#include <linux/i2c.h>
#include <linux/sfp.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/gpio/consumer.h>
#include "mv88x3310p.h"

#include <asm/irq.h>

MODULE_DESCRIPTION("Marvell 88X3310P Driver");
MODULE_AUTHOR("Nexcom");
MODULE_LICENSE("GPL v2");

#define AW4G3				0x0
#define AW8G3				0x1
#define AW16G3				0x2
#define AW24G3				0x3

#define DIGI_PCBA_XA			0x0
#define DIGI_PCBA_XB			0x1
#define DIGI_PCBA_XC			0x2
#define DIGI_PCBA_XD			0x3

struct mv88x3310p {
	struct phy_device *phydev;
};

static int i2c_read_reg(struct i2c_adapter *adapter, u8 adr, u8 reg, u8 *val)
{
	struct i2c_msg msgs[2] = {{.addr = adr, .flags = 0,
			.buf = &reg, .len = 1 },
			{.addr = adr, .flags = I2C_M_RD,
			.buf = val, .len = 1 } };
	return (i2c_transfer(adapter, msgs ,2) == 2) ? 0 : -1;
}


static int mv88x3310p_sfp_insert(void *upstream, const struct sfp_eeprom_id *id)
{
	struct phy_device *phydev = upstream;
	struct mv88x3310p *info = phydev->priv;
	if (!info) {
		return -EINVAL;
	}

	__ETHTOOL_DECLARE_LINK_MODE_MASK(support) = { 0, };
	DECLARE_PHY_INTERFACE_MASK(interfaces);
	phy_interface_t iface;

	sfp_parse_support(phydev->sfp_bus, id, support, interfaces);
	iface = sfp_select_interface(phydev->sfp_bus, support);

	if (iface == PHY_INTERFACE_MODE_10GBASER) {
		/* MAC type: XFI with rate matching
		 * Fiber type: 10GBASE-R */
		phy_write_mmd(info->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x1e);
		/* Software reset */
		phy_write_mmd(info->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x801e);
	} else if ((iface == PHY_INTERFACE_MODE_1000BASEX) ||
		   (iface == PHY_INTERFACE_MODE_SGMII)) {
		/* MAC type: XFI with rate matching
		 * Fiber type: 1000BASE-X */
		phy_write_mmd(info->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x06);
		/* Software reset */
		phy_write_mmd(info->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x8006);
	} else {
		dev_err(&phydev->mdio.dev, "Incompatible SFP module inserted: %d\n", iface);
		return -EINVAL;
	}

	return 0;
}

static const struct sfp_upstream_ops mv88x3310p_sfp_ops = {
	.attach = phy_sfp_attach,
	.detach = phy_sfp_detach,
	.module_insert = mv88x3310p_sfp_insert,
};

static u8 model_name, hw_ver;
static int mv88x3310p_probe(struct phy_device *phydev)
{
	struct mv88x3310p *info;
	int err = -ENOMEM;
	u8 hw_info;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return err;

	i2c_read_reg(i2c_get_adapter(0), 0x59, 0x2, &hw_info);
	model_name = (hw_info & 0x60) >> 5;
	hw_ver = (hw_info & 0x0c) >> 2;

	info->phydev = phydev;
	phydev->priv = info;

	return phy_sfp_probe(phydev, &mv88x3310p_sfp_ops);
}

static int mv88x3310p_aneg_done(struct phy_device *phydev)
{
	int reg, regnum;

	reg = phy_read_mmd(phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG);
	switch (reg & CUNIT_FIBER_TYPE_MASK) {
	case 0x18:
		regnum = XUNIT_TGBASER_PCS_STATUS_REG;
		break;
	case 0x0:
		regnum = XUNIT_1000BASE_STATUS_REG;
		break;
	case 0x8:
	case 0x10:
	default:
		return 0;
	}

	phy_read_mmd(phydev, XUNIT_DEV, regnum);
	reg = phy_read_mmd(phydev, XUNIT_DEV, regnum);
	if (reg & MDIO_STAT1_LSTATUS) {
		return reg & MDIO_STAT1_LSTATUS;
	} else {
		if (phydev->autoneg == AUTONEG_DISABLE)
			return 1;
		phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_STATUS_REG);
		reg = phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_STATUS_REG);
		return (reg < 0) ? reg : (reg & BMSR_ANEGCOMPLETE);
	}

	return 0;
}

static int mv88x3310p_config_aneg(struct phy_device *phydev)
{
	int reg;
	linkmode_copy(phydev->supported, PHY_10GBIT_FULL_FEATURES);
	if (phydev->autoneg == AUTONEG_DISABLE) {
		switch (phydev->speed) {
		case SPEED_1000:
			phy_write_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_CTRL_REG, TUNIT_SPEED_1000);
			break;
		case SPEED_100:
			phy_write_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_CTRL_REG, TUNIT_SPEED_100);
			break;
		case SPEED_10:
			phy_write_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_CTRL_REG, TUNIT_SPEED_10);
			break;
		}

		reg = phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_1000_AN_CTRL_STA_REG);
		switch (phydev->duplex) {
		case DUPLEX_FULL:
			phy_write_mmd(phydev, TUNIT_AN_DEV, TUNIT_1000_AN_CTRL_STA_REG, reg | TUNIT_1000_DUPLEX_FULL_MASK);
			break;
		case DUPLEX_HALF:
			phy_write_mmd(phydev, TUNIT_AN_DEV, TUNIT_1000_AN_CTRL_STA_REG, reg & (~TUNIT_1000_DUPLEX_FULL_MASK));
			break;
		}
		phy_write_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_CTRL_REG, 0x0);
	}
	if (phydev->autoneg == AUTONEG_ENABLE) {
		linkmode_copy(phydev->advertising, phydev->supported);
		phy_write_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_CTRL_REG, TUNIT_SPEED_10000);
		return 0;
	}

	reg = phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_CTRL_REG);
	phy_write_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_CTRL_REG, reg | 0x8000 );
	return 0;
}

static int mv88x3310p_read_status(struct phy_device *phydev)
{
	int reg, regnum;
	int val = 0;

	/* For now just lie and say it's 10G all the time */
	phydev->link = 1;
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;

	/* Check MAC link status */
	phy_read_mmd(phydev, HUNIT_DEV, HUNIT_TGBASER_PCS_STATUS_1_REG);
	reg = phy_read_mmd(phydev, HUNIT_DEV, HUNIT_TGBASER_PCS_STATUS_1_REG);
	if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)) {
		phydev->link = 0;
		goto LinkDown;
	}

	/* Check fiber type */
	reg = phy_read_mmd(phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG);
	switch (reg & CUNIT_FIBER_TYPE_MASK) {
	case 0x18:
		regnum = XUNIT_TGBASER_PCS_STATUS_REG;
		val = CUNIT_GPIO_SFP_10G;
		break;
	case 0x0:
		regnum = XUNIT_1000BASE_STATUS_REG;
		phydev->speed = SPEED_1000;
		val = CUNIT_GPIO_SFP_1G;
		break;
	}

	/* Check fiber link status, otherwise copper link status */
	phy_read_mmd(phydev, XUNIT_DEV, regnum);
	reg = phy_read_mmd(phydev, XUNIT_DEV, regnum);
	if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)) {
		/* Read twice because link state is latched and a
		 * read moves the current state into the register
		 */
		phy_read_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_STATUS_REG);
		reg = phy_read_mmd(phydev, TUNIT_PMAPMD_DEV, TUNIT_PMAPMD_STATUS_REG);
		if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)) {
			phydev->link = 0;
			goto LinkDown;
		}
		
		phy_read_mmd(phydev, TUNIT_PCS_DEV, TUNIT_PCS_STATUS_REG);
		reg = phy_read_mmd(phydev, TUNIT_PCS_DEV, TUNIT_PCS_STATUS_REG);
		if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)) {
			phydev->link = 0;
			goto LinkDown;
		}
		
		phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_STATUS_REG);
		reg = phy_read_mmd(phydev, TUNIT_AN_DEV, TUNIT_AN_STATUS_REG);
		if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)) {
			phydev->link = 0;
			goto LinkDown;
		}

		phy_read_mmd(phydev, TUNIT_PCS_DEV, TUNIT_COPPER_STATUS_REG);
		reg = phy_read_mmd(phydev, TUNIT_PCS_DEV, TUNIT_COPPER_STATUS_REG);
		if (!(reg & 0x400)) {
			phydev->link = 0;
			goto LinkDown;
		} else {
			switch (reg >> 14) {
			case 0x0:
				phydev->speed = SPEED_10;
				val = 0;
				break;
			case 0x1:
				phydev->speed = SPEED_100;
				val = 0;
				break;
			case 0x2:
				phydev->speed = SPEED_1000;
				val = CUNIT_GPIO_COPPER_1G;
				break;
			case 0x3:
			default:
				phydev->speed = SPEED_10000;
				val = CUNIT_GPIO_COPPER_10G;
				break;
			}
			if ((reg & 0x800) == 0x800) {
				switch (reg & 0x2000) {
				case 0x0:
					phydev->duplex = DUPLEX_HALF;
					break;
				case 0x2000:
				default:
					phydev->duplex = DUPLEX_FULL;
				}
			}
		}
	}

	/* LED control */
	if (phydev->link) {
		phy_write_mmd(phydev, CUNIT_DEV, CUNIT_GPIO_DATA_REG, val);
		if (model_name == AW24G3 && hw_ver == DIGI_PCBA_XA) {
			switch (val) {
			case 0:
			case CUNIT_GPIO_COPPER_10G:
			case CUNIT_GPIO_COPPER_1G:
				/* Positive polarity, off */
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED0_CTRL_REG, 0x0001);
				/* Negative polarity, solid copper link, blink TX/RX activity  */
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED1_CTRL_REG, 0x0128);
				break;
			case CUNIT_GPIO_SFP_10G:
			case CUNIT_GPIO_SFP_1G:
				/* Positive polarity, solid fiber link, blink RX/TX activity */
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED0_CTRL_REG, 0x0131);
				/* Negative polarity, off */
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED1_CTRL_REG, 0x0000);
				break;
			}
		}
	}

	return 0;

LinkDown:
	phy_write_mmd(phydev, CUNIT_DEV, CUNIT_GPIO_DATA_REG, 0x0);
	return 0;
}

static int mv88x3310p_config_init(struct phy_device *phydev)
{
	/* Temporarily just say we support everything */
	linkmode_copy(phydev->supported, PHY_10GBIT_FULL_FEATURES);

	/* Positive polarity, solid link, blink RX/TX activity */
	phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED0_CTRL_REG, 0x0138);
	/* Negative polarity, solid SFP link */
	phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED1_CTRL_REG, 0x0031);
	/* Negative polarity, solid copper link */
	phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED2_CTRL_REG, 0x0029);

	/* Enable GPIO outputs */
	phy_write_mmd(phydev, CUNIT_DEV, CUNIT_GPIO_TRISTATE_REG,
		      CUNIT_GPIO_SFP_1G    | CUNIT_GPIO_SFP_10G |
		      CUNIT_GPIO_COPPER_1G | CUNIT_GPIO_COPPER_10G);


	return 0;
}

static struct phy_driver mv88x3310p_driver[] = { {
	.phy_id         = 0x002b09a0,
	.phy_id_mask    = 0xfffffff0,
	.name           = "Marvell 88X3310P",
//	.flags          = PHY_HAS_INTERRUPT,
	.probe		= mv88x3310p_probe,
	.config_init    = mv88x3310p_config_init,
	.features       = PHY_10GBIT_FULL_FEATURES,
	.aneg_done      = mv88x3310p_aneg_done,
	.config_aneg    = mv88x3310p_config_aneg,
	.read_status    = mv88x3310p_read_status,
} };

module_phy_driver(mv88x3310p_driver);

static struct mdio_device_id __maybe_unused marvell_alaska_tbl[] = {
	{0x002b09a0, 0xfffffff0},
	{}
};

MODULE_DEVICE_TABLE(mdio, marvell_alaska_tbl);
