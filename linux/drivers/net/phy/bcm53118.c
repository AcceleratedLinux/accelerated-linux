/*
 * Driver for Broadcom BCM53118 8-port ethernet switch
 *
 * (C) Copyright 2013, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This file was based on: drivers/net/phy/spi_ks8995.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/mii.h>

#define BCM53118_CMD_WRITE	0x61
#define BCM53118_CMD_READ	0x60

#define BCM53118_REGS_SIZE	0x100

#define BCM53118_REG_DATA	0xf0
#define BCM53118_REG_PAGE	0xff

#define BCM53118_REG_STATUS	0xfe
#define BCM53118_STATUS_SPIF	0x80
#define BCM53118_STATUS_RACK	0x20

#define TIMEOUT			100000

struct bcm53118_switch {
	struct spi_device	*spi;
	struct mutex		lock;
	struct net_device	*master;
	struct net_device	*slave;
};

struct bcm53118_slave {
	struct bcm53118_switch *sp;
};

static int bcm53118_read_spi(struct device *dev, u8 reg, void *buf, u8 cnt)
{
	struct bcm53118_switch *sp;
	struct spi_transfer t[2];
	struct spi_message m;
	int err;
	u8 cmd[2];

	sp = dev_get_drvdata(dev);

	cmd[0] = BCM53118_CMD_READ;
	cmd[1] = reg;

	spi_message_init(&m);

	memset(&t, 0, sizeof(t));
	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = cnt;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&sp->lock);
	err = spi_sync(sp->spi, &m);
	mutex_unlock(&sp->lock);

	return err;
}

static int bcm53118_write_spi(struct device *dev, u8 reg, void *buf, u8 cnt)
{
	struct bcm53118_switch *sp;
	struct spi_transfer t[2];
	struct spi_message m;
	int err;
	u8 cmd[2];

	sp = dev_get_drvdata(dev);

	cmd[0] = BCM53118_CMD_WRITE;
	cmd[1] = reg;

	spi_message_init(&m);

	memset(&t, 0, sizeof(t));
	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	t[1].len = cnt;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&sp->lock);
	err = spi_sync(sp->spi, &m);
	mutex_unlock(&sp->lock);

	return err;
}

static ssize_t bcm53118_sysfs_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t off, size_t count)
{
	struct device *dev;

	dev = container_of(kobj, struct device, kobj);

	if (unlikely(off > BCM53118_REGS_SIZE))
		return 0;

	if ((off + count) > BCM53118_REGS_SIZE)
		count = BCM53118_REGS_SIZE - off;

	if (unlikely(!count))
		return count;

	return bcm53118_read_spi(dev, off, buf, count) ?: count;
}

static ssize_t bcm53118_sysfs_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t off, size_t count)
{
	struct device *dev;

	dev = container_of(kobj, struct device, kobj);

	if (unlikely(off >= BCM53118_REGS_SIZE))
		return -EFBIG;

	if ((off + count) > BCM53118_REGS_SIZE)
		count = BCM53118_REGS_SIZE - off;

	if (unlikely(!count))
		return count;

	return bcm53118_write_spi(dev, off, buf, count) ?: count;
}

static struct bin_attribute bcm53118_registers_attr = {
	.attr = {
		.name   = "registers",
		.mode   = S_IRUSR | S_IWUSR,
	},
	.size   = BCM53118_REGS_SIZE,
	.read   = bcm53118_sysfs_read,
	.write  = bcm53118_sysfs_write,
};

static void bcm53118_slave_getinfo(struct net_device *dev,
				 struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, "bcm53118", sizeof(info->driver));
}

static const struct ethtool_ops bcm53118_slave_ethtool_ops = {
	.get_drvinfo	= bcm53118_slave_getinfo,
	.get_link	= ethtool_op_get_link,
};

static int bcm53118_get_iflink(const struct net_device *dev)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	return priv->sp->master->ifindex;
}

static int bcm53118_slave_init(struct net_device *dev)
{
	return 0;
}

static int bcm53118_slave_open(struct net_device *dev)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;
	int err;

	memcpy(dev->dev_addr, master->dev_addr, ETH_ALEN);

	if (dev->flags & IFF_ALLMULTI) {
		err = dev_set_allmulti(master, 1);
		if (err < 0)
			goto err_allmulti;
	}

	if (dev->flags & IFF_PROMISC) {
		err = dev_set_promiscuity(master, 1);
		if (err < 0)
			goto err_promisc;
	}

	return 0;

err_promisc:
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(master, -1);
err_allmulti:
	return err;
}

static int bcm53118_slave_stop(struct net_device *dev)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;

	dev_mc_unsync(master, dev);
	dev_uc_unsync(master, dev);
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(master, -1);
	if (dev->flags & IFF_PROMISC)
		dev_set_promiscuity(master, -1);
	return 0;
}

static void bcm53118_slave_change_rx_flags(struct net_device *dev, int change)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;

	if (change & IFF_ALLMULTI)
		dev_set_allmulti(master, dev->flags & IFF_ALLMULTI ? 1 : -1);
	if (change & IFF_PROMISC)
		dev_set_promiscuity(master, dev->flags & IFF_PROMISC ? 1 : -1);
}

static void bcm53118_slave_set_rx_mode(struct net_device *dev)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;

	dev_mc_sync(master, dev);
	dev_uc_sync(master, dev);
}

static int bcm53118_slave_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;

	if (master->netdev_ops->ndo_do_ioctl)
		return master->netdev_ops->ndo_do_ioctl(master, ifr, cmd);

        return -EOPNOTSUPP;
}

#define BCM53118_TAG_LEN 4

static bool bcm53118_tag(struct sk_buff *skb)
{
	if (skb_cow_head(skb, BCM53118_TAG_LEN) < 0) {
		kfree_skb(skb);
		return false;
	}

	/* The switch drops frames that are too short after removing the tag */
	if (unlikely(skb->len < ETH_ZLEN)) {
		if (skb_padto(skb, ETH_ZLEN))
			return false;
		skb_put(skb, ETH_ZLEN - skb->len);
	}

	/* Add the tag */
	skb_push(skb, BCM53118_TAG_LEN);
	memmove(skb->data, skb->data + BCM53118_TAG_LEN, 2 * ETH_ALEN);
	memset(skb->data + 2 * ETH_ALEN, 0, BCM53118_TAG_LEN);
	/* Leave skb->protocol unchanged so hardware can TSO and checksum. */
	return true;
}

