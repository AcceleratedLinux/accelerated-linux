/*
 * drivers/i2c/busses/i2c-mt7621-manual.c
 *
 * Copyright (C) 2013 Steven Liu <steven_liu@mediatek.com>
 * Copyright (C) 2016 Michael Lee <igvtee@gmail.com>
 * Copyright (C) 2018 Jan Breuer <jan.breuer@jaybee.cz>
 * Copyright (C) 2019 Digi International Inc.
 *
 * Improve driver for i2cdetect from i2c-tools to detect i2c devices on the bus.
 * (C) 2014 Sittisak <sittisaks@hotmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/slab.h>

#define REG_SM0CFG0		0x08
#define REG_SM0DOUT		0x10
#define REG_SM0DIN		0x14
#define REG_SM0ST		0x18
#define REG_SM0AUTO		0x1C
#define REG_SM0CFG1		0x20
#define REG_SM0CFG2		0x28
#define REG_SM0CTL0		0x40
#define REG_SM0CTL1		0x44
#define REG_SM0D0		0x50
#define REG_SM0D1		0x54
#define REG_PINTEN		0x5C
#define REG_PINTST		0x60
#define REG_PINTCL		0x64

/* REG_SM0CFG0 */
#define I2C_DEVADDR_MASK	0x7f

/* REG_SM0ST */
#define I2C_DATARDY		BIT(2)
#define I2C_SDOEMPTY		BIT(1)
#define I2C_BUSY		BIT(0)

/* REG_SM0AUTO */
#define READ_CMD		BIT(0)

/* REG_SM0CFG1 */
#define BYTECNT_MAX		64
#define SET_BYTECNT(x)		(x - 1)

/* REG_SM0CFG2 */
#define AUTOMODE_EN		BIT(0)

/* REG_SM0CTL0 */
#define ODRAIN_HIGH_SM0		BIT(31)
#define VSYNC_SHIFT		28
#define VSYNC_MASK		0x3
#define VSYNC_PULSE		(0x1 << VSYNC_SHIFT)
#define VSYNC_RISING		(0x2 << VSYNC_SHIFT)
#define CLK_DIV_SHIFT		16
#define CLK_DIV_MASK		0xfff
#define DEG_CNT_SHIFT		8
#define DEG_CNT_MASK		0xff
#define WAIT_HIGH		BIT(6)
#define DEG_EN			BIT(5)
#define CS_STATUA		BIT(4)
#define SCL_STATUS		BIT(3)
#define SDA_STATUS		BIT(2)
#define SM0_EN			BIT(1)
#define SCL_STRECH		BIT(0)

/* REG_SM0CTL1 */
#define SM0_ACK_MASK		(0xff << 16)
#define SM0_PGLEN_MASK		(0x7 << 8)
#define SM0_PGLEN(x)		(((x - 1) << 8) & SM0_PGLEN_MASK)
#define SM0_MODE_START		(0x1 << 4)
#define SM0_MODE_WRITE		(0x2 << 4)
#define SM0_MODE_STOP		(0x3 << 4)
#define SM0_MODE_READ_LAST	(0x4 << 4)
#define SM0_MODE_READ		(0x5 << 4)
#define SM0_TRI_BUSY		BIT(0)

/* timeout waiting for I2C devices to respond (clock streching) */
#define TIMEOUT_MS              1000
#define DELAY_INTERVAL_US       100

struct mtk_i2c {
	void __iomem *base;
	struct clk *clk;
	struct device *dev;
	struct i2c_adapter adap;
	u32 cur_clk;
	u32 clk_div;
	u32 flags;

	struct i2c_bus_recovery_info rinfo;

	struct pinctrl *pinctrl;
	struct pinctrl_state *pinctrl_pins_default;
	struct pinctrl_state *pinctrl_pins_gpio;
};

static void mtk_i2c_w32(struct mtk_i2c *i2c, u32 val, unsigned reg)
{
	iowrite32(val, i2c->base + reg);
}

static u32 mtk_i2c_r32(struct mtk_i2c *i2c, unsigned reg)
{
	return ioread32(i2c->base + reg);
}

static int poll_down_timeout(void __iomem *addr, u32 mask)
{
	/* Continuously poll until a certain time, then fall back to sleep */
	unsigned long timeout_before_sleep = jiffies + 2;
	unsigned long timeout = jiffies + msecs_to_jiffies(TIMEOUT_MS);

	do {
		if (!(readl_relaxed(addr) & mask))
			return 0;

		if (time_is_before_jiffies(timeout_before_sleep))
			usleep_range(DELAY_INTERVAL_US, DELAY_INTERVAL_US + 50);
	} while (time_before(jiffies, timeout));

	return (readl_relaxed(addr) & mask) ? -EAGAIN : 0;
}

