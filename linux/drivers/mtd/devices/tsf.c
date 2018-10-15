/*
 * tsf.c -- interface to the SPI flash on the Tolapai platform
 *
 * (C) Copyright 2009,  Greg Ungerer <gerg@snapgear.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/mutex.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

/*
 *	SPI registers of the EP80579. These are the offsets of these
 *	registers into the SPI region of the RBCA.
 */
#define	SPI_STATUS	0x20		/* Status register */
#define	SPI_CONTROL	0x22		/* Control register */
#define	SPI_ADDRESS	0x24		/* Address register */
#define	SPI_DATA0	0x28		/* Data[0] register */
#define	SPI_BBAR	0x70		/* BIOS base address */
#define	SPI_PREOP	0x74		/* Prefix OPcode configuration */
#define	SPI_OPTYPE	0x76		/* OPcode type */
#define	SPI_OPMENU	0x78		/* OPcode menu configuration */
#define	SPI_PBRO	0x80		/* Protected BIOS range */

/*
 *	Define the useful bits of the SPI Control register.
 */
#define	CMDC_GO		0x0002		/* Start SPI command */
#define	CMDC_CMDSEQ	0x0004		/* Execute command sequence */
#define	CMDC_SPOP0	0x0000		/* Sequence use entry 0 OP */
#define	CMDC_SPOP1	0x0008		/* Sequence uses entry 1 OP */
#define	CMDC_OPMASK	0x0070		/* OP code to execute */
#define	CMDC_CNTMASK	0x3f00		/* Mask for byte count */
#define	CMDC_DATA	0x4000		/* Data with this command */

#define	CMDC_OP		4		/* OP command shift bits */
#define	CMDC_CNT	8		/* Count of byets shift bits */

/*
 *	Define the useful bits of the Status register.
 */
#define	STC_SCIP	0x0001		/* SPI Cycle In Progress */
#define	STC_CDS		0x0004		/* Cycle Done Status */
#define	STC_BAS		0x0008		/* Blocked Access Status */
#define	STC_SCL		0x8000		/* SPI Configuration Lock-Down */

/*
 *	SPI Flash Commands. The actual commands tha tthe flash device
 *	understands.
 */
#define	CMD_WRSR	0x1		/* Write Status Register */
#define	CMD_WRITE	0x2		/* Write memory */
#define	CMD_READ	0x3		/* Read Memory */
#define	CMD_WRDI	0x4		/* Write Disable */
#define	CMD_RDSR	0x5		/* Read Status */
#define	CMD_WREN	0x6		/* Write Enable */
#define	CMD_ERASE4K	0x20		/* Erase 4K Block */
#define	CMD_EWSR	0x50		/* Enable Write Status */
#define	CMD_RDID	0x90		/* Read ID */
#define	CMD_AAIW	0xad		/* Byte Write with Auto-Increment */

/*
 *	The SPI/flash controller of the EP80597 has a table of commands
 *	that you use by index. Define the table offsets of commands that
 *	we will use.
 */
#define	OP_RDSR		0
#define	OP_READ		1
#define	OP_RDID		2
#define	OP_WREN		3
#define	OP_ERASE4K	4
#define	OP_WRITE	5
#define	OP_WRDI		6
#define	OP_WRSR		7

#define	OPTYP_READ	0x0		/* Read operation, no address */
#define	OPTYP_WRITE	0x1		/* Write operation, no address */
#define	OPTYP_READA	0x2		/* Read operation, with address */
#define	OPTYP_WRITEA	0x3		/* Write operation, with address */


/*
 *	For simplicity sake we only use 4k erase blocks. (The flash chips
 *	we use actuallyt allow for larger single erase commands, but we
 *	don't currently use them).
 */
#define	ERASESIZE	(4*1024)	/* Use 4k erase blocks */

/*
 *	Based on the index of commands define a struct to hold the
 *	commands, offsets and read/write types needed for init.
 */
struct opentry {
	u8	cmd;
	u8	typ;
};

#define	MAXTABLE	8

static struct opentry tsf_optable[MAXTABLE] = {
	[OP_RDSR] =	{ CMD_RDSR, OPTYP_READ, },
	[OP_READ] =	{ CMD_READ, OPTYP_READA, },
	[OP_RDID] =	{ CMD_RDID, OPTYP_READA, },
	[OP_WREN] =	{ CMD_WREN, OPTYP_WRITE, },
	[OP_ERASE4K] =	{ CMD_ERASE4K, OPTYP_WRITEA, },
	[OP_WRITE] =	{ CMD_WRITE, OPTYP_WRITEA, },
	[OP_WRDI] =	{ CMD_WRDI, OPTYP_WRITE, },
	[OP_WRSR] =	{ CMD_WRSR, OPTYP_WRITE, },
};

