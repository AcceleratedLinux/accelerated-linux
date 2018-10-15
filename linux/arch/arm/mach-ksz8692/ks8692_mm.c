/*
 *  linux/arch/arm/mach-ksz8692/ks8692_mm.c
 *
 *  Copyright (C) 2006-2009 Micrel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/interrupt.h>

#include <asm/system_info.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/param.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include <mach/hardware.h>
#include <mach/ks8692_utils.h>

#ifdef CONFIG_KSZ8692VB
unsigned long system_time_tick;
EXPORT_SYMBOL( system_time_tick );
#endif

unsigned int ksz_system_bus_clock;
EXPORT_SYMBOL( ksz_system_bus_clock );


#define	CONFIG_MTD_PHYSMAP_LEN	(1024 * 1024)


/*
 * All IO addresses are mapped onto VA 0xFFFx.xxxx, where x.xxxx
 * is the (PA >> 12).
 *
 * Setup a VA for the Integrator interrupt controller (for header #0,
 * just for now).
 */


/*
 * Logical    Physical
 * E0000000   10000000   PCI I/O             PHYS_PCI_IO_BASE        (max 64K)
 * F0000000   03200000   External IO         PHYS_EXTERNAL_IO_BASE   (max 16MB)
 * F0A00000   1C000000   Flash Space         PHYS_FLASH_MEM_BASE     (max 32MB)
 * F00F0000   1FFF0000   Chip Register Base  PHYS_REG_BASE           (max 64K)
 * 20000000   20000000   Host bridge memory  PHYS_PCIBG_MEM_BASE     (max 256M)
 * C8xxx000   30000000   PCI memory          PHYS_PCI_MEM_BASE       (max 256M)
 * C0000000   00000000   System memory       PAGE_OFFSET             (max 256M)
 * D0000000                                  HIGH_MEMORY
 * D0800000                                  VMALLOC_START
 * E0000000                                  VMALLOC_END
 */

static struct map_desc ks8692_io_desc[] __initdata = {
	{
		.virtual = IO_ADDRESS( KS8692_IO_BASE ),
		.pfn = __phys_to_pfn( KS8692_IO_BASE ),
		.length = KS8692_IO_SIZE,
		.type = MT_DEVICE,
	},
	{
		.virtual = PCI_IO_VADDR,
		.pfn = __phys_to_pfn( KS8692_PCI_IO_BASE ),
		.length = KS8692_PCI_IO_SIZE,
		.type = MT_DEVICE,
	},
	{ 
		.virtual = KS8692_EXTIO_VIRT_BASE,
		.pfn = __phys_to_pfn( KS8692_EXTIO_BASE ),
		.length = KS8692_EXTIO_SIZE,
		.type = MT_DEVICE,
	}
};

static void __init ks8692_processor_info(void)
{
	unsigned long id, rev;

	id = __raw_readl( VIO( KS8692_DEVICE_ID ));
	rev = __raw_readl( VIO( KS8692_REVISION_ID ));

	printk( "Pegasus ID=%04lx  SubID=%02lx  Revision=%02lx\n", id,
		( rev & ( 0x7 << 5 )) >> 5, ( rev & 0x1F ));
	system_rev = rev & 0x1F;
	system_serial_high = ( id << 16 ) | (( rev & ( 0x7 << 5 )) << 3 ) |
		system_rev;
#ifdef CONFIG_KSZ8692VB
	if ( ( rev & 0x1F ) < 2 )
		printk( "This kernel does not work for this chip.\n" );
#endif
}

static unsigned int sysclk[4] = { 50000, 125000, 166700, 200000 };
static unsigned int cpuclk[8] = { 50, 125, 166, 200, 250, 250, 250, 250 };
static unsigned int ipsecclk[4] = { 50, 125, 166, 200 };
static unsigned int memclk[4] = { 100, 125, 166, 200 };
static unsigned int pciclk[2] = { 33, 66 };

