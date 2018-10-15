/***************************************************************************/

/*
 *	linux/drivers/char/mcfwatchdog.c
 *
 *	Copyright (C) 1999-2000, Greg Ungerer (gerg@snapgear.com)
 * 	Copyright (C) 2000  Lineo Inc. (www.lineo.com)  
 *
 * Changes:
 * 10/28/2004    Christian Magnusson <mag@mag.cx>
 *               Bug: MCFSIM_SYPCR can only be written once after reset!
 *               MCF5272 support copied from 2.4.x driver.
 *               Reset on overflow. (For 5206e at least)
 *               Added module support.
 *               I have noticed that some flash-identification from mtd
 *                 locks the processor too long, and therefor this watchdog
 *                 has to be used as a module and started after mtd is done.
 *
 * Changes:
 * 06/01/2007    David Wu (www.ArcturusNetworks.com)
 *               Added support for MCF5329, no IRQ or timer version
 */

/***************************************************************************/

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/machdep.h>
#include <asm/coldfire.h>
#include <asm/mcftimer.h>
#include <asm/mcfsim.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/uaccess.h>

/***************************************************************************/

/*
 *	Define the watchdog vector.
 */
#ifdef CONFIG_M5272
#define	IRQ_WATCHDOG	92
#define	TIMEPOLL	100
#else
#define	IRQ_WATCHDOG	250
#define	TIMEPOLL	HZ/100
#endif

#if defined(CONFIG_M5206e)
#define RESET_ON_SWTR
#endif

/***************************************************************************/

void	watchdog_alive(unsigned long arg);
#ifndef CONFIG_M532x
static irqreturn_t watchdog_timeout(int irq, void *dummy, struct pt_regs *fp);
#endif
void watchdog_disable(void);
void watchdog_enable(void);

#ifndef MODULE
extern void	dump(struct pt_regs *fp);
#endif

/*
 *	Data for registering the watchdog alive routine with ticker.
 */
static struct timer_list	watchdog_timerlist;
#ifndef CONFIG_M532x
static int watchdog_overflows;
#endif
static unsigned long wdt_is_open;
static char expect_close;

#ifdef CONFIG_OLDMASK
/*
 *	The old mask 5307 has a broken watchdog timer. It will interrupt
 *	you regardless of writing to its "alive" register. It can still
 *	be useful but you have to play some tricks with it. This code
 *	supports a clock ticker timeout. If the right number of clock
 *	ticks are not counted then it is assumed that the watchdog saved
 *	us from a bad bus cycle.
 */
#define	SWTREF_COUNT	25

int	swt_inwatchdog = 0;		/* Has watchdog expired */
int	swt_doit = 0;			/* Start delay before tripping */
int	swt_lastjiffies = 0;		/* Tick count at last watchdog */
int	swt_reference = SWTREF_COUNT;	/* Refereence tick count */
#endif

#ifdef CONFIG_M532x
static ssize_t wdt_write(struct file *file, const char *data,
                            size_t len, loff_t *ppos)
{
        if (len) {
                if (1) {  /* expect_close can be set */
                        size_t i;

                        expect_close = 0;

                        for (i = 0; i != len; i++) {
                                char c;

                                if (get_user(c, data + i))
                                        return -EFAULT;
                                if (c == 'V')
                                        expect_close = 1;
                        }
                }
                /* Refresh watchdog timer. */
                watchdog_alive(0);
        }
 
        return len;
}

static int wdt_open(struct inode *inode, struct file *file)
{
        if (test_and_set_bit(0, &wdt_is_open))
                return -EBUSY;
	watchdog_enable();

        return 0;
}

static int wdt_close(struct inode *inode, struct file *file)
{
        if (expect_close == 1) {
		watchdog_disable();
        } else {
                printk(KERN_CRIT "Unexpected close, watchdog will be triggered!\n");
                watchdog_alive(0);
        }

        clear_bit(0, &wdt_is_open);
        expect_close = 0;

        return 0;
}
#endif


static struct file_operations watchdog_fops = {
	.owner		= THIS_MODULE,
#ifdef CONFIG_M532x
	write:		wdt_write,
	open:		wdt_open,
	release:	wdt_close,
#endif
};

static struct miscdevice watchdog_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &watchdog_fops,
};

/***************************************************************************/

/*
 *	Software Watchdog Timer enable. Seems to be the same across all
 *	ColdFire CPU members.
 */
void watchdog_enable(void)
{
#ifdef CONFIG_M5272
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WRRR) = 0x2001;	// upper watchdog limit
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WIRR) = 0x1000;	// we don't do interrupts, just reset (o;
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WCR) = 0x0000;	// clear counter
#else  /* CONFIG_M5272 */
	volatile unsigned char	*mbar = (volatile unsigned char *) MCF_MBAR;
	*(mbar + MCFSIM_SWSR) = 0x55;
	*(mbar + MCFSIM_SWSR) = 0xaa; // kick watchdog
