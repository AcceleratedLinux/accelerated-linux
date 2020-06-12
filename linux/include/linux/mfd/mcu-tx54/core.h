/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright 2019 Digi International Inc.
 */

#ifndef MFD_MCU_TX54_CORE_H_
#define MFD_MCU_TX54_CORE_H_

#include <linux/device.h>
#include <linux/mfd/mcu-tx54/comm.h>
#include <linux/mutex.h>
#include <asm-generic/unaligned.h>

enum mcu_tx54_irq_numbers {
	MCU_TX54_IRQ_PWR_BUTTON = 0,
	MCU_TX54_IRQ_IGN_PWDN,
	MCU_TX54_IRQ_PWR_FAIL,
	MCU_TX54_IRQ_RTC,
	MCU_TX54_IRQ_ACC,

	MCU_TX54_IRQ_NR
};

struct mcu_tx54 {
	struct device		*dev;
	struct mutex		lock;

	int			irq;
	struct irq_domain	*irq_domain;
	struct mutex		irq_lock;
	uint32_t		irq_en;

	bool			pwroff_only;
};

#define mcu_tx54_tx_pkt_size(_type) \
	(MCU_TX_HDR_SIZE + sizeof_field(struct mcu_tx_pkt, _type))

#define mcu_tx54_rx_pkt_size(_type) \
	(1 + sizeof_field(struct mcu_rx_pkt, _type))

#define mcu_tx54_transaction(mcu, tx, rx, type) \
	mcu_tx54_transaction_(mcu, tx, mcu_tx54_tx_pkt_size(type), rx, \
			      mcu_tx54_rx_pkt_size(type))

int mcu_tx54_transaction_(struct mcu_tx54 *mcu, struct mcu_tx_pkt *tx,
			  unsigned int tx_len, struct mcu_rx_pkt *rx,
			  unsigned int rx_len);


#endif /* MFD_MCU_TX54_CORE_H_ */
