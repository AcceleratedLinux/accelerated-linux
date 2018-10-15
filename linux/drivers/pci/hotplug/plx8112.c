/****************************************************************************/

/*
 *	plx8112.c  --  support for the PLX 8112 PCI-ExpressCard bridge
 *
 *	(C) Copyright 2009,  Greg Ungerer <gerg@snapgear.com>
 */

/****************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/pci_hotplug.h>
#include <linux/kthread.h>

#include <asm/mach-cavium-octeon/gpio.h>

/****************************************************************************/

/*
 * Some of the register address offsets of the PLX8112
 * (Only the ones we care about are listed here)
 */
#define	PLX8112_GPIOCTL		0x1020

/****************************************************************************/

struct plx8112_ctrler {
	struct pci_dev *pcidev;
	void __iomem *iomap;
	unsigned int state;
	struct task_struct *slotthread;
	struct hotplug_slot hpslot;
	struct hotplug_slot_info hpinfo;
};

#define	STATE_CARDIN		0x1	/* Card is logically inserted */
#define	STATE_POWERON		0x2	/* Port has power turned on */
#define	STATE_READY		0x4	/* Card is operational */
#define	STATE_USBIN		0x8	/* USB hub logically inserted */

/****************************************************************************/

static int plx8112_enable_slot(struct hotplug_slot *slot)
{
	printk(KERN_INFO "plx8112_enable_slot()\n");
	return 0;
}

/****************************************************************************/

static int plx8112_disable_slot(struct hotplug_slot *slot)
{
	printk(KERN_INFO "plx8112_disable_slot()\n");
	return 0;
}

/****************************************************************************/

struct hotplug_slot_ops plx8112_slotops = {
	.owner		= THIS_MODULE,
	.enable_slot	= plx8112_enable_slot,
	.disable_slot	= plx8112_disable_slot,
};

/****************************************************************************/

static void plx8112_release_slot(struct hotplug_slot *hpslot)
{
	printk(KERN_INFO "plx8112_release_slot()\n");
}

/****************************************************************************/

/*
 *	Assign resources to the newly discovered device. This doesn't seem
 *	to be as easy as it sounds. Problem is that expresscard is actually
 *	a PCI bus, and so resources need to be inhereted from parent buses.
 *	So we need to do a full bus/bridge sizing exercise, to be able to
 *	actually get some IO/mem resources assigned to the new device.
 *
 *	This routine is essentially the same as that used by the PCI core
 *	to do its initial resource allocation. But the realfunction that
 *	does this is marked "init". So we can't call it at hotplug time.
 */

static void plx8112_pci_assign_unassigned_resources(void)
{
	struct pci_bus *bus;

	/* Depth first, calculate sizes and alignments of all
	   subordinate buses. */
	list_for_each_entry(bus, &pci_root_buses, node) {
		pci_bus_size_bridges(bus);
	}
	/* Depth last, allocate resources and update the hardware. */
	list_for_each_entry(bus, &pci_root_buses, node) {
		pci_bus_assign_resources(bus);
		pci_enable_bridges(bus);
	}
}

/****************************************************************************/

static void plx8112_pci_assign_irqs(struct pci_bus *bus, int irq)
{
	struct pci_dev *dev;

	list_for_each_entry(dev, &bus->devices, bus_list) {
		u8 irq_pin;

		pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &irq_pin);
		if (irq_pin) {
			dev->irq = irq;
			pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
		}
	}
}

/****************************************************************************/

/*
 *	Power up the ExpressCard slot.
 */

static void plx8112_poweron(struct plx8112_ctrler *pc)
{
	octeon_gpio_raw_set(0x10000);
	pc->state |= STATE_POWERON;
}

/****************************************************************************/

/*
 *	Power down the ExpressCard slot.
 */

static void plx8112_poweroff(struct plx8112_ctrler *pc)
{
	octeon_gpio_raw_clear(0x10000);
	pc->state &= ~STATE_POWERON;
}

/****************************************************************************/

/*
 *	Process a card insertion event. Mark the slot as having something
 *	in it, and turn the power on. We wait one more poll period (which
 *	is 1s) before actually probing what is in the slot.
 */

static void plx8112_cardinsert(struct plx8112_ctrler *pc)
{
	pc->state |= STATE_CARDIN;
	plx8112_poweron(pc);
}

/****************************************************************************/

/*
 *	Card is powered up, and ready to be fully probed.
 */

static void plx8112_cardprobe(struct plx8112_ctrler *pc)
{
	struct pci_bus *bus;

	bus = pc->pcidev->subordinate;
	pci_scan_child_bus(bus);
	plx8112_pci_assign_unassigned_resources();
	plx8112_pci_assign_irqs(bus, pc->pcidev->irq);
	pci_bus_add_devices(bus);

	pc->state |= STATE_READY;
}

/****************************************************************************/

/*
 *	Handle a card removal event. Power of the slot, and remove the
 *	now removed PCI device from the device list.
 */

static void plx8112_cardremove(struct plx8112_ctrler *pc)
{
	struct pci_dev *dev;
	int i;

	plx8112_poweroff(pc);
	pc->state = 0;

	for (i = 0; (i < 8); i++) {
		dev = pci_get_slot(pc->pcidev->subordinate, PCI_DEVFN(0, i));
		if (dev) {
			pci_remove_bus_device(dev);
			pci_dev_put(dev);
		}
	}
}