static void __init ks8692_clock_info(void)
{
	unsigned int sys_clock = __raw_readl( VIO( KS8692_SYSTEM_BUS_CLOCK ));
	int bus = sys_clock & SYSTEM_BUS_CLOCK_MASK;
	int mem = ( sys_clock & MEM_CLOCK_MASK ) >> 2;
	int cpu = ( sys_clock & CPU_CLOCK_MASK ) >> 4;
	int pci = ( sys_clock & PCI_BUS_CLOCK_66 ) >> 7;
	int ipsec = ( sys_clock & IPSEC_CLOCK_MASK ) >> 9;

	printk("Clocks: System %u MHz, CPU %u MHz, DDR %u MHz, PCI %u MHz, \
IPsec %u MHz\n",
		sysclk[ bus ] / 1000, cpuclk[ cpu ], memclk[ mem ],
		pciclk[ pci ], ipsecclk[ ipsec ]);
	ksz_system_bus_clock = sysclk[ bus ];
}

static void __init ks8692_map_io(void)
{
	iotable_init(ks8692_io_desc, ARRAY_SIZE(ks8692_io_desc));

	ks8692_processor_info();
	ks8692_clock_info();
}

#define KS8692_VALID_INT1	0xff0ff7f0
#define KS8692_VALID_INT2	0x03ffffff

#define KS8692_NO_MASK_IRQ1  ( \
	( 1 << KS8692_INT_LAN_STOP_RX ) | \
	( 1 << KS8692_INT_LAN_STOP_TX ) | \
	( 1 << KS8692_INT_LAN_BUF_RX_STATUS ) | \
	( 1 << KS8692_INT_LAN_BUF_TX_STATUS ) | \
	( 1 << KS8692_INT_LAN_RX_STATUS ) | \
	( 1 << KS8692_INT_LAN_TX_STATUS ) | \
	( 1 << KS8692_INT_WAN_STOP_RX ) | \
	( 1 << KS8692_INT_WAN_STOP_TX ) | \
	( 1 << KS8692_INT_WAN_BUF_RX_STATUS ) | \
	( 1 << KS8692_INT_WAN_BUF_TX_STATUS ) | \
	( 1 << KS8692_INT_WAN_RX_STATUS ) | \
	( 1 << KS8692_INT_WAN_TX_STATUS ) | \
	0 )

#define KS8692_NO_MASK_IRQ2  ( \
	( 1 << KS8692_INT_UART1_TX ) | \
	( 1 << KS8692_INT_UART1_RX ) | \
	( 1 << KS8692_INT_UART1_LINE_ERR ) | \
	( 1 << KS8692_INT_UART1_MODEMS ) | \
	( 1 << KS8692_INT_UART2_TX ) | \
	( 1 << KS8692_INT_UART2_RX ) | \
	( 1 << KS8692_INT_UART2_LINE_ERR ) | \
	( 1 << KS8692_INT_UART3_TX ) | \
	( 1 << KS8692_INT_UART3_RX ) | \
	( 1 << KS8692_INT_UART3_LINE_ERR ) | \
	( 1 << KS8692_INT_UART4_TX ) | \
	( 1 << KS8692_INT_UART4_RX ) | \
	( 1 << KS8692_INT_UART4_LINE_ERR ) | \
	0 )


static void sc_ack_irq2 (struct irq_data *d)
{
	int irq = d->irq - LOW_IRQS;
	__raw_writel((1 << irq), VIO( KS8692_INT_STATUS2 ));
}

static void sc_mask_irq1 (struct irq_data *d)
{
	__raw_writel( __raw_readl( VIO( KS8692_INT_ENABLE1 )) &
		~(1 << d->irq), VIO( KS8692_INT_ENABLE1 ));
}

static void sc_mask_irq2 (struct irq_data *d)
{
	int irq = d->irq - LOW_IRQS;
	__raw_writel( __raw_readl( VIO( KS8692_INT_ENABLE2 )) &
		~(1 << irq), VIO( KS8692_INT_ENABLE2 ));
}

static void sc_unmask_irq1 (struct irq_data *d)
{
	__raw_writel( __raw_readl( VIO( KS8692_INT_ENABLE1 )) |
		(1 << d->irq), VIO( KS8692_INT_ENABLE1 ));
}

static void sc_unmask_irq2 (struct irq_data *d)
{
	int irq = d->irq - LOW_IRQS;
	__raw_writel( __raw_readl( VIO( KS8692_INT_ENABLE2 )) |
		(1 << irq), VIO( KS8692_INT_ENABLE2 ));
}

static unsigned int sc_startup (struct irq_data *d)
{
	if ( 2 + LOW_IRQS <= d->irq  &&  d->irq <= 5 + LOW_IRQS ) {
		ks8692_util_enable_interrupt(d->irq - LOW_IRQS, 1);
	}
	sc_unmask_irq2( d );
	return 0;
}

