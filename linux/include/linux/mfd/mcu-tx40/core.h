/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright 2023 Digi International Inc.
 */

#ifndef MFD_MCU_TX40_CORE_H_
#define MFD_MCU_TX40_CORE_H_

#include <linux/device.h>
#include <linux/mfd/mcu-tx40/comm.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <asm-generic/unaligned.h>

enum mcu_tx40_irq_numbers {
	MCU_TX40_IRQ_IGN_PWDN,
	MCU_TX40_IRQ_VIN_LOW,
	MCU_TX40_IRQ_VIN_CRITICAL,
	MCU_TX40_IRQ_RTC,
	MCU_TX40_IRQ_ACC,

	MCU_TX40_IRQ_NR
};

struct mcu_tx40 {
	struct device		*dev;
	struct mutex		lock;
	struct miscdevice	miscdev;

	int			irq;
	struct irq_domain	*irq_domain;
	struct mutex		irq_lock;
	uint32_t		irq_en;
};


#define mcu_tx40_tx_pkt_size(_type) \
	(MCU_TX_HDR_SIZE + sizeof_field(struct mcu_tx_pkt, _type))

#define mcu_tx40_rx_pkt_size(_type) \
	(MCU_RX_HDR_SIZE + sizeof_field(struct mcu_rx_pkt, _type))

#define mcu_tx40_transaction(mcu, tx, rx, type) \
	mcu_tx40_transaction_(mcu, tx, mcu_tx40_tx_pkt_size(type), rx, \
			      mcu_tx40_rx_pkt_size(type))

int mcu_tx40_transaction_(struct mcu_tx40 *mcu, struct mcu_tx_pkt *tx,
			  unsigned int tx_len, struct mcu_rx_pkt *rx,
			  unsigned int rx_len);

#endif /* MFD_MCU_TX40_CORE_H_ */