static int mtk_i2c_wait_idle(struct mtk_i2c *i2c)
{
	int ret;

	ret = poll_down_timeout(i2c->base + REG_SM0CTL1, SM0_TRI_BUSY);
	if (ret < 0)
		dev_dbg(i2c->dev, "idle err(%d)\n", ret);

	return ret;
}

static void mtk_i2c_reset(struct mtk_i2c *i2c)
{
	u32 reg;
	device_reset(i2c->adap.dev.parent);
	barrier();

	/* ctrl0 */
	reg = ODRAIN_HIGH_SM0 | VSYNC_PULSE | (i2c->clk_div << CLK_DIV_SHIFT) |
		WAIT_HIGH | SM0_EN | SCL_STRECH;
	mtk_i2c_w32(i2c, reg, REG_SM0CTL0);

	/* manual-mode */
	mtk_i2c_w32(i2c, 0, REG_SM0CFG2);
}

static void mtk_i2c_dump_reg(struct mtk_i2c *i2c)
{
	dev_dbg(i2c->dev, "cfg0 %08x, dout %08x, din %08x, " \
			"status %08x, auto %08x, cfg1 %08x, " \
			"cfg2 %08x, ctl0 %08x, ctl1 %08x, " \
			"d0: %08x, d1: %08x\n",
			mtk_i2c_r32(i2c, REG_SM0CFG0),
			mtk_i2c_r32(i2c, REG_SM0DOUT),
			mtk_i2c_r32(i2c, REG_SM0DIN),
			mtk_i2c_r32(i2c, REG_SM0ST),
			mtk_i2c_r32(i2c, REG_SM0AUTO),
			mtk_i2c_r32(i2c, REG_SM0CFG1),
			mtk_i2c_r32(i2c, REG_SM0CFG2),
			mtk_i2c_r32(i2c, REG_SM0CTL0),
			mtk_i2c_r32(i2c, REG_SM0CTL1),
			mtk_i2c_r32(i2c, REG_SM0D0),
			mtk_i2c_r32(i2c, REG_SM0D1));
}

static int mtk_i2c_check_ack(struct mtk_i2c *i2c, u32 expected)
{
	u32 ack = mtk_i2c_r32(i2c, REG_SM0CTL1);
	u32 ack_expected = (expected << 16) & SM0_ACK_MASK;

	return ((ack & ack_expected) == ack_expected) ? 0 : -ENXIO;
}

static int mtk_i2c_master_start(struct mtk_i2c *i2c)
{
	mtk_i2c_w32(i2c, SM0_MODE_START | SM0_TRI_BUSY, REG_SM0CTL1);

	return mtk_i2c_wait_idle(i2c);
}

static int mtk_i2c_master_stop(struct mtk_i2c *i2c)
{
	mtk_i2c_w32(i2c, SM0_MODE_STOP | SM0_TRI_BUSY, REG_SM0CTL1);

	return mtk_i2c_wait_idle(i2c);
}

static int mtk_i2c_master_cmd(struct mtk_i2c *i2c, u32 cmd, int page_len)
{
	mtk_i2c_w32(i2c, cmd | SM0_TRI_BUSY | SM0_PGLEN(page_len), REG_SM0CTL1);

	return mtk_i2c_wait_idle(i2c);
}