static bool bcm53118_untag(struct bcm53118_switch *sp, struct sk_buff *skb)
{
	static const u8 pae_group_addr[ETH_ALEN] __aligned(2)
		= { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
	struct vlan_ethhdr *vlan;
	u8 *tag;

	if (unlikely(!pskb_may_pull(skb, 2 * ETH_ALEN + BCM53118_TAG_LEN)))
		return false;

	/* Check for opcode = 0b000 and reserved bits = 0 */
	tag = skb->data + 2 * ETH_ALEN;
	if (tag[0] != 0 && tag[1] != 0)
		return false;

	vlan = (struct vlan_ethhdr *)skb->data;
	if (ether_addr_equal(vlan->h_dest, pae_group_addr)) {
		/*
		 * Our userspace 802.1x implementation expects EAPOL packets to
		 * have the VLAN tag of the port they arrived on.  The switch
		 * doesn't do that though, so fix it up here.  This uses a
		 * fixed mapping from port to VLAN; it probably should be made
		 * configurable.
		 */
		u16 tci = 4071 + (tag[3] & 0x1f);
		vlan->h_vlan_proto = htons(ETH_P_8021Q);
		vlan->h_vlan_TCI = htons(VLAN_CFI_MASK | tci);
	} else {
		/* Remove the tag */
		memmove(skb->data + BCM53118_TAG_LEN, skb->data, 2 * ETH_ALEN);
		skb_pull(skb, BCM53118_TAG_LEN);
	}
	/* FIXME: update skb->csum if ethernet driver supports CHECKSUM_COMPLETE */

	return true;
}

static int bcm53118_slave_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct bcm53118_slave *priv = netdev_priv(dev);
	struct net_device *master = priv->sp->master;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	if (!bcm53118_tag(skb))
		goto out;

	skb->dev = master;
	dev_queue_xmit(skb);
	return NETDEV_TX_OK;

out:
	skb->dev->stats.tx_errors++;
	return NETDEV_TX_OK;
}

static int bcm53118_slave_rcv(struct sk_buff *skb, struct net_device *dev,
		struct packet_type *pt, struct net_device *orig_dev)
{
	struct bcm53118_switch *sp = dev->bcm53118_ptr;