/****************************************************************************/

/*
 * Check if an express card has been inserted in the slot or not.
 */

static int plx8112_slotthread(void *arg)
{
	struct plx8112_ctrler *pc = arg;
	u32 gpio;

	printk(KERN_INFO "PLX8112: ExpressCard service thread started\n");

	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
		if (kthread_should_stop())
			break;

		gpio = readl(pc->iomap + PLX8112_GPIOCTL);

		/* Check for ExpressCard events first */
		if (gpio & 0x2) {
			/* no card in slot */
			if (pc->state & STATE_CARDIN)
				plx8112_cardremove(pc);
		} else {
			/* card is present */
			if ((pc->state & STATE_CARDIN) == 0)
				plx8112_cardinsert(pc);
			else if ((pc->state & STATE_READY) == 0)
				plx8112_cardprobe(pc);
		}

		/* Now check for USB hub insertions */
		if (gpio & 0x4) {
			/* no usb hub present */
			if (pc->state & STATE_USBIN) {
				plx8112_poweroff(pc);
				pc->state &= ~STATE_USBIN;
			}
		} else {
			/* usb hub present */
			if ((pc->state & STATE_USBIN) == 0) {
				plx8112_poweron(pc);
				pc->state |= STATE_USBIN;
			}
		}
	}

	if (pc->state & STATE_CARDIN)
		plx8112_cardremove(pc);
	if (pc->state & STATE_USBIN)
		plx8112_poweroff(pc);

	return 0;
}

/****************************************************************************/

static int plx8112_initdev(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct plx8112_ctrler *pc;
	int r;

	pc = kzalloc(sizeof(struct plx8112_ctrler), GFP_KERNEL);
	if (pc == NULL) {
		printk(KERN_ERR "PLX8112: failed to alloc memory?\n");
		return -ENOMEM;
	}

	pdev->dev.platform_data = pc;
	pc->pcidev = pdev;

	r = pci_request_region(pdev, 0, "plx8112");
	if (r)
		goto exit_free_ctrler;

	pc->iomap = ioremap(pdev->resource[0].start, 64*1024);
	if (!pc->iomap) {
		r = -ENODEV;
		goto exit_release_region;
	}

	r = pci_enable_device(pdev);
	if (r) {
		printk(KERN_ERR "PLX8112: failed to enable device?\n");
		goto exit_iounmap;
	}

	pc->hpslot.private = pc;
	pc->hpslot.info = &pc->hpinfo;
	pc->hpslot.ops = &plx8112_slotops;
	pc->hpslot.release = plx8112_release_slot;

	r = pci_hp_register(&pc->hpslot, pdev->bus, 0, "ExpressCard");
	if (r) {
		printk(KERN_ERR "PLX8112: failed to register as hotplug device?\n");
		goto exit_disable_device;
	}

	if (octeon_gpio_raw_read() & 0x10000) {
		printk(KERN_INFO "PLX8112: card was powered on, resetting\n");
		plx8112_cardremove(pc);
	}

	pc->slotthread = kthread_run(plx8112_slotthread, pc, "plx8112");
	if (IS_ERR(pc->slotthread)) {
		printk(KERN_ERR "PLX8112: failed to start slot thread?\n");
		r = PTR_ERR(pc->slotthread);
		goto exit_hp_deregister;
	}

	return 0;

exit_hp_deregister:
	pci_hp_deregister(&pc->hpslot);
exit_disable_device:
	pci_disable_device(pdev);
exit_iounmap:
	iounmap(pc->iomap);
exit_release_region:
	pci_release_region(pdev, 0);
exit_free_ctrler:
	kfree(pc);
	return r;
}

/****************************************************************************/

static void plx8112_remove(struct pci_dev *pdev)
{
	struct plx8112_ctrler *pc = pdev->dev.platform_data;

	kthread_stop(pc->slotthread);
	pci_hp_deregister(&pc->hpslot);
	pci_disable_device(pdev);
	iounmap(pc->iomap);
	pci_release_region(pdev, 0);
	kfree(pc);
}

/****************************************************************************/

static struct pci_device_id plx8112_pci_tbl[] = {
	{ 0x10b5, 0x8112, PCI_ANY_ID, PCI_ANY_ID, },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, plx8112_pci_tbl);

static struct pci_driver plx8112_driver = {
	.name		= "plx8112",
	.id_table	= plx8112_pci_tbl,
	.probe		= plx8112_initdev,
	.remove		= plx8112_remove,
};

/****************************************************************************/

static int __init plx8112_init(void)
{
	printk(KERN_INFO "PLX8112: ExpressCard bridge support\n");

	return pci_register_driver(&plx8112_driver);
}

/****************************************************************************/

static void __exit plx8112_exit(void)
{
	pci_unregister_driver(&plx8112_driver);
}

/****************************************************************************/

module_init(plx8112_init);
module_exit(plx8112_exit);

MODULE_AUTHOR("Greg Ungerer <gerg@snapgear.com>");
MODULE_DESCRIPTION("PLX 8112 PCI-ExpressCard Bridge Driver");
MODULE_LICENSE("GPL");

/****************************************************************************/
