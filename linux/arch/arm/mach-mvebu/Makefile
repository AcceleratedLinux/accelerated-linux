# SPDX-License-Identifier: GPL-2.0
ccflags-y := -I$(srctree)/arch/arm/plat-orion/include

obj-$(CONFIG_MACH_MVEBU_ANY)	 += system-controller.o mvebu-soc-id.o

ifeq ($(CONFIG_MACH_MVEBU_V7),y)
obj-y				 += cpu-reset.o board-v7.o coherency.o coherency_ll.o pmsu.o pmsu_ll.o

obj-$(CONFIG_PM)		 += pm.o pm-board.o
obj-$(CONFIG_SMP)		 += platsmp.o headsmp.o platsmp-a9.o headsmp-a9.o
endif

obj-$(CONFIG_MACH_DOVE)		 += dove.o

obj-$(CONFIG_MACH_8300)          += 8300.o
obj-$(CONFIG_MACH_5400_RM_DT)    += board-accelerated.o
obj-$(CONFIG_MACH_6300_EX_DT)    += board-accelerated.o
obj-$(CONFIG_MACH_6300CX)        += 6300.o
obj-$(CONFIG_MACH_6330MX)        += 6300.o board-accelerated.o
obj-$(CONFIG_MACH_6350SR)        += 6300.o board-accelerated.o

ifeq ($(CONFIG_MACH_KIRKWOOD),y)
obj-y				 += kirkwood.o
obj-$(CONFIG_PM)		 += kirkwood-pm.o
endif