	if (unlikely(sp == NULL))
		goto out_drop;

	skb = skb_unshare(skb, GFP_ATOMIC);
	if (skb == NULL)
		goto out;

	skb->dev = sp->slave;
	skb_push(skb, ETH_HLEN);

	if (!bcm53118_untag(sp, skb))
		goto out_drop;

	skb->pkt_type = PACKET_HOST;
	skb->protocol = eth_type_trans(skb, skb->dev);

	skb->dev->stats.rx_packets++;
	skb->dev->stats.rx_bytes += skb->len;

	netif_receive_skb(skb);
	return 0;

out_drop:
	skb->dev->stats.rx_errors++;
	kfree_skb(skb);
out:
	return 0;
}

static int bcm53118_slave_eth_change_mtu(struct net_device *dev, int new_mtu)
{
	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops bcm53118_slave_netdev_ops = {
	.ndo_init = bcm53118_slave_init,
	.ndo_open = bcm53118_slave_open,
	.ndo_stop = bcm53118_slave_stop,
	.ndo_start_xmit = bcm53118_slave_xmit,
	.ndo_change_rx_flags = bcm53118_slave_change_rx_flags,
	.ndo_set_rx_mode = bcm53118_slave_set_rx_mode,
	.ndo_change_mtu = bcm53118_slave_eth_change_mtu,
	.ndo_do_ioctl = bcm53118_slave_ioctl,
	.ndo_get_iflink = bcm53118_get_iflink,
};

static struct packet_type bcm53118_packet_type __read_mostly = {
	.type	= cpu_to_be16(ETH_P_BCM53118),
	.func	= bcm53118_slave_rcv,
};

struct net_device *
bcm53118_slave_create(struct bcm53118_switch *sp, struct device *parent)
{
	struct net_device *master = sp->master;
	struct net_device *slave;
	struct bcm53118_slave *priv;
	int err;

	slave = alloc_netdev(sizeof(*priv), "bcm%d", NET_NAME_UNKNOWN, ether_setup);
	if (slave == NULL)
		return slave;

	slave->netdev_ops = &bcm53118_slave_netdev_ops;
	slave->ethtool_ops = &bcm53118_slave_ethtool_ops;
	slave->hw_features = master->hw_features & (NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_TSO);
	slave->vlan_features = slave->hw_features;
	slave->features |= slave->hw_features;
	slave->tx_queue_len = 0;
	eth_hw_addr_inherit(slave, master);

	SET_NETDEV_DEV(slave, parent);

	priv = netdev_priv(slave);
	priv->sp = sp;

	err = register_netdev(slave);
	if (err) {
		netdev_err(slave, "failed to register: %d\n", err);
		free_netdev(slave);
		return NULL;
	}

	return slave;
}

static int bcm53118_match_class(struct device *dev, void *data)
{
	return dev->class && strcmp(dev->class->name, data) == 0;
}

struct net_device *bcm53118_find_master(struct spi_device *spi)
{
	struct device_node *node;
	struct platform_device *pd;
	struct device *d;
	struct net_device *nd;

	node = of_parse_phandle(spi->dev.of_node, "bcm53118,ethernet", 0);
	if (!node)
		return ERR_PTR(-EINVAL);

	pd = of_find_device_by_node(node);
	if (!pd)
		return ERR_PTR(-EPROBE_DEFER);

	d = device_find_child(&pd->dev, "net", bcm53118_match_class);
	if (!d)
		return ERR_PTR(-EPROBE_DEFER);

	nd = to_net_dev(d);
	dev_hold(nd);
	return nd;
}

static int bcm53118_spi_probe(struct spi_device *spi)
{
	struct device_node *node = spi->dev.of_node;
	struct bcm53118_switch *sp;
	struct net_device *master;
	int err;

	master = bcm53118_find_master(spi);
	if (IS_ERR(master)) {
		err = PTR_ERR(master);
		goto out;
	}

	sp = kzalloc(sizeof(*sp), GFP_KERNEL);
	if (!sp) {
		err = -ENOMEM;
		goto err_master;
	}

	mutex_init(&sp->lock);
	sp->spi = spi_dev_get(spi);
	spi_set_drvdata(spi, sp);

	err = spi_setup(spi);
	if (err) {
		dev_err(&spi->dev, "spi_setup failed, err=%d\n", err);
		goto err_drvdata;
	}

	err = sysfs_create_bin_file(&spi->dev.kobj, &bcm53118_registers_attr);
	if (err) {
		dev_err(&spi->dev, "failed to create sysfs file, err=%d\n", err);
		goto err_drvdata;
	}

	sp->master = master;
	sp->slave = bcm53118_slave_create(sp, &spi->dev);
	if (!sp->slave) {
		goto err_sysfs;
	}
	master->bcm53118_ptr = sp;
	dev_add_pack(&bcm53118_packet_type);

	dev_info(&spi->dev, "BCM53118 device found\n");

	of_platform_populate(node, NULL, NULL, &spi->dev);

	return 0;

err_sysfs:
	sysfs_remove_bin_file(&spi->dev.kobj, &bcm53118_registers_attr);
err_drvdata:
	spi_set_drvdata(spi, NULL);
	kfree(sp);
err_master:
	dev_put(master);
out:
	return err;
}

static int bcm53118_spi_remove(struct spi_device *spi)
{
	struct bcm53118_switch *sp;

	sp = spi_get_drvdata(spi);
	sysfs_remove_bin_file(&spi->dev.kobj, &bcm53118_registers_attr);
	spi_set_drvdata(spi, NULL);
	free_netdev(sp->slave);
	dev_put(sp->master);
	kfree(sp);
	return 0;
}

static struct spi_driver bcm53118_spi_driver = {
	.driver = {
		.name	= "bcm53118",
		.owner	= THIS_MODULE,
	},
	.probe	= bcm53118_spi_probe,
	.remove	= bcm53118_spi_remove,
};

module_spi_driver(bcm53118_spi_driver);

static int bcm53118_phy_config_init(struct phy_device *phydev)
{
	/* FIXME: store switch pointer in netdev */
	phydev->autoneg = AUTONEG_DISABLE;
	phydev->speed = SPEED_1000;
	phydev->duplex = DUPLEX_FULL;
	return 0;
}

static int bcm53118_phy_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static int bcm53118_phy_read_status(struct phy_device *phydev)
{
	return 0;
}

static int bcm53118_phy_soft_reset(struct phy_device *phydev)
{
	return 0;
}

#define PHY_ID_BCM53118 0x0143bfe0

static struct phy_driver bcm53118_phy_driver = {
	.phy_id		= PHY_ID_BCM53118,
	.phy_id_mask	= 0xffffffff,
	.name		= "Broadcom BCM53118",
	.config_init	= bcm53118_phy_config_init,
	.features	= PHY_GBIT_FEATURES,
	.config_aneg	= bcm53118_phy_config_aneg,
	.read_status	= bcm53118_phy_read_status,
	.soft_reset	= bcm53118_phy_soft_reset,
};

static int __init bcm53118_phy_init(void)
{
	return phy_driver_register(&bcm53118_phy_driver, THIS_MODULE);
}

static void __exit bcm53118_phy_exit(void)
{
	phy_driver_unregister(&bcm53118_phy_driver);
}

module_init(bcm53118_phy_init);
module_exit(bcm53118_phy_exit);

static struct mdio_device_id __maybe_unused bcm53118_phy_tbl[] = {
	{ PHY_ID_BCM53118, 0xfffffff0 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, bcm53118_phy_tbl);

static int bcm53118_intphy_read(int regnum)
{
	switch (regnum) {
	case MII_PHYSID1:
		return (PHY_ID_BCM53118 >> 16) & 0xffff;
	case MII_PHYSID2:
		return PHY_ID_BCM53118 & 0xffff;
	default:
		break;
	}

	return 0xffff;
}

static int bcm53118_intphy_write(int regnum, u16 val)
{
	return 0;
}

static int bcm53118_waitready(struct device *dev)
{
	u8 val;
	int i;
	int err;

	for (i = 0; i < TIMEOUT; i++) {
		err = bcm53118_read_spi(dev, BCM53118_REG_STATUS, &val, 1);
		if (err)
			return err;
		if ((val & BCM53118_STATUS_SPIF) == 0)
			return 0;
	}

	return -EIO;
}

static int bcm53118_readack(struct device *dev)
{
	u8 val;
	int i;
	int err;

	for (i = 0; i < TIMEOUT; i++) {
		err = bcm53118_read_spi(dev, BCM53118_REG_STATUS, &val, 1);
		if (err)
			return err;
		if (val & BCM53118_STATUS_RACK)
			return 0;
	}

	return -EIO;
}

static struct device *bcm53118_mii_to_spi_dev(struct mii_bus *bus)
{
	return bus->priv;
}

static int bcm53118_phy_read(struct mii_bus *bus, int phyid, int regnum)
{
	struct device *dev = bcm53118_mii_to_spi_dev(bus);
	u8 pagenum;
	u16 val;
	int err;

	err = bcm53118_waitready(dev);
	if (err)
		return err;

	pagenum = 0x10 + phyid;
	err = bcm53118_write_spi(dev, BCM53118_REG_PAGE, &pagenum, 1);
	if (err)
		return err;

	err = bcm53118_read_spi(dev, regnum * 2, &val, 1); /* dummy read */
	if (err)
		return err;

	err = bcm53118_readack(dev);
	if (err)
		return err;

	val = 0;
	err = bcm53118_read_spi(dev, BCM53118_REG_DATA, &val, 2);
	if (err)
		return err;

	return le16_to_cpu(val);
}

static int bcm53118_phy_write(struct mii_bus *bus, int phyid, int regnum,
			      u16 val)
{
	struct device *dev = bcm53118_mii_to_spi_dev(bus);
	u8 pagenum;
	int err;

	err = bcm53118_waitready(dev);
	if (err)
		return err;

	pagenum = 0x10 + phyid;
	err = bcm53118_write_spi(dev, BCM53118_REG_PAGE, &pagenum, 1);
	if (err)
		return err;

	val = cpu_to_le16(val);
	err = bcm53118_write_spi(dev, regnum * 2, &val, 2);
	if (err)
		return err;

	return 0;
}

static int bcm53118_mdio_read(struct mii_bus *bus, int phyid, int regnum)
{
	if (phyid == 8)
		return bcm53118_intphy_read(regnum);
	if (phyid >= 0 && phyid <= 7)
		return bcm53118_phy_read(bus, phyid, regnum);
	return -ENODEV;
}

static int bcm53118_mdio_write(struct mii_bus *bus, int phyid, int regnum, u16 val)
{
	if (phyid == 8)
		return bcm53118_intphy_write(regnum, val);
	if (phyid >= 0 && phyid <= 7)
		return bcm53118_phy_write(bus, phyid, regnum, val);
	return -ENODEV;
}

static int bcm53118_match_node(struct device *dev, const void *data)
{
	return dev->of_node == data;
}

static int bcm53118_mdio_probe(struct platform_device *pdev)
{
	struct device_node *spi_node;
	struct device *spi;
	struct mii_bus *bus;
	int ret;

	spi_node = of_parse_phandle(pdev->dev.of_node, "bcm53118,spi", 0);
	if (!spi_node)
		return -EINVAL;

	spi = driver_find_device(&bcm53118_spi_driver.driver, NULL, spi_node,
			bcm53118_match_node);
	if (!spi)
		return -EPROBE_DEFER;

	bus = mdiobus_alloc();
	if (!bus)
		return -ENODEV;

	bus->priv = spi;
	bus->name = "mdio-bcm53118";
	snprintf(bus->id, MII_BUS_ID_SIZE, "bcm53118");
	bus->parent = &pdev->dev;
	bus->read = bcm53118_mdio_read;
	bus->write = bcm53118_mdio_write;
	dev_set_drvdata(&pdev->dev, bus);

	ret = of_mdiobus_register(bus, pdev->dev.of_node);
	return ret;
}

static int bcm53118_mdio_remove(struct platform_device *pdev)
{
	struct mii_bus *bus = dev_get_drvdata(&pdev->dev);

	mdiobus_unregister(bus);
	return 0;
}

static struct of_device_id bcm53118_mdio_of_match[] = {
	{ .compatible = "virtual,mdio-bcm53118", },
	{ /* sentinel */ }
};

static struct platform_driver bcm53118_mdio_driver = {
	.probe = bcm53118_mdio_probe,
	.remove = bcm53118_mdio_remove,
	.driver		= {
		.name	= "mdio-bcm53118",
		.owner	= THIS_MODULE,
		.of_match_table = bcm53118_mdio_of_match,
	},
};

module_platform_driver(bcm53118_mdio_driver);

MODULE_DESCRIPTION("Broadcom BCM53118 Ethernet switch driver");
MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_LICENSE("GPL");
