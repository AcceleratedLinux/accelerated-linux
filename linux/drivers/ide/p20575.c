/*
 * linux/drivers/ide/pci/p20575.c
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/ioport.h>
#include <asm/io.h>

#if 0
#define	PRINTK(x...)	printk(x)
#else
#define	PRINTK(x...)	do { } while (0)
#endif

#if 0
static void hexdump(void *v, unsigned int len)
{
	unsigned int p = (unsigned int) v;
	int i;

	for (i = 0; (i < (len/4)); i++) {
		if ((i % 4) == 0) printk("%08x:  ", (int)p);
		printk("%08x ", readl(p));
		p += 4;
		if (((i+1) % 4) == 0) printk("\n");
	}
	if ((i % 4) != 0) printk("\n");
}
#endif

void *p20575_iomap;

static u8 p20575_inb(unsigned long port)
{
	u8 v;
	PRINTK("p20575_inb(port=%x)", (int)port);
	v = readl(p20575_iomap+port);
	PRINTK("=%x\n", (int)v);
	return v;
}

static void p20575_outb(u8 val, unsigned long port)
{
	PRINTK("p20575_outb(val=%x,port=%x)\n", (int)val, (int)port);
	writel(val, p20575_iomap+port);
}

static void p20575_outsw(unsigned long port, void *buf, u32 len)
{
	u16 w, *wp = buf;
	PRINTK("p20575_outsw(port=%x,buf=%x,len=%x)\n", (int)port, (int)buf, len);
	while (len--) {
		w = *wp++;
		w = (w << 8) | (w >> 8);
		writel(w, p20575_iomap+port);
	}
}

static void p20575_output_data(ide_drive_t *drive, struct ide_cmd *cmd, void *buf, unsigned int len)
{
	p20575_outsw(drive->hwif->io_ports.data_addr, buf, len/2);
}

static void p20575_insw(unsigned long port, void *buf, u32 len)
{
	u16 w, *wp = buf;
	PRINTK("p20575_insw(port=%x,buf=%x,len=%x)\n", (int)port, (int)buf, len);
	while (len--) {
		w = readl(p20575_iomap+port);
		*wp++ = (w << 8) | (w >> 8);
	}
}

static void p20575_input_data(ide_drive_t *drive, struct ide_cmd *cmd, void *buf, unsigned int len)
{
	p20575_insw(drive->hwif->io_ports.data_addr, buf, len/2);
}

static u8 p20575_read_status(struct hwif_s *hwif)
{
	return p20575_inb(hwif->io_ports.status_addr);
}

static u8 p20575_read_altstatus(struct hwif_s *hwif)
{
	return p20575_inb(hwif->io_ports.ctl_addr);
}

static void p20575_exec_command(struct hwif_s *hwif, u8 cmd)
{
	p20575_outb(cmd, hwif->io_ports.command_addr);
}

static void p20575_dev_select(ide_drive_t *drive)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 select = drive->select | ATA_DEVICE_OBS;
	p20575_outb(select, hwif->io_ports.device_addr);
}

static void p20575_write_devctl(ide_hwif_t *hwif, u8 ctl)
{
	p20575_outb(ctl, hwif->io_ports.ctl_addr);
}

static void p20575_tf_load(ide_drive_t *drive, struct ide_taskfile *tf, u8 valid)
{
	ide_hwif_t *hwif = drive->hwif;
	struct ide_io_ports *io_ports = &hwif->io_ports;

	if (valid & IDE_VALID_FEATURE)
		p20575_outb(tf->feature, io_ports->feature_addr);
	if (valid & IDE_VALID_NSECT)
		p20575_outb(tf->nsect, io_ports->nsect_addr);
	if (valid & IDE_VALID_LBAL)
		p20575_outb(tf->lbal, io_ports->lbal_addr);
	if (valid & IDE_VALID_LBAM)
		p20575_outb(tf->lbam, io_ports->lbam_addr);
	if (valid & IDE_VALID_LBAH)
		p20575_outb(tf->lbah, io_ports->lbah_addr);
	if (valid & IDE_VALID_DEVICE)
		p20575_outb(tf->device, io_ports->device_addr);
}

static void p20575_tf_read(ide_drive_t *drive, struct ide_taskfile *tf, u8 valid)
{
	struct ide_io_ports *io_ports = &drive->hwif->io_ports;

	if (valid & IDE_VALID_ERROR)
		tf->error  = p20575_inb(io_ports->feature_addr);
	if (valid & IDE_VALID_NSECT)
		tf->nsect  = p20575_inb(io_ports->nsect_addr);
	if (valid & IDE_VALID_LBAL)
		tf->lbal   = p20575_inb(io_ports->lbal_addr);
	if (valid & IDE_VALID_LBAM)
		tf->lbam   = p20575_inb(io_ports->lbam_addr);
	if (valid & IDE_VALID_LBAH)
		tf->lbah   = p20575_inb(io_ports->lbah_addr);
	if (valid & IDE_VALID_DEVICE)
		tf->device = p20575_inb(io_ports->device_addr);
}

static void p20575_early_clear_irq(ide_drive_t *drive)
{
	unsigned int v;

	PRINTK("%s(%d): p20575_early_clear_irq()\n", __FILE__, __LINE__);
	v = readl(p20575_iomap+0x40);
	writel(v, p20575_iomap+0x40);
	writel(1, p20575_iomap+(2*4));
}

static void p20575_init_iops(ide_hwif_t *hwif)
{
	PRINTK("%s(%d): p20575_init_iops()\n", __FILE__, __LINE__);

	memset(&hwif->io_ports, 0, sizeof(hwif->io_ports));
	hwif->io_ports.data_addr = 0x300;
	hwif->io_ports.error_addr = 0x304;
	hwif->io_ports.nsect_addr = 0x308;
	hwif->io_ports.lbal_addr = 0x30c;
	hwif->io_ports.lbam_addr = 0x310;
	hwif->io_ports.lbah_addr = 0x314;
	hwif->io_ports.device_addr = 0x318;
	hwif->io_ports.status_addr = 0x31C;
	hwif->io_ports.command_addr = 0x31C;
	hwif->io_ports.ctl_addr = 0x338;
}

static int p20575_init_chipset(struct pci_dev *dev)
{
	PRINTK("%s(%d): p20575_init_chipset()\n", __FILE__, __LINE__);

	p20575_iomap = ioremap(0x48060000, 0x1000);
	PRINTK("%s(%d): iomap=%x\n", __FILE__, __LINE__, (int)p20575_iomap);

	writel(2, p20575_iomap+0x360);
	writel(1, p20575_iomap+(2*4));
	return 0;
}

static void p20575_init_hwif(ide_hwif_t *hwif)
{
	PRINTK("%s(%d): p20575_init_hwif()\n", __FILE__, __LINE__);
}

static const struct ide_tp_ops p20575_tp_ops = {
	.exec_command	= p20575_exec_command,
	.read_status	= p20575_read_status,
	.read_altstatus	= p20575_read_altstatus,
	.write_devctl	= p20575_write_devctl,
	.dev_select	= p20575_dev_select,
	.tf_load	= p20575_tf_load,
	.tf_read	= p20575_tf_read,
	.input_data	= p20575_input_data,
	.output_data	= p20575_output_data,
};

static const struct ide_port_ops p20575_port_ops = {
	.early_clear_irq= p20575_early_clear_irq,
};

static const struct ide_port_info p20575_port_info = {
	.name		= "P20575",
	.chipset	= ide_pci,
	.port_ops	= &p20575_port_ops,
	.init_iops	= p20575_init_iops,
	.init_hwif	= p20575_init_hwif,
	.init_chipset	= p20575_init_chipset,
	.tp_ops		= &p20575_tp_ops,
	.host_flags	= IDE_HFLAG_NO_DMA | IDE_HFLAG_ISA_PORTS | IDE_HFLAG_SINGLE,
};

static int p20575_init_one(struct pci_dev *dev, const struct pci_device_id *id)
{
	return ide_pci_init_one(dev, &p20575_port_info, NULL);
}

static struct pci_device_id p20575_pci_tbl[] = {
	{ PCI_VENDOR_ID_PROMISE, 0x3575, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{ 0, },
};

MODULE_DEVICE_TABLE(pci, p20575_pci_tbl);

static struct pci_driver driver = {
	.name		= "P20575-IDE",
	.id_table	= p20575_pci_tbl,
	.probe		= p20575_init_one,
};

static int p20575_ide_init(void)
{
	PRINTK("%s(%d): p20575_ide_init()\n", __FILE__, __LINE__);
	return ide_pci_register_driver(&driver);
}

module_init(p20575_ide_init);

MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_DESCRIPTION("PCI driver module for PATA channel of Promise 20575");
MODULE_LICENSE("GPL");
