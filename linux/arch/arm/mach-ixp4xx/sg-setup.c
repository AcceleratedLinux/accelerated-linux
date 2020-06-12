/*
 * arch/arm/mach-ixp4xx/sg-setup.c
 *
 * SnapGear/Cyberguard/SecureComputing/McAfee board-setup 
 *
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 * Copyright (C) 2004-2009 Greg Ungerer <gerg@snapgear.com>
 *
 * Original Author: Deepak Saxena <dsaxena@mvista.com>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>
#include <linux/leds.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include "irqs.h"

#ifdef	__ARMEB__
#define	REG_OFFSET	3
#else
#define	REG_OFFSET	0
#endif

extern void ixp4xx_map_io(void);
extern void ixp4xx_init_irq(void);

/*
 *	Console serial port (always the high speed serial port)
 */
static struct resource sg_uart_resources[] = {
	{
		.start		= IXP4XX_UART1_BASE_PHYS,
		.end		= IXP4XX_UART1_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	},
	{
		.start		= IXP4XX_UART2_BASE_PHYS,
		.end		= IXP4XX_UART2_BASE_PHYS + 0x0fff,
		.flags		= IORESOURCE_MEM
	}
};

static struct plat_serial8250_port sg_uart_data[] = {
	{
		.mapbase	= (IXP4XX_UART1_BASE_PHYS),
		.membase	= (char*)(IXP4XX_UART1_BASE_VIRT + REG_OFFSET),
		.irq		= IRQ_IXP4XX_UART1,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,	
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL
	},
	{
		.mapbase	= (IXP4XX_UART2_BASE_PHYS),
		.membase	= (char*)(IXP4XX_UART2_BASE_VIRT + REG_OFFSET),
		.irq		= IRQ_IXP4XX_UART2,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,	
		.regshift	= 2,
		.uartclk	= IXP4XX_UART_XTAL
	},
	{ },
};

static struct platform_device sg_uart = {
	.name			= "serial8250",
	.id			= 0,
	.dev.platform_data	= sg_uart_data,
	.num_resources		= 2,
	.resource		= sg_uart_resources
};

#ifdef CONFIG_LEDS_GPIO

#ifdef CONFIG_MACH_SG565
#define SG_LED_HEARTBEAT	2
#define SG_LED_ETHERNET		3
#define SG_LED_USB		4
#define SG_LED_WIRELESS		5
#define SG_LED_SERIAL		6
#define SG_LED_ONLINE		7
#define SG_LED_VPN		10
#endif

static struct gpio_led sg_led_pins[] = {
#ifdef SG_LED_HEARTBEAT
	{
		.name		= "heartbeat",
		.gpio		= SG_LED_HEARTBEAT,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_ETHERNET
	{
		.name		= "ethernet",
		.gpio		= SG_LED_ETHERNET,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_USB
	{
		.name		= "usb",
		.gpio		= SG_LED_USB,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_WIRELESS
	{
		.name		= "wireless",
		.gpio		= SG_LED_WIRELESS,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_SERIAL
	{
		.name		= "serial",
		.gpio		= SG_LED_SERIAL,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_ONLINE
	{
		.name		= "online",
		.gpio		= SG_LED_ONLINE,
		.active_low	= true,
	},
#endif
#ifdef SG_LED_VPN
	{
		.name		= "vpn",
		.gpio		= SG_LED_VPN,
		.active_low	= true,
	},
#endif
};

static struct gpio_led_platform_data sg_led_data = {
	.num_leds		= ARRAY_SIZE(sg_led_pins),
	.leds			= sg_led_pins,
};

static struct platform_device sg_leds = {
	.name			= "leds-gpio",
	.id			= -1,
	.dev.platform_data	= &sg_led_data,
};
#endif

void __init sg_map_io(void) 
{
	ixp4xx_map_io();
}

static struct platform_device *sg_devices[] __initdata = {
	&sg_uart,
#ifdef CONFIG_LEDS_GPIO
	&sg_leds,
#endif
};

static void __init sg_init(void)
{
	ixp4xx_sys_init();
	platform_add_devices(sg_devices, ARRAY_SIZE(sg_devices));
}

#ifdef CONFIG_ARCH_SE4000
MACHINE_START(SE4000, "SnapGear/SE4000")
	/* Maintainer: Secure Computing Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG640) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG640, "McAfee/SG640")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG560) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG560, "McAfee/SG560")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG560USB) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG560USB, "McAfee/SG560U")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG560ADSL) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG560ADSL, "McAfee/SG560D")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG565) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG565, "McAfee/SG565")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG580) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG580, "McAfee/SG580")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG590) || defined(CONFIG_MACH_SGARMAUTO)
MACHINE_START(SG590, "McAfee/SG590")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_SE5100
MACHINE_START(SE5100, "SecureComputing/SE5100")
	/* Maintainer: Secure Computing Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_ESS710
/*
 *  Hard set the ESS710 memory size to be 128M. Early boot loaders
 *  passed in 64MB in their boot tags, but now we really can use the
 *  128M that the hardware has.
 */

static void __init ess710_fixup(struct tag *tags, char **cmdline)
{
	struct tag *t = tags;

	for (; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_MEM) {
			printk("ESS710: fixing memory size from %dMiB to 128MiB\n",
				t->u.mem.size / (1024 * 1024));
			t->u.mem.start = PHYS_OFFSET;
			t->u.mem.size  = (128*1024*1024);
			break;
		}
	}
}

MACHINE_START(ESS710, "SecureComputing/SG710")
	/* Maintainer: Secure Computing Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.fixup		= ess710_fixup,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#if defined(CONFIG_MACH_SG720)
#include <linux/i2c.h>

static struct i2c_board_info __initdata sg720_board_i2c_info[] = {
        {
                I2C_BOARD_INFO("m41t11", 0x68),
        },
};

static int __init sg720_i2c_init(void)
{
	i2c_register_board_info(0, sg720_board_i2c_info, ARRAY_SIZE(sg720_board_i2c_info));
	return 0;
}
arch_initcall(sg720_i2c_init);

MACHINE_START(SG720, "McAfee/SG720")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_SG8100
MACHINE_START(SG8100, "McAfee/SG8100")
	/* Maintainer: McAfee Corporation */
	.atag_offset	= 0x100,
	.map_io		= sg_map_io,
	.init_irq	= ixp4xx_init_irq,
	.init_time	= ixp4xx_timer_init,
	.init_machine	= sg_init,
#ifdef CONFIG_ZONE_DMA
	.dma_zone_size	= SZ_64M,
#endif
	.restart	= ixp4xx_restart,
MACHINE_END
#endif

