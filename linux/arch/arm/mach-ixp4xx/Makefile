# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the linux kernel.
#

obj-pci-y	:=
obj-pci-n	:=

obj-pci-$(CONFIG_ARCH_IXDP4XX)		+= ixdp425-pci.o
obj-pci-$(CONFIG_MACH_AVILA)		+= avila-pci.o
obj-pci-$(CONFIG_MACH_IXDPG425)		+= ixdpg425-pci.o
obj-pci-$(CONFIG_ARCH_ADI_COYOTE)	+= coyote-pci.o
obj-pci-$(CONFIG_MACH_GTWX5715)		+= gtwx5715-pci.o
obj-pci-$(CONFIG_MACH_MICCPT)		+= miccpt-pci.o
obj-pci-$(CONFIG_MACH_NSLU2)		+= nslu2-pci.o
obj-pci-$(CONFIG_MACH_NAS100D)		+= nas100d-pci.o
obj-pci-$(CONFIG_MACH_DSMG600)		+= dsmg600-pci.o
obj-pci-$(CONFIG_MACH_ESS710)		+= ess710-pci.o
obj-pci-$(CONFIG_MACH_SE5100)		+= ixdpg425-pci.o
obj-pci-$(CONFIG_MACH_SG560USB)		+= sg5xx-pci.o
obj-pci-$(CONFIG_MACH_SG560ADSL)	+= sg5xx-pci.o
obj-pci-$(CONFIG_MACH_SG565)		+= sg5xx-pci.o
obj-pci-$(CONFIG_MACH_SG590)		+= sg720-pci.o
obj-pci-$(CONFIG_MACH_SG720)		+= sg720-pci.o
obj-pci-$(CONFIG_MACH_SG8100)		+= sg8100-pci.o
obj-pci-$(CONFIG_MACH_GATEWAY7001)	+= gateway7001-pci.o
obj-pci-$(CONFIG_MACH_WG302V2)		+= wg302v2-pci.o
obj-pci-$(CONFIG_MACH_FSG)		+= fsg-pci.o
obj-pci-$(CONFIG_MACH_ARCOM_VULCAN)	+= vulcan-pci.o

obj-y	+= common.o

obj-$(CONFIG_ARCH_IXDP4XX)	+= ixdp425-setup.o
obj-$(CONFIG_MACH_AVILA)	+= avila-setup.o
obj-$(CONFIG_MACH_IXDPG425)	+= coyote-setup.o
obj-$(CONFIG_ARCH_ADI_COYOTE)	+= coyote-setup.o
obj-$(CONFIG_MACH_GTWX5715)	+= gtwx5715-setup.o
obj-$(CONFIG_ARCH_SE4000)	+= sg-setup.o
obj-$(CONFIG_MACH_SE5100)	+= sg-setup.o
obj-$(CONFIG_MACH_SG560)	+= sg-setup.o
obj-$(CONFIG_MACH_SG560USB)	+= sg-setup.o
obj-$(CONFIG_MACH_SG560ADSL)	+= sg-setup.o
obj-$(CONFIG_MACH_SG565)	+= sg-setup.o
obj-$(CONFIG_MACH_SG580)	+= sg-setup.o
obj-$(CONFIG_MACH_SG590)	+= sg-setup.o
obj-$(CONFIG_MACH_SG640)	+= sg-setup.o
obj-$(CONFIG_MACH_ESS710)	+= sg-setup.o
obj-$(CONFIG_MACH_SG720)	+= sg-setup.o
obj-$(CONFIG_MACH_SG8100)	+= sg-setup.o
obj-$(CONFIG_MACH_SGARMAUTO)	+= sg-setup.o
obj-$(CONFIG_MACH_DEVIXP)	+= omixp-setup.o
obj-$(CONFIG_MACH_MICCPT)	+= omixp-setup.o
obj-$(CONFIG_MACH_MIC256)	+= omixp-setup.o
obj-$(CONFIG_MACH_NSLU2)	+= nslu2-setup.o
obj-$(CONFIG_MACH_NAS100D)	+= nas100d-setup.o
obj-$(CONFIG_MACH_DSMG600)      += dsmg600-setup.o
obj-$(CONFIG_MACH_GATEWAY7001)	+= gateway7001-setup.o
obj-$(CONFIG_MACH_WG302V2)	+= wg302v2-setup.o
obj-$(CONFIG_MACH_FSG)		+= fsg-setup.o
obj-$(CONFIG_MACH_GORAMO_MLR)	+= goramo_mlr.o
obj-$(CONFIG_MACH_ARCOM_VULCAN)	+= vulcan-setup.o

obj-$(CONFIG_PCI)		+= $(obj-pci-$(CONFIG_PCI)) common-pci.o
obj-$(CONFIG_IXP4XX_QMGR)	+= ixp4xx_qmgr.o
obj-$(CONFIG_IXP4XX_NPE)	+= ixp4xx_npe.o
obj-$(CONFIG_IXP4XX_DEBUG_FIQ)	+= fiq.o
