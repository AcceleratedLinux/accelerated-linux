#
# Makefile for the Mediatek SoCs built-in ethernet macs
#

mtk-eth-soc-y					+= mtk_eth_soc.o ethtool.o

mtk-eth-soc-$(CONFIG_NET_MEDIATEK_MDIO)		+= mdio.o
mtk-eth-soc-$(CONFIG_NET_MEDIATEK_MDIO_MT7620)	+= mdio_mt7620.o

mtk-eth-soc-$(CONFIG_NET_MEDIATEK_MT7620)	+= soc_mt7620.o
mtk-eth-soc-$(CONFIG_NET_MEDIATEK_MT7621)	+= soc_mt7621.o

obj-$(CONFIG_NET_MEDIATEK_GSW_MT7620)		+= gsw_mt7620.o mt7530.o
obj-$(CONFIG_NET_MEDIATEK_GSW_MT7621)		+= gsw_mt7621.o mt7530.o
obj-$(CONFIG_NET_MEDIATEK_SOC)			+= mtk-eth-soc.o
