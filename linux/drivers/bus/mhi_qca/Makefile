# core layer
all:
	ln -sf ../../../include/linux/mhi_upstream.h ../../../include/linux/mhi.h

obj-y += core/

obj-$(CONFIG_MHI_BUS_PCI_GENERIC) += mhi_pci_generic.o
mhi_pci_generic-y += pci_generic.o

