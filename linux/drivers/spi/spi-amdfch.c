/*
 * spi-amdfch.c -- AMD Fusion Controller Hub SPI controller
 *
 * Copyright 2015, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mtd/mtd.h>
#include <linux/spi/spi.h>
#include <linux/mtd/spi-nor.h>
#include <linux/module.h>

/*
 * Registers of the AMD FCH SPI controller.
 */
#define SPI_CNTRL0	0x0
#define SPI_CNTRL1	0xc
#define SPI_TXBYTECNT	0x48
#define SPI_RXBYTECNT	0x4b
#define SPI_FIFO	0x80

/*
 * Maximum data bytes for FIFO (5 cmd/address bytes on top of this ok).
 */
#define FIFOLEN		64

struct amdfch_spi {
	struct spi_master	*master;
	void __iomem		*mapbase;
	u32			baseaddr;
};

#define TIMEOUT		1000000

static int amdfch_spi_exec_cmd(struct amdfch_spi *aspi)
{
	int i;
	u8 v;

	v = readb(aspi->mapbase + SPI_CNTRL0 + 2);
	writeb(v | 0x1, aspi->mapbase + SPI_CNTRL0 + 2);

	for (i = TIMEOUT; i; i--) {
		v = readb(aspi->mapbase + SPI_CNTRL0 + 2);
		if ((v & 0x1) == 0)
			break;
	}
	return (i) ? 0 : -1;
}

/*
 * In the case of flash write operations we will need to wait for the
 * current outstanding write to complete - and send a new WEN command.
 * Nothing extra required for other command types (like read).
 */
static void amdfch_spi_recmd(struct amdfch_spi *aspi)
{
	u32 cmd;
	u8 addr0, status;
	int i;

	cmd = readb(aspi->mapbase + SPI_CNTRL0);
	if ((cmd == SPINOR_OP_PP) || (cmd == SPINOR_OP_PP_4B)) {
		addr0 = readb(aspi->mapbase + SPI_FIFO);

		/* Wait for previous transaction to finish */
		writeb(0, aspi->mapbase + SPI_TXBYTECNT);
		writeb(1, aspi->mapbase + SPI_RXBYTECNT);
		writeb(SPINOR_OP_RDSR, aspi->mapbase + SPI_CNTRL0);
		for (i = TIMEOUT/10; i; i--) {
			amdfch_spi_exec_cmd(aspi);
			status = readb(aspi->mapbase + SPI_FIFO);
			if ((status & SR_WIP) == 0)
				break;
		}

		/* Send the WEN command */
		writeb(0, aspi->mapbase + SPI_TXBYTECNT);
		writeb(0, aspi->mapbase + SPI_RXBYTECNT);
		writeb(SPINOR_OP_WREN, aspi->mapbase + SPI_CNTRL0);
		amdfch_spi_exec_cmd(aspi);

		/* Put write command back into cmd register */
		writeb(cmd, aspi->mapbase + SPI_CNTRL0);
		writeb(addr0, aspi->mapbase + SPI_FIFO);
	}
}

static void amdfch_spi_addr_update(struct amdfch_spi *aspi, int cmdcnt, int cnt)
{
	u32 addr;
	u8 cmd;
	int i;

	cmd = readb(aspi->mapbase + SPI_CNTRL0);
	/* If fast read then allow for dummy byte */
	if ((cmd == SPINOR_OP_READ_FAST) || (cmd == SPINOR_OP_READ_FAST_4B))
		cmdcnt--;

	for (addr = 0, i = 0; i < cmdcnt; i++)
		addr = (addr << 8) | readb(aspi->mapbase + SPI_FIFO + i);
	addr += cnt;
	for (i = cmdcnt - 1; i >= 0; i--) {
		writeb(addr & 0xff, aspi->mapbase + SPI_FIFO + i);
		addr >>= 8;
	}
}

/*
 * Due to limitations of the SPI controller we can only send and receive
 * conventional - and short - SPI messages. This is annoying when talking
 * to a flash, but we can handle it. Basically we can only send/receive 70
 * bytes in a single SPI message (normal flash reads and writes tend to be
 * much large, typically a page or more).
 *
 * We carefully process the SPI message so we can keep track of the read/
 * write address. We will need to send multiple SPI messages to satisfy most
 * flash reads or writes.
 */
static int amdfch_spi_xfer_msg(struct spi_master *master,
			       struct spi_message *msg)
{
	struct amdfch_spi *aspi = spi_master_get_devdata(master);
	struct spi_transfer *cmdxfer, *dataxfer;
	int len, cnt, cmdcnt, txcnt, rxcnt, i, rc;
	u8 const *txp;
	u8 cmd, *rxp;

	rc = 0;
	txcnt = 0;
	rxcnt = 0;

	cmdxfer = list_first_entry_or_null(&msg->transfers, typeof(*cmdxfer), transfer_list);
	if (cmdxfer == NULL) {
		dev_err(&msg->spi->dev, "no SPI transfer message\n");
		rc = -EINVAL;
		goto out;
	}
	if ((cmdxfer->tx_buf == NULL) || (cmdxfer->len < 1)) {
		dev_err(&msg->spi->dev, "no SPI command in message\n");
		rc = -EINVAL;
		goto out;
	}

