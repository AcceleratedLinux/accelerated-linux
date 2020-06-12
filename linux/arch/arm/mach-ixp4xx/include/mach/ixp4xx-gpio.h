/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * arch/arm/mach-ixp4xx/include/mach/ixp4xx-gpio.h
 *
 * Quick access definitions for the GPIO registers of the IXP4xx family.
 * These are already defined in the proper GPIO driver code, these here
 * are for legacy drivers that want direct access.
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 */

#ifndef _ASM_ARM_IXP4XX_GPIO_H_
#define _ASM_ARM_IXP4XX_GPIO_H_
                                                                                
/*
 * Constants to make it easy to access GPIO registers
 */
#define IXP4XX_GPIO_GPOUTR_OFFSET       0x00
#define IXP4XX_GPIO_GPOER_OFFSET        0x04
#define IXP4XX_GPIO_GPINR_OFFSET        0x08
#define IXP4XX_GPIO_GPISR_OFFSET        0x0C
#define IXP4XX_GPIO_GPIT1R_OFFSET	0x10
#define IXP4XX_GPIO_GPIT2R_OFFSET	0x14
#define IXP4XX_GPIO_GPCLKR_OFFSET	0x18
#define IXP4XX_GPIO_GPDBSELR_OFFSET	0x1C

/* 
 * GPIO Register Definitions.
 * [Only perform 32bit reads/writes]
 */
#define IXP4XX_GPIO_REG(x) ((volatile u32 *)(IXP4XX_GPIO_BASE_VIRT+(x)))

#define IXP4XX_GPIO_GPOUTR	IXP4XX_GPIO_REG(IXP4XX_GPIO_GPOUTR_OFFSET)
#define IXP4XX_GPIO_GPOER       IXP4XX_GPIO_REG(IXP4XX_GPIO_GPOER_OFFSET)
#define IXP4XX_GPIO_GPINR       IXP4XX_GPIO_REG(IXP4XX_GPIO_GPINR_OFFSET)
#define IXP4XX_GPIO_GPISR       IXP4XX_GPIO_REG(IXP4XX_GPIO_GPISR_OFFSET)
#define IXP4XX_GPIO_GPIT1R      IXP4XX_GPIO_REG(IXP4XX_GPIO_GPIT1R_OFFSET)
#define IXP4XX_GPIO_GPIT2R      IXP4XX_GPIO_REG(IXP4XX_GPIO_GPIT2R_OFFSET)
#define IXP4XX_GPIO_GPCLKR      IXP4XX_GPIO_REG(IXP4XX_GPIO_GPCLKR_OFFSET)
#define IXP4XX_GPIO_GPDBSELR    IXP4XX_GPIO_REG(IXP4XX_GPIO_GPDBSELR_OFFSET)


#endif /* _ASM_ARM_IXP4XX_GPIO_H_ */