#ifdef CONFIG_M532x
	*(volatile unsigned short *)(MCFSIM_CWCR) = 0x01a0 | 30;  /* timeout = 2^30, CWRI= 01*/
#else
	/*
	  SYPCR Can only be written once after system reset!
	  0x80 Software Watchdog, 0="Disable" / 1="Enable"
	  0x40 Software Watchdog, 0="level7 interrupt" / 1="reset"
	  0x20 Watchdog prescaled (SWP), 0="1" / 1="512" 
	  0x10 Timing SWT1
	  0x08 Timing SWT0
	  	SWT|SWT1|SWT0
	  	000=2^9  001=2^11 010=2^13 011=2^15 / System Freq.
		100=2^18 101=2^20 110=2^22 111=2^24 / System Freq.
	  0x04 Bus Timeout Monitor BMTE 0="Disable" / 1="Enable"
	  0x02 Bus Monitor Timing BMT1
	  0x01 Bus Monitor Timing BMT0
	  	BMT1|BMT0
	  	00=1024 01=512 10=256 11=128 system clocks
	*/
#ifdef CONFIG_OLDMASK
	*(mbar + MCFSIM_SYPCR) = 0xbe; // level 7 interrupt, 2^22
#else

#ifdef RESET_ON_SWTR
	*(mbar + MCFSIM_SYPCR) = 0xfe; // reset, 2^22
#else
	*(mbar + MCFSIM_SYPCR) = 0xbe; // level 7 interrupt, 2^22
#endif /* RESET_ON_SWTR */
#endif /* CONFIG_OLDMASK */
#endif
#endif /* CONFIG_M5272 */
}

/***************************************************************************/

void watchdog_disable(void)
{
#ifdef CONFIG_M5272
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WRRR) = 0xFFFE;
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WIRR) = 0x0000;
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WCR) = 0x0000;
#else
	volatile unsigned char	*mbar = (volatile unsigned char *) MCF_MBAR;
	/*
	  If watchdog is set to 'reset', this function is useless...
	  If timer is disabled, this reset will occour at once.
	*/
	*(mbar + MCFSIM_SWSR) = 0x55;
	*(mbar + MCFSIM_SWSR) = 0xaa;
#ifdef CONFIG_M532x
	*(volatile unsigned short *)(MCFSIM_CWCR) &= ~0x0080;  /* disable */
	return;
#endif
#if 0
	/*
	  SYPCR Can only be written once after system reset!
	  This will probably be ignored according to MCF5206E User Manual
	*/
	*(mbar + MCFSIM_SYPCR) = 0x00 /*0x3e*/;
#endif
	mcf_setimr(mcf_getimr() | MCFSIM_IMR_SWD);
#endif
	del_timer(&watchdog_timerlist);
}

/***************************************************************************/

static int watchdog_notify_sys(struct notifier_block *this, unsigned long code,
	void *unused)
{
	if(code==SYS_DOWN || code==SYS_HALT) {
		/* Turn the card off */
		watchdog_disable();
		/*
		  If watchdog is set to 'reset', this function is useless...
		  When timer is disabled, reset will occour during reboot...
		*/
	}
	return NOTIFY_DONE;
}

static struct notifier_block watchdog_notifier = {
	.notifier_call = watchdog_notify_sys,
};

/***************************************************************************/

/*
 *	Process a watchdog timeout interrupt. For a normal clean watchdog
 *	we just do a process dump. For old broken 5307 we need to verify
 *	if this was a real watchdog event or not...
 */
#ifndef CONFIG_M532x
static irqreturn_t watchdog_timeout(int irq, void *dummy, struct pt_regs *fp)
{
#ifdef CONFIG_OLDMASK
#define	TIMEDELAY	45
	/*
	 *	Debuging code for software watchdog. If we get in here
	 *	and timer interrupt counts don't match we know that a
	 *	bad external bus cycle must have locked the CPU.
	 */
	if ((swt_doit++ > TIMEDELAY) &&
	    ((swt_lastjiffies + swt_reference) > jiffies)) {
		if (swt_inwatchdog) {
			cli();
			watchdog_disable();
			mcf_setimr(mcf_getimr() | MCFSIM_IMR_SWD);
			printk("%s(%d): Double WATCHDOG PANIC!!\n",
				__FILE__, __LINE__);
			for (;;)
				;
		}

		swt_inwatchdog++;
		swt_doit = TIMEDELAY - 8;	/* 8 seconds grace */
		printk("mcfwatchdog: expired last=%d(%d) jiffies=%d!\n",
			swt_lastjiffies, swt_reference, jiffies);
#ifndef MODULE
		dump(fp);
#endif
		force_sig(SIGSEGV, current);
		swt_inwatchdog  = 0;
	}
	swt_lastjiffies = jiffies;
#else

#ifdef RESET_ON_SWTR
	/* nothing will be done... reset will occour */
#else  /* RESET_ON_SWTR */
	// lev7 interrupt is used.
	if(++watchdog_overflows >= 10) {

	  printk("mcfwatchdog: expired!\n");
#ifndef MODULE
	dump(fp);
#endif
	  mcf_setimr(mcf_getimr() | MCFSIM_IMR_SWD);
	  HARD_RESET_NOW();
	  for (;;) ;  // hang until reboot
	} else {
	  volatile unsigned char *mbar = (volatile unsigned char *) MCF_MBAR;
	  *(mbar + MCFSIM_SWSR) = 0x55;
	  *(mbar + MCFSIM_SWSR) = 0xaa; // kick watchdog
	}
#endif /* RESET_ON_SWTR */
#endif /* CONFIG_OLDMASK */
	return IRQ_HANDLED;
}
#endif
/***************************************************************************/