static u32 tsf_rcba;
static void __iomem *tsf_map;
static DECLARE_MUTEX(tsf_mutex);

#if 0
static void dumpregs(void)
{
	printk("SPI FLASH registers:\n");
	printk(" 0x3020 = %04x\n", readw(tsf_map+0x20));
	printk(" 0x3022 = %04x\n", readw(tsf_map+0x22));
	printk(" 0x3024 = %08x\n", readl(tsf_map+0x24));
	printk(" 0x3028 = %08x,%08x\n", readl(tsf_map+0x28), readl(tsf_map+0x2c));
	printk(" 0x3030 = %08x,%08x\n", readl(tsf_map+0x30), readl(tsf_map+0x34));
	printk(" 0x3038 = %08x,%08x\n", readl(tsf_map+0x38), readl(tsf_map+0x3c));
	printk(" 0x3040 = %08x,%08x\n", readl(tsf_map+0x40), readl(tsf_map+0x44));
	printk(" 0x3048 = %08x,%08x\n", readl(tsf_map+0x48), readl(tsf_map+0x4c));
	printk(" 0x3050 = %08x,%08x\n", readl(tsf_map+0x50), readl(tsf_map+0x54));
	printk(" 0x3058 = %08x,%08x\n", readl(tsf_map+0x58), readl(tsf_map+0x5c));
	printk(" 0x3060 = %08x,%08x\n", readl(tsf_map+0x60), readl(tsf_map+0x64));
	printk(" 0x3070 = %08x\n", readl(tsf_map+0x70));
	printk(" 0x3074 = %04x\n", readw(tsf_map+0x74));
	printk(" 0x3076 = %04x\n", readw(tsf_map+0x76));
	printk(" 0x3078 = %08x,%08x\n", readl(tsf_map+0x78), readl(tsf_map+0x7c));
	printk(" 0x3080 = %08x\n", readl(tsf_map+0x80));
}
#endif

static void __init tsf_initopmenu(void)
{
	int i;
	u16 optyp = 0;

	for (i = 0; (i < MAXTABLE); i++) {
		writeb(tsf_optable[i].cmd, tsf_map + SPI_OPMENU + i);
		optyp |= tsf_optable[i].typ << (i * 2);
	}

	writew(optyp, tsf_map + SPI_OPTYPE);
}

static u16 tsf_spiop(u8 op, int cnt)
{
	u16 r = CMDC_GO;
	r |= (op << CMDC_OP);
	if (cnt)
		r |= CMDC_DATA | ((cnt - 1) << CMDC_CNT);
	return r;
}

#define	SPITIMEOUT		100

static int tsf_spiwait(void)
{
	int i;

	for (i = 0; (i < SPITIMEOUT); i++) {
		if ((readw(tsf_map + SPI_STATUS) & STC_SCIP) == 0)
			return 0;
		udelay(1);
	}
	printk("TSF: SPI command to flash timed out?\n");
	return -1;
}

static int tsf_waitcmddone(unsigned int maxt)
{
	unsigned long endj;
	u8 st;

	endj = jiffies + usecs_to_jiffies(maxt);
	for (;;) {
		if (time_is_before_jiffies(endj))
			return -EIO;
		writew(tsf_spiop(OP_RDSR, 1), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0)
			return -EIO;
		st = readb(tsf_map + SPI_DATA0);
		if ((st & 0x1) == 0)
			break;
	}

	return 0;
}

static unsigned int tsf_readid(void)
{
	writel(0x0, tsf_map + SPI_ADDRESS);
	writew(tsf_spiop(OP_RDID, 2), tsf_map + SPI_CONTROL);
	if (tsf_spiwait() < 0)
		return 0xffffffff;
	return readw(tsf_map + SPI_DATA0);
}

static void tsf_enablewrite(void)
{
	writeb(0, tsf_map + SPI_DATA0);
	writew(tsf_spiop(OP_WREN, 0), tsf_map + SPI_CONTROL);
	tsf_spiwait();

	writeb(0, tsf_map + SPI_DATA0);
	writew(tsf_spiop(OP_WRSR, 1), tsf_map + SPI_CONTROL);
	tsf_spiwait();

	writew(tsf_spiop(OP_WRDI, 0), tsf_map + SPI_CONTROL);
	tsf_spiwait();
}

