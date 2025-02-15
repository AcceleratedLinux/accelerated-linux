/****************************************************************************/
/*	vi:set tabstop=4 cindent shiftwidth=4:
 *
 *	ledman.c -- An LED manager,  primarily,  but not limited to SnapGear
 *              devices manages up to 32 seperate LED at once.
 *	            Copyright (C) Lineo, 2000-2001.
 *	            Copyright (C) SnapGear, 2001-2003.
 *
 *	This driver currently supports 4 types of LED modes:
 *
 *	SET      - transient LED's that show activity,  cleared at next poll
 *	ON       - always ON
 *	OFF      - always OFF
 *	FLASHING - a blinking LED with the frequency determined by the poll func
 *
 *	We have two sets of LED's to support non-standard LED usage without
 *	losing previously/during use set of std values.
 *
 *	Hopefully for most cases, adding new HW with new LED patterns will be
 *	as simple as adding two tables, a map and the defaults.
 *
 *	You can, however, add your own functions for XXX_bits, XXX_tick and
 *	take full control over all aspects of the LED's.
 */
/****************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/version.h>
#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/ledman.h>

#if LINUX_VERSION_CODE < 0x020300
#include <linux/malloc.h>
#else
#include <linux/slab.h>
#endif

#if LINUX_VERSION_CODE < 0x020100
#define INIT_RET_TYPE	int
#define Module_init(a)
#elif LINUX_VERSION_CODE < 0x020300
#include <linux/init.h>
#define INIT_RET_TYPE	int
#define Module_init(a)	module_init(a)
#else
#include <linux/init.h>
#define INIT_RET_TYPE	static int __init
#define Module_init(a)	module_init(a)
#endif

#if LINUX_VERSION_CODE < 0x020100
#define Get_user(a, b)	a = get_user(b)
#else
#include <linux/uaccess.h>
#define Get_user(a, b)	get_user(a, b)
#endif

#if LINUX_VERSION_CODE < 0x020100
static struct symbol_table ledman_syms = {
#include <linux/symtab_begin.h>
	X(ledman_cmd),
	X(__ledman_cmdset),
#include <linux/symtab_end.h>
};
#else
EXPORT_SYMBOL(ledman_cmd);
EXPORT_SYMBOL(__ledman_cmdset);
#endif

#if LINUX_VERSION_CODE >= 0x040b00
#include <linux/sched/signal.h>
#endif

#if LINUX_VERSION_CODE >= 0x02061d
#define	IRQT_FALLING		IRQ_TYPE_EDGE_FALLING
static int kill_proc(pid_t pid, int sig, int arg)
{
	struct task_struct *tsk;
	tsk = find_task_by_vpid(pid);
	if (tsk)
		send_sig(sig, tsk, 0);
	return 0;
}
#endif

static int ledman_bits(unsigned long cmd, unsigned long bits);
static void ledman_set(unsigned long bits);
static void ledman_tick(void);

/****************************************************************************/

static struct timer_list	ledman_timerlist;

/****************************************************************************/
/*
 *	some types to make adding new LED modes easier
 *
 *	First the elements for def array specifying default LED behaviour
 */

#define LEDS_SET	0
#define LEDS_ON		1
#define LEDS_OFF	2
#define LEDS_FLASH	3
#define LEDS_MAX	4

typedef unsigned long leddef_t[LEDS_MAX];

/*
 *	A LED map is a mapping from numbers in ledman.h to one or more
 *	physical LED bits.  Currently the typing limits us to 32 LED's
 *	though this would not be hard to change.
 */

typedef unsigned long ledmap_t[LEDMAN_MAX];

/*
 *	A LED mode is a definition of how a set of LED's should behave.
 *
 *	name    - a symbolic name for the LED mode,  used for changing modes
 *	map     - ledmap array,  maps ledman.h defines to real LED bits
 *	def     - default behaviour for the LED bits (ie, on, flashing ...)
 *	jiffies - how many clock ticks between runs of the tick routine.
 */


struct ledmode {
	char	name[LEDMAN_MAX_NAME];
	u_long	*map;
	u_long	*def;
	int	jiffies;
};

/****************************************************************************/

static struct ledmode *lmp;	/* current mode */
static int initted;

/*
 * We have two sets of LED's for transient operations like DHCP and so on
 * index 0 is the standard LED's and index 1 is the ALTBIT LED's
 */

static unsigned long leds_alt, leds_alt_cnt[32];
#if !defined(CONFIG_SH_KEYWEST) && !defined(CONFIG_SH_BIGSUR)
static unsigned long leds_set[2];
#endif
static unsigned long leds_on[2], leds_off[2], leds_flash[2];

static pid_t ledman_resetpid = -1;

/****************************************************************************/

/*
 *	Let the system specific defining begin
 */

#if defined(CONFIG_M586)
#define CONFIG_X86 1
#endif

#if defined(CONFIG_X86) && !defined(CONFIG_X86_64)
#if defined(CONFIG_MPENTIUMM)
#define	CONFIG_UTM2000		1
#elif defined(CONFIG_MTD_SNAPGEODE)
#define	CONFIG_GEODE		1
#else
#define	CONFIG_AMDSC520		1
#endif
#endif /* CONFIG_X86 */

/****************************************************************************/
/****************************************************************************/

#undef LT
#define LT	(((HZ) + 99) / 100)

/****************************************************************************/

/****************************************************************************/
/*
 *	boot arg processing ledman=mode
 */

#if LINUX_VERSION_CODE >= 0x020100
__setup("ledman=", ledman_setup);
#endif

int
ledman_setup(char *arg)
{
	ledman_cmd(LEDMAN_CMD_MODE, (unsigned long) arg);
	return 0;
}

/****************************************************************************/
/****************************************************************************/

void
ledman_killtimer(void)
{
/*
 *	stop the timer
 */
	del_timer(&ledman_timerlist);

/*
 *	set the LEDs up correctly at boot
 */
	ledman_cmd(LEDMAN_CMD_RESET, LEDMAN_ALL);
}

/****************************************************************************/

void
ledman_starttimer(void)
{
/*
 *	start the timer
 */
	mod_timer(&ledman_timerlist, jiffies + 1);

/*
 *	set the LEDs up correctly at boot
 */
	ledman_cmd(LEDMAN_CMD_RESET, LEDMAN_ALL);
}

/****************************************************************************/

static void
ledman_poll(struct timer_list *t)
{
	unsigned long expires;
	ledman_tick();
	expires = jiffies + lmp->jiffies;
	mod_timer(&ledman_timerlist, expires);
}

/****************************************************************************/

static long
ledman_ioctl(
	struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	char	mode[LEDMAN_MAX_NAME];
	int		i;

	/* Strip off the leading ioctl command identifer */
	cmd &= LEDMAN_IOC_BITMASK;

	if (cmd == LEDMAN_CMD_SIGNAL) {
		ledman_resetpid = current->pid;
		return 0;
	}

	if (cmd == LEDMAN_CMD_MODE) {
		for (i = 0; i < sizeof(mode) - 1; i++) {
			Get_user(mode[i], (char __user *) (arg + i));
			if (!mode[i])
				break;
		}
		mode[i] = '\0';
		arg = (unsigned long) &mode[0];
	}
	return ledman_cmd(cmd, arg);
}

/****************************************************************************/
/*
 * Signal the reset pid, if we have one
 */

void
ledman_signalreset(void)
{
	static unsigned long firstjiffies = INITIAL_JIFFIES;

	if (ledman_resetpid == -1)
		return;
	if (time_after(jiffies, (firstjiffies + (HZ / 4)))) {
		firstjiffies = jiffies;
		pr_info("reset switch interrupt!"
		       " (sending signal to pid=%d)\n", ledman_resetpid);
		kill_proc(ledman_resetpid, SIGUSR2, 1);
	}
}

/****************************************************************************/
#if !defined(CONFIG_SH_KEYWEST) && !defined(CONFIG_SH_BIGSUR)
/****************************************************************************/