static void sc_shutdown (struct irq_data *d)
{
	sc_mask_irq2( d );
	if ( 2 + LOW_IRQS <= d->irq  &&  d->irq <= 5 + LOW_IRQS ) {
		ks8692_util_enable_interrupt(d->irq - LOW_IRQS, 0);
	}
}

static struct irq_chip sc_chip2;
static struct irq_chip sc_edge_chip2;

static int sc_irq_set_type (struct irq_data *d, unsigned int type)
{
	unsigned int irqno = d->irq;
	unsigned int ctrl, mode;
	int level_triggered = 0;

	if ( 2 + LOW_IRQS > irqno  ||  irqno > 5 + LOW_IRQS ) {
		return -EINVAL;
	}

	irqno -= 2 + LOW_IRQS;
	irqno <<= 2;

	ctrl = KS8692_READ_REG( KS8692_GPIO_CTRL );
	ctrl &= ~( 0x7 << irqno );

	mode = GPIO_INT_LOW;
	switch (type) {
		case IRQ_TYPE_LEVEL_LOW:
			level_triggered = 1;
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			mode = GPIO_INT_HIGH;
			level_triggered = 1;
			break;
		case IRQ_TYPE_EDGE_RISING:
			mode = GPIO_INT_RISE_EDGE;
			break;
		case IRQ_TYPE_EDGE_FALLING:
			mode = GPIO_INT_FALL_EDGE;
			break;
		case IRQ_TYPE_EDGE_BOTH:
			mode = GPIO_INT_BOTH_EDGE;
			break;
		default:
			return -EINVAL;
	}

	if (level_triggered) {
		irq_set_chip_and_handler(irqno, &sc_chip2, handle_level_irq);
	}
	else {
		irq_set_chip_and_handler(irqno, &sc_edge_chip2, handle_edge_irq);
	}

	ctrl |= mode << irqno;
	KS8692_WRITE_REG( KS8692_GPIO_CTRL, ctrl );
	return 0;
}

static struct irq_chip sc_chip1 = {
	.name		= "pegasus",
	.irq_ack	= sc_mask_irq1,
	.irq_mask	= sc_mask_irq1,
	.irq_unmask	= sc_unmask_irq1,
};

static struct irq_chip sc_chip2 = {
	.name		= "pegasus",
	.irq_startup	= sc_startup,
	.irq_shutdown	= sc_shutdown,
	.irq_set_type	= sc_irq_set_type,
	.irq_ack	= sc_mask_irq2,
	.irq_mask	= sc_mask_irq2,
	.irq_unmask	= sc_unmask_irq2,
};

static struct irq_chip sc_edge_chip2 = {
	.name		= "pegasus",
	.irq_startup	= sc_startup,
	.irq_shutdown	= sc_shutdown,
	.irq_set_type	= sc_irq_set_type,

	.irq_ack	= sc_ack_irq2,
	.irq_mask	= sc_mask_irq2,
	.irq_unmask	= sc_unmask_irq2,
};

static void __init ks8692_init_irq(void)
{
	unsigned int i;
	unsigned int j;

	/* Disable all interrupts initially. */
	__raw_writel( 0, VIO( KS8692_INT_CONTL1 ));
	__raw_writel( 0, VIO( KS8692_INT_CONTL2 ));
	__raw_writel( 0, VIO( KS8692_INT_ENABLE1 ));
	__raw_writel( 0, VIO( KS8692_INT_ENABLE2 ));

	/* Assign low IRQ (0 - 31) */
	for (i = 0; i < LOW_IRQS; i++) {
		if (((1 << i) & KS8692_VALID_INT1) != 0) {
			if ( (( 1 << i ) & KS8692_NO_MASK_IRQ1 ) != 0 )
				irq_set_chip_and_handler(i, &dummy_irq_chip, handle_level_irq);
			else
				irq_set_chip_and_handler(i, &sc_chip1, handle_level_irq);
			irq_clear_status_flags(i, IRQ_NOREQUEST);
		}
	}

	/* Assign high IRQ (32 - 63) */
	for (i = LOW_IRQS, j = 0; i < NR_IRQS; i++, j++) {
		if (((1 << j) & KS8692_VALID_INT2) != 0) {
			if ( (( 1 << j ) & KS8692_NO_MASK_IRQ2 ) != 0 )
				irq_set_chip_and_handler(i, &dummy_irq_chip, handle_level_irq);
			else
				irq_set_chip_and_handler(i, &sc_chip2, handle_level_irq);
			irq_clear_status_flags(i, IRQ_NOREQUEST);
		}
	}
}