static int mtk_i2c_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
		int num)
{
	struct mtk_i2c *i2c;
	struct i2c_msg *pmsg;
	int i, j, ret, len;
	u32 cmd;

	i2c = i2c_get_adapdata(adap);

	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];

		dev_dbg(i2c->dev, "addr: 0x%x, len: %d, flags: 0x%x\n",
				pmsg->addr, pmsg->len, pmsg->flags);

		/* wait hardware idle */
		if ((ret = mtk_i2c_wait_idle(i2c))) {
			dev_err(i2c->dev, "waiting for idle timeout\n");
			goto err_timeout;
		}

		/* start sequence */
		if ((ret = mtk_i2c_master_start(i2c))) {
			dev_err(i2c->dev, "start sequence\n");
			goto err_timeout;
		}

		/* write address */
		if (pmsg->flags & I2C_M_TEN) {
			/* 10-bit address */
			u16 addr;

			addr = 0xF0 | ((pmsg->addr >> 7) & 0x06);
			addr |= (pmsg->addr & 0xFF) << 8;
			if (pmsg->flags & I2C_M_RD)
				addr |= 1;
			mtk_i2c_w32(i2c, addr, REG_SM0D0);
			ret = mtk_i2c_master_cmd(i2c, SM0_MODE_WRITE, 2);
			if (ret) {
				dev_err(i2c->dev, "10-bit address sending\n");
				goto err_timeout;
			}
		} else {
			/* 7-bit address */
			u8 addr;

			addr = pmsg->addr << 1;
			if (pmsg->flags & I2C_M_RD)
				addr |= 1;
			mtk_i2c_w32(i2c, addr, REG_SM0D0);
			ret = mtk_i2c_master_cmd(i2c, SM0_MODE_WRITE, 1);
			if (ret) {
				dev_err(i2c->dev, "7-bit address sending\n");
				goto err_timeout;
			}
		}

		/* check address ACK */
		if (!(pmsg->flags & I2C_M_IGNORE_NAK)) {
			if ((ret = mtk_i2c_check_ack(i2c, BIT(0)))) {
				dev_dbg(i2c->dev, "address ack\n");
				goto err_ack;
			}
		}

		/* transfer data */
		for (len = pmsg->len, j = 0; len > 0; len -= 8, j += 8) {
			u32 data[2];
			int page_len = (len >= 8) ? 8 : len;

			if (pmsg->flags & I2C_M_RD) {
				cmd = (len > 8) ? SM0_MODE_READ :
						  SM0_MODE_READ_LAST;
			} else {
				memcpy(data, &pmsg->buf[j], page_len);
				mtk_i2c_w32(i2c, data[0], REG_SM0D0);
				mtk_i2c_w32(i2c, data[1], REG_SM0D1);
				cmd = SM0_MODE_WRITE;
			}

			if ((ret = mtk_i2c_master_cmd(i2c, cmd, page_len))) {
				dev_err(i2c->dev, "data transmit\n");
				goto err_timeout;
			}

			if (pmsg->flags & I2C_M_RD) {
				data[0] = mtk_i2c_r32(i2c, REG_SM0D0);
				data[1] = mtk_i2c_r32(i2c, REG_SM0D1);
				memcpy(&pmsg->buf[j], data, page_len);
			} else {
				if (!(pmsg->flags & I2C_M_IGNORE_NAK)) {
					if ((ret = mtk_i2c_check_ack(i2c,
					    (1 << page_len) - 1))) {
						dev_dbg(i2c->dev,
							"data ack\n");
						goto err_ack;
					}
				}
			}
		}
	}

	if ((ret = mtk_i2c_master_stop(i2c))) {
		dev_err(i2c->dev, "stop\n");
		goto err_timeout;
	}

	/* the return value is number of executed messages */
	ret = i;

	return ret;

err_ack:
	if ((ret = mtk_i2c_master_stop(i2c))) {
		dev_err(i2c->dev, "stop on ack err\n");
		goto err_timeout;
	}
	return -ENXIO;

err_timeout:
	mtk_i2c_dump_reg(i2c);
	mtk_i2c_reset(i2c);
	return ret;
}

static u32 mtk_i2c_func(struct i2c_adapter *a)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm mtk_i2c_algo = {
	.master_xfer	= mtk_i2c_master_xfer,
	.functionality	= mtk_i2c_func,
};

static void mtk_i2c_prepare_recovery(struct i2c_adapter *adap)
{
	struct mtk_i2c *i2c;

	i2c = i2c_get_adapdata(adap);

	pinctrl_select_state(i2c->pinctrl, i2c->pinctrl_pins_gpio);
}

static void mtk_i2c_unprepare_recovery(struct i2c_adapter *adap)
{
	struct mtk_i2c *i2c;

	i2c = i2c_get_adapdata(adap);

	pinctrl_select_state(i2c->pinctrl, i2c->pinctrl_pins_default);
}

static int mtk_i2c_init_recovery_info(struct mtk_i2c *i2c,
	struct platform_device *pdev)
{
	struct i2c_bus_recovery_info *rinfo = &i2c->rinfo;

	i2c->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (!i2c->pinctrl || IS_ERR(i2c->pinctrl)) {
		dev_info(&pdev->dev,
				 "can't get pinctrl, bus recovery is not supported\n");
		return PTR_ERR(i2c->pinctrl);
	}