static int
ledman_bits(unsigned long cmd, unsigned long bits)
{
	int				 alt, i;
	unsigned long	 new_alt;

	alt = (cmd & LEDMAN_CMD_ALTBIT) ? 1 : 0;

	switch (cmd & ~LEDMAN_CMD_ALTBIT) {
	case LEDMAN_CMD_SET:
		leds_set[alt]   |= bits;
		break;
	case LEDMAN_CMD_ON:
		leds_on[alt]    |= bits;
		leds_off[alt]   &= ~bits;
		leds_flash[alt] &= ~bits;
		ledman_tick();
		break;
	case LEDMAN_CMD_OFF:
		leds_on[alt]    &= ~bits;
		leds_off[alt]   |= bits;
		leds_flash[alt] &= ~bits;
		ledman_tick();
		break;
	case LEDMAN_CMD_FLASH:
		leds_on[alt]    &= ~bits;
		leds_off[alt]   &= ~bits;
		leds_flash[alt] |= bits;
		break;
	case LEDMAN_CMD_RESET:
		leds_set[alt]   = (leds_set[alt]   & ~bits) | (bits & lmp->def[LEDS_SET]);
		leds_on[alt]    = (leds_on[alt]    & ~bits) | (bits & lmp->def[LEDS_ON]);
		leds_off[alt]   = (leds_off[alt]   & ~bits) | (bits & lmp->def[LEDS_OFF]);
		leds_flash[alt] = (leds_flash[alt] & ~bits) | (bits & lmp->def[LEDS_FLASH]);
		break;
	case LEDMAN_CMD_ALT_ON:
		new_alt = (bits & ~leds_alt);
		leds_alt |= bits;
		/*
		 * put any newly alt'd bits into a default state
		 */
		ledman_bits(LEDMAN_CMD_RESET | LEDMAN_CMD_ALTBIT, new_alt);
		if (alt) /* alt forces it,  no accounting */
			break;
		for (i = 0; i < 32; i++)
			if (bits & (1 << i))
				leds_alt_cnt[i]++;
		break;
	case LEDMAN_CMD_ALT_OFF:
		if (alt) { /* alt forces it,  no accounting */
			leds_alt &= ~bits;
			break;
		}
		for (i = 0; i < 32; i++)
			if ((bits & (1 << i)) && leds_alt_cnt[i]) {
				leds_alt_cnt[i]--;
				if (leds_alt_cnt[i] == 0)
					leds_alt &= ~(1 << i);
			}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/****************************************************************************/

static void
ledman_tick(void)
{
	static int flash_on;
	int new_value;
/*
 *	work out which LED's should be on
 */
	new_value = 0;
	new_value |= (((leds_set[0] | leds_on[0]) & ~leds_off[0]) & ~leds_alt);
	new_value |= (((leds_set[1] | leds_on[1]) & ~leds_off[1]) & leds_alt);
/*
 *	flashing LED's run on their own devices,  ie,  according to the
 *	value fo flash_on
 */
	if ((flash_on++ % 60) >= 30)
		new_value |= ((leds_flash[0]&~leds_alt) | (leds_flash[1]&leds_alt));
	else
		new_value &= ~((leds_flash[0]&~leds_alt) | (leds_flash[1]&leds_alt));
/*
 *	set the HW
 */
	ledman_set(new_value);
	leds_set[0] = leds_set[1] = 0;
}

/****************************************************************************/
#endif /* !defined(CONFIG_SH_KEYWEST) && !defined(CONFIG_SH_BIGSUR) */
/****************************************************************************/
#if defined(CONFIG_NETtel) && defined(CONFIG_M5307)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	Here it the definition of the LED's on the NETtel circuit board
 *	as per the labels next to them.  The two parallel port LED's steal
 *	some high bits so we can map it more easily onto the HW
 *
 *	LED - D1   D2   D3   D4   D5   D6   D7   D8   D11  D12
 *	HEX - 100  200  004  008  010  020  040  080  002  001
 *
 */

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/nettel.h>

static ledmap_t	nettel_old = {
	0x3ff, 0x200, 0x100, 0x008, 0x004, 0x020, 0x010, 0x080, 0x080, 0x080,
	0x080, 0x040, 0x040, 0x002, 0x002, 0x024, 0x018, 0x001, 0x0ff, 0x0ff,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x100, 0x200, 0x000,
	0x000, 0x000, 0x000, 0x000
};

#if defined(CONFIG_SNAPGEAR)

/*
 * all snapgear 5307 based boards have a SW link status on the front
 */

static ledmap_t nettel_new = {
	0x3ff, 0x200, 0x100, 0x040, 0x040, 0x002, 0x002, 0x008, 0x008, 0x020,
	0x020, 0x000, 0x000, 0x000, 0x000, 0x024, 0x018, 0x001, 0x0ff, 0x080,
	0x000, 0x000, 0x080, 0x004, 0x010, 0x000, 0x000, 0x100, 0x200, 0x000,
	0x000, 0x000, 0x000, 0x000
};

#else

static ledmap_t nettel_new = {
	0x3ff, 0x200, 0x100, 0x040, 0x040, 0x002, 0x002, 0x008, 0x004, 0x020,
	0x010, 0x000, 0x000, 0x000, 0x000, 0x024, 0x018, 0x001, 0x0ff, 0x080,
	0x000, 0x000, 0x080, 0x000, 0x000, 0x000, 0x000, 0x100, 0x200, 0x000,
	0x000, 0x000, 0x000, 0x000
};

#endif

static leddef_t	nettel_def = {
	0x000, 0x200, 0x000, 0x100,
};

#ifdef ENTERASYS
static ledmap_t enterasys_std = {
	0x3ff, 0x200, 0x100, 0x040, 0x040, 0x002, 0x002, 0x008, 0x004, 0x020,
	0x010, 0x000, 0x000, 0x000, 0x000, 0x024, 0x018, 0x001, 0x00c, 0x030,
	0x000, 0x000, 0x080, 0x000, 0x000, 0x000, 0x000, 0x100, 0x200, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t enterasys_def = {
	0x000, 0x200, 0x000, 0x100,
};
#endif

static void
ledman_set(unsigned long bits)
{
	unsigned long flags;

	local_irq_save(flags);
	*(volatile char *)NETtel_LEDADDR = (~bits & 0xff);
	mcf_setppdata(0x60, ~(bits >> 3) & 0x60);
	local_irq_restore(flags);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_NETtel) && defined(CONFIG_M5307) */
/****************************************************************************/
#if defined(CONFIG_NETtel) && defined(CONFIG_M5272)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	For the SecureEdge Firewall (5272), 5 operational LED's.
 *
 *	LED -   POWER HEARTBEAT TX     RX     VPN
 *	HEX -    001     002    004    008    010
 */

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/nettel.h>

static ledmap_t ledman_std = {
	0x01f, 0x001, 0x002, 0x008, 0x004, 0x008, 0x004, 0x000, 0x000, 0x008,
	0x004, 0x000, 0x000, 0x000, 0x000, 0x014, 0x008, 0x010, 0x01c, 0x010,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x001, 0x000, 0x002,
};

static void
ledman_set(unsigned long bits)
{
	*((volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT)) = (~bits & 0x1f);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_NETtel) && defined(CONFIG_M5272) */
/****************************************************************************/
#if defined(CONFIG_SE1100)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	For the SecureEdge SE1100 (5272), 3 operational LED's.
 *
 *	LED -   RUNNING INTERNAL1 INTERNAL2
 *	HEX -     001     200       002
 */

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/se1100.h>

static ledmap_t ledman_std = {
	0x203, 0x000, 0x001, 0x200, 0x200, 0x200, 0x200, 0x000, 0x000, 0x000,
	0x000, 0x002, 0x002, 0x002, 0x002, 0x200, 0x002, 0x000, 0x202, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x000, 0x000, 0x001
};

static void
ledman_set(unsigned long bits)
{
	mcf_setpa(0x203, bits & 0x203);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_SE1100) */
/****************************************************************************/
#if defined(CONFIG_GILBARCONAP) && defined(CONFIG_M5272)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	For the Gilbarco/NAP (5272), 2 operational LED's.
 *
 *	LED -   RUNNING DIAG
 *	HEX -     001    002
 */

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/nap.h>

static ledmap_t ledman_std = {
	0x003, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x002, 0x001, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman = {
	0x000, 0x001, 0x000, 0x002,
};

static void
ledman_set(unsigned long bits)
{
	mcf_setpa(0x3, ~bits & 0x3);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_GILBARCONAP) && defined(CONFIG_M5272) */
/****************************************************************************/
#if defined(CONFIG_AVNET5282)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <asm/coldfire.h>
#include <asm/mcfsim.h>

#define GPTASYSCR1  (*(volatile unsigned char *)0x401a0006)
#define GPTBSYSCR1  (*(volatile unsigned char *)0x401b0006)
#define GPTADR      (*(volatile unsigned char *)0x401a001d)
#define GPTBDR      (*(volatile unsigned char *)0x401b001d)
#define GPTADDR     (*(volatile unsigned char *)0x401a001e)
#define GPTBDDR     (*(volatile unsigned char *)0x401b001e)
#define PORT_TC     (*(volatile unsigned char *)0x4010000f)
#define PORT_TD     (*(volatile unsigned char *)0x40100010)
#define DDR_TC      (*(volatile unsigned char *)0x40100023)
#define DDR_TD      (*(volatile unsigned char *)0x40100024)
#define DR_TC       (*(volatile unsigned char *)0x40100037)
#define DR_TD       (*(volatile unsigned char *)0x40100038)


static ledmap_t ledman_std = {
	0x003, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x002, 0x001, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x001, 0x000, 0x002,
};

static void
ledman_set(unsigned long bits)
{
	static int first_call = 1;

	if (first_call) {
		GPTASYSCR1 = 0x00;
		GPTBSYSCR1 = 0x00;
		GPTADDR = 0x00;
		GPTBDDR = 0x0f;
		DDR_TC = 0x05;
		DDR_TD = 0x05;
		first_call = 0;
	}

	if (bits & 0x01) PORT_TC &= ~0x01; else PORT_TC |= 0x01;
	if (bits & 0x02) PORT_TC &= ~0x04; else PORT_TC |= 0x04;
	if (bits & 0x04) PORT_TD &= ~0x01; else PORT_TD |= 0x01;
	if (bits & 0x08) PORT_TD &= ~0x04; else PORT_TD |= 0x04;
	if (bits & 0x10) GPTBDR  &= ~0x01; else GPTBDR  |= 0x01;
	if (bits & 0x20) GPTBDR  &= ~0x02; else GPTBDR  |= 0x02;
	if (bits & 0x40) GPTBDR  &= ~0x04; else GPTBDR  |= 0x04;
	if (bits & 0x80) GPTBDR  &= ~0x08; else GPTBDR  |= 0x08;
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_AVNET5282) */
/****************************************************************************/
#if defined(CONFIG_SH_SECUREEDGE5410)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <asm/snapgear.h>

/*
 *	For the SecureEdge5410 7 (or 8 for eth2/DMZ port) operational LED's.
 *
 *	LED -   POWR  HBEAT  LAN1  LAN2 | LAN3 | COM ONLINE  VPN
 *	POS -    D2    D3    D4    D5   |  ??  | D6    D7    D8    DTR
 *	HEX -    01    02    04    08   |0x2000| 10    20    40    80
 */

#if defined(CONFIG_LEDMAP_TAMS_SOHO)
/*
 *	LED -   POWR  HBEAT  LAN1  LAN2    COM   ONLINE VPN
 *	POS -    D2    D3    D4    D5      ??    D6     D7    DTR
 *	HEX -    01    02    04    08    0x2000  10     20    80
 */
static ledmap_t ledman_std = {
	0x203f,0x0001,0x0002,0x2000,0x2000,0x2000,0x2000,0x0004,0x0004,0x0008,
	0x0008,0x0000,0x0000,0x0000,0x0000,0x2024,0x0018,0x0020,0x203c,0x0000,
	0x0000,0x0000,0x0010,0x0000,0x0000,0x0000,0x0000,0x0001,0x0002,0x0000,
	0x0000,0x0000,0x0000,0x0000
};
#else
static ledmap_t ledman_std = {
	0x207f,0x0001,0x0002,0x0010,0x0010,0x0010,0x0010,0x0004,0x0004,0x0008,
	0x0008,0x0000,0x0000,0x0000,0x0000,0x2054,0x0028,0x0040,0x207c,0x0000,
	0x0000,0x0000,0x0020,0x0000,0x0000,0x0000,0x0000,0x0001,0x0002,0x2000,
	0x2000,0x0000,0x0000,0x0000
};
#endif

static leddef_t	ledman_def = {
	0x0000, 0x0001, 0x0000, 0x0002,
};

static void
ledman_set(unsigned long bits)
{
	unsigned long flags;

	local_irq_save(flags);
	SECUREEDGE_WRITE_IOPORT(~bits, 0x207f);
	local_irq_restore(flags);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_SH_SECUREEDGE5410) */
/****************************************************************************/
#if defined(CONFIG_NETtel) && defined(CONFIG_M5206e)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	For the WebWhale/NETtel1500,  3 LED's (was 2)
 *
 *	LED - HEARTBEAT  DCD    DATA
 *	HEX -    001     002    004
 */

#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#include <asm/nettel.h>

static ledmap_t ledman_std = {
	0x007, 0x000, 0x001, 0x004, 0x004, 0x004, 0x004, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x004, 0x002, 0x000, 0x007, 0x000,
	0x002, 0x002, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x000, 0x000, 0x001,
};

static void
ledman_set(unsigned long bits)
{
	*(volatile char *)NETtel_LEDADDR = (~bits & 0x7);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* defined(CONFIG_NETtel) && defined(CONFIG_M5206e) */
/****************************************************************************/
#ifdef CONFIG_eLIA
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	For the eLIA, only 2 LED's.
 *
 *	LED - HEARTBEAT  USER
 *	HEX -    2        1
 */

#ifdef CONFIG_COLDFIRE
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#endif
#include <asm/elia.h>

static ledmap_t ledman_std = {
	0x003, 0x000, 0x002, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x002, 0x001, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x000, 0x000, 0x002,
};

static void
ledman_set(unsigned long bits)
{
	unsigned long flags;

	local_irq_save(flags);
	mcf_setppdata(0x3000, ~(bits << 12) & 0x3000);
	local_irq_restore(flags);
}

static void
ledman_initarch(void)
{
}

/****************************************************************************/
#endif /* CONFIG_eLIA */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_AMDSC520)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#if defined(CONFIG_CHINOOK)
/*
 *	Here is the definition of the LED's on the 6wind Chinook circuit board
 *	as per the LED order from left to right. These probably don't
 *  correspond to any known enclosure.
 *
 *	LED -  D1    D2    D3    D4    D5    D6    D7    D8    D9    D10
 *	HEX - 0001  0400  0008  0010  0800  0020  0040  0080  0100  0200
 *
 *  Sync LEDs - Activity  Link
 *  HEX       - 04000000  08000000
 */
static ledmap_t	ledman_std = {
	0x0c000ff9, 0x00000001, 0x00000400, 0x00000040, 0x00000040,
	0x04000000, 0x04000000, 0x00000010, 0x00000008, 0x00000020,
	0x00000800, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000028, 0x00000810, 0x00000200, 0x00000bf8, 0x00000820,
	0x00000000, 0x08000000, 0x00000100, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

static leddef_t	ledman_def = {
	0x0000, 0x0001, 0x0000, 0x0400,
};

#elif defined(CONFIG_SNAPGEAR)
/*
 *	Here is the definition of the LED's on the SnapGear x86 circuit board
 *	as per the labels next to them.
 *
 *	LED - D1   D2   D3   D4   D5   D6   D7   D8   D9   D10
 *	HEX - 001  002  004  008  010  020  040  100  080  200
 */
static ledmap_t	ledman_std = {
	0x3ff, 0x001, 0x002, 0x080, 0x080, 0x040, 0x040, 0x010, 0x010, 0x020,
	0x020, 0x000, 0x000, 0x000, 0x000, 0x048, 0x030, 0x200, 0x3fc, 0x004,
	0x000, 0x000, 0x100, 0x008, 0x004, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x0000, 0x0001, 0x0000, 0x0002,
};

/*
 *	Here is the definition of the LED's on the SnapGear x86 circuit board
 *	as per the labels next to them.  This is for the old enclosure.
 *
 *	LED - D1   D2   D3   D4   D5   D6   D7   D8   D9   D10
 *	HEX - 001  002  004  008  010  020  040  100  080  200
 */
static ledmap_t	nettel_old = {
	0x3ff, 0x002, 0x001, 0x080, 0x080, 0x040, 0x040, 0x010, 0x010, 0x020,
	0x020, 0x000, 0x000, 0x000, 0x000, 0x048, 0x030, 0x200, 0x3fc, 0x004,
	0x000, 0x000, 0x100, 0x008, 0x004, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	nettel_def_old = {
	0x0000, 0x0002, 0x0000, 0x0001,
};

#elif defined(CONFIG_SITECTRLER)
/*
 *	Here it the definition of the LED's on the SiteController circuit board
 *	as per the labels next to them. (D9 and D10 are not software controlled)
 *
 *	LED -  D1   D2   D3   D4   D5   D6   D7   D8
 *	HEX - 0001 0002 0004 0008 0010 0020 0040 0080
 */
static ledmap_t	ledman_std = {
	0x10fd,0x0001,0x1000,0x0004,0x0004,0x0008,0x0008,0x0040,0x0040,0x0080,
	0x0080,0x0000,0x0000,0x0000,0x0000,0x00cc,0x0030,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1000,
	0x0000,0x0000,0x0000,0x0000
};

static leddef_t	ledman_def = {
	0x0000, 0x0001, 0x0000, 0x1000,
};

#elif defined(CONFIG_ADTRAN_ADVANTA)
/*
 *	Here is the definition of the LED's on the Adtran Advanta3110 circuit
 *	board as per the labels next to them.
 *	The lower 8 bits are for IO port 0x300.
 *	The upper 4 bits are for PIO31-28.
 *
 *	LED - D1   D2   D3   D4   D7   D8   D15green D15red   D16green D16red
 *	HEX - 01   02   04   08   40   80   10000000 40000000 20000000 80000000
 */
static ledmap_t	ledman_std = {
	0xf00000cf, 0x00000000, 0x20000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000002, 0x00000001, 0x00000008,
	0x00000004, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000009, 0x00000006, 0x10000000, 0x100000cf, 0x0000000c,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000080, 0x00000040, 0x00000001, 0x00000002, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

static leddef_t	ledman_def = {
	0, 0, 0, 0x20000000,
};

#else
/*
 *	Here is the definition of the LED's on the x86 NETtel circuit board
 *	as per the labels next to them.
 *
 *	LED - D1   D2   D3   D4   D5   D6   D7   D8   D9   D10
 *	HEX - 001  002  004  008  010  020  040  100  080  200
 */
static ledmap_t	ledman_std = {
	0x3ff, 0x002, 0x001, 0x100, 0x100, 0x080, 0x080, 0x010, 0x008, 0x040,
	0x020, 0x000, 0x000, 0x000, 0x000, 0x048, 0x030, 0x200, 0x3fc, 0x004,
	0x000, 0x000, 0x004, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x0000, 0x0002, 0x0000, 0x0001,
};

#endif

static volatile unsigned long	*ledman_ledp;

static void
ledman_set(unsigned long bits)
{
#ifdef CONFIG_ADTRAN_ADVANTA
	outb(~bits, 0x300);
	*ledman_ledp = (*ledman_ledp & 0x0fffffff) | (~bits & 0xf0000000);
#else
	*ledman_ledp = (*ledman_ledp & ~ledman_std[LEDMAN_ALL])
			| (~bits & ledman_std[LEDMAN_ALL]);
#endif
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_initarch(void)
{
	volatile unsigned char	*mmcrp;

	/* Map the CPU MMCR register for access */
	mmcrp = (volatile unsigned char *) ioremap(0xfffef000, 4096);

#ifdef CONFIG_ADTRAN_ADVANTA
	/* Enable the PIO for the PWR and VPN LEDs */
	*(volatile unsigned short *)(mmcrp + 0xc22) &= 0x0fff;
	*(volatile unsigned short *)(mmcrp + 0xc2c) |= 0xf000;

	/* Enable GPIRQ2, low-to-high transitions, map to IRQ12 */
	*(volatile unsigned short *)(mmcrp + 0xc22) |= 0x0020;
	*(volatile unsigned long *)(mmcrp + 0xd10) |= 0x0004;
	*(mmcrp + 0xd52) = 0x07;
#endif

	ledman_ledp = (volatile unsigned long *) (mmcrp + 0xc30);

	/* Setup extern "factory default" switch on IRQ12 */
	if (request_irq(12, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ12 for ERASE witch\n");
	else
		pr_info("registered ERASE switch on IRQ12\n");
}

/****************************************************************************/
#endif /* CONFIG_AMDSC520 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_GEODE)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	Construct a mapping from virtual LED to gpio bit and bank.
 */
struct gpiomap {
		unsigned int	bank;
		unsigned long	bit;
};

#if defined(CONFIG_REEFEDGE) || defined(CONFIG_SE5000)
/*
 *	Definition of the LED's on the GEODE SC1100 ReefEdfe circuit board,
 *	as per the labels next to them.
 *
 *	LED      -  D20  D19  D18  D16
 *	GPIO BIT -   38   18   40   37
 *  VIRTNUM  -  0x8  0x4  0x2  0x1
 */
#define	GPIO0_OFF	0x00040000
#define	GPIO1_OFF	0x00000160

static ledmap_t	ledman_std = {
	0x00f, 0x001, 0x002, 0x000, 0x000, 0x000, 0x000, 0x004,
	0x004, 0x008, 0x008, 0x000, 0x000, 0x000, 0x000, 0x004,
	0x008, 0x000, 0x00e, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000
};

static struct gpiomap iomap[] = {
	/* VIRT 0x1 */	{ 1, 0x00000020 },
	/* VIRT 0x2 */	{ 1, 0x00000100 },
	/* VIRT 0x4 */	{ 0, 0x00040000 },
	/* VIRT 0x8 */	{ 1, 0x00000040 },
};

#elif defined(CONFIG_SE2910)
/*
 *	Definition of the LED's on the SnapGear SE2910 boards.
 *
 *	LED      -   D2   D4   D6  D11  D13  D18  D20  D22   D25   D28
 *	GPIO BIT -    2    3   17    0   36   47   39   40    18    38
 *  VIRTNUM  -  0x1  0x2  0x4  0x8 0x10 0x20 0x40 0x80 0x100 0x200
 */
#define	GPIO0_OFF	0x0006000d
#define	GPIO1_OFF	0x000081d0

static ledmap_t	ledman_std = {
	0x3ff, 0x001, 0x002, 0x080, 0x080, 0x080, 0x080, 0x010,
	0x008, 0x040, 0x020, 0x200, 0x200, 0x200, 0x200, 0x30c,
	0x0f0, 0x000, 0x3fc, 0x004, 0x000, 0x000, 0x004, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000
};

static struct gpiomap iomap[] = {
	/* VIRT 0x1 */   { 0, (1 << (02 - 00)) },
	/* VIRT 0x2 */   { 0, (1 << (03 - 00)) },
	/* VIRT 0x4 */   { 0, (1 << (17 - 00)) },
	/* VIRT 0x8 */   { 0, (1 << (00 - 00)) },
	/* VIRT 0x10 */  { 1, (1 << (36 - 32)) },
	/* VIRT 0x20 */  { 1, (1 << (47 - 32)) },
	/* VIRT 0x40 */  { 1, (1 << (39 - 32)) },
	/* VIRT 0x80 */  { 1, (1 << (40 - 32)) },
	/* VIRT 0x100 */ { 0, (1 << (18 - 00)) },
	/* VIRT 0x200 */ { 1, (1 << (38 - 32)) },
};

#else
/*
 *	Definition of the LED's on the SnapGear GEODE board.
 *
 *	LED      -  D11  D12  D13  D14  D15  D16  D17  D18   D19   D20
 *	GPIO BIT -   32   33   34   35   36   37   39   40    18    38
 *  VIRTNUM  -  0x1  0x2  0x4  0x8 0x10 0x20 0x40 0x80 0x100 0x200
 */
#define	GPIO0_OFF	0x00040000
#define	GPIO1_OFF	0x000001ff

static ledmap_t	ledman_std = {
	0x3ff, 0x001, 0x002, 0x080, 0x080, 0x080, 0x080, 0x010,
	0x008, 0x040, 0x020, 0x200, 0x200, 0x200, 0x200, 0x30c,
	0x0f0, 0x000, 0x3fc, 0x004, 0x000, 0x000, 0x004, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x000, 0x000
};

static struct gpiomap iomap[] = {
	/* VIRT 0x1 */   { 1, 0x00000001 },
	/* VIRT 0x2 */   { 1, 0x00000002 },
	/* VIRT 0x4 */   { 1, 0x00000004 },
	/* VIRT 0x8 */   { 1, 0x00000008 },
	/* VIRT 0x10 */  { 1, 0x00000010 },
	/* VIRT 0x20 */  { 1, 0x00000020 },
	/* VIRT 0x40 */  { 1, 0x00000080 },
	/* VIRT 0x80 */  { 1, 0x00000100 },
	/* VIRT 0x100 */ { 0, 0x00040000 },
	/* VIRT 0x200 */ { 1, 0x00000040 },
};

#endif /* !CONFIG_REEFEDGE && !CONFIG_SE5000 */

#define	GPIO_SIZE	(sizeof(iomap) / sizeof(*iomap))

static leddef_t	ledman_def = {
	0x0000, 0x0001, 0x0000, 0x0002,
};


static void
ledman_set(unsigned long bits)
{
	unsigned int gpio[2];
	unsigned int i, mask;

	gpio[0] = GPIO0_OFF;
	gpio[1] = GPIO1_OFF;

	for (i = 0, mask = 0x1; (i < GPIO_SIZE); i++, mask <<= 1) {
		if (bits & mask)
			gpio[iomap[i].bank] &= ~iomap[i].bit;
	}

	outl(gpio[0], 0x6400);
	outl(gpio[1], 0x6410);
}

static int ledman_button;
static struct timer_list ledman_timer;

static void ledman_buttonpoll(struct timer_list *t)
{
	if (inl(0x6404) & 0x0002) {
		if (ledman_button == 0) {
			pr_info("reset button pushed!\n");
			ledman_signalreset();
		}
		ledman_button = 1;
	} else {
		ledman_button = 0;
	}

	/* Re-arm timer interrupt. */
	mod_timer(&ledman_timer, jiffies + HZ/25);
}

static void
ledman_initarch(void)
{
	timer_setup(&ledman_timer, ledman_buttonpoll, 0);
	mod_timer(&ledman_timer, jiffies + HZ/25);
}

/****************************************************************************/
#endif /* CONFIG_GEODE */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_UTM2000)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/pci.h>

/*
 * Access to the GPIO registers is via the LPC bus PCI device.
 * Define relevant parts of that device here.
 */
#define	LPC_VENDOR_ID	0x8086
#define	LPC_DEVICE_ID	0x5031

#define	GPIO_BAR_OFFSET	0x48

/*
 * Offsets of registers into the GPIO register address space.
 */
#define	GPIO_USE_SEL1	0x0
#define	GP_IO_SEL1	0x4
#define	GP_LVL1		0xc
#define	GPO_BLINK	0x18
#define	GPI_INV		0x2c
#define	GPIO_USE_SEL2	0x30
#define	GP_IO_SEL2	0x34
#define	GP_LVL2		0x38

/*
 *	Here is the definition of the LED's on the SnapGear Tolapai board.
 */
#define	LED0		0x00100000
#define	LED1		0x01000000
#define	LED2		0x02000000
#define	LEDMASK		0x03100000

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]		= LEDMASK,
	[LEDMAN_HEARTBEAT]	= LED0,
	[LEDMAN_COM1_RX]	= LED1,
	[LEDMAN_COM1_TX]	= LED1,
	[LEDMAN_COM2_RX]	= LED1,
	[LEDMAN_COM2_TX]	= LED1,
	[LEDMAN_LAN1_RX]	= LED2,
	[LEDMAN_LAN1_TX]	= LED2,
	[LEDMAN_LAN2_RX]	= LED2,
	[LEDMAN_LAN2_TX]	= LED2,
	[LEDMAN_LAN3_RX]	= LED2,
	[LEDMAN_LAN3_TX]	= LED2,
};

static leddef_t	ledman_def = {
	[LEDS_FLASH]		= LED0,
};

static u32 ledman_gpioaddr;

void gpio_enable_bit(int nr, int dir)
{
	u32 addr, v;

	addr = ledman_gpioaddr + ((nr < 32) ? GPIO_USE_SEL1 : GPIO_USE_SEL2);
	v = inl(addr);
	v |= (0x1 << (nr & 0x1f));
	outl(v, addr);

	addr = ledman_gpioaddr + ((nr < 32) ? GP_IO_SEL1 : GP_IO_SEL2);
	v = inl(addr);
	if (dir)
		v &= ~(0x1 << (nr & 0x1f));
	else
		v |= (0x1 << (nr & 0x1f));
	outl(v, addr);

	if (nr < 32) {
		addr = ledman_gpioaddr + GPO_BLINK;
		v = inl(addr);
		v &= ~(0x1 << nr);
		outl(v, addr);
	}
}
EXPORT_SYMBOL(gpio_enable_bit);

void gpio_dump_reg(void)
{
	pr_info("%s(%d): [GPIO_USE_SEL1]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GPIO_USE_SEL1));
	pr_info("%s(%d): [GP_IO_SEL1]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GP_IO_SEL1));
	pr_info("%s(%d): [GP_LVL1]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GP_LVL1));

	pr_info("%s(%d): [GPIO_USE_SEL2]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GPIO_USE_SEL2));
	pr_info("%s(%d): [GP_IO_SEL2]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GP_IO_SEL2));
	pr_info("%s(%d): [GP_LVL2]=0x%08x\n", __FILE__, __LINE__, inl(ledman_gpioaddr + GP_LVL2));
}
EXPORT_SYMBOL(gpio_dump_reg);

void gpio_set_bit(int nr)
{
	u32 addr, v;
	addr = ledman_gpioaddr + ((nr < 32) ? GP_LVL1 : GP_LVL2);
	v = inl(addr);
	v |= (0x1 << (nr & 0x1f));
	outl(v, addr);
}
EXPORT_SYMBOL(gpio_set_bit);

void gpio_clear_bit(int nr)
{
	u32 addr, v;
	addr = ledman_gpioaddr + ((nr < 32) ? GP_LVL1 : GP_LVL2);
	v = inl(addr);
	v &= ~(0x1 << (nr & 0x1f));
	outl(v, addr);
}
EXPORT_SYMBOL(gpio_clear_bit);

static void
ledman_set(unsigned long bits)
{
	u32 v;
	v = inl(ledman_gpioaddr + GP_LVL1);
	v = (v & ~LEDMASK) | (~bits & LEDMASK);
	outl(v, ledman_gpioaddr + GP_LVL1);
}

static void
ledman_initarch(void)
{
	struct pci_dev *dev;
	u32 v;

	dev = pci_get_device(LPC_VENDOR_ID, LPC_DEVICE_ID, NULL);
	if (dev == NULL) {
		pr_err("cannot find GPIO device?\n");
		return;
	}
	pci_read_config_dword(dev, GPIO_BAR_OFFSET, &ledman_gpioaddr);
	ledman_gpioaddr &= 0xfffffff0;
	pr_info("GPIO base IO addr=0x%x\n", ledman_gpioaddr);

	/* Set GPIO lines for LEDs to be GPIO lines (not alternate function) */
	v = inl(ledman_gpioaddr + GPIO_USE_SEL1);
	v |= LEDMASK;
	outl(v, ledman_gpioaddr + GPIO_USE_SEL1);

	v = inl(ledman_gpioaddr + GP_IO_SEL1);
	v &= ~((0x1 << 24) | (0x1 << 25));
	outl(v, ledman_gpioaddr + GP_IO_SEL1);

	/* Enable the external USB adapter */
	gpio_enable_bit(18, 1);
	gpio_set_bit(18);

	ledman_set(0);
}

/*
 *	Access to the expansion bus is enabled via PCI device.
 */
#define	EXPBUS_VENDOR_ID	0x8086
#define	EXPBUS_DEVICE_ID	0x503d

#define	EXPBUS_MMR_BAR_OFFSET	0x10
#define	EXPBUS_MMR_SIZE		(4 * 1024)
#define	EXPBUS_CS_BAR_OFFSET	0x14
#define	EXPBUS_CS_SIZE		(16 * 1024 * 1024)

#define	EXP_TIMING_CS0		0x0
#define	EXP_TIMING_CS1		0x4
#define	EXP_TIMING_CS2		0x8
#define	EXP_TIMING_CS3		0xc
#define	EXP_TIMING_CS4		0x10
#define	EXP_TIMING_CS5		0x14
#define	EXP_TIMING_CS6		0x18
#define	EXP_TIMING_CS7		0x1c
#define	EXP_CNFG0		0x20

u32 expbus_cs0;
u32 expbus_cs1;
u32 expbus_cs2;

EXPORT_SYMBOL(expbus_cs0);
EXPORT_SYMBOL(expbus_cs1);
EXPORT_SYMBOL(expbus_cs2);

static int expbus_init(void)
{
	volatile void __iomem *expmmrp;
	struct pci_dev *dev;
	u32 expmmr;

	dev = pci_get_device(EXPBUS_VENDOR_ID, EXPBUS_DEVICE_ID, NULL);
	if (dev == NULL) {
		pr_err("expbus: cannot find EXPBUS device?\n");
		return 0;
	}
	pci_read_config_dword(dev, EXPBUS_MMR_BAR_OFFSET, &expmmr);
	expmmr &= 0xfffffff0;

	expmmrp = ioremap(expmmr, EXPBUS_MMR_SIZE);
	if (expmmrp == NULL) {
		pr_err("expbus: failed to map Expansion Bus MMR region?\n");
		return 0;
	}
	pr_info("expbus: mapped Expansion Bus MMR (0x%x) to 0x%p\n",
		expmmr, expmmrp);

	pci_read_config_dword(dev, EXPBUS_CS_BAR_OFFSET, &expbus_cs0);
	expbus_cs0 &= 0xfffffff0;
	expbus_cs1 = expbus_cs0 + EXPBUS_CS_SIZE;
	expbus_cs2 = expbus_cs1 + EXPBUS_CS_SIZE;

	/* Map CS0 and CS1 for the lcd panel and buttons */
	writel(0xbfff0043, expmmrp + EXP_TIMING_CS0);
	writel(0xbfff0043, expmmrp + EXP_TIMING_CS1);

	/* Map CS2 for the power-reset controller watchdog timer */
	writel(0xbfff0043, expmmrp + EXP_TIMING_CS2);

	iounmap(expmmrp);
	return 0;
}

fs_initcall(expbus_init);

/****************************************************************************/
#endif /* CONFIG_TOLAPAI */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_SH_KEYWEST) || defined(CONFIG_SH_BIGSUR)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

/*
 *	Here it the definition of the how we use the 8 segment LED display on
 *	the Hitachi Keywest
 *
 *	LED - LD0  LD1  LD2  LD3  LD4  LD5  LD6  LD7
 *	HEX - 001  002  004  008  010  020  040  080
 *        HB   CNT  L1R  L1T  L2R  L2T  COM  VPN
 *
 */

#include <linux/kernel_stat.h>

#undef LT
#define LT	((HZ)/10)

#define KEYWEST_NUM_LEDS 8

#if defined(CONFIG_SH_BIGSUR)
#define LED_BASE 0xb1fffe00
#define LED_ADDR(x) (LED_BASE+((x)<<2))
#else
#define LED_BASE 0xb1ffe000
#define LED_ADDR(x) (LED_BASE+(x))
#endif

static ledmap_t	ledman_std = {
	0x0ff, 0x000, 0x001, 0x040, 0x040, 0x040, 0x040, 0x004, 0x008, 0x010,
	0x020, 0x000, 0x000, 0x000, 0x000, 0x054, 0x02a, 0x080, 0x07e, 0x000,
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x002, 0x000,
	0x000, 0x000, 0x000, 0x000
};

static leddef_t	ledman_def = {
	0x000, 0x000, 0x000, 0x001,
};

static struct keywest_led_value {
	int				count;
	int				max;
	int				prev;
	unsigned char	disp;
} keywest_led_values[KEYWEST_NUM_LEDS][2];


struct keywest_font_s {
	unsigned char row[7];
} keywest_font[] = {
	{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}, /* bar 0 */
	{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f }}, /* bar 1 */
	{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f }}, /* bar 2 */
	{{ 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f }}, /* bar 3 */
	{{ 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f }}, /* bar 4 */
	{{ 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f }}, /* bar 5 */
	{{ 0x00, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f }}, /* bar 6 */
	{{ 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f }}, /* bar 7 */
	{{ 0x00, 0x0a, 0x1f, 0x1f, 0x0e, 0x04, 0x00 }}, /* heart */
	{{ 0x08, 0x14, 0x14, 0x1c, 0x1c, 0x1c, 0x1c }}, /* vpn locked */
	{{ 0x02, 0x05, 0x05, 0x1c, 0x1c, 0x1c, 0x1c }}, /* vpn unlocked */
};

static unsigned int keywest_old_cntx;

/*
 * program up some display bars
 */

static void
ledman_initarch(void)
{
	int i, j;

	for (i = 0; i < ARRAY_SIZE(keywest_font); i++) {
		*(unsigned char *)(LED_ADDR(0x20)) = i;
		for (j = 0; j < 7; j++)
			*(unsigned char *)(LED_ADDR(0x28+j)) = keywest_font[i].row[j];
	}
	keywest_old_cntx = kstat.context_swtch;
}

/*
 *	We just rip through and write all LED 'disp' chars each tick.
 */

static void
ledman_set(unsigned long bits)
{
	int i, alt;
	for (i = 0; i < KEYWEST_NUM_LEDS; i++) {
		alt = (leds_alt & (1 << i)) ? 1 : 0;
		*(unsigned char *)(LED_ADDR(0x38+i)) = keywest_led_values[i][alt].disp;
	}
}

static int
ledman_bits(unsigned long cmd, unsigned long bits)
{
	int				 alt, i;
	unsigned long	 new_alt;

	alt = (cmd & LEDMAN_CMD_ALTBIT) ? 1 : 0;

	switch (cmd & ~LEDMAN_CMD_ALTBIT) {
	case LEDMAN_CMD_SET:
		bits &= ~(leds_flash[alt]|leds_on[alt]|leds_off[alt]);
		for (i = 0; i < KEYWEST_NUM_LEDS; i++)
			if (bits & (1 << i))
				keywest_led_values[i][alt].count++;
		break;
	case LEDMAN_CMD_ON:
		leds_on[alt]    |= bits;
		leds_off[alt]   &= ~bits;
		leds_flash[alt] &= ~bits;
		ledman_tick();
		break;
	case LEDMAN_CMD_OFF:
		leds_on[alt]    &= ~bits;
		leds_off[alt]   |= bits;
		leds_flash[alt] &= ~bits;
		ledman_tick();
		break;
	case LEDMAN_CMD_FLASH:
		leds_on[alt]    &= ~bits;
		leds_off[alt]   &= ~bits;
		leds_flash[alt] |= bits;
		break;
	case LEDMAN_CMD_RESET:
		leds_on[alt]    = (leds_on[alt]    & ~bits) | (bits & lmp->def[LEDS_ON]);
		leds_off[alt]   = (leds_off[alt]   & ~bits) | (bits & lmp->def[LEDS_OFF]);
		leds_flash[alt] = (leds_flash[alt] & ~bits) | (bits & lmp->def[LEDS_FLASH]);
		memset(keywest_led_values, 0, sizeof(keywest_led_values));
		break;
	case LEDMAN_CMD_ALT_ON:
		new_alt = (bits & ~leds_alt);
		leds_alt |= bits;
		/*
		 * put any newly alt'd bits into a default state
		 */
		ledman_bits(LEDMAN_CMD_RESET | LEDMAN_CMD_ALTBIT, new_alt);
		for (i = 0; i < 32; i++)
			if (bits & (1 << i))
				leds_alt_cnt[i]++;
		break;
	case LEDMAN_CMD_ALT_OFF:
		for (i = 0; i < 32; i++)
			if ((bits & (1 << i)) && leds_alt_cnt[i]) {
				leds_alt_cnt[i]--;
				if (leds_alt_cnt[i] == 0)
					leds_alt &= ~(1 << i);
			}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void
ledman_tick(void)
{
	static int flash_on;
	struct keywest_led_value *led_value;
	int alt, i;

	/*
	 * we take over the second LED as a context switch indicator
	 */
	keywest_led_values[1][0].count = kstat.context_swtch - keywest_old_cntx;
	keywest_old_cntx = kstat.context_swtch;

	for (i = 0; i < KEYWEST_NUM_LEDS; i++) {
		alt = (leds_alt >> i) & 1;
		led_value = &keywest_led_values[i][alt];
		if (leds_off[alt] & (1 << i)) {
			if ((1 << i) == 0x080) /* VPN unlock */
				led_value->disp = 0x8a;
			else
				led_value->disp = 0x20;
		} else if (leds_on[alt] & (1 << i)) {
			if ((1 << i) == 0x080) /* VPN lock */
				led_value->disp = 0x89;
			else
				led_value->disp = 0x87;
		} else if (leds_flash[alt] & (1 << i)) {
			if ((flash_on % 6) >= 3) {
				if ((1 << i) == 0x001) /* heart beat */
					led_value->disp = 0x88;
				else
					led_value->disp = 0x87;
			} else
				led_value->disp = 0x20;
		} else {
			int val;

			if (led_value->count > led_value->max)
				led_value->max = led_value->count;

			val = (led_value->prev + led_value->count) / 2;
			led_value->prev = val;

			val = (val * 7) / led_value->max;
			if (val == 0 && led_value->count)
				val = 1;
			led_value->disp = 0x80 + (val & 0x7);
			led_value->count = 0;
			/* degrade the maximum over time (except load) */
			if (i != 1)
				led_value->max = (led_value->max * 9)/10;
		}
	}
	flash_on++;
	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_SH_KEYWEST */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_ARCH_EP9312)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <mach/hardware.h>

/*
 *	Here is the definition of the LED's on the Trusonic IPD Board
 *
 *	It has no visible LED's, though it has provision for deug LED's,
 *	however, we need flatfsd support ;-)
 */
static ledmap_t	ledman_std;
static leddef_t	ledman_def;

static void
ledman_set(unsigned long bits)
{
}

static irqreturn_r ledman_interrupt(int irq, void *dev_id)
{
	while (inl(VIC1RAWINTR) & 0x1)
		;
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_initarch(void)
{
	if (request_irq(32, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ32 for ERASE witch\n");
	else
		pr_info("registered ERASE switch on IRQ32\n");
}

/****************************************************************************/
#endif /* CONFIG_ARCH_EP9312 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_SG590) || defined(CONFIG_SG8200)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <asm/mach-cavium-octeon/gpio.h>
#include <asm/mach-cavium-octeon/irq.h>

#ifdef CONFIG_SG8200
/*
 *	Here is the definition of the LEDs on the SG8200.
 */
#define LED_D2  0x0004
#define LED_D3  0x0008
#define LED_D4  0x0010
#define LED_D5  0x0020
#define LED_D6  0x0040
#define LED_D7  0x0080
#define LED_D8  0x0100
#define	LED_D9	0x0400
#define LEDMASK 0x05fc

#ifdef CONFIG_VENDOR_ATT_NETGATE
static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_COM1_RX]   = LED_D3,
	[LEDMAN_LAN1_RX]   = LED_D7,
	[LEDMAN_LAN1_TX]   = LED_D7,
	[LEDMAN_LAN2_RX]   = LED_D5,
	[LEDMAN_LAN2_TX]   = LED_D5,
	[LEDMAN_NVRAM_1]   = LED_D5 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D6 | LED_D7,
	[LEDMAN_VPN]       = LED_D4,
	[LEDMAN_LAN2_DHCP] = LED_D2,
	[LEDMAN_VPN_RX]    = LED_D8,
	[LEDMAN_VPN_TX]    = LED_D8,
	[LEDMAN_LAN3_RX]   = LED_D6,
	[LEDMAN_LAN3_TX]   = LED_D6,
	[LEDMAN_HIGHAVAIL] = LED_D9,
};

static leddef_t ledman_def = {
	[LEDS_FLASH] = 0,
};
#else
/*
 *	For SG support we do a couple of things a little different.
 *	The "ONLINE" LED becomes our usual heartbeat.. The "Dial" LED
 *	becomes a general serial acivity (not just modem).
 */
static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED_D2,
	[LEDMAN_COM1_RX]   = LED_D3,
	[LEDMAN_COM1_TX]   = LED_D3,
	[LEDMAN_COM2_RX]   = LED_D3,
	[LEDMAN_COM2_TX]   = LED_D3,
	[LEDMAN_LAN1_RX]   = LED_D7,
	[LEDMAN_LAN1_TX]   = LED_D7,
	[LEDMAN_LAN2_RX]   = LED_D5,
	[LEDMAN_LAN2_TX]   = LED_D5,
	[LEDMAN_NVRAM_1]   = LED_D5 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D6 | LED_D7,
	[LEDMAN_VPN]       = LED_D4,
	[LEDMAN_VPN_RX]    = LED_D8,
	[LEDMAN_VPN_TX]    = LED_D8,
	[LEDMAN_LAN3_RX]   = LED_D6,
	[LEDMAN_LAN3_TX]   = LED_D6,
	[LEDMAN_HIGHAVAIL] = LED_D9,
};

static leddef_t ledman_def = {
	[LEDS_FLASH] = LED_D2,
};
#endif /* !CONFIG_VENDOR_ATT_NETGATE */

#else
/*
 *	Here is the definition of the LEDs on the SG590.
 */
#define LED_D2  0x0004
#define LED_D3  0x0008
#define LED_D4  0x0010
#define LED_D5  0x0020
#define LED_D6  0x0040
#define LED_D7  0x0080
#define LED_D8  0x0100
#define LEDMASK 0x01fc

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED_D2,
	[LEDMAN_LAN1_RX]   = LED_D3,
	[LEDMAN_LAN1_TX]   = LED_D3,
	[LEDMAN_LAN2_RX]   = LED_D4,
	[LEDMAN_LAN2_TX]   = LED_D4,
	[LEDMAN_HIGHAVAIL] = LED_D5,
	[LEDMAN_COM1_RX]   = LED_D6,
	[LEDMAN_COM1_TX]   = LED_D6,
	[LEDMAN_COM2_RX]   = LED_D6,
	[LEDMAN_COM2_TX]   = LED_D6,
	[LEDMAN_ONLINE]    = LED_D7,
	[LEDMAN_VPN]       = LED_D8,
	[LEDMAN_NVRAM_1]   = LED_D2 | LED_D3 | LED_D7 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D4 | LED_D5 | LED_D6,
	[LEDMAN_LAN1_DHCP] = LEDMASK,
};

static leddef_t	ledman_def = {
	[LEDS_FLASH] = LED_D2,
};
#endif /* !CONFIG_SG8200 */

#define	ERASEGPIO	11
#define	ERASEIRQ	51

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_set(unsigned long bits)
{
	octeon_gpio_raw_clear(bits & LEDMASK);
	octeon_gpio_raw_set(~bits & LEDMASK);
}

static void
ledman_initarch(void)
{
	int i;

	/* SET all LED GPIO bits as outputs */
	for (i = 0; (i < 32); i++) {
		if ((0x1 << i) & LEDMASK)
			octeon_gpio_raw_config(i, OCTEON_GPIO_OUTPUT);
	}
	ledman_set(0);

	/* Set up the factory erase button GPIO line */
	octeon_gpio_raw_interrupt_ack(ERASEGPIO);
	irq_set_irq_type(ERASEIRQ, IRQ_TYPE_EDGE_FALLING);
	if (request_irq(ERASEIRQ, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE button\n", ERASEIRQ);
	else
		pr_info("registered ERASE button on IRQ%d\n", ERASEIRQ);
}

/****************************************************************************/
#endif /* CONFIG_SG590 || CONFIG_SG8200 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_SG770)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <asm/mach-cavium-octeon/gpio.h>
#include <asm/mach-cavium-octeon/irq.h>

/*
 *	Here is the definition of the LEDs on the SG770.
 */
#define LED0	0x01
#define LED1	0x02
#define LED2	0x04
#define LED3	0x08
#define LED4	0x10
#define LED5	0x20
#define LED6	0x40
#define LEDMASK 0x7f

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED0,
	[LEDMAN_LAN1_RX]   = LED2,
	[LEDMAN_LAN1_TX]   = LED2,
	[LEDMAN_LAN2_RX]   = LED3,
	[LEDMAN_LAN2_TX]   = LED3,
	[LEDMAN_HIGHAVAIL] = LED4,
	[LEDMAN_COM1_RX]   = LED1,
	[LEDMAN_COM1_TX]   = LED1,
	[LEDMAN_COM2_RX]   = LED1,
	[LEDMAN_COM2_TX]   = LED1,
	[LEDMAN_ONLINE]    = LED5,
	[LEDMAN_VPN]       = LED6,
	[LEDMAN_NVRAM_1]   = LED1 | LED3 | LED5,
	[LEDMAN_NVRAM_2]   = LED0 | LED2 | LED4 | LED6,
	[LEDMAN_LAN1_DHCP] = LEDMASK,
};

static leddef_t	ledman_def = {
	[LEDS_FLASH] = LED0,
};

#define	ERASEGPIO	8
#define	ERASEIRQ	48

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	octeon_gpio_raw_interrupt_ack(ERASEGPIO);
	ledman_signalreset();
	return IRQ_HANDLED;
}

static volatile void *sgoct_ledlatch;

static void
ledman_set(unsigned long bits)
{
	writeb(~bits, sgoct_ledlatch);
}

static void
ledman_initarch(void)
{
	/* SET all LED GPIO bits as outputs */
	sgoct_ledlatch = ioremap(0x1f800000, 4096);
	ledman_set(0);

	/* Set up the factory erase button GPIO line */
	if (request_irq(ERASEIRQ, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE witch\n", ERASEIRQ);
	else
		pr_info("registered ERASE switch on IRQ%d\n", ERASEIRQ);

	octeon_gpio_raw_config(ERASEGPIO, OCTEON_GPIO_INTERRUPT |
		OCTEON_GPIO_INT_EDGE | OCTEON_GPIO_INPUT_XOR);
}

/****************************************************************************/
#endif /* CONFIG_SG770 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_UTM300)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

/*
 * Here is the GPIO line definition of the LEDs on the UTM300.
 */
#define LED_D2  0x01 /* HEARTBEAT */
#define LED_D3  0x02 /* ETH */
#define LED_D4  0x04 /* USB */
#define LED_D5  0x08 /* SERIAL */
#define LED_D6  0x10 /* HA */
#define LED_D7  0x20 /* ONLINE */
#define LED_D8  0x40 /* VPN */
#define LEDMASK 0x7f

static int led_to_pin[] = {
	38, /* LED_D2 - HEARTBEAT */
	39, /* LED_D3 - ETH */
	40, /* LED_D4 - USB */
	41, /* LED_D5 - SERIAL */
	42, /* LED_D6 - HA */
	43, /* LED_D7 - ONLINE */
	44, /* LED_D8 - VPN */
};

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED_D2,
	[LEDMAN_LAN1_RX]   = LED_D3,
	[LEDMAN_LAN1_TX]   = LED_D3,
	[LEDMAN_LAN2_RX]   = LED_D3,
	[LEDMAN_LAN2_TX]   = LED_D3,
	[LEDMAN_LAN3_RX]   = LED_D3,
	[LEDMAN_LAN3_TX]   = LED_D3,
	[LEDMAN_USB1_RX]   = LED_D4,
	[LEDMAN_USB1_TX]   = LED_D4,
	[LEDMAN_COM1_RX]   = LED_D5,
	[LEDMAN_COM1_TX]   = LED_D5,
	[LEDMAN_HIGHAVAIL] = LED_D6,
	[LEDMAN_ONLINE]    = LED_D7,
	[LEDMAN_VPN]       = LED_D8,
	[LEDMAN_NVRAM_1]   = LED_D2 | LED_D4 | LED_D6 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D3 | LED_D5 | LED_D7,
	[LEDMAN_LAN1_DHCP] = LEDMASK,
};

static leddef_t	ledman_def = {
	[LEDS_FLASH] = LED_D2,
};

#define	ERASEGPIO	37
#define	ERASEIRQ	gpio_to_irq(ERASEGPIO)

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_set(unsigned long bits)
{
	int i;
	for (i = 0; i < 7; i++)
		gpio_set_value(led_to_pin[i], (bits & (1 << i)) ? 0 : 1);
}

static void
ledman_initarch(void)
{
	irq_set_irq_type(ERASEIRQ, IRQ_TYPE_EDGE_FALLING);

	/* Set up the factory erase button GPIO line */
	if (request_irq(ERASEIRQ, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE witch\n", ERASEIRQ);
	else
		pr_info("registered ERASE switch on IRQ%d\n", ERASEIRQ);

	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_MACH_UTM300 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_5400_RM_DT)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

/*
 * Here is the GPIO line definition of the LEDs on the 5400-RM.
 */
#define LED_D2   0x01 /* ONLINE */
#define LED_D4   0x02 /* 4G */
#define LED_D5   0x04 /* 3G */
#define LED_D6   0x08 /* BACKUP */
#define LED_D7   0x10 /* SS1 */
#define LED_D8   0x20 /* SS2 */
#define LED_D9   0x40 /* SS3 */
#define LED_D10  0x80 /* SS4 */
#define LED_D11  0x100 /* SS5 */
#define LEDMASK  0x1ff
#define LED_NR   9

struct ledgpiomap {
	char	*name;
	int	gpio;
};

static struct ledgpiomap ledgpio[] = {
	{ .name = "Online", .gpio = 8, },	/* LED D2 */
	{ .name = "4G", .gpio = 9, },		/* LED D4 */
	{ .name = "3G", .gpio = 13, },		/* LED D5 */
	{ .name = "BACKUP", .gpio = 14, },	/* LED D6 */
	{ .name = "RSS1", .gpio = 16, },	/* LED D7 */
	{ .name = "RSS2", .gpio = 35, },	/* LED D8 */
	{ .name = "RSS3", .gpio = 40, },	/* LED D9 */
	{ .name = "RSS4", .gpio = 41, },	/* LED D10 */
	{ .name = "RSS5", .gpio = 42, },	/* LED D11 */
};

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,

	[LEDMAN_ONLINE]    = LED_D2,
	[LEDMAN_LAN3_RX]   = LED_D4,
	[LEDMAN_LAN3_TX]   = LED_D5,
	[LEDMAN_ETH]       = LED_D6,

	[LEDMAN_RSS1]      = LED_D7,
	[LEDMAN_RSS2]      = LED_D8,
	[LEDMAN_RSS3]      = LED_D9,
	[LEDMAN_RSS4]      = LED_D10,
	[LEDMAN_RSS5]      = LED_D11,

	[LEDMAN_NVRAM_1]   = LEDMASK,
	[LEDMAN_NVRAM_2]   = 0,
};

static leddef_t	ledman_def = {
	/* [LEDS_FLASH] = LED_D6, */
};

#define	GPIO_ERASE	39
#define	GPIO_BUZZER	44

static unsigned long buzzer_progress[] = {
	0,
	LED_D11,
	LED_D10|LED_D11,
	LED_D9|LED_D10|LED_D11,
	LED_D8|LED_D9|LED_D10|LED_D11,
	LED_D7|LED_D8|LED_D9|LED_D10|LED_D11,
};

static void buzzer_init(void);
static void buzzer_start(void);

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	buzzer_start();
	return IRQ_HANDLED;
}

static void
ledman_set(unsigned long bits)
{
	int i;
	for (i = 0; i < LED_NR; i++)
		gpio_set_value(ledgpio[i].gpio, (bits & (1 << i)) ? 0 : 1);
}

static void
ledman_initarch(void)
{
	int i;

	for (i = 0; i < LED_NR; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(ledgpio[i].gpio, 1);
	}
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);

	buzzer_init();
}

/****************************************************************************/
#endif /* CONFIG_MACH_5400_RM_DT */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_UTM400)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

/*
 * Here is the GPIO line definition of the LEDs on the UTM400.
 */

#define LED_D2  0x01 /* HEARTBEAT */
#define LED_D3  0x02 /* ETH */
#define LED_D4  0x04 /* USB */
#define LED_D5  0x08 /* SERIAL */
#define LED_D6  0x10 /* HA */
#define LED_D7  0x20 /* ONLINE */
#define LED_D8  0x40 /* VPN */
#define LEDMASK 0x7f

static int led_to_pin[] = {
	12, /* LED_D2 - HEARTBEAT */
	13, /* LED_D3 - ETH */
	28, /* LED_D4 - USB */
	29, /* LED_D5 - SERIAL */
	36, /* LED_D6 - HA */
	37, /* LED_D7 - ONLINE */
	38, /* LED_D8 - VPN */
};

static ledmap_t	ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED_D2,
	[LEDMAN_LAN1_RX]   = LED_D3,
	[LEDMAN_LAN1_TX]   = LED_D3,
	[LEDMAN_LAN2_RX]   = LED_D3,
	[LEDMAN_LAN2_TX]   = LED_D3,
	[LEDMAN_LAN3_RX]   = LED_D3,
	[LEDMAN_LAN3_TX]   = LED_D3,
	[LEDMAN_USB1_RX]   = LED_D4,
	[LEDMAN_USB1_TX]   = LED_D4,
	[LEDMAN_COM1_RX]   = LED_D5,
	[LEDMAN_COM1_TX]   = LED_D5,
	[LEDMAN_HIGHAVAIL] = LED_D6,
	[LEDMAN_ONLINE]    = LED_D7,
	[LEDMAN_VPN]       = LED_D8,
	[LEDMAN_NVRAM_1]   = LED_D2 | LED_D4 | LED_D6 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D3 | LED_D5 | LED_D7,
	[LEDMAN_LAN1_DHCP] = LEDMASK,
};

static leddef_t	ledman_def = {
	[LEDS_FLASH] = LED_D2,
};

#define	ERASEGPIO	41
#define	ERASEIRQ	gpio_to_irq(ERASEGPIO)

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_set(unsigned long bits)
{
	int i;
	for (i = 0; i < 8; i++)
		gpio_set_value(led_to_pin[i], (bits & (1 << i)) ? 0 : 1);
}

static void
ledman_initarch(void)
{
	irq_set_irq_type(ERASEIRQ, IRQ_TYPE_EDGE_FALLING);

	/* Set up the factory erase button GPIO line */
	if (request_irq(ERASEIRQ, ledman_interrupt, 0, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE witch\n", ERASEIRQ);
	else
		pr_info("registered ERASE switch on IRQ%d\n", ERASEIRQ);

	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_MACH_UTM400 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_ATH79_MACH_NETREACH)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

/*
 *	Here is the definition of the LEDs on the NetReach. They are labelled
 *	on the board as LEDx.
 */
#define GPIO_LED2	0
#define GPIO_LED3	1
#define GPIO_LED4	13
#define GPIO_LED5	17
#define GPIO_LED6	27

#define	LED2		0x1
#define	LED3		0x2
#define	LED4		0x4
#define	LED5		0x8
#define	LED6		0x10
#define LEDMASK		0x1f

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_HEARTBEAT] = LED3,
	[LEDMAN_LAN1_RX]   = LED4,
	[LEDMAN_LAN1_TX]   = LED4,
	[LEDMAN_LAN2_RX]   = LED5,
	[LEDMAN_LAN2_TX]   = LED5,
	[LEDMAN_LAN3_RX]   = LED2,
	[LEDMAN_LAN3_TX]   = LED2,
	[LEDMAN_NVRAM_1]   = LED6,
};

static leddef_t ledman_def = {
	[LEDS_FLASH] = LED3,
};

#define	GPIO_BUTTON_RESET	12
#define	GPIO_BUTTON_WPS		11

#define	BUTTON_RESET		(1 << GPIO_BUTTON_RESET)
#define	BUTTON_WPS		(1 << GPIO_BUTTON_WPS)

#define	POLLTIME		(HZ / 20)	/* poll timer - 5ms */

static unsigned int netreach_buttons;
static struct timer_list netreach_timer;

static void netreach_poll(struct timer_list *t)
{
	if (gpio_get_value(GPIO_BUTTON_RESET) == 0) {
		if ((netreach_buttons & BUTTON_RESET) == 0) {
			netreach_buttons |= BUTTON_RESET;
			printk("LEDMAN: RESET button pushed!\n");
			ledman_signalreset();
		}
	} else {
		netreach_buttons &= ~BUTTON_RESET;
	}

	if (gpio_get_value(GPIO_BUTTON_WPS) == 1) {
		if ((netreach_buttons & BUTTON_WPS) == 0) {
			netreach_buttons |= BUTTON_WPS;
			ledman_cmd(LEDMAN_CMD_SET, LEDMAN_NVRAM_1);
			printk("LEDMAN: WPS button pushed!\n");
		}
	} else {
		netreach_buttons &= ~BUTTON_WPS;
	}

	netreach_timer.expires = jiffies + POLLTIME;
	add_timer(&netreach_timer);
}

static void
ledman_set(unsigned long bits)
{
	gpio_set_value(GPIO_LED2, (bits & LED2));
	gpio_set_value(GPIO_LED3, (bits & LED3));
	gpio_set_value(GPIO_LED4, (bits & LED4));
	gpio_set_value(GPIO_LED5, !(bits & LED5));
	gpio_set_value(GPIO_LED6, !(bits & LED6));
}

static void
ledman_initarch(void)
{
	gpio_request(GPIO_LED2, "WLAN");
	gpio_direction_output(GPIO_LED2, 1);
	gpio_request(GPIO_LED3, "HEARTBEAT");
	gpio_direction_output(GPIO_LED3, 1);
	gpio_request(GPIO_LED4, "LAN");
	gpio_direction_output(GPIO_LED4, 1);
	gpio_request(GPIO_LED5, "WAN");
	gpio_direction_output(GPIO_LED5, 1);
	gpio_request(GPIO_LED6, "WPS");
	gpio_direction_output(GPIO_LED6, 1);

	gpio_request(GPIO_BUTTON_RESET, "RESET BUTTON");
	gpio_direction_input(GPIO_BUTTON_RESET);
	gpio_request(GPIO_BUTTON_WPS, "WPS BUTTON");
	gpio_direction_input(GPIO_BUTTON_WPS);

	timer_setup(&netreach_timer, netreach_poll, 0);
	add_timer(&netreach_timer);

	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_ATH79_MACH_NETREACH */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_SOC_IMX28) /* Digi/WR11 */
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

#define LED_POWER_G	0x1
#define LED_NET_G	0x2
#define LED_NET_Y	0x4
#define LEDMASK		0x7

#define GPIO_LED0	112
#define GPIO_LED1	17
#define GPIO_LED2	21
#define GPIO_ERASE	73

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LEDMASK,
	[LEDMAN_POWER]     = LED_POWER_G,
	[LEDMAN_ETH]       = LED_NET_Y,
	[LEDMAN_ONLINE]    = LED_NET_G,
};

static leddef_t ledman_def = {
	[LEDS_ON]          = LED_POWER_G,
};

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_set(unsigned long bits)
{
	gpio_set_value(GPIO_LED0, (bits & LED_POWER_G) ? 1 : 0);
	gpio_set_value(GPIO_LED1, (bits & LED_NET_G) ? 1 : 0);
	gpio_set_value(GPIO_LED2, (bits & LED_NET_Y) ? 1 : 0);
}

static void
ledman_initarch(void)
{
#if 0
	int irq;
#endif

	if (!of_machine_is_compatible("digi,wr11"))
		return;

	gpio_request(GPIO_LED0, "Power");
	gpio_direction_output(GPIO_LED0, 0);
	gpio_request(GPIO_LED1, "NET Green");
	gpio_direction_output(GPIO_LED1, 0);
	gpio_request(GPIO_LED2, "NET Yellow");
	gpio_direction_output(GPIO_LED2, 0);
	ledman_set(LED_POWER_G);

#if 0
	/* This not enabled when USB debug mode in use */
	gpio_request(GPIO_ERASE, "ERASE");
	gpio_direction_input(GPIO_ERASE);
	irq = gpio_to_irq(GPIO_ERASE);
	if (request_irq(irq, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", irq);
	else
		pr_info("registered ERASE switch on IRQ%d\n", irq);
#endif
}

/****************************************************************************/
#endif /* CONFIG_SOC_IMX28 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_8300)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

#define LED_D2		0x1
#define LED_D3		0x2
#define LED_D4		0x4
#define LED_D5		0x8
#define LED_D6		0x10
#define LED_D7		0x20
#define LED_D8		0x40
#define LED_D9		0x80
#define LED_MASK	0xff
#define	LED_NR		8

#define	GPIO_ERASE	53

struct ledgpiomap {
	char	*name;
	int	gpio;
};

static struct ledgpiomap ledgpio[] = {
	{ .name = "Online", .gpio = 55, },
	{ .name = "Dial", .gpio = 56, },
	{ .name = "VPN", .gpio = 57, },
	{ .name = "WAN1", .gpio = 58, },
	{ .name = "WAN2", .gpio = 59, },
	{ .name = "LAN", .gpio = 60, },
	{ .name = "VPN", .gpio = 61, },
	{ .name = "USB", .gpio = 62, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,
	[LEDMAN_HEARTBEAT] = LED_D2,
	[LEDMAN_COM1_RX]   = LED_D3,
	[LEDMAN_LAN1_RX]   = LED_D7,
	[LEDMAN_LAN1_TX]   = LED_D7,
	[LEDMAN_LAN2_RX]   = LED_D5,
	[LEDMAN_LAN2_TX]   = LED_D5,
	[LEDMAN_NVRAM_1]   = LED_D5 | LED_D8,
	[LEDMAN_NVRAM_2]   = LED_D6 | LED_D7,
	[LEDMAN_VPN]       = LED_D4,
	/*[LEDMAN_LAN2_DHCP] = LED_D2,*/
	[LEDMAN_VPN_RX]    = LED_D8,
	[LEDMAN_VPN_TX]    = LED_D8,
	[LEDMAN_LAN3_RX]   = LED_D6,
	[LEDMAN_LAN3_TX]   = LED_D6,
	[LEDMAN_HIGHAVAIL] = LED_D9,
};

static leddef_t ledman_def = {
	[LEDS_FLASH] = LED_D2,
};

static unsigned int leds_state = LED_MASK;

static void
ledman_set(unsigned long bits)
{
	unsigned int i, b;
	unsigned int changed;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_initarch(void)
{
	int i;

	for (i = 0; i < LED_NR; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(ledgpio[i].gpio, 1);
	}
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* CONFIG_MACH_8300 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_6300CX)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char	*name;
	int	gpio;
};

#define LED_RED		0x1
#define LED_BLUE	0x2
#define LED_GREEN	0x4
#define LED_RSS1	0x8
#define LED_RSS2	0x10
#define LED_RSS3	0x20
#define LED_RSS4	0x40
#define LED_RSS5	0x80
#define LED_MASK	0xff
#define	LED_NR		8

#define	GPIO_ERASE	53

static struct ledgpiomap ledgpio[] = {
	{ .name = "Red", .gpio = 55, },
	{ .name = "Blue", .gpio = 56, },
	{ .name = "Green", .gpio = 57, },
	{ .name = "RSS1", .gpio = 58, },
	{ .name = "RSS2", .gpio = 59, },
	{ .name = "RSS3", .gpio = 60, },
	{ .name = "RSS4", .gpio = 61, },
	{ .name = "RSS5", .gpio = 62, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_ONLINE]    = LED_BLUE,
	[LEDMAN_COM]       = LED_RED,

	[LEDMAN_NVRAM_1]   = LED_RED,
	[LEDMAN_NVRAM_2]   = LED_RED | LED_GREEN,
	[LEDMAN_NVRAM_ALL] = LED_RED | LED_GREEN | LED_BLUE,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,
};

static leddef_t ledman_def = {
	[LEDS_ON] = LED_RED | LED_GREEN,
};

static unsigned int leds_state = LED_MASK;

static void
ledman_set(unsigned long bits)
{
	unsigned int i, b;
	unsigned int changed;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void
ledman_initarch(void)
{
	int i;

	for (i = 0; i < LED_NR; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(ledgpio[i].gpio, 1);
	}
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* CONFIG_MACH_6300CX */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_ARMADA_38X) && !(defined(CONFIG_MACH_U115) || defined(CONFIG_MACH_U120))
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char	*name;
	int	gpio;
};

#define LED_RSS1	0x1
#define LED_RSS2	0x2
#define LED_RSS3	0x4
#define LED_RSS4	0x8
#define LED_LTE		0x10
#define LED_3G		0x20
#define LED_SIM1	0x40
#define LED_SIM2	0x80
#define LED_MASK	0xff
#define	LED_NR		8

#define	GPIO_ERASE	54

static struct ledgpiomap ledgpio[] = {
	{ .name = "RSS1", .gpio = 31, },
	{ .name = "RSS2", .gpio = 29, },
	{ .name = "RSS3", .gpio = 27, },
	{ .name = "RSS4", .gpio = 26, },
	{ .name = "LTE", .gpio = 18, },
	{ .name = "3G", .gpio = 21, },
	{ .name = "SIM1", .gpio = 37, },
	{ .name = "SIM2", .gpio = 24, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = LED_LTE | LED_3G | LED_SIM1 | LED_SIM2 |
			     LED_RSS1 | LED_RSS2 | LED_RSS3 | LED_RSS4,
	[LEDMAN_NVRAM_ALL] = LED_MASK,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,

	[LEDMAN_ONLINE]    = LED_LTE,
	[LEDMAN_ETH]       = LED_3G,
	[LEDMAN_LAN1_LINK] = LED_SIM1,
	[LEDMAN_LAN2_LINK] = LED_SIM2,
};

static leddef_t ledman_def = { };

static unsigned int leds_state = LED_MASK;

static void ledman_set(unsigned long bits)
{
	unsigned int i, b;
	unsigned int changed;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_initarch(void)
{
	int i;

	if (!of_machine_is_compatible("accelerated,sprite"))
		return;

	for (i = 0; i < LED_NR; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(ledgpio[i].gpio, 1);
	}
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* defined(CONFIG_MACH_ARMADA_38X) && !(defined(CONFIG_MACH_U115) || defined(CONFIG_MACH_U120)) */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_U115)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char		*name;
	unsigned int	flags;
	unsigned int	gpio;
};

#define	F_INITED	0x1

#define LED_ONLINE	0x1
#define LED_VPN		0x2
#define LED_USB		0x4
#define LED_WAN1	0x8
#define LED_WAN2	0x10
#define LED_SERIAL	0x20

#define LED_RED		0x40
#define LED_BLUE	0x80
#define LED_GREEN	0x100
#define LED_RSS1	0x200
#define LED_RSS2	0x400
#define LED_RSS3	0x800
#define LED_RSS4	0x1000
#define LED_RSS5	0x2000
#define LED_SIM1_RED	0x4000
#define LED_SIM1_BLUE	0x8000
#define LED_SIM1_GREEN	0x10000
#define LED_SIM2_RED	0x20000
#define LED_SIM2_BLUE	0x40000
#define LED_SIM2_GREEN	0x80000

#define LED_MASK	0xfffff
#define	LED_NR		20

static struct ledgpiomap ledgpio[] = {
	{ .name = "ONLINE",	.gpio = 37, },
	{ .name = "VPN",	.gpio = 39, },
	{ .name = "USB",	.gpio = 43, },
	{ .name = "WAN1",	.gpio = 44, },
	{ .name = "WAN2",	.gpio = 52, },
	{ .name = "SERIAL",	.gpio = 53, },

	{ .name = "RED",	.gpio = 512, },
	{ .name = "BLUE",	.gpio = 513, },
	{ .name = "GREEN",	.gpio = 514, },
	{ .name = "RSS1",	.gpio = 515, },
	{ .name = "RSS2",	.gpio = 516, },
	{ .name = "RSS3",	.gpio = 517, },
	{ .name = "RSS4",	.gpio = 518, },
	{ .name = "RSS5",	.gpio = 519, },
	{ .name = "SIM1_RED",	.gpio = 520, },
	{ .name = "SIM1_BLUE",	.gpio = 521, },
	{ .name = "SIM1_GREEN",	.gpio = 522, },
	{ .name = "SIM2_RED",	.gpio = 523, },
	{ .name = "SIM2_BLUE",	.gpio = 524, },
	{ .name = "SIM2_GREEN",	.gpio = 525, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = LED_MASK,
	[LEDMAN_NVRAM_ALL] = LED_MASK,

	[LEDMAN_ONLINE]    = LED_ONLINE,
	[LEDMAN_VPN]       = LED_VPN,
	[LEDMAN_USB]       = LED_USB,
	[LEDMAN_LAN2_RX]   = LED_WAN1,
	[LEDMAN_LAN2_TX]   = LED_WAN1,
	[LEDMAN_LAN3_RX]   = LED_WAN2,
	[LEDMAN_LAN3_TX]   = LED_WAN2,
	[LEDMAN_COM]       = LED_SERIAL,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_COM1_DCD]  = LED_BLUE,
	[LEDMAN_COM2_DCD]  = LED_RED,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,

	[LEDMAN_SIM1]      = LED_SIM1_GREEN,
	[LEDMAN_SIM2]      = LED_SIM2_GREEN,
};

/* Erase is common to both */
#define	GPIO_ERASE	54

static leddef_t ledman_def = { };

static unsigned int leds_state = LED_MASK;
static unsigned int leds_bits = LED_MASK;
static struct work_struct leds_work;
static int leds_scheduled;
static int leds_deferedsetup;

static void ledman_set(unsigned long bits)
{
	leds_bits = bits;
	if (!leds_scheduled) {
		leds_scheduled++;
		schedule_work(&leds_work);
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_initgpio(void)
{
	int i, rc;

	leds_deferedsetup = 0;
	for (i = 0; i < LED_NR; i++) {
		if (ledgpio[i].flags & F_INITED)
			continue;

		rc = gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		if (rc < 0) {
			leds_deferedsetup++;
			continue;
		}
		gpio_direction_output(ledgpio[i].gpio, 1);
		ledgpio[i].flags |= F_INITED;
	}
}

static void ledman_work(struct work_struct *work)
{
	unsigned int i, changed;
	unsigned int b, bits;

	if (leds_deferedsetup)
		ledman_initgpio();

	/*
	 * Get new state and mark that we have processed that.
	 * This ordering avoids need for preempt protection.
	 */
	bits = leds_bits;
	leds_scheduled = 0;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value_cansleep(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static void ledman_initarch(void)
{
	int i;

	if (!of_machine_is_compatible("accelerated,u115"))
		return;

	INIT_WORK(&leds_work, ledman_work);

	/* Set up direct connect local LEDs now */
	ledman_initgpio();
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* defined(CONFIG_MACH_U115) */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_U120) && !defined(CONFIG_LEDS_GPIO)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char		*name;
	unsigned int	flags;
	unsigned int	gpio;
};

#define	F_INITED	0x1

// NOTE: the led names on the board DO NOT match the front case names
#define LED_ONLINE	0x200000
#define LED_VPN		0x400000
#define LED_USB		0x000001
#define LED_WAN1	0x000002
#define LED_WAN2	0x000004
#define LED_SERIAL	0x000008
#define LED_SLIDE_RED	0x000010
#define LED_SLIDE_BLUE	0x000020
#define LED_SLIDE_GREEN	0x000040

#define LED_RED		0x000080
#define LED_BLUE	0x000100
#define LED_GREEN	0x000200
#define LED_RSS1	0x000400
#define LED_RSS2	0x000800
#define LED_RSS3	0x001000
#define LED_RSS4	0x002000
#define LED_RSS5	0x004000
#define LED_SIM1_RED	0x008000
#define LED_SIM1_BLUE	0x010000
#define LED_SIM1_GREEN	0x020000
#define LED_SIM2_RED	0x040000
#define LED_SIM2_BLUE	0x080000
#define LED_SIM2_GREEN	0x100000

#define LED_MASK	0x7fffff
#define	LED_NR		23

static struct ledgpiomap ledgpio[] = {
	{ .name = "USB",	.gpio = 528, },
	{ .name = "SERIAL",	.gpio = 529, },
	{ .name = "WAN1",	.gpio = 530, },
	{ .name = "WAN2",	.gpio = 531, },
	{ .name = "SLIDE_RED",	.gpio = 532, },
	{ .name = "SLIDE_BLUE",	.gpio = 533, },
	{ .name = "SLIDE_GREEN",.gpio = 534, },

	{ .name = "RED",	.gpio = 512, },
	{ .name = "BLUE",	.gpio = 513, },
	{ .name = "GREEN",	.gpio = 514, },
	{ .name = "RSS1",	.gpio = 515, },
	{ .name = "RSS2",	.gpio = 516, },
	{ .name = "RSS3",	.gpio = 517, },
	{ .name = "RSS4",	.gpio = 518, },
	{ .name = "RSS5",	.gpio = 519, },
	{ .name = "SIM1_RED",	.gpio = 520, },
	{ .name = "SIM1_BLUE",	.gpio = 521, },
	{ .name = "SIM1_GREEN",	.gpio = 522, },
	{ .name = "SIM2_RED",	.gpio = 523, },
	{ .name = "SIM2_BLUE",	.gpio = 524, },
	{ .name = "SIM2_GREEN",	.gpio = 525, },
	{ .name = "ONLINE",	.gpio = 526, },
	{ .name = "VPN",	.gpio = 527, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_NVRAM_1]   = LED_ONLINE | LED_VPN | LED_WAN2 | LED_SERIAL,
	[LEDMAN_NVRAM_2]   = LED_USB | LED_WAN1,
	[LEDMAN_NVRAM_ALL] = LED_MASK,

	[LEDMAN_ONLINE]    = LED_ONLINE,
	[LEDMAN_VPN]       = LED_VPN,
	[LEDMAN_LAN2_RX]   = LED_WAN1,
	[LEDMAN_LAN2_TX]   = LED_WAN1,
	[LEDMAN_LAN3_RX]   = LED_WAN2,
	[LEDMAN_LAN3_TX]   = LED_WAN2,
	[LEDMAN_COM]       = LED_SERIAL,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_COM1_DCD]  = LED_BLUE,
	[LEDMAN_COM2_DCD]  = LED_RED,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,

	[LEDMAN_SIM1_RED]  = LED_SIM1_RED,
	[LEDMAN_SIM1_GREEN]= LED_SIM1_GREEN,
	[LEDMAN_SIM1_BLUE] = LED_SIM1_BLUE,

	[LEDMAN_SIM2_RED]  = LED_SIM2_RED,
	[LEDMAN_SIM2_GREEN]= LED_SIM2_GREEN,
	[LEDMAN_SIM2_BLUE] = LED_SIM2_BLUE,

	[LEDMAN_XBEE1]     = LED_SLIDE_RED,
	[LEDMAN_XBEE2]     = LED_SLIDE_GREEN,
	[LEDMAN_XBEE3]     = LED_SLIDE_BLUE,
};

/* Erase is common to both */
#define	GPIO_ERASE	54

static leddef_t ledman_def = { };

static unsigned int leds_state = LED_MASK;
static unsigned int leds_bits = LED_MASK;
static struct work_struct leds_work;
static int leds_scheduled;
static int leds_deferedsetup;

static void ledman_set(unsigned long bits)
{
	leds_bits = bits;
	if (!leds_scheduled) {
		leds_scheduled++;
		schedule_work(&leds_work);
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_initgpio(void)
{
	int i, rc;

	leds_deferedsetup = 0;
	for (i = 0; i < LED_NR; i++) {
		if (ledgpio[i].flags & F_INITED)
			continue;

		rc = gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		if (rc < 0) {
			leds_deferedsetup++;
			continue;
		}
		gpio_direction_output(ledgpio[i].gpio, 1);
		ledgpio[i].flags |= F_INITED;
	}
}

static void ledman_work(struct work_struct *work)
{
	unsigned int i, changed;
	unsigned int b, bits;

	if (leds_deferedsetup)
		ledman_initgpio();

	/*
	 * Get new state and mark that we have processed that.
	 * This ordering avoids need for preempt protection.
	 */
	bits = leds_bits;
	leds_scheduled = 0;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value_cansleep(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static void ledman_initarch(void)
{
	int i;

	INIT_WORK(&leds_work, ledman_work);

	/* Set up direct connect local LEDs now */
	ledman_initgpio();
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* defined(CONFIG_MACH_U120) && !defined(CONFIG_LEDS_GPIO) */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_MACH_6350SR) || defined(CONFIG_MACH_6330MX)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/workqueue.h>
#include <linux/gpio.h>

struct ledgpiomap {
	char		*name;
	unsigned int	flags;
	unsigned int	gpio;
};

#define LED0		0x1
#define LED1		0x2
#define LED2		0x4
#define LED3		0x8
#define LED_RED		0x10
#define LED_BLUE	0x20
#define LED_GREEN	0x40
#define LED_RSS1	0x80
#define LED_RSS2	0x100
#define LED_RSS3	0x200
#define LED_RSS4	0x400
#define LED_RSS5	0x800
#define LED_SIM1	0x1000
#define LED_SIM2	0x2000
#define LED_MASK	0x3fff
#define LED_NR		14

#define GPIO_ERASE	53

#define	F_INITED	0x1

static struct ledgpiomap ledgpio[] = {
	{ .name = "LED0", .gpio = 32, },
	{ .name = "LED1", .gpio = 65, },
	{ .name = "LED2", .gpio = 64, },
#if defined(CONFIG_MACH_6330MX)
	{ .name = "LED3", .gpio = 48, },
#else
	{ .name = "LED3", .gpio = 63, },
#endif
	{ .name = "RED", .gpio = 512, },
	{ .name = "BLUE", .gpio = 513, },
	{ .name = "GREEN", .gpio = 514, },
	{ .name = "RSS1", .gpio = 515, },
	{ .name = "RSS2", .gpio = 516, },
	{ .name = "RSS3", .gpio = 517, },
	{ .name = "RSS4", .gpio = 518, },
	{ .name = "RSS5", .gpio = 519, },
	{ .name = "SIM1", .gpio = 520, },
	{ .name = "SIM2", .gpio = 521, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_POWER]     = LED0,

#if defined(CONFIG_MACH_6330MX)
	[LEDMAN_LAN1_LINK] = LED1,
	[LEDMAN_LAN2_LINK] = LED2,
	[LEDMAN_LAN3_LINK] = LED3,
#else
	[LEDMAN_LAN1_RX]   = LED1,
	[LEDMAN_LAN1_TX]   = LED1,
	[LEDMAN_LAN2_RX]   = LED2,
	[LEDMAN_LAN2_TX]   = LED2,
	[LEDMAN_LAN3_RX]   = LED3,
	[LEDMAN_LAN3_TX]   = LED3,
#endif

	[LEDMAN_NVRAM_1]   = LED0 | LED1 | LED2 | LED3 |
			     LED_RED | LED_RSS1 | LED_RSS2 | LED_RSS3 |
			     LED_RSS4 | LED_RSS5 | LED_SIM1 | LED_SIM2,
			     LED_RSS5 | LED_SIM1 | LED_SIM2,
	[LEDMAN_NVRAM_2]   = LED_RED | LED_GREEN,
	[LEDMAN_NVRAM_ALL] = LED0 | LED1 | LED2 | LED3 |
			     LED_RED | LED_GREEN | LED_BLUE |
			     LED_RSS1 | LED_RSS2 | LED_RSS3 | LED_RSS4 |
			     LED_RSS5 | LED_SIM1 | LED_SIM2,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_ONLINE]    = LED_BLUE,
	[LEDMAN_COM]       = LED_RED,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,

	[LEDMAN_SIM1]      = LED_SIM1,
	[LEDMAN_SIM2]      = LED_SIM2,
};

static leddef_t ledman_def = {
	[LEDS_ON] = LED0 | LED_RED | LED_GREEN,
};

static unsigned int leds_state;
static unsigned int leds_bits = LED_MASK;
static struct work_struct leds_work;
static int leds_scheduled;
static unsigned int leds_deferedsetup;

static void ledman_set(unsigned long bits)
{
	leds_bits = bits;
	if (!leds_scheduled) {
		leds_scheduled++;
		schedule_work(&leds_work);
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_initgpio(void)
{
	int i, rc;

	leds_deferedsetup = 0;
	for (i = 0; i < LED_NR; i++) {
		if (ledgpio[i].flags & F_INITED)
			continue;

		rc = gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		if (rc < 0) {
			leds_deferedsetup |= 0x1 << i;
			continue;
		}
		gpio_direction_output(ledgpio[i].gpio, 1);
		ledgpio[i].flags |= F_INITED;
	}
}

static void ledman_work(struct work_struct *work)
{
	unsigned int i, changed;
	unsigned int b, bits;

	if (leds_deferedsetup)
		ledman_initgpio();

	/*
	 * Get new state and mark that we have processed that.
	 * This ordering avoids need for preempt protection.
	 */
	bits = leds_bits & ~leds_deferedsetup;
	leds_scheduled = 0;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value_cansleep(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static void ledman_initarch(void)
{
	int i;

	INIT_WORK(&leds_work, ledman_work);

	/* Set up direct connect local LEDs now */
	ledman_initgpio();
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
}

/****************************************************************************/
#endif /* CONFIG_MACH_6350SR || CONFIG_MACH_6330MX */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_SOC_IMX6UL)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char	*name;
	int	gpio;
	int flags;
};

#define F_INITED		0x1

#define AC6310DX_LED_RED	0x1
#define AC6310DX_LED_BLUE	0x2
#define AC6310DX_LED_GREEN	0x4
#define AC6310DX_LED_RSS1	0x8
#define AC6310DX_LED_RSS2	0x10
#define AC6310DX_LED_RSS3	0x20
#define AC6310DX_LED_RSS4	0x40
#define AC6310DX_LED_RSS5	0x80
#define AC6310DX_LED_SIM1	0x100
#define AC6310DX_LED_SIM2	0x200
#define AC6310DX_LED_ETH1	0x400
#define AC6310DX_LED_ETH2	0x800
#define AC6310DX_LED_MASK	0xfff
#define AC6310DX_LED_NR		12

#define AC6310DX_GPIO_ERASE	49

static struct ledgpiomap ac6310dx_ledgpio[] = {
	{ .name = "RED", .gpio = 0, },
	{ .name = "BLUE", .gpio = 1, },
	{ .name = "GREEN", .gpio = 2, },
	{ .name = "RSS1", .gpio = 3, },
	{ .name = "RSS2", .gpio = 4, },
	{ .name = "RSS3", .gpio = 5, },
	{ .name = "RSS4", .gpio = 128, },
	{ .name = "RSS5", .gpio = 129, },
	{ .name = "SIM1", .gpio = 9, },
	{ .name = "SIM2", .gpio = 8, },
	{ .name = "ETH1", .gpio = 114, },
	{ .name = "ETH2", .gpio = 113, },
};

static ledmap_t ac6310dx_ledman_std = {
	[LEDMAN_ALL]       = AC6310DX_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = AC6310DX_LED_RED | AC6310DX_LED_GREEN |
			     AC6310DX_LED_SIM1 | AC6310DX_LED_SIM2 |
			     AC6310DX_LED_RSS1 | AC6310DX_LED_RSS2 |
			     AC6310DX_LED_RSS3 | AC6310DX_LED_RSS4 |
			     AC6310DX_LED_RSS5,
	[LEDMAN_NVRAM_ALL] = AC6310DX_LED_MASK,

	[LEDMAN_RSS1]      = AC6310DX_LED_RSS1,
	[LEDMAN_RSS2]      = AC6310DX_LED_RSS2,
	[LEDMAN_RSS3]      = AC6310DX_LED_RSS3,
	[LEDMAN_RSS4]      = AC6310DX_LED_RSS4,
	[LEDMAN_RSS5]      = AC6310DX_LED_RSS5,

	[LEDMAN_ETH]       = AC6310DX_LED_GREEN,
	[LEDMAN_ONLINE]    = AC6310DX_LED_BLUE,
	[LEDMAN_COM]       = AC6310DX_LED_RED,

	[LEDMAN_SIM1]      = AC6310DX_LED_SIM1,
	[LEDMAN_SIM2]      = AC6310DX_LED_SIM2,

	[LEDMAN_LAN1_LINK] = AC6310DX_LED_ETH1,
	[LEDMAN_LAN2_LINK] = AC6310DX_LED_ETH2,
};

static leddef_t ac6310dx_ledman_def = {
	[LEDS_ON] = AC6310DX_LED_RED | AC6310DX_LED_GREEN,
};


#define CONNECTIT_MINI_LED_RED		0x1
#define CONNECTIT_MINI_LED_BLUE		0x2
#define CONNECTIT_MINI_LED_GREEN	0x4
#define CONNECTIT_MINI_LED_RSS1		0x8
#define CONNECTIT_MINI_LED_RSS2		0x10
#define CONNECTIT_MINI_LED_RSS3		0x20
#define CONNECTIT_MINI_LED_RSS4		0x40
#define CONNECTIT_MINI_LED_RSS5		0x80
#define CONNECTIT_MINI_LED_POWER	0x100
#define CONNECTIT_MINI_LED_ONLINE	0x200
#define CONNECTIT_MINI_LED_MASK		0x3ff
#define CONNECTIT_MINI_LED_NR		10

static struct ledgpiomap connectit_mini_ledgpio[] = {
	{ .name = "RED", .gpio = 0, },
	{ .name = "BLUE", .gpio = 1, },
	{ .name = "GREEN", .gpio = 2, },
	{ .name = "RSS1", .gpio = 3, },
	{ .name = "RSS2", .gpio = 4, },
	{ .name = "RSS3", .gpio = 5, },
	{ .name = "RSS4", .gpio = 128, },
	{ .name = "RSS5", .gpio = 129, },
	{ .name = "POWER", .gpio = 9, },
	{ .name = "ONLINE", .gpio = 8, },
};

static ledmap_t connectit_mini_ledman_std = {
	[LEDMAN_ALL]       = CONNECTIT_MINI_LED_MASK,
	[LEDMAN_POWER]     = CONNECTIT_MINI_LED_POWER,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = CONNECTIT_MINI_LED_RED | CONNECTIT_MINI_LED_GREEN |
			     CONNECTIT_MINI_LED_RSS1 | CONNECTIT_MINI_LED_RSS2 |
			     CONNECTIT_MINI_LED_RSS3 | CONNECTIT_MINI_LED_RSS4 |
			     CONNECTIT_MINI_LED_RSS5,
	[LEDMAN_NVRAM_ALL] = CONNECTIT_MINI_LED_MASK,

	[LEDMAN_RSS1]      = CONNECTIT_MINI_LED_RSS1,
	[LEDMAN_RSS2]      = CONNECTIT_MINI_LED_RSS2,
	[LEDMAN_RSS3]      = CONNECTIT_MINI_LED_RSS3,
	[LEDMAN_RSS4]      = CONNECTIT_MINI_LED_RSS4,
	[LEDMAN_RSS5]      = CONNECTIT_MINI_LED_RSS5,

	[LEDMAN_ETH]       = CONNECTIT_MINI_LED_GREEN,
	[LEDMAN_ONLINE]    = CONNECTIT_MINI_LED_BLUE,
	[LEDMAN_COM]       = CONNECTIT_MINI_LED_RED,
	[LEDMAN_LAN3_RX]   = CONNECTIT_MINI_LED_ONLINE,
	[LEDMAN_LAN3_TX]   = CONNECTIT_MINI_LED_ONLINE,
};

static leddef_t connectit_mini_ledman_def = {
	[LEDS_ON] = CONNECTIT_MINI_LED_POWER |CONNECTIT_MINI_LED_RED | CONNECTIT_MINI_LED_GREEN,
};


#define CONNECTIT4_LED_RED	0x1
#define CONNECTIT4_LED_BLUE	0x2
#define CONNECTIT4_LED_GREEN	0x4
#define CONNECTIT4_LED_RSS1	0x8
#define CONNECTIT4_LED_RSS2	0x10
#define CONNECTIT4_LED_RSS3	0x20
#define CONNECTIT4_LED_RSS4	0x40
#define CONNECTIT4_LED_RSS5	0x80
#define CONNECTIT4_LED_SIM1	0x100
#define CONNECTIT4_LED_SIM2	0x200
#define CONNECTIT4_LED_POWER	0x400
#define CONNECTIT4_LED_ONLINE	0x800
#define CONNECTIT4_LED_BACKUP	0x1000
#define CONNECTIT4_LED_SERIAL	0x2000
#define CONNECTIT4_LED_MASK	0x3fff
#define CONNECTIT4_LED_NR	14

static struct ledgpiomap connectit4_ledgpio[] = {
	{ .name = "RED", .gpio = 0, },
	{ .name = "BLUE", .gpio = 1, },
	{ .name = "GREEN", .gpio = 2, },
	{ .name = "RSS1", .gpio = 3, },
	{ .name = "RSS2", .gpio = 4, },
	{ .name = "RSS3", .gpio = 5, },
	{ .name = "RSS4", .gpio = 128, },
	{ .name = "RSS5", .gpio = 129, },
	{ .name = "SIM1", .gpio = 8, },
	{ .name = "SIM2", .gpio = 9, },
	{ .name = "POWER", .gpio = 116, },
	{ .name = "ONLINE", .gpio = 115, },
	{ .name = "BACKUP", .gpio = 114, },
	{ .name = "SERIAL", .gpio = 113, },
};

static ledmap_t connectit4_ledman_std = {
	[LEDMAN_POWER]     = CONNECTIT4_LED_POWER,
	[LEDMAN_ALL]       = CONNECTIT4_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = CONNECTIT4_LED_RED | CONNECTIT4_LED_GREEN |
			     CONNECTIT4_LED_SIM1 | CONNECTIT4_LED_SIM2 |
			     CONNECTIT4_LED_RSS1 | CONNECTIT4_LED_RSS2 |
			     CONNECTIT4_LED_RSS3 | CONNECTIT4_LED_RSS4 |
			     CONNECTIT4_LED_RSS5,
	[LEDMAN_NVRAM_ALL] = CONNECTIT4_LED_MASK,

	[LEDMAN_RSS1]      = CONNECTIT4_LED_RSS1,
	[LEDMAN_RSS2]      = CONNECTIT4_LED_RSS2,
	[LEDMAN_RSS3]      = CONNECTIT4_LED_RSS3,
	[LEDMAN_RSS4]      = CONNECTIT4_LED_RSS4,
	[LEDMAN_RSS5]      = CONNECTIT4_LED_RSS5,

	[LEDMAN_ETH]       = CONNECTIT4_LED_GREEN,
	[LEDMAN_ONLINE]    = CONNECTIT4_LED_BLUE,
	[LEDMAN_COM]       = CONNECTIT4_LED_RED,

	[LEDMAN_SIM1]      = CONNECTIT4_LED_SIM1,
	[LEDMAN_SIM2]      = CONNECTIT4_LED_SIM2,

	[LEDMAN_LAN3_RX]   = CONNECTIT4_LED_ONLINE,
	[LEDMAN_LAN3_TX]   = CONNECTIT4_LED_BACKUP,
	[LEDMAN_COM1_RX]   = CONNECTIT4_LED_SERIAL,
	[LEDMAN_COM1_TX]   = CONNECTIT4_LED_SERIAL,
};

static leddef_t connectit4_ledman_def = {
	[LEDS_ON] = CONNECTIT4_LED_POWER | CONNECTIT4_LED_RED | CONNECTIT4_LED_GREEN,
};


#define IX20_LED_RED	0x1
#define IX20_LED_BLUE	0x2
#define IX20_LED_GREEN	0x4
#define IX20_LED_RSS1	0x8
#define IX20_LED_RSS2	0x10
#define IX20_LED_RSS3	0x20
#define IX20_LED_RSS4	0x40
#define IX20_LED_RSS5	0x80
#define IX20_LED_INT	0x100
#define IX20_LED_WIFI	0x200
#define IX20_LED_SIM1	0x400
#define IX20_LED_SIM2	0x800
#define IX20_LED_MASK	0xfff
#define IX20_LED_NR	12

static struct ledgpiomap ix20_ledgpio[] = {
	{ .name = "RED", .gpio = 0, },
	{ .name = "BLUE", .gpio = 1, },
	{ .name = "GREEN", .gpio = 2, },
	{ .name = "RSS1", .gpio = 3, },
	{ .name = "RSS2", .gpio = 4, },
	{ .name = "RSS3", .gpio = 5, },
	{ .name = "RSS4", .gpio = 64, },
	{ .name = "RSS5", .gpio = 65, },
	{ .name = "INT", .gpio = 8, },
	{ .name = "WIFI", .gpio = 9, },
	{ .name = "SIM1", .gpio = 128, },
	{ .name = "SIM2", .gpio = 129, },
};

static ledmap_t ix20_ledman_std = {
	[LEDMAN_ALL]       = IX20_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = IX20_LED_RED | IX20_LED_GREEN |
			     IX20_LED_RSS1 | IX20_LED_RSS2 |
			     IX20_LED_RSS3 | IX20_LED_RSS4 |
			     IX20_LED_RSS5 |
			     IX20_LED_SIM1 | IX20_LED_SIM2 |
			     IX20_LED_INT | IX20_LED_WIFI,
	[LEDMAN_NVRAM_ALL] = IX20_LED_MASK,

	[LEDMAN_RSS1]      = IX20_LED_RSS1,
	[LEDMAN_RSS2]      = IX20_LED_RSS2,
	[LEDMAN_RSS3]      = IX20_LED_RSS3,
	[LEDMAN_RSS4]      = IX20_LED_RSS4,
	[LEDMAN_RSS5]      = IX20_LED_RSS5,

	[LEDMAN_ETH]       = IX20_LED_GREEN,
	[LEDMAN_ONLINE]    = IX20_LED_BLUE,
	[LEDMAN_COM]       = IX20_LED_RED,

	[LEDMAN_SIM1]      = IX20_LED_SIM1,
	[LEDMAN_SIM2]      = IX20_LED_SIM2,

	[LEDMAN_LAN3_RX]   = IX20_LED_INT,
	[LEDMAN_LAN3_TX]   = IX20_LED_WIFI,
};

static leddef_t ix20_ledman_def = {
	[LEDS_ON] = IX20_LED_RED | IX20_LED_GREEN,
};


#define IX10_LED_RED		0x1
#define IX10_LED_BLUE		0x2
#define IX10_LED_GREEN		0x4
#define IX10_LED_RSS1		0x8
#define IX10_LED_RSS2		0x10
#define IX10_LED_RSS3		0x20
#define IX10_LED_RSS4		0x40
#define IX10_LED_RSS5		0x80
#define IX10_LED_ONLINE		0x100
#define IX10_LED_SIM_RED	0x200
#define IX10_LED_SIM_GREEN	0x400
#define IX10_LED_SIM_BLUE	0x800
#define IX10_LED_MASK		0xfff
#define IX10_LED_NR		12

static struct ledgpiomap ix10_ledgpio[] = {
	{ .name = "RED", .gpio = 0, },
	{ .name = "BLUE", .gpio = 1, },
	{ .name = "GREEN", .gpio = 2, },
	{ .name = "RSS1", .gpio = 3, },
	{ .name = "RSS2", .gpio = 4, },
	{ .name = "RSS3", .gpio = 5, },
	{ .name = "RSS4", .gpio = 64, },
	{ .name = "RSS5", .gpio = 65, },
	{ .name = "ONLINE", .gpio = 8, },
	{ .name = "SIM-RED", .gpio = 9, },
	{ .name = "SIM-GREEN", .gpio = 128, },
	{ .name = "SIM-BLUE", .gpio = 129, },
};

static ledmap_t ix10_ledman_std = {
	[LEDMAN_ALL]       = IX10_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = IX10_LED_RED | IX10_LED_GREEN |
			     IX10_LED_RSS1 | IX10_LED_RSS2 |
			     IX10_LED_RSS3 | IX10_LED_RSS4 |
			     IX10_LED_RSS5 | IX10_LED_ONLINE |
			     IX10_LED_SIM_RED | IX10_LED_SIM_GREEN,
	[LEDMAN_NVRAM_ALL] = IX10_LED_MASK,

	[LEDMAN_RSS1]      = IX10_LED_RSS1,
	[LEDMAN_RSS2]      = IX10_LED_RSS2,
	[LEDMAN_RSS3]      = IX10_LED_RSS3,
	[LEDMAN_RSS4]      = IX10_LED_RSS4,
	[LEDMAN_RSS5]      = IX10_LED_RSS5,

	[LEDMAN_ETH]       = IX10_LED_GREEN,
	[LEDMAN_ONLINE]    = IX10_LED_BLUE,
	[LEDMAN_COM]       = IX10_LED_RED,

	[LEDMAN_SIM1]      = IX10_LED_SIM_GREEN,
	[LEDMAN_SIM2]      = IX10_LED_SIM_BLUE,
	[LEDMAN_SIM_FAIL]  = IX10_LED_SIM_RED,

	[LEDMAN_LAN3_RX]   = IX10_LED_ONLINE,
	[LEDMAN_LAN3_TX]   = IX10_LED_ONLINE,
};

static leddef_t ix10_ledman_def = {
	[LEDS_ON] = IX10_LED_RED | IX10_LED_GREEN,
};


#define IX15_LED_RED		0x1
#define IX15_LED_BLUE		0x2
#define IX15_LED_GREEN		0x4
#define IX15_LED_RSS1		0x8
#define IX15_LED_RSS2		0x10
#define IX15_LED_RSS3		0x20
#define IX15_LED_RSS4		0x40
#define IX15_LED_RSS5		0x80
#define IX15_LED_ONLINE		0x100
#define IX15_LED_SIM_RED	0x200
#define IX15_LED_SIM_GREEN	0x400
#define IX15_LED_SIM_BLUE	0x800
#define IX15_LED_XBEE_RED	0x1000
#define IX15_LED_XBEE_GREEN	0x2000
#define IX15_LED_XBEE_BLUE	0x4000
#define IX15_LED_MASK		0xffff
#define IX15_LED_NR		15

static struct ledgpiomap ix15_ledgpio[] = {
	{ .name = "RED", .gpio = 45, },
	{ .name = "BLUE", .gpio = 43, },
	{ .name = "GREEN", .gpio = 44, },
	{ .name = "RSS1", .gpio = 14, },
	{ .name = "RSS2", .gpio = 11, },
	{ .name = "RSS3", .gpio = 13, },
	{ .name = "RSS4", .gpio = 12, },
	{ .name = "RSS5", .gpio = 15, },
	{ .name = "ONLINE", .gpio = 28, },
	{ .name = "SIM-RED", .gpio = 40, },
	{ .name = "SIM-GREEN", .gpio = 46, },
	{ .name = "SIM-BLUE", .gpio = 117, },
	{ .name = "XBEE-RED", .gpio = 42, },
	{ .name = "XBEE-GREEN", .gpio = 47, },
	{ .name = "XBEE-BLUE", .gpio = 41, },
};

static ledmap_t ix15_ledman_std = {
	[LEDMAN_ALL]       = IX15_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = IX15_LED_RED | IX15_LED_GREEN |
			     IX15_LED_RSS1 | IX15_LED_RSS2 |
			     IX15_LED_RSS3 | IX15_LED_RSS4 |
			     IX15_LED_RSS5 | IX15_LED_ONLINE |
			     IX15_LED_SIM_RED | IX15_LED_SIM_GREEN |
			     IX15_LED_XBEE_RED | IX15_LED_XBEE_GREEN,
	[LEDMAN_NVRAM_ALL] = IX15_LED_MASK,

	[LEDMAN_RSS1]      = IX15_LED_RSS1,
	[LEDMAN_RSS2]      = IX15_LED_RSS2,
	[LEDMAN_RSS3]      = IX15_LED_RSS3,
	[LEDMAN_RSS4]      = IX15_LED_RSS4,
	[LEDMAN_RSS5]      = IX15_LED_RSS5,

	[LEDMAN_ETH]       = IX15_LED_GREEN,
	[LEDMAN_ONLINE]    = IX15_LED_BLUE,
	[LEDMAN_COM]       = IX15_LED_RED,

	[LEDMAN_SIM1]      = IX15_LED_SIM_GREEN,
	[LEDMAN_SIM2]      = IX15_LED_SIM_BLUE,
	[LEDMAN_SIM_FAIL]  = IX15_LED_SIM_RED,

	[LEDMAN_LAN3_RX]   = IX15_LED_ONLINE,
	[LEDMAN_LAN3_TX]   = IX15_LED_ONLINE,

	[LEDMAN_XBEE1]   = IX15_LED_XBEE_RED,
	[LEDMAN_XBEE2]   = IX15_LED_XBEE_GREEN,
	[LEDMAN_XBEE3]   = IX15_LED_XBEE_BLUE,
};

static leddef_t ix15_ledman_def = {
	[LEDS_ON] = IX15_LED_RED | IX15_LED_GREEN,
};

#define IX30_LED_RED	0x1
#define IX30_LED_BLUE	0x2
#define IX30_LED_GREEN	0x4
#define IX30_LED_RSS1	0x8
#define IX30_LED_RSS2	0x10
#define IX30_LED_RSS3	0x20
#define IX30_LED_RSS4	0x40
#define IX30_LED_RSS5	0x80
#define IX30_LED_ONLINE	0x100
#define IX30_LED_MASK	0x1ff
#define IX30_LED_NR	9

#define IX30_LEDS_BASE	528

static struct ledgpiomap ix30_ledgpio[] = {
	{ .name = "RED",    .gpio = IX30_LEDS_BASE + 0, },
	{ .name = "BLUE",   .gpio = IX30_LEDS_BASE + 1, },
	{ .name = "GREEN",  .gpio = IX30_LEDS_BASE + 2, },
	{ .name = "RSS1",   .gpio = IX30_LEDS_BASE + 3, },
	{ .name = "RSS2",   .gpio = IX30_LEDS_BASE + 4, },
	{ .name = "RSS3",   .gpio = IX30_LEDS_BASE + 5, },
	{ .name = "RSS4",   .gpio = IX30_LEDS_BASE + 6, },
	{ .name = "RSS5",   .gpio = IX30_LEDS_BASE + 7, },
	{ .name = "ONLINE", .gpio = IX30_LEDS_BASE + 8, },
};

static ledmap_t ix30_ledman_std = {
	[LEDMAN_ALL]       = IX30_LED_MASK,

	[LEDMAN_NVRAM_1]   = 0,
	[LEDMAN_NVRAM_2]   = IX30_LED_RED | IX30_LED_GREEN |
			     IX30_LED_RSS1 | IX30_LED_RSS2 |
			     IX30_LED_RSS3 | IX30_LED_RSS4 |
			     IX30_LED_RSS5 | IX30_LED_ONLINE,
	[LEDMAN_NVRAM_ALL] = IX30_LED_MASK,

	[LEDMAN_RSS1]      = IX30_LED_RSS1,
	[LEDMAN_RSS2]      = IX30_LED_RSS2,
	[LEDMAN_RSS3]      = IX30_LED_RSS3,
	[LEDMAN_RSS4]      = IX30_LED_RSS4,
	[LEDMAN_RSS5]      = IX30_LED_RSS5,

	[LEDMAN_ETH]       = IX30_LED_GREEN,
	[LEDMAN_ONLINE]    = IX30_LED_BLUE,
	[LEDMAN_COM]       = IX30_LED_RED,

	[LEDMAN_LAN3_RX]   = IX30_LED_ONLINE,
};

static leddef_t ix30_ledman_def = {
	[LEDS_ON] = IX30_LED_RED | IX30_LED_GREEN,
};

static unsigned int led_nr;
static unsigned int leds_state;
static struct ledgpiomap *ledgpio;
static ledmap_t ledman_std;
static leddef_t ledman_def;

static unsigned int leds_bits;
static struct work_struct leds_work;
static bool leds_use_wq;
static int leds_deferred_setup;
static int leds_scheduled;

static void ac6310dx_ledman_init(void)
{
	led_nr = AC6310DX_LED_NR;
	leds_state = AC6310DX_LED_MASK;
	ledgpio = ac6310dx_ledgpio;
	memcpy(&ledman_std, &ac6310dx_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ac6310dx_ledman_def, sizeof(ledman_def));
}

static void connectit4_ledman_init(void)
{
	led_nr = CONNECTIT4_LED_NR;
	leds_state = CONNECTIT4_LED_MASK;
	ledgpio = connectit4_ledgpio;
	memcpy(&ledman_std, &connectit4_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &connectit4_ledman_def, sizeof(ledman_def));
}

static void connectit_mini_ledman_init(void)
{
	led_nr = CONNECTIT_MINI_LED_NR;
	leds_state = CONNECTIT_MINI_LED_MASK;
	ledgpio = connectit_mini_ledgpio;
	memcpy(&ledman_std, &connectit_mini_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &connectit_mini_ledman_def, sizeof(ledman_def));
}

static void ix20_ledman_init(void)
{
	led_nr = IX20_LED_NR;
	leds_state = IX20_LED_MASK;
	ledgpio = ix20_ledgpio;
	memcpy(&ledman_std, &ix20_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ix20_ledman_def, sizeof(ledman_def));
}

static void ix10_ledman_init(void)
{
	led_nr = IX10_LED_NR;
	leds_state = IX10_LED_MASK;
	ledgpio = ix10_ledgpio;
	memcpy(&ledman_std, &ix10_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ix10_ledman_def, sizeof(ledman_def));
}

static void ix15_ledman_init(void)
{
	led_nr = IX15_LED_NR;
	leds_state = IX15_LED_MASK;
	ledgpio = ix15_ledgpio;
	memcpy(&ledman_std, &ix15_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ix15_ledman_def, sizeof(ledman_def));
}

static void ix30_ledman_init(void)
{
	leds_use_wq = true;
	leds_bits = IX30_LED_MASK;

	led_nr = IX30_LED_NR;
	leds_state = IX30_LED_MASK;
	ledgpio = ix30_ledgpio;
	memcpy(&ledman_std, &ix30_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ix30_ledman_def, sizeof(ledman_def));
}

static void ledman_wq_leds_init(void)
{
	int i, rc;

	leds_deferred_setup = 0;
	for (i = 0; i < led_nr; i++) {
		if (ledgpio[i].flags & F_INITED)
			continue;

		rc = gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		if (rc < 0) {
			leds_deferred_setup++;
			continue;
		}
		gpio_direction_output(ledgpio[i].gpio, 1);
		ledgpio[i].flags |= F_INITED;
	}
}

static void ledman_work(struct work_struct *work)
{
	unsigned int i, changed;
	unsigned int b, bits;

	if (leds_deferred_setup)
		ledman_wq_leds_init();

	/*
	 * Get new state and mark that we have processed that.
	 * This ordering avoids need for preempt protection.
	 */
	bits = leds_bits;
	leds_scheduled = 0;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < led_nr; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value_cansleep(ledgpio[i].gpio,
							(bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static void ledman_set(unsigned long bits)
{
	unsigned int i, b;
	unsigned int changed;

	if (leds_use_wq) {
		leds_bits = bits;
		if (!leds_scheduled) {
			leds_scheduled++;
			schedule_work(&leds_work);
		}
		return;
	}

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < led_nr; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_setup_erase(unsigned int gpio)
{
	int irq;

	gpio_request(gpio, "Erase");
	gpio_direction_input(gpio);
	irq = gpio_to_irq(gpio);
	if (request_irq(irq, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", irq);
	else
		pr_info("registered ERASE switch on IRQ%d\n", irq);
}

static void ledman_initarch(void)
{
	int i;

	if (of_machine_is_compatible("digi,imx6ulix14")) {
		ledman_setup_erase(135);
		return;
	}

	leds_use_wq = false;

	if (of_machine_is_compatible("accelerated,6310dx")) {
		ac6310dx_ledman_init();
		ledman_setup_erase(AC6310DX_GPIO_ERASE);
	} else if (of_machine_is_compatible("digi,ix10")) {
		ix10_ledman_init();
	} else if (of_machine_is_compatible("digi,ix15")) {
		ix15_ledman_init();
	} else if (of_machine_is_compatible("digi,ix20")) {
		ix20_ledman_init();
	} else if (of_machine_is_compatible("digi,ix30")) {
		ix30_ledman_init();
	} else if (of_machine_is_compatible("digi,connectit-mini")) {
		connectit_mini_ledman_init();
	} else if (of_machine_is_compatible("digi,ex12")) {
		connectit_mini_ledman_init();
	} else if (of_machine_is_compatible("digi,connectit4")) {
		connectit4_ledman_init();
	} else {
		return;
	}

	if (leds_use_wq) {
		INIT_WORK(&leds_work, ledman_work);
		ledman_wq_leds_init();
	} else {
		for (i = 0; i < led_nr; i++) {
			gpio_request(ledgpio[i].gpio, ledgpio[i].name);
			gpio_direction_output(ledgpio[i].gpio, 1);
		}
	}
	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_SOC_IMX6UL */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_ARCH_STM32) /* Digi/CCMP13DK and Digi/IoT-Gateway Small/Large */
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char	*name;
	int	gpio;
	int flags;
};

#define F_INITED		0x1
#define F_ACTIVE_HIGH		0x2

#define CCMP13DK_LED_ULED	0x1
#define CCMP13DK_LED_USER1	0x2
#define CCMP13DK_LED_USER2	0x4
#define CCMP13DK_LED_MASK	0x7
#define CCMP13DK_LED_NR		3

#define GPIO_ULED	96 /* PG0 */
#define GPIO_USER1	13 /* PA13 */
#define GPIO_USER2	14 /* PA14 */

static struct ledgpiomap ccmp13dk_ledgpio[] = {
	{ .name = "ULED",  .gpio = 96, .flags = F_ACTIVE_HIGH, }, /* PG0 */
	{ .name = "USER1", .gpio = 13, }, /* PA13 */
	{ .name = "USER2", .gpio = 14, }, /* PA14 */
};
static ledmap_t ccmp13dk_ledman_std = {
	[LEDMAN_ALL]       = CCMP13DK_LED_MASK,
	[LEDMAN_POWER]     = CCMP13DK_LED_ULED,
	[LEDMAN_ETH]       = CCMP13DK_LED_USER1,
	[LEDMAN_XBEE1]     = CCMP13DK_LED_USER2,
};
static leddef_t ccmp13dk_ledman_def = {
	[LEDS_ON] = CCMP13DK_LED_ULED,
};

static unsigned int led_nr;
static unsigned int leds_state;
static struct ledgpiomap *ledgpio;
static ledmap_t ledman_std;
static leddef_t ledman_def;

static void ccmp13dk_ledman_init(void)
{
	led_nr = CCMP13DK_LED_NR;
	leds_state = CCMP13DK_LED_MASK;
	ledgpio = ccmp13dk_ledgpio;
	memcpy(&ledman_std, &ccmp13dk_ledman_std, sizeof(ledman_std));
	memcpy(&ledman_def, &ccmp13dk_ledman_def, sizeof(ledman_def));
}

#if 0
static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}
#endif

static void
ledman_set(unsigned long bits)
{
	unsigned int i, b;
	const unsigned int changed = bits ^ leds_state;

	if (changed) {
		for (i = 0; i < led_nr; i++) {
			b = 0x1 << i;
			if (changed & b) {
				// If we're turning on an LED, we set the I/O low.
				int value = (bits & b) ? 0 : 1;
				// Unless the LED is wired as active-high...
				if (ledgpio[i].flags & F_ACTIVE_HIGH)
					value = !value;
				gpio_set_value(ledgpio[i].gpio, value);
			}
		}
		leds_state = bits;
	}
}

static void
ledman_initarch(void)
{
	int i;
	if (of_machine_is_compatible("digi,ccmp13-dvk") ||
            of_machine_is_compatible("digi,iot-gateway-large") ||
            of_machine_is_compatible("digi,iot-gateway-small"))
        {
		ccmp13dk_ledman_init();
	} else {
		return;
	}

	for (i = 0; i < led_nr; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(
			ledgpio[i].gpio,
			(ledgpio[i].flags & F_ACTIVE_HIGH) ? 0 : 1);
	}
	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_ARCH_STM32 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_DTB_MT7621_EX15)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

struct ledgpiomap {
	char		*name;
	unsigned int	gpio;
	unsigned int	flags;
};

#define	F_INITED	0x1

#define LED_RED		0x1
#define LED_BLUE	0x2
#define LED_GREEN	0x4
#define LED_RSS1	0x8
#define LED_RSS2	0x10
#define LED_RSS3	0x20
#define LED_RSS4	0x40
#define LED_RSS5	0x80
#define LED_SIM1	0x100
#define LED_SIM2	0x200
#define LED_ETH0	0x400
#define LED_ETH1	0x800
#define LED_MASK	0xfff
#define LED_NR		12

static struct ledgpiomap ledgpio[] = {
	{ .name = "RED", .gpio = 608, },
	{ .name = "BLUE", .gpio = 609, },
	{ .name = "GREEN", .gpio = 610, },
	{ .name = "RSS1", .gpio = 611, },
	{ .name = "RSS2", .gpio = 612, },
	{ .name = "RSS3", .gpio = 613, },
	{ .name = "RSS4", .gpio = 614, },
	{ .name = "RSS5", .gpio = 615, },
	{ .name = "SIM1", .gpio = 544, },
	{ .name = "SIM2", .gpio = 543, },
	{ .name = "ETH0", .gpio = 534, },
	{ .name = "ETH1", .gpio = 535, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,

	[LEDMAN_LAN1_LINK] = LED_ETH0,
	[LEDMAN_LAN2_LINK] = LED_ETH1,

	[LEDMAN_NVRAM_1]   = LED_RED | LED_RSS1 | LED_RSS2 | LED_RSS3 |
			     LED_RSS4 | LED_RSS5 | LED_SIM1 | LED_SIM2 |
			     LED_ETH0 | LED_ETH1,
	[LEDMAN_NVRAM_2]   = LED_RED | LED_GREEN,
	[LEDMAN_NVRAM_ALL] = LED_RED | LED_GREEN | LED_BLUE |
			     LED_RSS1 | LED_RSS2 | LED_RSS3 | LED_RSS4 |
			     LED_RSS5 | LED_SIM1 | LED_SIM2 | LED_ETH0 |
			     LED_ETH1,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_ONLINE]    = LED_BLUE,
	[LEDMAN_COM]       = LED_RED,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,

	[LEDMAN_SIM1]      = LED_SIM1,
	[LEDMAN_SIM2]      = LED_SIM2,
};

static leddef_t ledman_def = {
	[LEDS_ON]	   = LED_GREEN | LED_RED,
};

static unsigned int leds_state = LED_MASK;
static unsigned int leds_bits = LED_MASK;
static struct work_struct leds_work;
static int leds_scheduled;
static int leds_deferedsetup;
static int leds_irq;

static void ledman_set(unsigned long bits)
{
	leds_bits = bits;
	if (!leds_scheduled) {
		leds_scheduled++;
		schedule_work(&leds_work);
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static void ledman_initgpio(void)
{
	int i, rc;

	leds_deferedsetup = 0;
	for (i = 0; i < LED_NR; i++) {
		if (ledgpio[i].flags & F_INITED)
			continue;

		rc = gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		if (rc < 0) {
			leds_deferedsetup++;
			continue;
		}
		gpio_direction_output(ledgpio[i].gpio, 1);
		ledgpio[i].flags |= F_INITED;
	}
}

static void ledman_work(struct work_struct *work)
{
	unsigned int i, changed;
	unsigned int b, bits;

	if (leds_deferedsetup) {
		ledman_initgpio();
		/* Force fresh set of all LEDs */
		leds_state = ~leds_bits;
	}

	/*
	 * Get new state and mark that we have processed that.
	 * This ordering avoids need for preempt protection.
	 */
	bits = leds_bits;
	leds_scheduled = 0;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value_cansleep(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static void ledman_initarch(void)
{
	INIT_WORK(&leds_work, ledman_work);
	ledman_initgpio();
	ledman_set(0);
}

/****************************************************************************/
#endif /* CONFIG_DTB_MT7621_EX15 */
/****************************************************************************/
/****************************************************************************/
#if defined(CONFIG_GPIO_AMDFCH)
/****************************************************************************/
#define LEDMAN_PLATFORM_DEFINED

#include <linux/gpio.h>

#define LED_RED		0x1
#define LED_BLUE	0x2
#define LED_GREEN	0x4
#define LED_3		0x8
#define LED_4		0x10
#define LED_5		0x20
#define LED_6		0x40
#define LED_7		0x80
#define LED_8		0x100
#define LED_9		0x200
#define LED_10		0x400
#define LED_11		0x800
#define LED_RSS1	0x1000
#define LED_RSS2	0x2000
#define LED_RSS3	0x4000
#define LED_RSS4	0x8000
#define LED_RSS5	0x10000
#define LED_MASK	0x0001ffff
#define LED_NR		17

#define	GPIO_ERASE	33

struct ledgpiomap {
	char	*name;
	int	gpio;
};

static struct ledgpiomap ledgpio[] = {
	{ .name = "Red", .gpio = 73, },
	{ .name = "Blue", .gpio = 74, },
	{ .name = "Green", .gpio = 75, },
	{ .name = "LED3", .gpio = 76, },
	{ .name = "LED4", .gpio = 77, },
	{ .name = "LED5", .gpio = 78, },
	{ .name = "LED6", .gpio = 79, },
	{ .name = "LED7", .gpio = 80, },
	{ .name = "LED8", .gpio = 45, },
	{ .name = "LED9", .gpio = 49, },
	{ .name = "LED10", .gpio = 50, },
	{ .name = "LED11", .gpio = 51, },

	{ .name = "RSS1", .gpio = 170, },
	{ .name = "RSS2", .gpio = 169, },
	{ .name = "RSS3", .gpio = 168, },
	{ .name = "RSS4", .gpio = 167, },
	{ .name = "RSS5", .gpio = 55, },
};

static ledmap_t ledman_std = {
	[LEDMAN_ALL]       = LED_MASK,
	[LEDMAN_POWER]     = LED_8,
	[LEDMAN_HEARTBEAT] = LED_9,

	[LEDMAN_LAN1_RX]   = LED_10,
	[LEDMAN_LAN1_TX]   = LED_10,
	[LEDMAN_LAN2_RX]   = LED_11,
	[LEDMAN_LAN2_TX]   = LED_11,
	[LEDMAN_LAN3_RX]   = LED_3,
	[LEDMAN_LAN3_TX]   = LED_3,
	[LEDMAN_LAN4_RX]   = LED_4,
	[LEDMAN_LAN4_TX]   = LED_4,
	[LEDMAN_USB1_RX]   = LED_5,
	[LEDMAN_USB1_TX]   = LED_5,
	[LEDMAN_USB2_RX]   = LED_5,
	[LEDMAN_USB2_TX]   = LED_5,
	[LEDMAN_COM1_RX]   = LED_6,
	[LEDMAN_COM1_TX]   = LED_6,
	[LEDMAN_VM]        = LED_7,

	[LEDMAN_ETH]       = LED_GREEN,
	[LEDMAN_ONLINE]    = LED_BLUE,
	[LEDMAN_COM]       = LED_RED,

	[LEDMAN_NVRAM_1]   = LED_3 | LED_4 | LED_5 | LED_6 | LED_7 | LED_8 |
			     LED_9 | LED_10 | LED_10 | LED_11 | LED_RED |
			     LED_RSS1 | LED_RSS2 | LED_RSS3 | LED_RSS4 |
			     LED_RSS5,
	[LEDMAN_NVRAM_2]   = LED_RED | LED_GREEN,
	[LEDMAN_NVRAM_ALL] = LED_RED | LED_GREEN | LED_BLUE |
				 LED_3 | LED_4 | LED_5 | LED_6 | LED_7 | LED_8 |
			     LED_9 | LED_10 | LED_10 | LED_11 |
			     LED_RSS1 | LED_RSS2 | LED_RSS3 | LED_RSS4 | LED_RSS5,

	[LEDMAN_RSS1]      = LED_RSS1,
	[LEDMAN_RSS2]      = LED_RSS2,
	[LEDMAN_RSS3]      = LED_RSS3,
	[LEDMAN_RSS4]      = LED_RSS4,
	[LEDMAN_RSS5]      = LED_RSS5,
};

static leddef_t ledman_def = {
	[LEDS_FLASH] = LED_9,
	[LEDS_ON] = LED_8,
};

static unsigned int leds_state = LED_MASK;

static void
ledman_set(unsigned long bits)
{
	unsigned int i, b;
	unsigned int changed;

	changed = bits ^ leds_state;
	if (changed) {
		for (i = 0; i < LED_NR; i++) {
			b = 0x1 << i;
			if (changed & b)
				gpio_set_value(ledgpio[i].gpio, (bits & b) ? 0 : 1);
		}
		leds_state = bits;
	}
}

static irqreturn_t ledman_interrupt(int irq, void *dev_id)
{
	ledman_signalreset();
	return IRQ_HANDLED;
}

static int ledman_button;
static struct timer_list ledman_timer;

static void ledman_buttonpoll(struct timer_list *t)
{
	if (gpio_get_value(GPIO_ERASE) == 0) {
		if (ledman_button == 0) {
			pr_info("reset button pushed!\n");
			ledman_interrupt(GPIO_ERASE, NULL);
		}
		ledman_button = 1;
	} else {
		ledman_button = 0;
	}

	/* Re-arm timer interrupt. */
	mod_timer(&ledman_timer, jiffies + HZ/25);
}

static void
ledman_initarch(void)
{
	int i;

	for (i = 0; i < LED_NR; i++) {
		gpio_request(ledgpio[i].gpio, ledgpio[i].name);
		gpio_direction_output(ledgpio[i].gpio, 1);
	}
	ledman_set(0);

	gpio_request(GPIO_ERASE, "Erase");
	gpio_direction_input(GPIO_ERASE);
#if 1
	timer_setup(&ledman_timer, ledman_buttonpoll, 0);
	mod_timer(&ledman_timer, jiffies + HZ/25);
#endif
#if 0
	i = gpio_to_irq(GPIO_ERASE);
	if (request_irq(i, ledman_interrupt, IRQF_TRIGGER_FALLING, "Erase", NULL))
		pr_err("failed to register IRQ%d for ERASE switch\n", i);
	else
		pr_info("registered ERASE switch on IRQ%d\n", i);
#endif
}

/****************************************************************************/
#endif /* CONFIG_GPIO_AMDFCH */
/****************************************************************************/
#if defined(GPIO_BUZZER)
/****************************************************************************/

static struct timer_list buzzer_timer;
static int buzzer_counter = 0;

static void buzzer_poll(struct timer_list *t)
{
	if (gpio_get_value(GPIO_ERASE) == 1) {
		buzzer_counter = 0;
		ledman_bits(LEDMAN_CMD_ALT_OFF|LEDMAN_CMD_ALTBIT, ~0);
		gpio_set_value(GPIO_BUZZER, 0);
		return;
	}
	buzzer_counter += HZ / 50;
	ledman_bits(LEDMAN_CMD_ALT_ON, ~0);
	ledman_bits(LEDMAN_CMD_ON|LEDMAN_CMD_ALTBIT,
		buzzer_progress[(buzzer_counter / HZ) % sizeof(buzzer_progress)]);
	ledman_bits(LEDMAN_CMD_OFF|LEDMAN_CMD_ALTBIT,
		~buzzer_progress[(buzzer_counter / HZ) % sizeof(buzzer_progress)]);
	gpio_set_value(GPIO_BUZZER, ((buzzer_counter%HZ) > (HZ/2)) ? 0 : 1 );
	if (buzzer_counter >= HZ * 5) {
		gpio_set_value(GPIO_BUZZER, 1);
		ledman_signalreset();
	} else {
		mod_timer(&buzzer_timer, jiffies + HZ/50);
	}
}

static void buzzer_start(void)
{
	buzzer_counter = 0;
	mod_timer(&buzzer_timer, jiffies + HZ/50);
}

static void buzzer_init(void)
{
	timer_setup(&buzzer_timer, buzzer_poll, 0);
	add_timer(&buzzer_timer);
	pr_info("BUZZER initialized for ERASE switch\n");
}

/****************************************************************************/
#endif /* GPIO_BUZZER */
/****************************************************************************/
#if !defined(LEDMAN_PLATFORM_DEFINED)
/****************************************************************************/
/* Empty stubs for platforms with no specific implementation. */

static ledmap_t	ledman_std = {};
static leddef_t	ledman_def = {};

static void
ledman_set(unsigned long bits)
{
}

static void
ledman_initarch(void)
{
}
/****************************************************************************/
#endif /* !defined(LEDMAN_PLATFORM_DEFINED) */
/****************************************************************************/

static struct ledmode led_mode[] = {

#ifdef ENTERASYS /* first in the list is the default */
	{ "enterasys", ledman_std, ledman_def, LT },
#endif
#if defined(CONFIG_NETtel) && defined(CONFIG_M5307)
	/*
	 * by default the first entry is used.  You can select the old-style
	 * LED patterns for acient boards with the command line parameter:
	 *
	 *      ledman=old
	 */
	{ "new", nettel_new, nettel_def, LT },
	{ "old", nettel_old, nettel_def, LT },
#else
	{ "std", ledman_std, ledman_def, LT },
#endif

	{ "",  NULL, NULL, HZ, }
};

/****************************************************************************/

/*
 *	Fast path LED setting function. The most common call (by far) is
 *	the SET command. Most drivers call this _alot_. So this function
 *	implements just that, and does it fast.
 */

void
__ledman_cmdset(unsigned long led)
{
	if (lmp)
		leds_set[0] |= lmp->map[led];
}

/****************************************************************************/

/*
 *	cmd - from ledman.h
 *	led - led code from ledman.h
 *
 *	check parameters and then call
 */

int
ledman_cmd(int cmd, unsigned long led)
{
	int			i;

	switch (cmd & ~LEDMAN_CMD_ALTBIT) {
	case LEDMAN_CMD_SET:
	case LEDMAN_CMD_ON:
	case LEDMAN_CMD_OFF:
	case LEDMAN_CMD_FLASH:
	case LEDMAN_CMD_RESET:
	case LEDMAN_CMD_ALT_ON:
	case LEDMAN_CMD_ALT_OFF:
		break;
	case LEDMAN_CMD_STARTTIMER:
		ledman_starttimer();
		return 0;
	case LEDMAN_CMD_KILLTIMER:
		ledman_killtimer();
		return 0;
	case LEDMAN_CMD_MODE:
		for (i = 0; led_mode[i].name[0]; i++)
			if (strcmp((char *) led, led_mode[i].name) == 0) {
				lmp = led_mode + i;
				if (initted)
					ledman_cmd(LEDMAN_CMD_RESET, LEDMAN_ALL);
				return 0;
			}
		return -EINVAL;
	default:
		return -EINVAL;
	}

	if (led < 0 || led >= LEDMAN_MAX)
		return -EINVAL;

	if (lmp)
		ledman_bits(cmd, lmp->map[led]);
	return 0;
}

/****************************************************************************/

static const struct file_operations ledman_fops = {
	.unlocked_ioctl = ledman_ioctl,
};

/****************************************************************************/

INIT_RET_TYPE ledman_init(void)
{
	int expires;
	pr_info("Copyright (C) SnapGear, 2000-2010.\n");

	if (register_chrdev(LEDMAN_MAJOR, "nled",  &ledman_fops) < 0) {
		pr_err("ledman_init() can't get Major %d\n", LEDMAN_MAJOR);
		return -EBUSY;
	}

	lmp = led_mode;

	ledman_initarch();

/*
 *	set the LEDs up correctly at boot
 */
	ledman_cmd(LEDMAN_CMD_RESET, LEDMAN_ALL);
/*
 *	start the timer
 */
	timer_setup(&ledman_timerlist, ledman_poll, 0);
	expires = jiffies + lmp->jiffies;
	mod_timer(&ledman_timerlist, expires);

#if LINUX_VERSION_CODE < 0x020100
	register_symtab(&ledman_syms);
#endif

	initted = 1;
	return 0;
}

Module_init(ledman_init);