#define irq_suspend NULL
#define irq_resume NULL

static struct bus_type irq_subsys = {
	.name		= "irq",
	.dev_name	= "irq",
};

static struct device irq_device = {
	.id	= 0,
	.bus	= &irq_subsys,
};

static int __init irq_init_sysfs(void)
{
	int ret = subsys_system_register(&irq_subsys, NULL);
	if (ret == 0)
		ret = device_register(&irq_device);
	return ret;
}

device_initcall(irq_init_sysfs);

#ifdef CONFIG_MTD_CFI
/*
 * Flash handling.
 */

static int ks8692_flash_init(void)
{
	return 0;
}

static void ks8692_flash_exit(void)
{
}

static void ks8692_flash_set_vpp(int on)
{
}

static struct flash_platform_data ks8692_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
	.init		= ks8692_flash_init,
	.exit		= ks8692_flash_exit,
	.set_vpp	= ks8692_flash_set_vpp,
};

static struct resource cfi_flash_resource = {
	.start		= KS8692_FLASH_START,
	.end		= KS8692_FLASH_START + CONFIG_MTD_PHYSMAP_LEN - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device cfi_flash_device = {
	.name		= "armflash",
	.id		= 0,
	.dev		= {
	.platform_data	= &ks8692_flash_data,
	},
	.num_resources	= 1,
	.resource	= &cfi_flash_resource,
};
#endif

#if defined( CONFIG_MICREL_WATCHDOG ) || defined( CONFIG_MICREL_WATCHDOG_MODULE )
static struct platform_device watchdog_device = {
	.name		= "micrel_wdt",
	.id		= -1,
	.num_resources	= 0,
};
#endif


#include "devs.c"

struct ks8692_board {
	struct platform_device  **devices;
	unsigned int              devices_count;
//	struct clk              **clocks;
//	unsigned int              clocks_count;
};

static struct platform_device *ks8692_devices[] __initdata = {
#if 0
	&ks8692_device_syscfg1,
	&ks8692_device_pci,
	&ks8692_device_ddr,
	&ks8692_device_sdram,
	&ks8692_device_wan,
	&ks8692_device_lan,
#endif
	&ks8692_device_usbgadget,
	&ks8692_device_ohci,
	&ks8692_device_ehci,
	&ks8692_device_i2c,
	&ks8692_device_sdi,
	&ks8692_device_spi,
#if 0
	&ks8692_device_syscfg2,
#endif

};

static struct ks8692_board _ks8692_board __initdata = {
	.devices       = ks8692_devices,
	.devices_count = ARRAY_SIZE(ks8692_devices)
};


static int ks8692_add_devices(struct ks8692_board *board)
{
	int ret;

	if (board != NULL) {
		struct platform_device **ptr = board->devices;
		int i;

		for (i = 0; i < board->devices_count; i++, ptr++)
		{
			ret = platform_device_register(*ptr);

			if (ret)
			{
				printk(KERN_ERR "ks8692: failed to add board device %s (%d) @%p\n", (*ptr)->name, ret, *ptr);
			}
		}

		/* mask any error, we may not need all these board
		 * devices */
		ret = 0;
	}

	return ret;
}


static void __init ks8692_init(void)
{

#if defined( CONFIG_MICREL_WATCHDOG ) || defined( CONFIG_MICREL_WATCHDOG_MODULE )
	platform_device_register( &watchdog_device );
#endif
	ks8692_add_devices(&_ks8692_board);

#ifdef CONFIG_MTD_CFI
	platform_device_register(&cfi_flash_device);
#if 0
#ifdef CONFIG_MTD_PARTITIONS
	physmap_set_partitions(physmap_partitions, NUM_PARTITIONS);
#endif
#endif
#endif
}


/*
 * How long is the timer interval?
 */
#define TIMER_INTERVAL	     (TICKS_PER_uSEC * 1000000 / CONFIG_HZ)
#define TIMER_DATA_VALUE     (TIMER_INTERVAL >> 1)
#define TIMER_PULSE_VALUE    (TIMER_INTERVAL - TIMER_DATA_VALUE)
#define TIMER_VALUE          (TIMER_DATA_VALUE + TIMER_PULSE_VALUE)
#define TICKS2USECS(x)	     ((x) / TICKS_PER_uSEC)
#define TIMER_GET_VALUE()    (__raw_readl(VIO(KS8692_TIMER1)) + __raw_readl(VIO(KS8692_TIMER1_PCOUNT)))


#ifdef CONFIG_GENERIC_TIME
#include "time.c"

#else


/*
 * Returns number of ms since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 */
static unsigned long ks8692_gettimeoffset(void)
{
	unsigned long ticks1, ticks2, status;

	/*
	 * Get the current number of ticks.  Note that there is a race
	 * condition between us reading the timer and checking for
	 * an interrupt.  We get around this by ensuring that the
	 * counter has not reloaded between our two reads.
	 */
	ticks2 = TIMER_GET_VALUE();
	do {
		ticks1 = ticks2;
		status = __raw_readl( VIO( KS8692_INT_STATUS2 ));
		ticks2 = TIMER_GET_VALUE();
	} while (ticks2 > ticks1);

	/*
	 * Number of ticks since last interrupt.
	 */
	ticks1 = TIMER_VALUE - ticks2;

	/*
	 * Interrupt pending?  If so, we've reloaded once already.
	 */
	if (status & KS8692_INTMASK_TIMERINT1)
		ticks1 += TIMER_VALUE;

	/*
	 * Convert the ticks to usecs
	 */
	return TICKS2USECS(ticks1);
}
#endif


#ifndef CONFIG_GENERIC_CLOCKEVENTS
/*
 * IRQ handler for the timer
 */
static irqreturn_t ks8692_timer_interrupt(int irq, void *dev_id)
{
	// ...clear the interrupt
        __raw_writel( KS8692_INTMASK_TIMERINT1, VIO( KS8692_INT_STATUS2 ));

	timer_tick();
	return IRQ_HANDLED;
}


static struct irqaction ks8692_timer_irq = {
	.name		= "timer",
	.flags		= IRQF_TIMER,
	.handler	= ks8692_timer_interrupt
};


#if defined(CONFIG_DELAYED_TIMER) || (defined(CONFIG_SNAPDOG) && defined(CONFIG_MACH_ACM500X))
static inline void disable_timer0 ( void )
{
	unsigned long reg;

	reg = KS8692_READ_REG( KS8692_TIMER_CTRL );
	reg &= ~( TIMER_TIME0_ENABLE );
	KS8692_WRITE_REG( KS8692_TIMER_CTRL, reg );
}  /* disable_timer0 */


static inline void enable_timer0 ( void )
{
	unsigned long reg;

	reg = KS8692_READ_REG( KS8692_TIMER_CTRL );
	reg |= TIMER_TIME0_ENABLE;
	KS8692_WRITE_REG( KS8692_TIMER_CTRL, reg );
}  /* enable_timer0 */


void service_watchdog( void );

void service_watchdog( void )
{
	disable_timer0();
	/* Give ~5.3 seconds (25Mhz clock) */
	KS8692_WRITE_REG(KS8692_TIMER0, (0x8 << 24) | 0xFF);
	KS8692_WRITE_REG(KS8692_TIMER0_PCOUNT, 2);
	enable_timer0();
}

#ifdef CONFIG_DELAYED_TIMER
static inline void setup_timer0 (
	unsigned int microsec )
{
	unsigned long interval;
	unsigned long pcount;
	unsigned long value;

	interval = TICKS_PER_uSEC * microsec;
	value = interval >> 1;
	pcount = interval - value;
	KS8692_WRITE_REG( KS8692_TIMER0, value );
	KS8692_WRITE_REG( KS8692_TIMER0_PCOUNT, pcount );
	enable_timer0();
}  /* setup_timer0 */


static struct delayed_tasklet delayed_tasklet_list = { NULL, 0 };

void tasklet_delayed_kill (
	struct delayed_tasklet *t )
{
	while ( t->count ) {
		yield();
	}
}  /* tasklet_delayed_kill */

EXPORT_SYMBOL(tasklet_delayed_kill);


void tasklet_delayed_schedule (
	struct delayed_tasklet *t,
	unsigned int microsec )
{
	unsigned long flags;

	microsec /= DELAYED_TIMER_RESOLUTION;
	if ( !microsec )
		microsec = 1;
	t->count = microsec;
	local_irq_save(flags);
	if ( !delayed_tasklet_list.next )
		setup_timer0( DELAYED_TIMER_RESOLUTION );
	t->next = delayed_tasklet_list.next;
	delayed_tasklet_list.next = t;
	local_irq_restore(flags);
}

EXPORT_SYMBOL(tasklet_delayed_schedule);


irqreturn_t delayed_timer_interrupt (int irq, void *dev_id)
{
	struct delayed_tasklet *prev = &delayed_tasklet_list;
	struct delayed_tasklet *list = delayed_tasklet_list.next;

	KS8692_WRITE_REG( KS8692_INT_STATUS2, KS8692_INTMASK_TIMERINT0 );

	while ( list ) {
		struct delayed_tasklet *t = list;

		list = list->next;

		if ( !( --t->count ) ) {
			tasklet_schedule( &t->tasklet );
			prev->next = t->next;
			continue;
		}
		prev = t;
	}
	if ( !delayed_tasklet_list.next )
		disable_timer0();

	return IRQ_HANDLED;
}  /* delayed_timer_interrupt */


static struct irqaction delayed_timer_irq = {
	.name		= "delayed timer",
	.flags		= 0,
	.handler	= delayed_timer_interrupt
};
#endif /* CONFIG_DELAYED_TIMER */
#endif /* CONFIG_DELAYED_TIMER || CONFIG_SNAPDOG */

void __init pegasus_timer_init (void);

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static void __init ks8692_init_time ( void )
{
	/*
	 * Initialise to a known state (all timers off)
	 */
        __raw_writel( 0, VIO( KS8692_TIMER_CTRL ));

	/*
	 * Make irqs happen for the system timer
	 */
	setup_irq((KS8692_INT_TIMERINT1 + LOW_IRQS), &ks8692_timer_irq);

#ifdef CONFIG_DELAYED_TIMER
	setup_irq((KS8692_INT_TIMERINT0 + LOW_IRQS), &delayed_timer_irq);
#endif

	/*
	 * enable timer 1, timer 0 for watchdog
	 */
        __raw_writel( TIMER_DATA_VALUE, VIO( KS8692_TIMER1 ));
        __raw_writel( TIMER_PULSE_VALUE, VIO( KS8692_TIMER1_PCOUNT ));
        __raw_writel( TIMER_TIME1_ENABLE, VIO( KS8692_TIMER_CTRL ));

#ifdef CONFIG_KSZ8692VB
	system_time_tick = KS8692_READ_REG( KS8692_TIMER1_COUNTER );
#endif

#ifdef CONFIG_GENERIC_TIME
	pegasus_timer_init();
#endif
}
#endif

MACHINE_START(KS8695P, "Micrel Pegasus")
	/* Maintainer: Micrel, Inc. */
	.atag_offset	= 0x00000100,
	.init_machine	= ks8692_init,
	.map_io		= ks8692_map_io,
	.init_irq	= ks8692_init_irq,
#ifdef CONFIG_GENERIC_CLOCKEVENTS
	.init_time	= pegasus_timer_init,
#else
	.init_time	= ks8692_init_time,
#endif
MACHINE_END

MACHINE_START(ACM500X, "OpenGear ACM500X")
	/* Maintainer: OpenGear Inc. */
	.atag_offset	= 0x00000100,
	.init_machine	= ks8692_init,
	.map_io		= ks8692_map_io,
	.init_irq	= ks8692_init_irq,
#ifdef CONFIG_GENERIC_CLOCKEVENTS
	.init_time	= pegasus_timer_init,
#else
	.init_time	= ks8692_init_time,
#endif
MACHINE_END

MACHINE_START(ACM550X, "OpenGear ACM550X")
	/* Maintainer: OpenGear Inc */
	.atag_offset	= 0x00000100,
	.init_machine	= ks8692_init,
	.map_io		= ks8692_map_io,
	.init_irq	= ks8692_init_irq,
#ifdef CONFIG_GENERIC_CLOCKEVENTS
	.init_time	= pegasus_timer_init,
#else
	.init_time	= ks8692_init_time,
#endif
MACHINE_END