	i2c->pinctrl_pins_default = pinctrl_lookup_state(i2c->pinctrl,
			PINCTRL_STATE_DEFAULT);
	i2c->pinctrl_pins_gpio = pinctrl_lookup_state(i2c->pinctrl, "gpio");
	rinfo->sda_gpiod = devm_gpiod_get(&pdev->dev, "sda", GPIOD_OUT_HIGH);
	rinfo->scl_gpiod = devm_gpiod_get(&pdev->dev, "scl", GPIOD_OUT_HIGH);

	if (PTR_ERR(rinfo->sda_gpiod) == -EPROBE_DEFER ||
		PTR_ERR(rinfo->scl_gpiod) == -EPROBE_DEFER) {
		return -EPROBE_DEFER;
	} else if (IS_ERR(rinfo->sda_gpiod) ||
		   IS_ERR(rinfo->scl_gpiod) ||
		   IS_ERR(i2c->pinctrl_pins_default) ||
		   IS_ERR(i2c->pinctrl_pins_gpio)) {
		dev_dbg(&pdev->dev, "recovery information incomplete\n");
		return 0;
	}

	dev_dbg(&pdev->dev, "using scl and sda for recovery\n");

	rinfo->prepare_recovery = mtk_i2c_prepare_recovery;
	rinfo->unprepare_recovery = mtk_i2c_unprepare_recovery;
	rinfo->recover_bus = i2c_generic_scl_recovery;
	i2c->adap.bus_recovery_info = rinfo;

	return 0;
}

static const struct of_device_id i2c_mtk_dt_ids[] = {
	{ .compatible = "mediatek,mt7621-i2c" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, i2c_mtk_dt_ids);

static void mtk_i2c_init(struct mtk_i2c *i2c)
{
	i2c->clk_div = clk_get_rate(i2c->clk) / i2c->cur_clk;
	if (i2c->clk_div > CLK_DIV_MASK)
		i2c->clk_div = CLK_DIV_MASK;

	mtk_i2c_reset(i2c);
}

static int mtk_i2c_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mtk_i2c *i2c;
	struct i2c_adapter *adap;
	const struct of_device_id *match;
	int ret;

	match = of_match_device(i2c_mtk_dt_ids, &pdev->dev);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "no memory resource found\n");
		return -ENODEV;
	}

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct mtk_i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "failed to allocate i2c_adapter\n");
		return -ENOMEM;
	}

	i2c->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2c->base))
		return PTR_ERR(i2c->base);

	i2c->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "no clock defined\n");
		return -ENODEV;
	}
	clk_prepare_enable(i2c->clk);
	i2c->dev = &pdev->dev;

	if (of_property_read_u32(pdev->dev.of_node,
				"clock-frequency", &i2c->cur_clk))
		i2c->cur_clk = 100000;

	adap = &i2c->adap;
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	adap->algo = &mtk_i2c_algo;
	adap->retries = 3;
	adap->dev.parent = &pdev->dev;
	i2c_set_adapdata(adap, i2c);
	adap->dev.of_node = pdev->dev.of_node;
	strlcpy(adap->name, dev_name(&pdev->dev), sizeof(adap->name));

	platform_set_drvdata(pdev, i2c);

	if (mtk_i2c_init_recovery_info(i2c, pdev) == -EPROBE_DEFER)
		return -EPROBE_DEFER;

	mtk_i2c_init(i2c);

	ret = i2c_add_adapter(adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add adapter\n");
		clk_disable_unprepare(i2c->clk);
		return ret;
	}

	dev_info(&pdev->dev, "clock %uKHz, re-start not support\n",
			i2c->cur_clk/1000);

	return ret;
}

static int mtk_i2c_remove(struct platform_device *pdev)
{
	struct mtk_i2c *i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c->adap);
	clk_disable_unprepare(i2c->clk);

	return 0;
}

static struct platform_driver mtk_i2c_driver = {
	.probe		= mtk_i2c_probe,
	.remove		= mtk_i2c_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "i2c-mt7621",
		.of_match_table = i2c_mtk_dt_ids,
	},
};

static int __init i2c_mtk_init (void)
{
	return platform_driver_register(&mtk_i2c_driver);
}
subsys_initcall(i2c_mtk_init);

static void __exit i2c_mtk_exit (void)
{
	platform_driver_unregister(&mtk_i2c_driver);
}
module_exit(i2c_mtk_exit);

MODULE_AUTHOR("Steven Liu <steven_liu@mediatek.com>");
MODULE_DESCRIPTION("MT7621 I2c host driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:MT7621-I2C");