	/* First message is command and possibly address */
	txp = cmdxfer->tx_buf;
	cmd = txp[0];
	writeb(cmd, aspi->mapbase + SPI_CNTRL0);
	for (i = 1, cmdcnt = 0; i < cmdxfer->len; i++, cmdcnt++)
		writeb(txp[i], aspi->mapbase + SPI_FIFO + cmdcnt);
	msg->actual_length += cmdxfer->len;

	/* Get data portion of the message (if any) */
	len = 0;
	txp = NULL;
	rxp = NULL;
	dataxfer = list_next_entry(cmdxfer, transfer_list);
	if (&dataxfer->transfer_list != &msg->transfers) {
		len = dataxfer->len;
		txp = dataxfer->tx_buf;
		rxp = dataxfer->rx_buf;
	}

	do {
		cnt = 0;
		txcnt = cmdcnt;
		if (txp) {
			/* Write to FIFO and transmit data */
			cnt = (len > FIFOLEN) ? FIFOLEN : len;
			for (i = 0; i < cnt; i++, txcnt++)
				writeb(*txp++, aspi->mapbase + SPI_FIFO + txcnt);
		}
		if (rxp) {
			cnt = (len > FIFOLEN) ? FIFOLEN : len;
			rxcnt = cnt;
		}

		/* Setup and execute the command */
		writeb(txcnt, aspi->mapbase + SPI_TXBYTECNT);
		writeb(rxcnt, aspi->mapbase + SPI_RXBYTECNT);
		if (amdfch_spi_exec_cmd(aspi) < 0) {
			dev_err(&msg->spi->dev, "SPI command timeout\n");
			msg->status = -ETIMEDOUT;
			goto out;
		}

		if (rxp) {
			/* Read back from FIFO any receive data */
			for (i = 0; i < rxcnt; i++) {
				*rxp++ = readb(aspi->mapbase + SPI_FIFO + cmdcnt + i);
			}
		}

		msg->actual_length += cnt;
		len -= cnt;
		if (len > 0) {
			amdfch_spi_recmd(aspi);
			amdfch_spi_addr_update(aspi, cmdcnt, cnt);
		}
	} while (len > 0);

out:
	if (msg->status == -EINPROGRESS)
		msg->status = rc;
	if (msg->status && master->handle_err)
		master->handle_err(master, msg);
	spi_finalize_current_message(master);
	return rc;
}

static int amdfch_spi_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct device *dev = &pdev->dev;
	struct amdfch_spi *aspi;
	struct spi_master *master;
	void __iomem *mapbase;
	u32 base;
	int rc;

	rc = pci_read_config_dword(pdev, 0xa0, &base);
	if (rc < 0) {
		dev_err(dev, "failed to read SPI BaseAddr\n");
		return -ENODEV;
	}
	if ((base & 0x2) == 0) {
		dev_err(dev, "SPI (flash) not enabled?\n");
		return -ENODEV;
	}
	base &= 0xffffffc0;

	mapbase = devm_ioremap(dev, base, 4096);
	if (IS_ERR(mapbase)) {
		dev_err(dev, "failed to ioremap(0x%x)=%p\n", base, mapbase);
		return -ENOMEM;
	}

	master = spi_alloc_master(dev, sizeof(*aspi));
	if (master == NULL) {
		dev_err(dev, "spi master alloc failed\n");
		return -ENOMEM;
	}

	master->bus_num = 0;
	master->mode_bits = SPI_CPHA | SPI_CPOL;
	master->num_chipselect = 1;
	master->transfer_one_message = amdfch_spi_xfer_msg;
	master->bits_per_word_mask = SPI_BPW_MASK(8);

	aspi = spi_master_get_devdata(master);
	pci_set_drvdata(pdev, aspi);
	aspi->master = master;
	aspi->baseaddr = base;
	aspi->mapbase = mapbase;

	rc = spi_register_master(master);
	return rc;
}

static void amdfch_spi_remove(struct pci_dev *pdev)
{
	struct amdfch_spi *aspi = pci_get_drvdata(pdev);
	struct spi_master *master = aspi->master;

	if (aspi->mapbase)
		iounmap(aspi->mapbase);
	spi_unregister_master(master);
}

static const struct pci_device_id amdfch_spi_devices[] = {
        { PCI_VDEVICE(AMD, 0x780e), 0 },
        { },
};
MODULE_DEVICE_TABLE(pci, amdfch_spi_devices);

static struct pci_driver amdfch_spi_driver = {
        .name           = "amdfch_spi",
        .id_table       = amdfch_spi_devices,
        .probe          = amdfch_spi_probe,
        .remove         = amdfch_spi_remove,
};

module_pci_driver(amdfch_spi_driver);

MODULE_DESCRIPTION("AMD Fusion Controller Hub SPI driver");
MODULE_AUTHOR("Greg Ungerer <greg.ungerer@accelerated.com>");
MODULE_LICENSE("GPL");