static int __init watchdog_init(void)
{
#ifndef CONFIG_M532x
	printk("mcfwatchdog: initializing at vector=%d\n", IRQ_WATCHDOG);
#endif

        if(misc_register(&watchdog_miscdev))
		return -ENODEV;

	if(register_reboot_notifier(&watchdog_notifier)) {
		printk("watchdog: cannot register reboot notifier\n");
		return 1;
	}

#ifndef CONFIG_M532x
	request_irq(IRQ_WATCHDOG, watchdog_timeout, SA_INTERRUPT,
		    "Watchdog Timer", &watchdog_miscdev);

	timer_setup (&watchdog_timerlist, watchdog_alive, 0);
	add_timer(&watchdog_timerlist);
#endif

#ifdef CONFIG_M5272
{
	volatile unsigned long	*icrp;
	icrp = (volatile unsigned long *) (MCF_MBAR + MCFSIM_ICR4);
	*icrp = (*icrp & 0x77707777) | 0x000E0000;
	watchdog_enable();
}
#else  /* CONFIG_M5272 */
#ifdef CONFIG_M532x
{
	printk("mcfwatchdog: Installed\n");

}
#else  /* CONFIG_M532x */
{
	volatile unsigned char	*mbar = (volatile unsigned char *) MCF_MBAR;
	unsigned char ch;

	ch = *(mbar + MCFSIM_RSR);
	printk("mcfwatchdog: Last reset was generated by %s\n",
	       (ch&0x80 ? "HRST": (ch&0x20 ? "SWTR":"")));

#ifdef RESET_ON_SWTR
	// high priority just to make sure watchdog won't overflow.
	*(mbar + MCFSIM_SWDICR) = MCFSIM_ICR_LEVEL1 | MCFSIM_ICR_PRI1;
#else
	*(mbar + MCFSIM_SWDICR) = MCFSIM_ICR_LEVEL1 | MCFSIM_ICR_PRI3;
#endif
	*(mbar + MCFSIM_SWIVR) = IRQ_WATCHDOG;
	mcf_setimr(mcf_getimr() & ~MCFSIM_IMR_SWD);
	watchdog_enable();

	printk("mcfwatchdog: Coldfire watchdog is enabled, \"%s\" is generated on error\n",
	       ((*(mbar + MCFSIM_SYPCR) & 0x40)?
		"Reset" : "Level7 interrupt"));
	
#ifdef MODULE
	if(*(mbar + MCFSIM_SYPCR) & 0x40) {
	  printk("mcfwatchdog: Warning: If module is unloaded, Watchdog will reset card.\n");
	}
#endif
}
#endif /* CONFIG_M532x */
#endif /* CONFIG_M5272 */
	return 0;
}

/***************************************************************************/

static void __exit watchdog_exit(void)
{
#ifdef CONFIG_M5272
	/* Reset watchdog counter */
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WCR) = 0x0000;
#else
	volatile unsigned char	*mbar = (volatile unsigned char *) MCF_MBAR;
	*(mbar + MCFSIM_RSR) = 0; // clear reset cause
#endif
	unregister_reboot_notifier(&watchdog_notifier);
	misc_deregister(&watchdog_miscdev);
	watchdog_disable();
	printk("mcfwatchdog: Coldfire watchdog is disabled and unloaded\n");
}

/***************************************************************************/

void watchdog_alive(struct timer_list *t)
{
#ifdef CONFIG_M5272
	/* Reset watchdog counter */
	*(volatile unsigned short *)(MCF_MBAR + MCFSIM_WCR) = 0x0000;
#else
	volatile unsigned char	*mbar = (volatile unsigned char *) MCF_MBAR;
	*(mbar + MCFSIM_SWSR) = 0x55;
	*(mbar + MCFSIM_SWSR) = 0xaa; // kick watchdog
#endif
#ifndef CONFIG_M532x
	/* Re-arm the watchdog alive poll */
	mod_timer(&watchdog_timerlist, jiffies+TIMEPOLL);
	watchdog_overflows = 0;
#endif
}

/***************************************************************************/

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Greg Ungerer");
MODULE_DESCRIPTION("Coldfire Watchdog Driver");
MODULE_LICENSE("GPL");

/***************************************************************************/