static int tsf_erase(struct mtd_info *mtd, struct erase_info *einfo)
{
	u32 addr, eaddr;
	int rc = 0;

	down(&tsf_mutex);

	eaddr = einfo->addr + einfo->len;
	for (addr = einfo->addr; (addr < eaddr); addr += ERASESIZE) {
		writew(tsf_spiop(OP_WREN, 0), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0) {
			rc = -EIO;
			goto erase_done;
		}
		writel(addr & 0xffffff, tsf_map + SPI_ADDRESS);
		writew(tsf_spiop(OP_ERASE4K, 0), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0) {
			rc = -EIO;
			goto erase_done;
		}
		/* Datasheet says maximum 4k erase is 25ms */
		tsf_waitcmddone(25000);
	}

	if (einfo->callback) {
		einfo->state |= MTD_ERASE_DONE;
		einfo->callback(einfo);
	}

erase_done:
	up(&tsf_mutex);
	return rc;
}

static int tsf_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	unsigned int cnt, rc = 0;
	size_t total = 0;

	down(&tsf_mutex);

	while (len > 0) {
		cnt = (len > 64) ? 64 : len;
		writel(from & 0xffffff, tsf_map + SPI_ADDRESS);
		writew(tsf_spiop(OP_READ, cnt), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0) {
			rc = -EIO;
			goto read_done;
		}
		memcpy_fromio(buf, tsf_map + SPI_DATA0, cnt);
		from += cnt;
		len -= cnt;
		buf += cnt;
		total += cnt;
	}

read_done:
	up(&tsf_mutex);
	*retlen = total;
	return 0;
}

static int tsf_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	size_t total = 0;
	int rc = 0;

	down(&tsf_mutex);

	for (; (len > 0); len--) {
		writew(tsf_spiop(OP_WREN, 0), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0) {
			rc = -EIO;
			goto write_done;
		}

		writel(to & 0xffffff, tsf_map + SPI_ADDRESS);
		writeb(*buf, tsf_map + SPI_DATA0);
		writew(tsf_spiop(OP_WRITE, 1), tsf_map + SPI_CONTROL);
		if (tsf_spiwait() < 0) {
			rc = -EIO;
			goto write_done;
		}

		/* Datasheet says maximum byte write time is 10us */
		/* Hardly seems worth polling status bit for that */
		udelay(10);

		buf++;
		to++;
		total++;
	}

write_done:
	up(&tsf_mutex);
	*retlen = total;
	return rc;
}

static struct mtd_info tsf_mtdinfo = {
	.name =		"TSF",
	.type =		MTD_NORFLASH,
	.flags =	MTD_CAP_NORFLASH | MTD_WRITEABLE,
	.size =		4 * 1024 * 1024,
	.erasesize =	ERASESIZE,
	.writesize =	1,
	.erase =	tsf_erase,
	.read =		tsf_read,
	.write =	tsf_write,
};

static struct mtd_partition tsf_parts[] = {
	{
		.name =	"Boot Loader",
	},
};

static int __init tsf_init(void)
{
	unsigned int id;
	struct pci_dev *dev;
	int rc;
	u8 v;

	dev = pci_get_device(PCI_VENDOR_ID_INTEL, 0x5031, NULL);
	if (dev == NULL) {
		printk("TSF: cannot find RCBA device?\n");
		return -ENODEV;
        }

	pci_read_config_byte(dev, 0xdc, &v);
	if ((v & 0x1) == 0) {
		v |= 0x1;
		pci_write_config_byte(dev, 0xdc, v);
	}

        pci_read_config_dword(dev, 0xf0, &tsf_rcba);
	if (tsf_rcba & 0x1) {
		tsf_rcba |= 0x1;
		pci_write_config_dword(dev, 0xf0, tsf_rcba);
	}
        tsf_rcba &= 0xfffffffe;
        printk("TSF: RCBA mapped address=0x%08x\n", tsf_rcba);

	tsf_map = ioremap(tsf_rcba+0x3000, 0xfff);
	if (tsf_map == NULL) {
		printk("TSF: failed to ioremap(0x%x) RCBA\n", tsf_rcba);
		return -ENOMEM;
	}

	tsf_initopmenu();
	id = tsf_readid();
	printk("TSF: flash present MANUFACTURER=0x%02x DEVICE=0x%02x\n",
		(id & 0xff), (id >> 8) & 0xff);

	tsf_mtdinfo.owner = THIS_MODULE;
	rc = add_mtd_partitions(&tsf_mtdinfo, tsf_parts, ARRAY_SIZE(tsf_parts));
	if (rc) {
		printk("TSF: failed to add partitions?\n");
		return rc;
	}

	/* Clear error conditions */
	writew(0xc, tsf_map + SPI_STATUS);

	tsf_enablewrite();

	init_MUTEX(&tsf_mutex);
	return 0;
}

static void __exit tsf_exit(void)
{
	del_mtd_partitions(&tsf_mtdinfo);
	if (tsf_map)
		iounmap(tsf_map);
}

module_init(tsf_init);
module_exit(tsf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_DESCRIPTION("Intel Tolapai SPI flash driver");
