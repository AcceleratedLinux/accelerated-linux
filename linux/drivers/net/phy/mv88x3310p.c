/* MV88X3310P PHY drivers.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/ethtool.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include "../../gpio/gpiolib.h"
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

#define SFP_1000BASE_MASK		0X0b
#define SFP_TGBASE_MASK			0xf0

#define FSL_LS1046ARDB_GPIO_BASE	384
#define FSL_GPIO_NUM(g, n)		(FSL_LS1046ARDB_GPIO_BASE+32*(4-g)+n)
#define PHY1_SFP_GPIO			FSL_GPIO_NUM(1, 24)
#define PHY2_SFP_GPIO			FSL_GPIO_NUM(1, 25)

struct sfp_module {
	struct phy_device *phydev;
	struct i2c_adapter *sfp_i2c_adapter;
	int gpio;
	int i2c_bus;
	int i2c_addr;
	int irq;
	struct delayed_work sfp_queue;
};

static int i2c_read_reg(struct i2c_adapter *adapter, u8 adr, u8 reg, u8 *val)
{
	struct i2c_msg msgs[2] = {{.addr = adr, .flags = 0,
			.buf = &reg, .len = 1 },
			{.addr = adr, .flags = I2C_M_RD,
			.buf = val, .len = 1 } };
	return (i2c_transfer(adapter, msgs ,2) == 2) ? 0 : -1;
}

static int sfp_module_read_byte(struct sfp_module *sfp, u8 reg, u8 *val)
{
	return i2c_read_reg(sfp->sfp_i2c_adapter, sfp->i2c_addr, reg, val);
}

static void phy_fiber_type_select(struct sfp_module *sfp)
{
	uint8_t tg_code, gigabit_code;

	sfp_module_read_byte(sfp, 0x3, &tg_code);
	sfp_module_read_byte(sfp, 0x6, &gigabit_code);
	if ((gigabit_code & SFP_1000BASE_MASK) && !(tg_code & SFP_TGBASE_MASK)) {
		/* 1G mode */
		phy_write_mmd(sfp->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x06);
		phy_write_mmd(sfp->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x8006);
	} else if (!(gigabit_code & SFP_1000BASE_MASK) && (tg_code & SFP_TGBASE_MASK)) {
		/* 10G mode */
		phy_write_mmd(sfp->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x1e);
		phy_write_mmd(sfp->phydev, CUNIT_DEV, CUNIT_PORT_CTRL_REG, 0x801e);
	}
}

static void fiber_type_select(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct sfp_module *sfp = 
		container_of(dwork, struct sfp_module, sfp_queue);

	phy_fiber_type_select(sfp);
	enable_irq(sfp->irq);
}

static irqreturn_t sfp_interrupt_handler(int irq, void* data)
{
	struct sfp_module *sfp = data;

	disable_irq_nosync(irq);
	queue_delayed_work(system_power_efficient_wq, &sfp->sfp_queue, msecs_to_jiffies(1000));

	return IRQ_HANDLED;
}

static int sfp_interrupt_init(struct sfp_module *sfp)
{
	int ret = 0, value = 1;

	ret = gpio_request_one(sfp->gpio, GPIOF_DIR_IN, "Fiber_Type_Select");
	if (ret) {
		printk(KERN_ERR "Failed to request gpio %d\n", sfp->gpio);
		return ret;
	}

	value = gpio_get_value(sfp->gpio);
	if(!value)
		phy_fiber_type_select(sfp);

	sfp->irq = gpio_to_irq(sfp->gpio);
	ret = irq_set_irq_type(sfp->irq, IRQ_TYPE_EDGE_FALLING);
	if (ret) {
		printk(KERN_ERR "Failed to set IRQ(%d) type for gpio %d\n", sfp->irq, sfp->gpio);
		return ret;
	}

	ret = request_irq(sfp->irq, sfp_interrupt_handler, 0, "sfp_interrupt", sfp);
	if (ret) {
		printk(KERN_EMERG "Cannot initialize Fiber IRQ (%d) \n", ret);
		return ret;
	}

	return 0;
}
	u8 hw_info, model_name, hw_ver;

static int mv88x3310p_probe(struct phy_device *phydev)
{
	struct sfp_module *sfp;
	int err = -ENOMEM;

	sfp = kzalloc(sizeof(struct sfp_module), GFP_KERNEL);
	if (!sfp)
		return err;

	i2c_read_reg(i2c_get_adapter(0), 0x59, 0x2, &hw_info);
	model_name = (hw_info & 0x60) >> 5;
	hw_ver = (hw_info & 0x0c) >> 2;

	sfp->phydev = phydev;
	phydev->priv = sfp;
	switch (phydev->mdio.addr) {
	case 0x1c:
		sfp->gpio = PHY1_SFP_GPIO;
		sfp->i2c_bus = 0;
		break;
	case 0x1d:
		if (model_name == AW8G3 && hw_ver == DIGI_PCBA_XA) {
			sfp->gpio = PHY1_SFP_GPIO;
			sfp->i2c_bus = 0;
		} else {
			sfp->gpio = PHY2_SFP_GPIO;
			sfp->i2c_bus = 1;
		}
		break;
	default:
		break;
	}

	sfp->i2c_addr = 0x50;
	sfp->sfp_i2c_adapter = i2c_get_adapter(sfp->i2c_bus);
	if (sfp->sfp_i2c_adapter) {
		sfp_interrupt_init(sfp);
		INIT_DELAYED_WORK(&sfp->sfp_queue, fiber_type_select);
	} else {
		pr_warn("Failed to get i2c adapter from i2c bus %d.\n", sfp->i2c_bus);
	}

	return 0;	
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
		val = CUNIT_GPIO1_SFP_10G;
		break;
	case 0x0:
		regnum = XUNIT_1000BASE_STATUS_REG;
		phydev->speed = SPEED_1000;
		val = CUNIT_GPIO0_SFP_1G;
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
				val = CUNIT_GPIO2_COPPER_1G;
				break;
			case 0x3:
			default:
				phydev->speed = SPEED_10000;
				val = CUNIT_GPIO3_COPPER_10G;
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
			case CUNIT_GPIO3_COPPER_10G:
			case CUNIT_GPIO2_COPPER_1G:
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED0_CTRL_REG, 0x0001);
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED1_CTRL_REG, 0x0128);
				break;
			case CUNIT_GPIO1_SFP_10G:
			case CUNIT_GPIO0_SFP_1G:
				phy_write_mmd(phydev, CUNIT_DEV, CUNIT_LED0_CTRL_REG, 0x0131);
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
