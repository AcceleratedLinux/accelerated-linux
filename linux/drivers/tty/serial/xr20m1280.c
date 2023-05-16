/*
 * XR20M1280 tty serial driver - Copyright (C) 2020 Digi International, Inc.
 *
 * Author: Digi International, http://www.digi.com
 *
 *  Based on sc16is7xx.c, by Jon Ringle <jringle@gridpoint.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio/driver.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <uapi/linux/sched/types.h>
#include <asm-generic/digi_realport.h>

#define XR20M1280_NAME			"xr20m1280"
#define XR20M1280_DEVNAME		"ttyXR"
#define XR20M1280_MAX_DEVS		8

//#define VALIDATE_REG_ACCESS

/* XR20M1280 register definitions */

/*
 * NOTE: Regarding register states...
 *
 *       The driver should generally assume that (after startup) EFR[4] == 1,
 *       LCR != 0xBF, LCR[7] == 0, SFR[0] == 0, and FCTR[6] == 1
 *
 *       Essentially, the default state has enhanced features enabled,
 *       holding registers available (not divisor registers), GPIO registers
 *       hidden, and scratch pad hidden (making EMSR and FC available).
 *
 *       If a function needs to make any of these untrue temporarily, it
 *       must restore the registers to this standard state.
 */

/*
 * NOTE: Regarding register mode locking
 *
 *       Since some configuration changes require the mode above to be
 *       changed, and since any mode change can break assumptions made
 *       by other code, protections must exist so that changing the mode
 *       (and returning it) happens more or less atomically from the
 *       perspective of the driver.
 *
 *       Since interrupts are actually handled in kernel worker threads,
 *       a mutex may be used for synchronization.  It must be held whenever
 *       a change to the register states is made, until the change is
 *       complete.  Similarly, interrupt handlers should take the mutex,
 *       allowing the non-interrupt code to temporarily block interruption.
 */

/*
 * NOTE: Regarding register caching
 *
 *       Avoiding early optimization in favor or straightforward
 *       function, the caching type is being set to NONE.  As a result,
 *       every register is essentially "volatile", and no register will
 *       be cached.  The RHR and ISR will still be marked as precious,
 *       so that debug functions don't read those registers (and lose
 *       information) in lieu of standard operation.
 */


/*
 *  -- Register 0 --
 */

/* Only if (LCR[7] == 0) */
#define XR20M1280_RHR_REG		(0x00) /* RX FIFO: RD only */
#define XR20M1280_THR_REG		(0x00) /* TX FIFO: WR only */

/* Enhanced Register set: Only if (LCR == 0xBF) */
#define XR20M1280_FC_REG		(0x00) /* RX/TX FIFO level: RD only */
#define XR20M1280_TRIG_REG		(0x00) /* RX/TX FIFO trigger: WR only */

/* Special Register set: Only if ((LCR[7] == 1) && (LCR != 0xBF)) */
#define XR20M1280_DLL_REG		(0x00) /* Divisor Latch LSB */

/* Only if ((LCR[7] == 1) && (LCR != 0xBF) && (DLL==0x00) && (DLM==0x00)) */
#define XR20M1280_DREV_REG		(0x00) /* Device Revision */


/*
 *  -- Register 1 --
 */

/* Only if (LCR[7] == 0) */
#define XR20M1280_IER_REG		(0x01) /* Interrupt enable */
    /* IER register bits */
    #define XR20M1280_IER_RHRI_BIT	(1 << 0) /* RHR interrupt */
    #define XR20M1280_IER_THRI_BIT	(1 << 1) /* THR interrupt */
    #define XR20M1280_IER_RLSI_BIT	(1 << 2) /* RX line status interrupt */
    #define XR20M1280_IER_MSI_BIT	(1 << 3) /* Modem status interrupt */
    /* IER register bits - write only if (EFR[4] == 1) */
    #define XR20M1280_IER_SLEEP_BIT	(1 << 4) /* Enable Sleep mode */
    #define XR20M1280_IER_XOFFI_BIT	(1 << 5) /* Enable Xoff interrupt */
    #define XR20M1280_IER_RTSI_BIT	(1 << 6) /* Enable nRTS interrupt */
    #define XR20M1280_IER_CTSI_BIT	(1 << 7) /* Enable nCTS interrupt */

/* Enhanced Register set: Only if (LCR == 0xBF) */
#define XR20M1280_FCTR_REG		(0x01) /* Feature control register */
    /* FCTR register bits */
    #define XR20M1280_FCTR_PWRDN_BIT	(1 << 0) /* Select sleep or nPWRDN */
    #define XR20M1280_FCTR_IRXI_BIT	(1 << 2) /* Enable IrDa RX inversion */
    #define XR20M1280_FCTR_AUTO_RS485_BIT (1 << 3) /* Auto RS-485 dir control */
    #define XR20M1280_FCTR_TRIGTAB_MASK	0x30     /* TX/RX trig. table select */
    #define XR20M1280_FCTR_TRIGTAB_A	0x00     /* TX/RX trig. table A */
    #define XR20M1280_FCTR_TRIGTAB_B	0x10     /* TX/RX trig. table B */
    #define XR20M1280_FCTR_TRIGTAB_C	0x20     /* TX/RX trig. table C */
    #define XR20M1280_FCTR_TRIGTAB_D	0x30     /* TX/RX trig. table D */
    #define XR20M1280_FCTR_SPSW_BIT	(1 << 6) /* Scratchpad swap */
    #define XR20M1280_FCTR_TRIGSEL_BIT	(1 << 7) /* Select TX for TRIG/FC */

/* Special Register set: Only if ((LCR[7] == 1) && (LCR != 0xBF)) */
#define XR20M1280_DLM_REG		(0x01) /* Divisor Latch MSB */


/*
 *  -- Register 2 --
 */

/* if ((EFR[4] == 1) && (LCR[7] == 0)) or */
/* if ((EFR[4] == 0) && (LCR != 0xBF))    */

#define XR20M1280_ISR_REG		(0x02) /* Interrupt status: RD only */
    /* ISR register bits, assuming (EFR[4] == 1) */
    /* Interrupt sources are in order of priority from 1-7 */
    #define XR20M1280_ISR_NO_INT_BIT	(1 << 0) /* No interrupts pending */
    #define XR20M1280_ISR_ID_MASK	0x3e     /* Mask for the interrupt ID */
    #define XR20M1280_ISR_RLSE_SRC	0x06     /* RX line status error */
    #define XR20M1280_ISR_RTOI_SRC	0x0c     /* RX time-out interrupt */
    #define XR20M1280_ISR_RHRI_SRC	0x04     /* RHR interrupt */
    #define XR20M1280_ISR_THRI_SRC	0x02     /* TX holding register empty */
    #define XR20M1280_ISR_MSI_SRC	0x00     /* Modem status interrupt */
    #define XR20M1280_ISR_XOFFI_SRC	0x10     /* Received Xoff */
    #define XR20M1280_ISR_CTSRTS_SRC	0x20     /* nCTS,nRTS rising edge */

#define XR20M1280_FCR_REG		(0x02) /* FIFO control: WR only */
    /* FCR register bits */
    #define XR20M1280_FCR_FIFO_BIT	(1 << 0) /* Enable FIFO */
    #define XR20M1280_FCR_RXRESET_BIT	(1 << 1) /* Reset RX FIFO */
    #define XR20M1280_FCR_TXRESET_BIT	(1 << 2) /* Reset TX FIFO */
    #define XR20M1280_FCR_RXLVLL_BIT	(1 << 6) /* RX Trigger level LSB */
    #define XR20M1280_FCR_RXLVLH_BIT	(1 << 7) /* RX Trigger level MSB */
    /* FCR register bits - write only if (EFR[4] == 1) */
    #define XR20M1280_FCR_WAKEI_BIT	(1 << 3) /* Enable wake up interrupt */
    #define XR20M1280_FCR_TXLVLL_BIT	(1 << 4) /* TX Trigger level LSB */
    #define XR20M1280_FCR_TXLVLH_BIT	(1 << 5) /* TX Trigger level MSB */
    #define XR20M1280_FCR_DEFAULT	(XR20M1280_FCR_FIFO_BIT | XR20M1280_FCR_RXLVLL_BIT | XR20M1280_FCR_TXLVLH_BIT)

/* Enhanced Register set: Only if (LCR == 0xBF) */
#define XR20M1280_EFR_REG		(0x02) /* Enhanced Function */
    /* EFR register bits */
    #define XR20M1280_EFR_AUTORTS_BIT	(1 << 6) /* Auto RTS flow ctrl enable */
    #define XR20M1280_EFR_AUTOCTS_BIT	(1 << 7) /* Auto CTS flow ctrl enable */
    #define XR20M1280_EFR_XOFF2_DETECT_BIT	(1 << 5) /* Enable Xoff2 detection */
    #define XR20M1280_EFR_ENABLE_BIT	(1 << 4) /* Enable enhanced functions
						  * and writing to IER[7:4],
						  * FCR[5:4], MCR[7:5]
						  */
    #define XR20M1280_EFR_SWFLOW3_BIT	(1 << 3) /* SWFLOW bit 3 */
    #define XR20M1280_EFR_SWFLOW2_BIT	(1 << 2) /* SWFLOW bit 2
						  *
						  * SWFLOW bits 3 & 2 table:
						  * 00 -> no transmitter flow
						  *       control
						  * 01 -> transmitter generates
						  *       XON2 and XOFF2
						  * 10 -> transmitter generates
						  *       XON1 and XOFF1
						  * 11 -> transmitter generates
						  *       XON1, XON2, XOFF1 and
						  *       XOFF2
						  */
    #define XR20M1280_EFR_TXSWFLOW_MASK	0xc0     /* SWFLOW (transmit) bits */
    #define XR20M1280_EFR_TXSWFLOW_NONE	0x00     /* No TX software flow */
    #define XR20M1280_EFR_TXSWFLOW_XON2	0x40     /* TX flow with XON2 & XOFF2 */
    #define XR20M1280_EFR_TXSWFLOW_XON1	0x80     /* TX flow with XON1 & XOFF1 */
    #define XR20M1280_EFR_TXSWFLOW_BOTH	0xc0     /* TX flow with XON/OFF 1&2  */
    #define XR20M1280_EFR_SWFLOW1_BIT	(1 << 1) /* SWFLOW bit 2 */
    #define XR20M1280_EFR_SWFLOW0_BIT	(1 << 0) /* SWFLOW bit 3
						  *
						  * SWFLOW bits 3 & 2 table:
						  * 00 -> no received flow
						  *       control
						  * 01 -> receiver compares
						  *       XON2 and XOFF2
						  * 10 -> receiver compares
						  *       XON1 and XOFF1
						  * 11 -> receiver compares
						  *       XON1, XON2, XOFF1 and
						  *       XOFF2
						  */
    #define XR20M1280_EFR_RXSWFLOW_MASK	0xc0     /* SWFLOW (receive) bits */
    #define XR20M1280_EFR_RXSWFLOW_NONE	0x00     /* No RX software flow */
    #define XR20M1280_EFR_RXSWFLOW_XON2	0x40     /* RX flow with XON2 & XOFF2 */
    #define XR20M1280_EFR_RXSWFLOW_XON1	0x80     /* RX flow with XON1 & XOFF1 */
    #define XR20M1280_EFR_RXSWFLOW_BOTH	0xc0     /* RX flow with XON/OFF 1&2  */

/* Special Register set:                                     */
/* Only if ((LCR[7] == 1) && (LCR != 0xBF) && (EFR[4] == 1)) */
#define XR20M1280_DLD_REG		(0x02) /* Divisor Fractional Register */
    /* DLD register bits */
    #define XR20M1280_DLD_FRAC_MASK	0x0F     /* Divisor fraction (*16) */
    #define XR20M1280_DLD_8XMODE_BIT	(1 << 4) /* 8X sampling if !4X mode */
    #define XR20M1280_DLD_4XMODE_BIT	(1 << 5) /* Enable 4X sampling */
    #define XR20M1280_DLD_IBRG_BIT	(1 << 6) /* Independent RX/TX BRG */
    #define XR20M1280_DLD_BRGSEL_BIT	(1 << 7) /* Choose RX or TX BRG */


/*
 *  -- Register 3 --
 */

#define XR20M1280_LCR_REG		(0x03) /* Line Control */
    /* LCR register bits */
    #define XR20M1280_LCR_LENGTH0_BIT	(1 << 0) /* Word length bit 0 */
    #define XR20M1280_LCR_LENGTH1_BIT	(1 << 1) /* Word length bit 1 */
    #define XR20M1280_LCR_WORD_LEN_MASK	(0x03)
    #define XR20M1280_LCR_WORD_LEN_5	(0x00)
    #define XR20M1280_LCR_WORD_LEN_6	(0x01)
    #define XR20M1280_LCR_WORD_LEN_7	(0x02)
    #define XR20M1280_LCR_WORD_LEN_8	(0x03)
    #define XR20M1280_LCR_STOPLEN_BIT	(1 << 2) /* STOP length bit
						  *
						  * STOP length bit table:
						  * 0 -> 1 stop bit
						  * 1 -> 1-1.5 stop bits if
						  *      word length is 5,
						  *      2 stop bits otherwise
						  */
    #define XR20M1280_LCR_PAR_BIT	(1 << 3) /* Parity bit enable */
    #define XR20M1280_LCR_EVENPAR_BIT	(1 << 4) /* Even parity bit enable */
    #define XR20M1280_LCR_FORCEPAR_BIT	(1 << 5) /* Force parity bit */
                                                 /* Parity mode: [5:3]
						  *  XX0 -> no parity
						  *  001 -> odd parity
						  *  011 -> even parity
						  *  101 -> mark parity
						  *  111 -> space parity
						  */
    #define XR20M1280_LCR_TXBREAK_BIT	(1 << 6) /* TX break enable */
    #define XR20M1280_LCR_DLAB_BIT	(1 << 7) /* Divisor Latch enable */
    #define XR20M1280_LCR_CONF_MODE_A	XR20M1280_LCR_DLAB_BIT /* Special
								* reg set */
    #define XR20M1280_LCR_CONF_MODE_B	0xBF                   /* Enhanced
								* reg set */


/*
 *  -- Register 4 --
 */

/* Only if LCR != 0xBF */
#define XR20M1280_MCR_REG		(0x04) /* Modem Control */
    /* MCR register bits */
    #define XR20M1280_MCR_DTR_BIT	(1 << 0) /* DTR complement */
    #define XR20M1280_MCR_RTS_BIT	(1 << 1) /* RTS complement */
    #define XR20M1280_MCR_MIOSEL_BIT	(1 << 2) /* Modem IO select */
    #define XR20M1280_MCR_LOOP_BIT	(1 << 4) /* Enable loopback test mode */
    #define XR20M1280_MCR_XONANY_BIT	(1 << 5) /* Enable Xon Any
						  * - write enabled
						  * if (EFR[4] == 1)
						  */
    #define XR20M1280_MCR_IRDA_BIT	(1 << 6) /* Enable IrDA mode
						  * - write enabled
						  * if (EFR[4] == 1)
						  */
    #define XR20M1280_MCR_CLKSEL_BIT	(1 << 7) /* Divide clock by 4
						  * - write enabled
						  * if (EFR[4] == 1)
						  */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 0)) */
#define XR20M1280_XON1_REG		(0x04) /* Xon1 character */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 1)) */
#define XR20M1280_GPIOINT_REG		(0x04) /* GPIO Interrupt Enable */


/*
 *  -- Register 5 --
 */

/* Only if LCR != 0xBF */
#define XR20M1280_LSR_REG		(0x05) /* Line Status: RD only */
    /* LSR register bits */
    #define XR20M1280_LSR_DR_BIT	(1 << 0) /* Receiver data ready */
    #define XR20M1280_LSR_OE_BIT	(1 << 1) /* Overrun Error */
    #define XR20M1280_LSR_PE_BIT	(1 << 2) /* Parity Error */
    #define XR20M1280_LSR_FE_BIT	(1 << 3) /* Frame Error */
    #define XR20M1280_LSR_BI_BIT	(1 << 4) /* Break Interrupt */
    #define XR20M1280_LSR_BRK_ERROR_MASK 0x1E    /* BI, FE, PE, OE bits */
    #define XR20M1280_LSR_THRE_BIT	(1 << 5) /* TX holding register empty */
    #define XR20M1280_LSR_TEMT_BIT	(1 << 6) /* Transmitter empty */
    #define XR20M1280_LSR_FIFOE_BIT	(1 << 7) /* RX Fifo global error */

#define XR20M1280_SHR_REG		(0x05) /* Setup/Hysteresis: WR only */
    /* SHR register bits */
    #define XR20M1280_SHR_RTSPOST_MASK	0x0F   /* Half-duplex RTS post-delay */
    #define XR20M1280_SHR_RTSTRIG_MASK	0x0F   /* AutoRTS hysteresis level */
    #define XR20M1280_SHR_RTSPRE_MASK	0xF0   /* Half-duplex RTS pre-delay */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 0)) */
#define XR20M1280_XON2_REG		(0x05) /* Xon2 character */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 1)) */
#define XR20M1280_GPIO3T_REG		(0x05) /* GPIO 3-State Control */


/*
 *  -- Register 6 --
 */

/* Only if LCR != 0xBF */
#define XR20M1280_MSR_REG		(0x06) /* Modem Status: RD only */
    /* MSR register bits */
    #define XR20M1280_MSR_DCTS_BIT	(1 << 0) /* Delta CTS Clear To Send */
    #define XR20M1280_MSR_DDSR_BIT	(1 << 1) /* Delta DSR Data Set Ready */
    #define XR20M1280_MSR_DRI_BIT	(1 << 2) /* Delta RI Ring Indicator */
    #define XR20M1280_MSR_DCD_BIT	(1 << 3) /* Delta CD Carrier Detect */
    #define XR20M1280_MSR_CTS_BIT	(1 << 4) /* CTS */
    #define XR20M1280_MSR_DSR_BIT	(1 << 5) /* DSR */
    #define XR20M1280_MSR_RI_BIT	(1 << 6) /* RI */
    #define XR20M1280_MSR_CD_BIT	(1 << 7) /* CD */
    #define XR20M1280_MSR_DELTA_MASK	0x0F     /* Any of the delta bits! */


/* Only if ((LCR != 0xBF) && (EFR[4] == 1)) */
#define XR20M1280_SFR_REG		(0x06) /* Special Function: WR only */
    /* SFR register bits */
    #define XR20M1280_SFR_GPIOE_BIT	(1 << 0) /* Enable GPIO registers */
    #define XR20M1280_SFR_GPIOBANK_BIT	(1 << 1) /* Select bank of GPIO regs */
    #define XR20M1280_SFR_GPIOIE_BIT	(1 << 2) /* Enable GPIO interrupts */
    #define XR20M1280_SFR_IRDA_MODE_BIT	(1 << 3) /* Enable FAST IrDA (v1.1) */
    #define XR20M1280_SFR_TXDISABLE_BIT	(1 << 4) /* Disable transmitter */
    #define XR20M1280_SFR_RXDISABLE_BIT	(1 << 5) /* Disable receiver */
    #define XR20M1280_SFR_9BIT_MODE_BIT	(1 << 6) /* 9-bit/multidrop mode */
    #define XR20M1280_SFR_TXADDR_BIT	(1 << 7) /* 9-bit: next byte is addr. */
    #define XR20M1280_SFR_DEFAULT	0x00     /* special features off,
						  * TX/RX both enabled
						  */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 0)) */
#define XR20M1280_XOFF1_REG		(0x06) /* Xoff1 character */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 1)) */
#define XR20M1280_GPIOINV_REG		(0x06) /* GPIO Parity Control */


/*
 *  -- Register 7 --
 */

/* Only if ((LCR != 0xBF) && (FCTR[6] == 0) && (SFR[0] == 0)) */
#define XR20M1280_SPR_REG		(0x07) /* Scratch Pad */

/* Only if ((LCR != 0xBF) && (FCTR[6] == 0) && (SFR[0] == 1)) */
#define XR20M1280_GPIOLVL_REG		(0x07) /* GPIO Level Register */

/* Only if ((LCR != 0xBF) && (FCTR[6] == 1) && (SFR[0] == 0)) */
#define XR20M1280_FCNT_REG		(0x07) /* TX/RX FIFO level: RD only */
					       /* Alternate version of FC */
#define XR20M1280_EMSR_REG		(0x07) /* Enhanced Mode Select: WR only */
    /* EMSR register bits */
    #define XR20M1280_EMSR_FCCTL0_BIT	(1 << 0) /* FIFO count control bit 0 */
    #define XR20M1280_EMSR_FCCTL1_BIT	(1 << 1) /* FIFO count control bit 1 */
    #define XR20M1280_EMSR_TXI_BIT	(1 << 2) /* Send TX immediately */
    #define XR20M1280_EMSR_RTSINV_BIT	(1 << 3) /* Invert RTS in RS485 mode */
    #define XR20M1280_EMSR_M3ST_BIT	(1 << 4) /* Modem outputs tri-state */
    #define XR20M1280_EMSR_LSRI_BIT	(1 << 6) /* LSR interrupt mode select */
    #define XR20M1280_EMSR_XOFFI_BIT	(1 << 7) /* Xoff interrupt mode select */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 0)) */
#define XR20M1280_XOFF2_REG		(0x07) /* Xoff2 word */

/* Enhanced Register set: Only if ((LCR == 0xBF) && (SFR[0] == 1)) */
#define XR20M1280_GPIOSEL_REG		(0x07) /* GPIO Select Register */


/* =================== */


#define XR20M1280_IODIR_REG		(0x0a) /* I/O Direction
						* - only on 75x/76x
						*/
#define XR20M1280_IOSTATE_REG		(0x0b) /* I/O State
						* - only on 75x/76x
						*/
#define XR20M1280_IOINTENA_REG		(0x0c) /* I/O Interrupt Enable
						* - only on 75x/76x
						*/
#define XR20M1280_IOCONTROL_REG		(0x0e) /* I/O Control
						* - only on 75x/76x
						*/
/* TCR/TLR Register set: Only if ((MCR[2] == 1) && (EFR[4] == 1)) */
#define XR20M1280_TCR_REG		(0x06) /* Transmit control */
#define XR20M1280_TLR_REG		(0x07) /* Trigger level */


/*
 * TCR register bits
 * TCR trigger levels are available from 0 to 60 characters with a granularity
 * of four.
 * The programmer must program the TCR such that TCR[3:0] > TCR[7:4]. There is
 * no built-in hardware check to make sure this condition is met. Also, the TCR
 * must be programmed with this condition before auto RTS or software flow
 * control is enabled to avoid spurious operation of the device.
 */
#define XR20M1280_TCR_RX_HALT(words)	((((words) / 4) & 0x0f) << 0)
#define XR20M1280_TCR_RX_RESUME(words)	((((words) / 4) & 0x0f) << 4)

/*
 * TLR register bits
 * If TLR[3:0] or TLR[7:4] are logical 0, the selectable trigger levels via the
 * FIFO Control Register (FCR) are used for the transmit and receive FIFO
 * trigger levels. Trigger levels from 4 characters to 60 characters are
 * available with a granularity of four.
 *
 * When the trigger level setting in TLR is zero, the SC16IS740/750/760 uses the
 * trigger level setting defined in FCR. If TLR has non-zero trigger level value
 * the trigger level defined in FCR is discarded. This applies to both transmit
 * FIFO and receive FIFO trigger level setting.
 *
 * When TLR is used for RX trigger level control, FCR[7:6] should be left at the
 * default state, that is, '00'.
 */
#define XR20M1280_TLR_TX_TRIGGER(words)	((((words) / 4) & 0x0f) << 0)
#define XR20M1280_TLR_RX_TRIGGER(words)	((((words) / 4) & 0x0f) << 4)

/* IOControl register bits (Only 750/760) */
#define XR20M1280_IOCONTROL_SRESET_BIT	(1 << 3) /* Software Reset */


/* Misc definitions */
#define XR20M1280_FIFO_SIZE		(128)
//#define XR20M1280_FIFO_SIZE		(64)
#define XR20M1280_REG_SHIFT		2

/**
 * enum uart_pm_state - power states for UARTs
 * @UART_PM_STATE_ON: UART is powered, up and operational
 * @UART_PM_STATE_OFF: UART is powered off
 * @UART_PM_STATE_UNDEFINED: sentinel
 */
#if 0
enum uart_pm_state {
	UART_PM_STATE_ON = 0,
	UART_PM_STATE_OFF = 3, /* number taken from ACPI */
	UART_PM_STATE_UNDEFINED,
};
#endif

#define RTS_TOGGLE_LSR_POLL_INTERVAL 200000

enum xr20m1280_tx_state {
	xr20m1280_tx_state_stopped,
	xr20m1280_tx_state_starting,
	xr20m1280_tx_state_started,
	xr20m1280_tx_state_polling,
	xr20m1280_tx_state_stopping
};

struct xr20m1280_devtype {
	char	name[10];
	int	nr_gpio;
	int	nr_uart;
};

#define XR20M1280_RECONF_MD		(1 << 0)
#define XR20M1280_RECONF_IER		(1 << 1)
#define XR20M1280_RECONF_RS485	(1 << 2)
#define XR20M1280_RECONF_RTS_TOGGLE	(1 << 3)

struct xr20m1280_one_config {
	unsigned int			flags;
	u8				ier_clear;
	u8				ier_set;
};

struct xr20m1280_one {
	struct uart_port		port;
	u8				line;
	unsigned int			msr_cache;
	struct kthread_work		start_work;
	struct kthread_work		tx_work;
	struct kthread_work		stop_work;
	struct kthread_work		reg_work;
	struct hrtimer			start_tx_timer;
	struct hrtimer			stop_tx_timer;
	struct xr20m1280_one_config	config;
	struct serial_rts_toggle	rts_toggle;
	unsigned int 			estat;
	unsigned int			flow;
	enum xr20m1280_tx_state	tx_state;
	bool				stopped;
};

struct xr20m1280_port {
	const struct xr20m1280_devtype	*devtype;
	struct regmap			*regmap;
	struct clk			*clk;
#ifdef CONFIG_GPIOLIB
	struct gpio_chip        gpio;
#endif
	unsigned char			buf[XR20M1280_FIFO_SIZE];
	struct kthread_worker		kworker;
	struct task_struct		*kworker_task;
	struct kthread_work		irq_work;
	struct mutex			reg_mode_mutex;
#ifdef VALIDATE_REG_ACCESS
	struct {
		u8 lcr;
		u8 sfr;
		u8 fctr;
		u8 efr;
		u8 dll;
		u8 dlm;
		u8 dld;
	} cache;
#endif
	struct xr20m1280_one		p[0];
};

#ifdef VALIDATE_REG_ACCESS
enum xr20m1280_register {
	XR20M1280_RHR_REG_ENUM,
	XR20M1280_THR_REG_ENUM,
	XR20M1280_IER_REG_ENUM,
	XR20M1280_ISR_REG_ENUM,
	XR20M1280_FCR_REG_ENUM,
	XR20M1280_LCR_REG_ENUM,
	XR20M1280_MCR_REG_ENUM,
	XR20M1280_LSR_REG_ENUM,
	XR20M1280_SHR_REG_ENUM,
	XR20M1280_MSR_REG_ENUM,
	XR20M1280_SFR_REG_ENUM,
	XR20M1280_SPR_REG_ENUM,
	XR20M1280_GPIOLVL_REG_ENUM,
	XR20M1280_EMSR_REG_ENUM,
	XR20M1280_FCNT_REG_ENUM,
	XR20M1280_DREV_REG_ENUM,
	XR20M1280_DLL_REG_ENUM,
	XR20M1280_DLM_REG_ENUM,
	XR20M1280_DLD_REG_ENUM,
	XR20M1280_FC_REG_ENUM,
	XR20M1280_TRIG_REG_ENUM,
	XR20M1280_FCTR_REG_ENUM,
	XR20M1280_EFR_REG_ENUM,
	XR20M1280_XON1_REG_ENUM,
	XR20M1280_XON2_REG_ENUM,
	XR20M1280_XOFF1_REG_ENUM,
	XR20M1280_XOFF2_REG_ENUM,
	XR20M1280_GPIOINT_REG_ENUM,
	XR20M1280_GPIO3T_REG_ENUM,
	XR20M1280_GPIOINV_REG_ENUM,
	XR20M1280_GPIOSEL_REG_ENUM,
};
#endif

static unsigned long xr20m1280_lines;

static struct uart_driver xr20m1280_uart = {
	.owner		= THIS_MODULE,
	.driver_name	= XR20M1280_NAME,
	.dev_name	= XR20M1280_DEVNAME,
	.nr		= XR20M1280_MAX_DEVS,
};

#define to_xr20m1280_port(p,e)	((container_of((p), struct xr20m1280_port, e)))
#define to_xr20m1280_one(p,e)	((container_of((p), struct xr20m1280_one, e)))
#define RTS_TOGGLE_ENABLED(one) (((one)->port.rs485.flags & SER_RS485_ENABLED) == 0 && (one)->rts_toggle.enable)
static int xr20m1280_line(struct uart_port *port)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	return one->line;
}

static u8 _xr20m1280_port_read(struct uart_port *port, u8 reg)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	unsigned int val = 0;
	const u8 line = xr20m1280_line(port);

	regmap_read(s->regmap, (reg << XR20M1280_REG_SHIFT) | line, &val);

	return val;
}

static void _xr20m1280_port_write(struct uart_port *port, u8 reg, u8 val)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	const u8 line = xr20m1280_line(port);

	pr_debug("port_write: reg: 0x%02x, val: 0x%02x\n", reg, val);
	regmap_write(s->regmap, (reg << XR20M1280_REG_SHIFT) | line, val);
}

static void _xr20m1280_port_update(struct uart_port *port, u8 reg,
				  u8 mask, u8 val)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	const u8 line = xr20m1280_line(port);

	pr_debug("port_update: reg: 0x%02x, mask: 0x%02x, val: 0x%02x\n", reg, mask, val);
	regmap_update_bits(s->regmap, (reg << XR20M1280_REG_SHIFT) | line,
			   mask, val);
}

#ifdef VALIDATE_REG_ACCESS
static inline u8 conf_mode_a(struct xr20m1280_port *s)
{
	return s->cache.lcr & XR20M1280_LCR_DLAB_BIT;
}

static inline u8 enhanced_mode(struct xr20m1280_port *s)
{
	return s->cache.efr & XR20M1280_EFR_ENABLE_BIT;
}

static inline u8 conf_mode_b(struct xr20m1280_port *s)
{
	return s->cache.lcr == XR20M1280_LCR_CONF_MODE_B;
}

static inline u8 gpio_mode(struct xr20m1280_port *s)
{
	return s->cache.sfr & XR20M1280_SFR_GPIOE_BIT;
}

static inline u8 scratchpad_disabled(struct xr20m1280_port *s)
{
	return s->cache.fctr & XR20M1280_FCTR_SPSW_BIT;
}

static void validate_register_access(struct xr20m1280_port *s, char const * const str, enum xr20m1280_register reg, int line, u8 write)
{
	u8 valid = 0;

	switch (reg) {
	case XR20M1280_RHR_REG_ENUM:
		valid = !conf_mode_a(s) && !write;
		break;
	case XR20M1280_THR_REG_ENUM:
		valid = !conf_mode_a(s) && write;
		break;
	case XR20M1280_IER_REG_ENUM:
		valid = !conf_mode_a(s);
		break;
	case XR20M1280_ISR_REG_ENUM:
		valid = enhanced_mode(s) ? !conf_mode_a(s) : !conf_mode_b(s);
		valid = valid && !write;
		break;
	case XR20M1280_FCR_REG_ENUM:
		valid = enhanced_mode(s) ? !conf_mode_a(s) : !conf_mode_b(s);
		valid = valid && write;
		break;
	case XR20M1280_LCR_REG_ENUM:
		valid = 1;
		break;
	case XR20M1280_MCR_REG_ENUM:
		valid = !conf_mode_b(s);
		break;
	case XR20M1280_LSR_REG_ENUM:
	case XR20M1280_MSR_REG_ENUM:
		valid = !conf_mode_b(s) && !write;
		break;
	case XR20M1280_SHR_REG_ENUM:
	case XR20M1280_SFR_REG_ENUM:
		valid = !conf_mode_b(s) && write;
		break;
	case XR20M1280_SPR_REG_ENUM:
		valid = !conf_mode_b(s) && !scratchpad_disabled(s) && !gpio_mode(s);
		break;
	case XR20M1280_GPIOLVL_REG_ENUM:
		valid = !conf_mode_b(s) && !scratchpad_disabled(s) && gpio_mode(s);
		break;
	case XR20M1280_EMSR_REG_ENUM:
		valid = !conf_mode_b(s) && scratchpad_disabled(s) && !gpio_mode(s) && write;
		break;
	case XR20M1280_FCNT_REG_ENUM:
		valid = !conf_mode_b(s) && scratchpad_disabled(s) && !gpio_mode(s) && !write;
		break;
	case XR20M1280_DREV_REG_ENUM:
		valid = conf_mode_a(s) && !conf_mode_b(s) && s->cache.dll == 0 && s->cache.dlm == 0 && !write;
		break;
	case XR20M1280_DLL_REG_ENUM:
	case XR20M1280_DLM_REG_ENUM:
		valid = conf_mode_a(s) && !conf_mode_b(s);
		break;
	case XR20M1280_DLD_REG_ENUM:
		valid = conf_mode_a(s) && !conf_mode_b(s) && enhanced_mode(s);
		break;
	case XR20M1280_FC_REG_ENUM:
		valid = conf_mode_b(s) && !write;
		break;
	case XR20M1280_TRIG_REG_ENUM:
		valid = conf_mode_b(s) && write;
		break;
	case XR20M1280_FCTR_REG_ENUM:
	case XR20M1280_EFR_REG_ENUM:
		valid = conf_mode_b(s);
		break;
	case XR20M1280_XON1_REG_ENUM:
	case XR20M1280_XON2_REG_ENUM:
	case XR20M1280_XOFF1_REG_ENUM:
	case XR20M1280_XOFF2_REG_ENUM:
		valid = conf_mode_b(s) && !gpio_mode(s);
		break;
	case XR20M1280_GPIOINT_REG_ENUM:
	case XR20M1280_GPIO3T_REG_ENUM:
	case XR20M1280_GPIOINV_REG_ENUM:
	case XR20M1280_GPIOSEL_REG_ENUM:
		valid = conf_mode_b(s) && gpio_mode(s);
		break;
	}
	if (!valid) {
		pr_warn("Invalid access to %s at %d! LCR: %hhu EFR: %hhu SFR: %hhu FCTR: %hhu DLL: %hhu DLM: %hhu DLD: %hhu write %hhu\n",
		str, line, s->cache.lcr, s->cache.efr, s->cache.sfr, s->cache.fctr, s->cache.dll, s->cache.dlm, s->cache.dld, write);
	}
}

static u8 _xr20m1280_port_validate_and_read(struct uart_port *port, char const * const str, enum xr20m1280_register reg, int line, u8 addr)
{
	u8 expected, actual;
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	validate_register_access(s, str, reg, line, 0);
	actual = _xr20m1280_port_read(port, addr);
	switch (reg) {
	case XR20M1280_LCR_REG_ENUM:
		expected = s->cache.lcr;
		s->cache.lcr = actual;
		break;
	case XR20M1280_EFR_REG_ENUM:
		expected = s->cache.efr;
		s->cache.efr = actual;
		break;
	case XR20M1280_SFR_REG_ENUM:
		expected = s->cache.sfr;
		s->cache.sfr = actual;
		break;
	case XR20M1280_FCTR_REG_ENUM:
		expected = s->cache.fctr;
		s->cache.fctr = actual;
		break;
	case XR20M1280_DLL_REG_ENUM:
		expected = s->cache.dll;
		s->cache.dll = actual;
		break;
	case XR20M1280_DLM_REG_ENUM:
		expected = s->cache.dlm;
		s->cache.dlm = actual;
		break;
	case XR20M1280_DLD_REG_ENUM:
		expected = s->cache.dld;
		s->cache.dld = actual;
		break;
	default:
		expected = actual;
		break;
	}
	if (expected != actual) {
		pr_warn("Cache mismatch for %s! Expected: %hhu Actual: %hhu\n", str, expected, actual);
	}
	return actual;
}

static void _xr20m1280_port_validate_and_write(struct uart_port *port, char const * const str, enum xr20m1280_register reg, int line, u8 addr, u8 val)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	switch (reg) {
	case XR20M1280_LCR_REG_ENUM:
		s->cache.lcr = val;
		break;
	case XR20M1280_EFR_REG_ENUM:
		s->cache.efr = val;
		break;
	case XR20M1280_SFR_REG_ENUM:
		s->cache.sfr = val;
		break;
	case XR20M1280_FCTR_REG_ENUM:
		s->cache.fctr = val;
		break;
	case XR20M1280_DLL_REG_ENUM:
		s->cache.dll = val;
		break;
	case XR20M1280_DLM_REG_ENUM:
		s->cache.dlm = val;
		break;
	case XR20M1280_DLD_REG_ENUM:
		s->cache.dld = val;
		break;
	default:
		break;
	}
	validate_register_access(s, str, reg, line, 1);
	_xr20m1280_port_write(port, addr, val);
}

static void _xr20m1280_port_validate_and_update(struct uart_port *port, char const * const str, enum xr20m1280_register reg, int line, u8 addr, u8 mask, u8 val)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	switch (reg) {
	case XR20M1280_LCR_REG_ENUM:
		s->cache.lcr = (s->cache.lcr & ~mask) | (val & mask);
		break;
	case XR20M1280_EFR_REG_ENUM:
		s->cache.efr = (s->cache.efr & ~mask) | (val & mask);
		break;
	case XR20M1280_SFR_REG_ENUM:
		s->cache.sfr = (s->cache.sfr & ~mask) | (val & mask);
		break;
	case XR20M1280_FCTR_REG_ENUM:
		s->cache.fctr = (s->cache.fctr & ~mask) | (val & mask);
		break;
	case XR20M1280_DLL_REG_ENUM:
		s->cache.dll = (s->cache.dll & ~mask) | (val & mask);
		break;
	case XR20M1280_DLM_REG_ENUM:
		s->cache.dlm = (s->cache.dlm & ~mask) | (val & mask);
		break;
	case XR20M1280_DLD_REG_ENUM:
		s->cache.dld = (s->cache.dld & ~mask) | (val & mask);
		break;
	default:
		break;
	}
	validate_register_access(s, str, reg, line, 0);
	validate_register_access(s, str, reg, line, 1);
	_xr20m1280_port_update(port, addr, mask, val);
}
#define xr20m1280_port_read(port, reg)					_xr20m1280_port_validate_and_read(port, #reg, reg##_ENUM, __LINE__, reg)
#define xr20m1280_port_write(port, reg, val)			_xr20m1280_port_validate_and_write(port, #reg, reg##_ENUM, __LINE__, reg, val)
#define xr20m1280_port_update(port, reg, mask, val)		_xr20m1280_port_validate_and_update(port, #reg, reg##_ENUM, __LINE__, reg, mask, val)
#else
#define xr20m1280_port_read(port, reg)					_xr20m1280_port_read(port, reg)
#define xr20m1280_port_write(port, reg, val)			_xr20m1280_port_write(port, reg, val)
#define xr20m1280_port_update(port, reg, mask, val)		_xr20m1280_port_update(port, reg, mask, val)
#endif

static void start_hrtimer_ms(struct hrtimer *hrt, unsigned long msec)
{
	long sec = msec / 1000;
	long nsec = (msec % 1000) * 1000000;
	ktime_t t = ktime_set(sec, nsec);

	hrtimer_start(hrt, t, HRTIMER_MODE_REL);
}

static void xr20m1280_fifo_read(struct uart_port *port, unsigned int rxlen)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	const u8 line = xr20m1280_line(port);
	u8 addr = (XR20M1280_RHR_REG << XR20M1280_REG_SHIFT) | line;

	regmap_noinc_read(s->regmap, addr, s->buf, rxlen);
}

static void xr20m1280_fifo_write(struct uart_port *port, u8 to_send)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	const u8 line = xr20m1280_line(port);
	u8 addr = (XR20M1280_THR_REG << XR20M1280_REG_SHIFT) | line;

	/*
	 * Don't send zero-length data, at least on SPI it confuses the chip
	 * delivering wrong TXLVL data.
	 */
	if (unlikely(!to_send))
		return;

	regmap_noinc_write(s->regmap, addr, s->buf, to_send);
}

#define XR20M1280_FIFO_COUNT_RX 0
#define XR20M1280_FIFO_COUNT_TX 1
/* Register accesses in fifo_count are protected by reg_mode_mutex in ist */
static u8 xr20m1280_fifo_count(struct uart_port *port, u8 dir)
{
	u8 last, len;

	/* Set direction for fifo count read */
	xr20m1280_port_write(port, XR20M1280_EMSR_REG, dir);
	last = xr20m1280_port_read(port, XR20M1280_FCNT_REG);

	if (dir == XR20M1280_FIFO_COUNT_RX)
	{
		int i;
		for (i = 0; i < 10; i++) {
			len = xr20m1280_port_read(port, XR20M1280_FCNT_REG);
			if (len == last)
				break;
			last = len;
		}
		if (i >= 10) {
			dev_warn_ratelimited(port->dev, "%s: RX count did not stabilize\n", port->name);
		}
	}

	return last;
}

static int xr20m1280_alloc_line(void)
{
	int i;

	BUILD_BUG_ON(XR20M1280_MAX_DEVS > BITS_PER_LONG);

	for (i = 0; i < XR20M1280_MAX_DEVS; i++)
		if (!test_and_set_bit(i, &xr20m1280_lines))
			break;

	return i;
}

static void xr20m1280_power(struct uart_port *port, int on)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

/* TODO */
/*	xr20m1280_port_update(port, XR20M1280_IER_REG,
			      XR20M1280_IER_SLEEP_BIT,
			      on ? 0 : XR20M1280_IER_SLEEP_BIT);*/

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);
}

static const struct xr20m1280_devtype xr20m1280_devtype = {
	.name		= "XR20M1280",
	.nr_gpio	= 4, /* 0-3 are modem control, 4-7 are gpiochip exposed for MEI control */
	.nr_uart	= 1,
};

static bool xr20m1280_regmap_precious(struct device *dev, unsigned int reg)
{
	switch (reg >> XR20M1280_REG_SHIFT) {
	case XR20M1280_RHR_REG:
	case XR20M1280_ISR_REG:
		return true;
	default:
		break;
	}

	return false;
}

#if 0
static int dump_register(struct uart_port *port)
{
    u8 lcr;
    u8 reg;

    lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	printk(KERN_INFO "Cur LCR: 0x%02x\n", lcr);

	{
		xr20m1280_port_write(port, XR20M1280_LCR_REG, 0xBF);
		printk("******Dump register at LCR=0xBF\n");
		reg = xr20m1280_port_read(port, XR20M1280_EFR_REG);
		printk("EFR: 0x%02x\n", reg);

		reg = xr20m1280_port_read(port, XR20M1280_SFR_REG);
		printk("SFR: 0x%02x\n", reg);

		reg = xr20m1280_port_read(port, XR20M1280_FCTR_REG);
		printk("FCTR: 0x%02x\n", reg);
	}
	{
		xr20m1280_port_write(port, XR20M1280_LCR_REG, 0x03);
		printk("******Dump register at LCR=0x03\n");
		reg = xr20m1280_port_read(port, XR20M1280_MCR_REG);
		printk("MCR: 0x%02x\n", reg);

		reg = xr20m1280_port_read(port, XR20M1280_LSR_REG);
		printk("LSR: 0x%02x\n", reg);

		reg = xr20m1280_port_read(port, XR20M1280_MSR_REG);
		printk("MSR: 0x%02x\n", reg);

		reg = xr20m1280_port_read(port, XR20M1280_IER_REG);
		printk("IER: 0x%02x\n", reg);
	}

    /* Put LCR back to the normal mode */
    xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
    return 0;
}
#endif

#ifdef CONFIG_GPIOLIB
#define XR20M1280_485_TERM_EN (4) /* pulled low, active high */
#define XR20M1280_485_4W_EN   (5) /* pulled low, active high */
#define XR20M1280_485_RX_EN   (6) /* pulled high, active low */
#define XR20M1280_485_EN      (7) /* pulled high, active low */
#define XRM1280_MAP_OFFSET(x) (x + 4) /* Kernel interface exposes 0-3, but we're using hardware pins 4-7 */
#define XRM1280_MAX_GPIO 7
/* Internal interface */
static int _xr20m1280_gpio_get(struct gpio_chip *chip, unsigned offset, int lock)
{
	u8 lvl;
	u8 fctr;
	u8 lcr;
	struct xr20m1280_port *s = container_of(chip, struct xr20m1280_port, gpio);
	struct uart_port *port = &s->p[0].port;

	if (lock)
		mutex_lock(&s->reg_mode_mutex);

	/* Enable GPIO regs (SFR[0] == 1) */
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_GPIOE_BIT);

	/* Unset swap scratchpad */
	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);
	fctr = xr20m1280_port_read(port, XR20M1280_FCTR_REG);
	xr20m1280_port_update(port, XR20M1280_FCTR_REG, XR20M1280_FCTR_SPSW_BIT, 0);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);

	lvl = xr20m1280_port_read(port, XR20M1280_GPIOLVL_REG);
	pr_debug("_xr20m1280_gpio_get: offset: %u, lvl: 0x%02x\n", offset, lvl);

	/* Restore required state */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);
	xr20m1280_port_write(port, XR20M1280_FCTR_REG, fctr);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);

	if (lock)
		mutex_unlock(&s->reg_mode_mutex);

	return (lvl & BIT(offset)) ? 1 : 0;
}

/* External interface */
static int xr20m1280_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	offset = XRM1280_MAP_OFFSET(offset);
	if (offset > XRM1280_MAX_GPIO) {
		pr_debug("gpio_get: GPIO offset %ul is greater than number of chip GPIOs!\n", offset);
		return -EINVAL;
	}

	return _xr20m1280_gpio_get(chip, offset, 1/*lock*/);
}

/* Internal interface */
static void _xr20m1280_gpio_set(struct gpio_chip *chip, unsigned offset, int val, int lock)
{
	u8 fctr;
	u8 lcr;
	struct xr20m1280_port *s = container_of(chip, struct xr20m1280_port, gpio);
	struct uart_port *port = &s->p[0].port;

	if (lock)
		mutex_lock(&s->reg_mode_mutex);

		/* Enable GPIO regs (SFR[0] == 1) */
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_GPIOE_BIT);

	/* Unset swap scratchpad */
	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);
	fctr = xr20m1280_port_read(port, XR20M1280_FCTR_REG);
	xr20m1280_port_update(port, XR20M1280_FCTR_REG, XR20M1280_FCTR_SPSW_BIT, 0);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);

	pr_debug("gpio_set: offset: %u, val: %d\n", offset, val);
	xr20m1280_port_update(port, XR20M1280_GPIOLVL_REG, BIT(offset), val ? BIT(offset) : 0);

	/* Restore required state */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);
	xr20m1280_port_write(port, XR20M1280_FCTR_REG, fctr);
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);

	if (lock)
		mutex_unlock(&s->reg_mode_mutex);
}

/* External interface */
static void xr20m1280_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	offset = XRM1280_MAP_OFFSET(offset);
	if (offset > XRM1280_MAX_GPIO) {
		pr_err("gpio_set: GPIO offset %ul is greater than number of chip GPIOs!\n", offset);
		return;
	}

	_xr20m1280_gpio_set(chip, offset, val, 1/*use lock*/);
}

static int xr20m1280_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	u8 lcr;
	struct xr20m1280_port *s = container_of(chip, struct xr20m1280_port, gpio);
	struct uart_port *port = &s->p[0].port;

	pr_debug("gpio_direction_input: offset: %u\n", offset);
	offset = XRM1280_MAP_OFFSET(offset);
	if (offset > XRM1280_MAX_GPIO) {
		pr_err("gpio_direction_input: GPIO offset %ul is greater than number of chip GPIOs!\n", offset);
		return -EINVAL;
	}

	mutex_lock(&s->reg_mode_mutex);

	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	/* Enable GPIO regs (SFR[0] == 1) */
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_GPIOE_BIT);
	/* Enable config mode B (LCR == 0xBF) */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);

	/* Set the GPIO as input */
	xr20m1280_port_update(port, XR20M1280_GPIOSEL_REG, BIT(offset), BIT(offset));

	/* Restore required state */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);

	mutex_unlock(&s->reg_mode_mutex);

	return 0;
}
/*
 * Per maxlinear there's no way to set the pin as an output without first
 * setting the pin low.  So, if the pin default is pulled high, and it needs to
 * be high, it's recommended that the software simply leaves the pin as an
 * input, until it needs to drive the pin low.  This seems the only way to
 * avoid a glitch.
 */
static int xr20m1280_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int val)
{
	u8 lcr;
	u8 cur_dir;
	struct xr20m1280_port *s = container_of(chip, struct xr20m1280_port, gpio);
	struct uart_port *port = &s->p[0].port;

	offset = XRM1280_MAP_OFFSET(offset);
	if (offset > XRM1280_MAX_GPIO) {
		pr_err("gpio_direction_output: GPIO offset %ul is greater than number of chip GPIOs!\n", offset);
		return -EINVAL;
	}

	mutex_lock(&s->reg_mode_mutex);
	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	/* Enable GPIO regs (SFR[0] == 1) */
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_GPIOE_BIT);
	/* Enable config mode B (LCR == 0xBF) */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);

	/* the pin is already output */
	cur_dir = xr20m1280_port_read(port, XR20M1280_GPIOSEL_REG);
	pr_debug("gpio_direction_output: offset: %u, val: %d, cur_dir: 0x%02x\n", offset, val, cur_dir);
	if (!(cur_dir & BIT(offset))) {
		/* Restore required state */
		xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
		xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);
		/* set value */
		_xr20m1280_gpio_set(chip, offset, val, 0/*use lock*/);
		goto out;
	}

	/* Set the GPIO as output */
	pr_debug("gpio_direction_output: enabling output mode\n");
	xr20m1280_port_update(port, XR20M1280_GPIOSEL_REG, BIT(offset), 0);
	
	/* Restore required state */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);
	/* set value */
	_xr20m1280_gpio_set(chip, offset, val, 0/*use lock*/);

out:
	mutex_unlock(&s->reg_mode_mutex);

	return 0;
}
#endif

static int xr20m1280_set_baud(struct uart_port *port, int baud)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	u8 lcr;
	u8 prescaler = 0;
	unsigned long clk = port->uartclk, div = clk / 16 / baud;

printk(KERN_INFO "xr20m1280_set_baud: %d %lu %lu\n", baud, clk, div);
	if (div > 0xffff) {
		prescaler = XR20M1280_MCR_CLKSEL_BIT;
		div /= 4;
	}
printk(KERN_INFO "xr20m1280_set_baud: prescaler: %d %lu\n", prescaler, div);

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);

	/* Adjust prescaler */
	xr20m1280_port_update(port, XR20M1280_MCR_REG,
			      XR20M1280_MCR_CLKSEL_BIT,
			      prescaler);

	/* Open the LCR divisors for configuration */
	xr20m1280_port_write(port, XR20M1280_LCR_REG,
			     XR20M1280_LCR_CONF_MODE_A);

	/* Write the new divisor */
	xr20m1280_port_write(port, XR20M1280_DLM_REG, div / 256);
	xr20m1280_port_write(port, XR20M1280_DLL_REG, div % 256);

	/* EFR should already be enabled */ /* TODO */
	/* Open the LCR divisors for configuration */
	//xr20m1280_port_write(port, XR20M1280_LCR_REG,
	//		     XR20M1280_LCR_CONF_MODE_B);

	/* Latch enhanced features, then re-enable enhanced feature registers */
	//xr20m1280_port_update(port, XR20M1280_EFR_REG, XR20M1280_EFR_ENABLE_BIT, 0);
	//xr20m1280_port_update(port, XR20M1280_EFR_REG,
	//		      XR20M1280_EFR_ENABLE_BIT,
	//		      XR20M1280_EFR_ENABLE_BIT);

	/* Put LCR back to the normal mode */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);

	return DIV_ROUND_CLOSEST(clk / 16, div);
}

/* Register accesses in handle_rx are protected by reg_mode_mutex in ist */
static void xr20m1280_handle_rx(struct uart_port *port, unsigned int rxlen,
				unsigned int iir)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	unsigned int lsr = 0, ch, flag, bytes_read, i;
	__u32 rx, buf_overrun = 0, brk = 0, parity = 0, frame = 0, overrun = 0;
	bool read_lsr = true;

	if (unlikely(rxlen >= sizeof(s->buf))) {
		dev_warn_ratelimited(port->dev,
		                    "%s: Possible RX FIFO overrun: %d\n",
		                    port->name, rxlen);
		buf_overrun = 1;
		/* Ensure sanity of RX level */
		rxlen = sizeof(s->buf);
	}
	rx = rxlen;

	// JAP // printk(KERN_INFO "xr20m1280_handle_rx: %d\n", rxlen); // JAP //
	// JAP // printk(KERN_INFO "rx %d\n", rxlen); // JAP //
	// JAP // dump_register(port); // JAP //

	while (rxlen) {
		/* Only read lsr if there are possible errors in FIFO */
		if (read_lsr) {
			lsr = xr20m1280_port_read(port, XR20M1280_LSR_REG);
			if (!(lsr & XR20M1280_LSR_FIFOE_BIT))
				read_lsr = false; /* No errors left in FIFO */
		} else
			lsr = 0;

		if (read_lsr) {
			s->buf[0] = xr20m1280_port_read(port, XR20M1280_RHR_REG);
			bytes_read = 1;
		} else {
			xr20m1280_fifo_read(port, rxlen);
			bytes_read = rxlen;
		}

		lsr &= XR20M1280_LSR_BRK_ERROR_MASK;
		flag = TTY_NORMAL;

		if (unlikely(lsr)) {
			if (lsr & XR20M1280_LSR_BI_BIT) {
				brk++;
				if (uart_handle_break(port)) {
					rxlen -= bytes_read;
					continue;
				}
			} else if (lsr & XR20M1280_LSR_PE_BIT)
				parity++;
			else if (lsr & XR20M1280_LSR_FE_BIT)
				frame++;
			else if (lsr & XR20M1280_LSR_OE_BIT)
				overrun++;

			lsr &= port->read_status_mask;
			if (lsr & XR20M1280_LSR_BI_BIT)
				flag = TTY_BREAK;
			else if (lsr & XR20M1280_LSR_PE_BIT)
				flag = TTY_PARITY;
			else if (lsr & XR20M1280_LSR_FE_BIT)
				flag = TTY_FRAME;
			else if (lsr & XR20M1280_LSR_OE_BIT)
				flag = TTY_OVERRUN;
		}

		for (i = 0; i < bytes_read; ++i) {
			ch = s->buf[i];
			if (uart_handle_sysrq_char(port, ch))
				continue;

			if (lsr & port->ignore_status_mask)
				continue;

			uart_insert_char(port, lsr, XR20M1280_LSR_OE_BIT, ch,
					 flag);
		}
		rxlen -= bytes_read;
	}

	spin_lock(&port->lock);
	port->icount.buf_overrun += buf_overrun;
	port->icount.rx += rx;
	port->icount.brk += brk;
	port->icount.parity += parity;
	port->icount.frame += frame;
	port->icount.overrun += overrun;
	spin_unlock(&port->lock);

	tty_flip_buffer_push(&port->state->port);
}

static void xr20m1280_do_stop_tx(struct uart_port *port)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	if (RTS_TOGGLE_ENABLED(one)) {
		ktime_t t = ktime_set(0, RTS_TOGGLE_LSR_POLL_INTERVAL);
		one->tx_state = xr20m1280_tx_state_polling;
		hrtimer_start(&one->stop_tx_timer, t, HRTIMER_MODE_REL);
	} else {
		one->tx_state = xr20m1280_tx_state_stopped;
	}
	return;
}

/* Register accesses in handle_tx are protected by reg_mode_mutex in ist */
static void xr20m1280_handle_tx(struct uart_port *port)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);
	struct circ_buf *xmit = &port->state->xmit;
	unsigned int txlen, to_send, i;
	
	txlen = xr20m1280_fifo_count(port, XR20M1280_FIFO_COUNT_TX);	

	if (one->tx_state != xr20m1280_tx_state_started) {
		return;
	}

	spin_lock(&port->lock);
	if (uart_circ_empty(xmit) || one->stopped || uart_tx_stopped(port)) {
		if (txlen == 0)
			xr20m1280_do_stop_tx(port);
		spin_unlock(&port->lock);
		return;
	}

	/* Get length of data pending in circular buffer */
	to_send = uart_circ_chars_pending(xmit);
	// JAP // printk(KERN_INFO "xr20m1280_handle_tx: to_send: %d\n", to_send); // JAP //
	// JAP // dump_register(port); // JAP //
	/* Limit to size of TX FIFO */
	// JAP // printk(KERN_INFO "xr20m1280_handle_tx: fifo reports %d bytes available\n", txlen); // JAP //
	if (txlen > XR20M1280_FIFO_SIZE) {
		dev_err_ratelimited(port->dev,
			"chip reports %d free bytes in TX fifo, but it only has %d",
			txlen, XR20M1280_FIFO_SIZE);
		txlen = 0;
	}
	txlen = XR20M1280_FIFO_SIZE - txlen;
	to_send = (to_send > txlen) ? txlen : to_send;

	/* Add data to send */
	port->icount.tx += to_send;

	/* Convert to linear buffer */
	for (i = 0; i < to_send; ++i) {
		s->buf[i] = xmit->buf[xmit->tail];
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	}
	// JAP // printk(KERN_INFO "xr20m1280_fifo_write: writing %d bytes\n", to_send); // JAP //
	spin_unlock(&port->lock);
	xr20m1280_fifo_write(port, to_send);
	spin_lock(&port->lock);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	spin_unlock(&port->lock);
}

static void xr20m1280_ier_set(struct uart_port *port, u8 bit)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	lockdep_assert_held(&port->lock);
	one->config.flags |= XR20M1280_RECONF_IER;
	one->config.ier_set |= bit;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void xr20m1280_ier_clear(struct uart_port *port, u8 bit)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	lockdep_assert_held(&port->lock);
	one->config.flags |= XR20M1280_RECONF_IER;
	one->config.ier_clear |= bit;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void xr20m1280_stop_tx(struct uart_port *port)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	one->stopped = true;
}

static void xr20m1280_stop_rx(struct uart_port *port)
{
	xr20m1280_ier_clear(port, XR20M1280_IER_RHRI_BIT);
}

static void xr20m1280_start_tx(struct uart_port *port)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	if ((one->estat & (RP_OPH | RP_TXB)) == 0) {
		one->stopped = false;
		kthread_queue_work(&s->kworker, &one->start_work);
	}
}

static void xr20m1280_check_ms(struct xr20m1280_one *one)
{
	unsigned int flow;
	lockdep_assert_held(&one->port.lock);
	flow = one->flow & (TIOCM_CTS | TIOCM_CD | TIOCM_DSR);
	if ((one->msr_cache & flow) != flow) {
		if ((one->estat & RP_OPH) == 0) {
			one->estat |= RP_OPH;
			if (flow != TIOCM_CTS)
				xr20m1280_stop_tx(&one->port);
		}
	} else if (one->estat & RP_OPH) {
		one->estat &= ~RP_OPH;
		if (flow != TIOCM_CTS)
			xr20m1280_start_tx(&one->port);
	}
}

/* Ensure that cached MSR is updated if changes are detected by UART */
static void xr20m1280_handle_msi(struct uart_port *port)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);
	unsigned int msr;
	unsigned int res = 0;

	msr = xr20m1280_port_read(port, XR20M1280_MSR_REG);

	if (msr & XR20M1280_MSR_CTS_BIT) res |= TIOCM_CTS;
	if (msr & XR20M1280_MSR_DSR_BIT) res |= TIOCM_DSR;
	if (msr & XR20M1280_MSR_CD_BIT)  res |= TIOCM_CAR;
	if (msr & XR20M1280_MSR_RI_BIT)  res |= TIOCM_RI;

	// JAP // printk(KERN_INFO "%02x %02x\n", msr, res); // JAP //

	spin_lock(&port->lock);
	one->msr_cache = res;
	if (msr & XR20M1280_MSR_DELTA_MASK) {
		if (msr & XR20M1280_MSR_DCTS_BIT)
		 	uart_handle_cts_change(port, msr & XR20M1280_MSR_CTS_BIT);
		if (msr & XR20M1280_MSR_DDSR_BIT)
			port->icount.dsr++;
		if ((msr & (XR20M1280_MSR_DRI_BIT | XR20M1280_MSR_RI_BIT)) == XR20M1280_MSR_DRI_BIT)
			port->icount.rng++;
		if (msr & XR20M1280_MSR_DCD_BIT)
			uart_handle_dcd_change(port, msr & XR20M1280_MSR_CD_BIT);
		wake_up_interruptible(&port->state->port.delta_msr_wait);
	}
	xr20m1280_check_ms(one);
	spin_unlock(&port->lock);
}

/* Register accesses in port_irq are protected by reg_mode_mutex in ist */
static bool xr20m1280_port_irq(struct xr20m1280_port *s, int portno)
{
	struct uart_port *port = &s->p[portno].port;

	do {
		unsigned int iir, rxlen;

		iir = xr20m1280_port_read(port, XR20M1280_ISR_REG);
		if (iir & XR20M1280_ISR_NO_INT_BIT)
			return false;

		iir &= XR20M1280_ISR_ID_MASK;

		switch (iir) {
		case XR20M1280_ISR_RHRI_SRC:
			xr20m1280_handle_rx(port, 16, iir);
			break;
		case XR20M1280_ISR_RLSE_SRC:
		case XR20M1280_ISR_RTOI_SRC:
			// JAP // dump_register(port); // JAP //
			// JAP // printk(KERN_INFO "%02x\n", iir); // JAP //
			rxlen = xr20m1280_fifo_count(port, XR20M1280_FIFO_COUNT_RX);
			if (rxlen)
				xr20m1280_handle_rx(port, rxlen, iir);
			break;
		case XR20M1280_ISR_THRI_SRC:
			xr20m1280_handle_tx(port);
			break;
		case XR20M1280_ISR_MSI_SRC:
			xr20m1280_handle_msi(port);
			break;
		default:
			dev_err_ratelimited(port->dev,
			                    "%s: Unexpected interrupt: %x",
			                    port->name, iir);
			break;
		}
	} while (0);
	return true;
}

static void xr20m1280_ist(struct kthread_work *ws)
{
	struct xr20m1280_port *s = to_xr20m1280_port(ws, irq_work);

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

	while (1) {
		bool keep_polling = false;
		int i;

		for (i = 0; i < s->devtype->nr_uart; ++i)
			keep_polling |= xr20m1280_port_irq(s, i);
		if (!keep_polling)
			break;
	}

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);
}

static irqreturn_t xr20m1280_irq(int irq, void *dev_id)
{
	struct xr20m1280_port *s = (struct xr20m1280_port *)dev_id;

	// JAP // printk(KERN_INFO "xr20m1280_irq\n");
	kthread_queue_work(&s->kworker, &s->irq_work);

	return IRQ_HANDLED;
}

static enum hrtimer_restart xr20m1280_queue_stop_tx(struct hrtimer *t)
{
	struct xr20m1280_one *one = to_xr20m1280_one(t, stop_tx_timer);
	struct uart_port *port = &(one->port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	kthread_queue_work(&s->kworker, &one->stop_work);

	return HRTIMER_NORESTART;
}

static void xr20m1280_stop_tx_proc(struct kthread_work *ws)
{
	u8 lsr;
	struct xr20m1280_one *one = to_xr20m1280_one(ws, stop_work);
	struct uart_port *port = &(one->port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	mutex_lock(&s->reg_mode_mutex);
	if (one->tx_state == xr20m1280_tx_state_polling) {
		lsr = xr20m1280_port_read(port, XR20M1280_LSR_REG);
		if (lsr & XR20M1280_LSR_TEMT_BIT) {
			unsigned long delay;
			one->tx_state = xr20m1280_tx_state_stopping;
			spin_lock(&port->lock);
			delay = one->rts_toggle.postdelay;
			spin_unlock(&port->lock);
			if (delay) {
				start_hrtimer_ms(&one->stop_tx_timer, delay);
				mutex_unlock(&s->reg_mode_mutex);
				return;
			}
		} else {
			ktime_t t = ktime_set(0, RTS_TOGGLE_LSR_POLL_INTERVAL);
			hrtimer_start(&one->stop_tx_timer, t, HRTIMER_MODE_REL);
		}
	}
	if (one->tx_state == xr20m1280_tx_state_stopping) {
		one->tx_state = xr20m1280_tx_state_stopped;
		xr20m1280_port_update(&one->port, XR20M1280_MCR_REG, XR20M1280_MCR_RTS_BIT, 0);
	}
	mutex_unlock(&s->reg_mode_mutex);
}

static enum hrtimer_restart xr20m1280_queue_start_tx(struct hrtimer *t)
{
	struct xr20m1280_one *one = to_xr20m1280_one(t, start_tx_timer);
	struct uart_port *port = &(one->port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	kthread_queue_work(&s->kworker, &one->tx_work);

	return HRTIMER_NORESTART;
}

static void xr20m1280_tx_proc(struct kthread_work *ws)
{
	struct xr20m1280_one *one = to_xr20m1280_one(ws, tx_work);
	struct uart_port *port = &(one->port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	if (one->tx_state == xr20m1280_tx_state_starting) {
		one->tx_state = xr20m1280_tx_state_started;
	}

	if (one->tx_state == xr20m1280_tx_state_started) {
		mutex_lock(&s->reg_mode_mutex);
		xr20m1280_handle_tx(port);
		mutex_unlock(&s->reg_mode_mutex);
	}
}

static void xr20m1280_start_proc(struct kthread_work *ws)
{
	struct xr20m1280_one *one = to_xr20m1280_one(ws, start_work);
	struct uart_port *port = &(one->port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct circ_buf *xmit = &port->state->xmit;
	bool raise_rts = false;
	unsigned long delay = 0;

	if (one->tx_state == xr20m1280_tx_state_stopped) {
		spin_lock(&port->lock);
		if (uart_circ_empty(xmit) || one->stopped || uart_tx_stopped(port)) {
			spin_unlock(&port->lock);
			return;
		}
		one->tx_state = xr20m1280_tx_state_starting;
	
		if (RTS_TOGGLE_ENABLED(one)) {
			raise_rts = true;
			delay = one->rts_toggle.predelay;
			one->port.mctrl |= TIOCM_RTS;
			spin_unlock(&port->lock);
			mutex_lock(&s->reg_mode_mutex);
			xr20m1280_port_update(&one->port, XR20M1280_MCR_REG, XR20M1280_MCR_RTS_BIT, XR20M1280_MCR_RTS_BIT);
			mutex_unlock(&s->reg_mode_mutex);
			if (delay) {
				start_hrtimer_ms(&one->start_tx_timer, delay);
				return;
			}
		} else
			spin_unlock(&port->lock);
		kthread_queue_work(&s->kworker, &one->tx_work);
	} else if (one->tx_state != xr20m1280_tx_state_starting) {
		one->tx_state = xr20m1280_tx_state_started;
		kthread_queue_work(&s->kworker, &one->tx_work);
	}
}

static void xr20m1280_reconf_rs485(struct uart_port *port)
{
	const u8 fctr_mask = XR20M1280_FCTR_AUTO_RS485_BIT;
	u8 fctr = 0;
	u8 lcr;
	struct serial_rs485 *rs485;

	spin_lock(&port->lock);
	rs485 = &port->rs485;
	if (rs485->flags & SER_RS485_ENABLED) {
		fctr = fctr_mask;

		/*
		 * Connect EZ Mini HW does not support RTS inversion,
		 * ignore RTS_ON_SEND and RTS_AFTER_SEND
		 */
	}
	spin_unlock(&port->lock);

	/* FCTR only if LCR == 0xBF */
	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
	xr20m1280_port_write(port, XR20M1280_LCR_REG,
			     XR20M1280_LCR_CONF_MODE_B);

	xr20m1280_port_update(port, XR20M1280_FCTR_REG, fctr_mask, fctr);

	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
}

static void xr20m1280_reg_proc(struct kthread_work *ws)
{
	struct xr20m1280_one *one = to_xr20m1280_one(ws, reg_work);
	struct xr20m1280_port *s = dev_get_drvdata(one->port.dev);
	struct xr20m1280_one_config config;
	int mctrl;

	u8 mask = XR20M1280_MCR_LOOP_BIT | XR20M1280_MCR_RTS_BIT | XR20M1280_MCR_DTR_BIT;
	spin_lock(&one->port.lock);
	config = one->config;
	memset(&one->config, 0, sizeof(one->config));
	if ((one->port.rs485.flags & SER_RS485_ENABLED) || one->rts_toggle.enable)
		mask = XR20M1280_MCR_LOOP_BIT | XR20M1280_MCR_DTR_BIT;
	if (config.flags & XR20M1280_RECONF_RTS_TOGGLE) {
		if (RTS_TOGGLE_ENABLED(one)) {
			if (one->tx_state == xr20m1280_tx_state_stopped && (one->port.mctrl & TIOCM_RTS) == TIOCM_RTS) {
				one->port.mctrl &= ~TIOCM_RTS;
				config.flags |= XR20M1280_RECONF_MD;
			} else if ((one->port.mctrl & TIOCM_RTS) == 0) {
				one->port.mctrl |= TIOCM_RTS;
				config.flags |= XR20M1280_RECONF_MD;
			}
			mask = XR20M1280_MCR_LOOP_BIT | XR20M1280_MCR_RTS_BIT | XR20M1280_MCR_DTR_BIT;
		} else if (one->tx_state == xr20m1280_tx_state_polling || one->tx_state == xr20m1280_tx_state_stopping) {
			one->tx_state = xr20m1280_tx_state_stopped;
		}
	}
	mctrl = one->port.mctrl;
	spin_unlock(&one->port.lock);

	mutex_lock(&s->reg_mode_mutex);

	if (config.flags & XR20M1280_RECONF_MD) {
		u8 val = 0;

		if (mctrl & TIOCM_LOOP)
			val |=  XR20M1280_MCR_LOOP_BIT;
		if (mctrl & TIOCM_RTS)
			val |= XR20M1280_MCR_RTS_BIT;
		if (mctrl & TIOCM_DTR)
			val |= XR20M1280_MCR_DTR_BIT;
		
		xr20m1280_port_update(&one->port, XR20M1280_MCR_REG,
				      mask,
				      val);
	}
	if (config.flags & XR20M1280_RECONF_IER) {
		xr20m1280_port_update(&one->port, XR20M1280_IER_REG,
				      config.ier_clear | config.ier_set,
				      config.ier_set);
	}

	if (config.flags & XR20M1280_RECONF_RS485)
		xr20m1280_reconf_rs485(&one->port);

	mutex_unlock(&s->reg_mode_mutex);
}

static unsigned int xr20m1280_tx_empty(struct uart_port *port)
{
	unsigned int lsr;
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	/* FIXME: Documentation/serial/driver claims this can't sleep */
	lockdep_assert_irqs_enabled();
	mutex_lock(&s->reg_mode_mutex);
	lsr = xr20m1280_port_read(port, XR20M1280_LSR_REG);
	mutex_unlock(&s->reg_mode_mutex);

	return (lsr & XR20M1280_LSR_TEMT_BIT) ? TIOCSER_TEMT : 0;
}

static unsigned int xr20m1280_get_mctrl(struct uart_port *port)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	lockdep_assert_held(&port->lock);
	return one->msr_cache;
}

static void xr20m1280_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	lockdep_assert_held(&port->lock);
	one->config.flags |= XR20M1280_RECONF_MD;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void xr20m1280_break_ctl(struct uart_port *port, int break_state)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	mutex_lock(&s->reg_mode_mutex);
	xr20m1280_port_update(port, XR20M1280_LCR_REG,
			      XR20M1280_LCR_TXBREAK_BIT,
			      break_state ? XR20M1280_LCR_TXBREAK_BIT : 0);
	mutex_unlock(&s->reg_mode_mutex);
}

/* Expects LCR in CONF_MODE_B */
static void xr20m1280_configure_flow_control(struct uart_port *port, struct ktermios *termios)
{
	unsigned int flow = 0;
	
	if (termios->c_cflag & CRTSCTS)
		flow |= XR20M1280_EFR_AUTORTS_BIT | XR20M1280_EFR_AUTOCTS_BIT;

	/* Can't handle IXANY or VLNEXT in HW */
	if (((termios->c_iflag & (IXON | IXANY)) == IXON) &&
		((termios->c_cc[VLNEXT] == __DISABLED_CHAR || (termios->c_lflag & (ICANON | IEXTEN)) != (ICANON | IEXTEN)) &&
		termios->c_cc[VSTART] != __DISABLED_CHAR && termios->c_cc[VSTOP] != __DISABLED_CHAR))
	{

		xr20m1280_port_write(port, XR20M1280_XON1_REG, termios->c_cc[VSTART]);
		xr20m1280_port_write(port, XR20M1280_XOFF1_REG, termios->c_cc[VSTOP]);
		flow |= XR20M1280_EFR_SWFLOW3_BIT;
	}

	/* Reset software flow bits to zero before writing a new setting */
	xr20m1280_port_update(port, XR20M1280_EFR_REG,
			      XR20M1280_EFR_AUTORTS_BIT | XR20M1280_EFR_AUTOCTS_BIT | XR20M1280_EFR_SWFLOW3_BIT,
			      0);

	if (flow) {
		/* Update EFR register for automatic flow control */
		xr20m1280_port_update(port, XR20M1280_EFR_REG,
				      XR20M1280_EFR_AUTORTS_BIT | XR20M1280_EFR_AUTOCTS_BIT | XR20M1280_EFR_SWFLOW3_BIT,
				      flow);
	}
}

static void xr20m1280_set_ldisc(struct uart_port *port, struct ktermios *termios)
{
	u8 lcr;
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);
	
	spin_lock(&port->lock);
	one->flow = 0;
	if (termios->c_line != N_REALPORT)
		port->status &= ~UPSTAT_SYNC_FIFO;
	else
		port->status |= UPSTAT_SYNC_FIFO; /* Call throttle() even when no flow control */
	spin_unlock(&port->lock);

	mutex_lock(&s->reg_mode_mutex);
	
	lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);

	xr20m1280_port_write(port, XR20M1280_LCR_REG,
			     XR20M1280_LCR_CONF_MODE_B);

	if (termios->c_line != N_REALPORT)
		xr20m1280_configure_flow_control(port, termios);
	else
		xr20m1280_port_update(port, XR20M1280_EFR_REG,
			      XR20M1280_EFR_AUTORTS_BIT | XR20M1280_EFR_AUTOCTS_BIT | XR20M1280_EFR_SWFLOW3_BIT,
			      0);

	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);

	mutex_unlock(&s->reg_mode_mutex);
}

static void xr20m1280_set_termios(struct uart_port *port,
				  struct ktermios *termios,
				  struct ktermios *old)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	unsigned int lcr;
	int baud;

	/* Mask termios capabilities we don't support */
	termios->c_cflag &= ~CMSPAR;

	/* Word size */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		lcr = XR20M1280_LCR_WORD_LEN_5;
		break;
	case CS6:
		lcr = XR20M1280_LCR_WORD_LEN_6;
		break;
	case CS7:
		lcr = XR20M1280_LCR_WORD_LEN_7;
		break;
	case CS8:
		lcr = XR20M1280_LCR_WORD_LEN_8;
		break;
	default:
		lcr = XR20M1280_LCR_WORD_LEN_8;
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS8;
		break;
	}

	/* Parity */
	if (termios->c_cflag & PARENB) {
		lcr |= XR20M1280_LCR_PAR_BIT;
		if (!(termios->c_cflag & PARODD))
			lcr |= XR20M1280_LCR_EVENPAR_BIT;
	}

	/* Stop bits */
	if (termios->c_cflag & CSTOPB)
		lcr |= XR20M1280_LCR_STOPLEN_BIT; /* 2 stops */

	/* Set status ignore mask */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNBRK)
		port->ignore_status_mask |= XR20M1280_LSR_BI_BIT;
	if (!(termios->c_cflag & CREAD))
		port->ignore_status_mask |= XR20M1280_LSR_BRK_ERROR_MASK;

	/* Set read status mask */
	port->read_status_mask = port->ignore_status_mask | XR20M1280_LSR_OE_BIT;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= XR20M1280_LSR_PE_BIT |
					  XR20M1280_LSR_FE_BIT;
	if (termios->c_iflag & (BRKINT | PARMRK))
		port->read_status_mask |= XR20M1280_LSR_BI_BIT;

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

	xr20m1280_port_write(port, XR20M1280_LCR_REG,
			     XR20M1280_LCR_CONF_MODE_B);
			     
	spin_lock(&port->lock);
	if (termios->c_cflag & CRTSCTS)
		port->status |= UPSTAT_AUTORTS | UPSTAT_AUTOCTS;
	else
		port->status &= ~(UPSTAT_AUTORTS | UPSTAT_AUTOCTS);
	if (termios->c_iflag & IXOFF)
		port->status |= UPSTAT_AUTOXOFF; /* xr20m1280_throttle handles XOFF to ensure '\0' chars work in RealPort */
	else
		port->status &= ~UPSTAT_AUTOXOFF;
	spin_unlock(&port->lock);

	if (termios->c_line != N_REALPORT)
		xr20m1280_configure_flow_control(port, termios);

	/* Update LCR register, simultaneously restoring register mode */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);

	/* Get baud rate generator configuration */
	baud = uart_get_baud_rate(port, termios, old,
				  port->uartclk / 16 / 4 / 0xffff,
				  port->uartclk / 16);

	/* Setup baudrate generator */
	baud = xr20m1280_set_baud(port, baud);

	/* Update timeout according to new baud rate */
	uart_update_timeout(port, termios->c_cflag, baud);
}

static int xr20m1280_config_rs485(struct uart_port *port,
				  struct serial_rs485 *rs485)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	if (rs485->flags & SER_RS485_ENABLED) {
		/*
		 * Connect EZ Mini requires SER_RS485_RTS_AFTER_SEND,
		 * because RTS must be logic low during transmission.
		 * This matches standard use of RTS, which would indicate
		 * to the remote when we are capable of receiving.
		 */
		bool rts_during_rx, rts_during_tx;

		rts_during_rx = rs485->flags & SER_RS485_RTS_AFTER_SEND;
		rts_during_tx = rs485->flags & SER_RS485_RTS_ON_SEND;

		if (rts_during_rx == rts_during_tx)
			dev_err(port->dev,
				"unsupported RTS signalling on_send:%d after_send:%d - exactly one of RS485 RTS flags should be set\n",
				rts_during_tx, rts_during_rx);

		/* Change flags to match our capability */
		rs485->flags &= ~SER_RS485_RTS_ON_SEND;
		rs485->flags |= SER_RS485_RTS_AFTER_SEND;

		/*
		 * RTS signal is handled by HW, it's timing can't be influenced.
		 */
		rs485->delay_rts_before_send = 0;
		rs485->delay_rts_after_send = 0;
	}
	if ((port->rs485.flags & SER_RS485_ENABLED) != (rs485->flags & SER_RS485_ENABLED) && one->rts_toggle.enable)
		one->config.flags |= XR20M1280_RECONF_RTS_TOGGLE;

	lockdep_assert_held(&port->lock);
	port->rs485 = *rs485;
	one->config.flags |= XR20M1280_RECONF_RS485;
	kthread_queue_work(&s->kworker, &one->reg_work);

	return 0;
}

static int xr20m1280_startup(struct uart_port *port)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	unsigned int val;

	xr20m1280_power(port, 1);

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

	/* Reset FIFOs*/
	val = XR20M1280_FCR_RXRESET_BIT | XR20M1280_FCR_TXRESET_BIT;
	xr20m1280_port_write(port, XR20M1280_FCR_REG, val);
	udelay(5);
	/* Enable FIFOs & set FIFO interrupt trigger levels (RX=16, TX=32) */
	xr20m1280_port_write(port, XR20M1280_FCR_REG, XR20M1280_FCR_DEFAULT);

	/* Enable EFR */
	xr20m1280_port_write(port, XR20M1280_LCR_REG,
			     XR20M1280_LCR_CONF_MODE_B);

	/* Enable write access to enhanced features and internal clock div */
	xr20m1280_port_update(port, XR20M1280_EFR_REG,
			      XR20M1280_EFR_ENABLE_BIT,
			      XR20M1280_EFR_ENABLE_BIT);

	/* Set scratch pad to known value for debug */
	xr20m1280_port_update(port, XR20M1280_FCTR_REG, 0, XR20M1280_FCTR_SPSW_BIT);
	xr20m1280_port_write(port, XR20M1280_SPR_REG, 0xFF);

	/* Disable scratch pad register */
	/* Set FIFO interrupt trigger level table (C) */
	xr20m1280_port_update(port, XR20M1280_FCTR_REG,
			     XR20M1280_FCTR_TRIGTAB_C|XR20M1280_FCTR_SPSW_BIT,
			     XR20M1280_FCTR_TRIGTAB_C|XR20M1280_FCTR_SPSW_BIT);

#if 0 /* TODO */
	/* Configure flow control levels */
	/* Flow control halt level 48, resume level 24 */
	xr20m1280_port_write(port, XR20M1280_TCR_REG,
			     XR20M1280_TCR_RX_RESUME(24) |
			     XR20M1280_TCR_RX_HALT(48));
#endif

	/* Latch enhanced features, then re-enable enhanced feature registers */
	xr20m1280_port_update(port, XR20M1280_EFR_REG, XR20M1280_EFR_ENABLE_BIT, 0);
	xr20m1280_port_update(port, XR20M1280_EFR_REG,
			      XR20M1280_EFR_ENABLE_BIT,
			      XR20M1280_EFR_ENABLE_BIT);

	/* Now, initialize the UART */
	xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_WORD_LEN_8);
	xr20m1280_port_write(port, XR20M1280_SFR_REG, XR20M1280_SFR_DEFAULT);

	/* Initialize MSR cache for port by simulating modem status interrupt*/
	xr20m1280_handle_msi(port);

	/* Enable RX, TX, LSR, MSI interrupts */
	val = XR20M1280_IER_RHRI_BIT | XR20M1280_IER_THRI_BIT | 
	      XR20M1280_IER_RLSI_BIT | XR20M1280_IER_MSI_BIT;
	xr20m1280_port_write(port, XR20M1280_IER_REG, val);

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);

	return 0;
}

static void xr20m1280_shutdown(struct uart_port *port)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	/* Must flush work to ensure any user initiated functions are actually complete */
	kthread_flush_worker(&s->kworker);

	/* Grab lock protecting interrupt handling from mode changes */
	mutex_lock(&s->reg_mode_mutex);

	/* Disable all interrupts */
	xr20m1280_port_write(port, XR20M1280_IER_REG, 0);

	/* Disable break */
	xr20m1280_port_update(port, XR20M1280_LCR_REG, XR20M1280_LCR_TXBREAK_BIT, 0);

	/* Flush FIFO */
	xr20m1280_port_write(port, XR20M1280_FCR_REG, XR20M1280_FCR_DEFAULT | XR20M1280_FCR_TXRESET_BIT);

	/* Disable TX/RX */
	xr20m1280_port_write(port, XR20M1280_SFR_REG,
			     XR20M1280_SFR_DEFAULT |
			     XR20M1280_SFR_RXDISABLE_BIT |
			     XR20M1280_SFR_TXDISABLE_BIT);

	/* Release lock protecting interrupt handling from mode changes */
	mutex_unlock(&s->reg_mode_mutex);

	spin_lock(&port->lock);
	uart_circ_clear(&port->state->xmit);
	spin_unlock(&port->lock);
	/* Kick tx_work so it notices tx is done and updates tx_state */
	kthread_queue_work(&s->kworker, &one->tx_work);

	/* FIXME: We may still need to lower RTS later. Would this still be safe if it actually did anything? */
	xr20m1280_power(port, 0);
}

static const char *xr20m1280_type(struct uart_port *port)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	return (port->type == PORT_XR20M1280) ? s->devtype->name : NULL;
}

static int xr20m1280_request_port(struct uart_port *port)
{
	/* Do nothing */
	return 0;
}

static void xr20m1280_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_XR20M1280;
}

static int xr20m1280_verify_port(struct uart_port *port,
				 struct serial_struct *s)
{
	if ((s->type != PORT_UNKNOWN) && (s->type != PORT_XR20M1280))
		return -EINVAL;
	if (s->irq != port->irq)
		return -EINVAL;

	return 0;
}

#ifdef CONFIG_PM
static void xr20m1280_pm(struct uart_port *port, unsigned int state,
			 unsigned int oldstate)
{
	xr20m1280_power(port, (state == UART_PM_STATE_ON) ? 1 : 0);
}
#endif

static void xr20m1280_null_void(struct uart_port *port)
{
	/* Do nothing */
}

static void xr20m1280_throttle_ms(struct uart_port *port)
{
	unsigned int old;
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);
	
	old = port->mctrl;
	port->mctrl &= ~(one->flow & (TIOCM_DTR | TIOCM_RTS));
	if (port->mctrl != old)
		xr20m1280_set_mctrl(port, port->mctrl);
}

static void xr20m1280_unthrottle_ms(struct uart_port *port)
{
	unsigned int old;
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);

	old = port->mctrl;
	port->mctrl |= one->flow & (TIOCM_DTR | TIOCM_RTS);
	if (port->mctrl != old)
		xr20m1280_set_mctrl(port, port->mctrl);
}

static void xr20m1280_send_xchar(struct uart_port *port, char ch)
{
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	lockdep_assert_held(&port->state->port.tty->termios_rwsem);
	if (ch == __DISABLED_CHAR && port->state->port.tty->termios.c_line != N_REALPORT)
		return;
	mutex_lock(&s->reg_mode_mutex);
	xr20m1280_port_write(port, XR20M1280_EMSR_REG, XR20M1280_EMSR_TXI_BIT);
	xr20m1280_port_write(port, XR20M1280_THR_REG, ch);
	mutex_unlock(&s->reg_mode_mutex);
}

static int xr20m1280_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg)
{
	struct xr20m1280_one *one = to_xr20m1280_one(port, port);
	struct xr20m1280_port *s = dev_get_drvdata(port->dev);

	switch(cmd) {
	case DIGI_REALPORT_SEND_IMMEDIATE: {
		char immed;
		if (copy_from_user(&immed, (char __user *) arg, sizeof(immed)))
			return -EFAULT;
		xr20m1280_send_xchar(port, immed);
		return 0;
	}
	case DIGI_REALPORT_SET_FLOW_SIGNALS: {
		unsigned int sigs;
		u8 lcr, flow = 0;
		if(copy_from_user(&sigs, (unsigned int __user *) arg, sizeof(sigs)))
			return -EFAULT;
		/* HW does not interrupt on RI change so can't use it for flow control */
		sigs &= ~TIOCM_RI;
		mutex_lock(&s->reg_mode_mutex);
		lcr = xr20m1280_port_read(port, XR20M1280_LCR_REG);
		xr20m1280_port_write(port, XR20M1280_LCR_REG, XR20M1280_LCR_CONF_MODE_B);
		if ((sigs & (TIOCM_DTR | TIOCM_RTS)) == TIOCM_RTS) {
			flow |= XR20M1280_EFR_AUTORTS_BIT;
			sigs &= ~TIOCM_RTS;
		}
		if ((sigs & (TIOCM_CTS | TIOCM_CD | TIOCM_DSR)) == TIOCM_CTS) {
			flow |= XR20M1280_EFR_AUTOCTS_BIT;
		}
		xr20m1280_port_update(port, XR20M1280_EFR_REG, XR20M1280_EFR_AUTORTS_BIT | XR20M1280_EFR_AUTOCTS_BIT, flow);
		xr20m1280_port_write(port, XR20M1280_LCR_REG, lcr);
		mutex_unlock(&s->reg_mode_mutex);
		spin_lock(&port->lock);
		one->flow = sigs;
		if (tty_throttled(port->state->port.tty))
			xr20m1280_throttle_ms(port);
		else
			xr20m1280_unthrottle_ms(port);
		xr20m1280_check_ms(one);
		spin_unlock(&port->lock);
		return 0;
	}
	case DIGI_REALPORT_GET_HW_STATS: {
		struct realport_hw_stats stats;
		stats.tiocser_temt = 1;
		spin_lock(&port->lock);
		stats.tiocmget = xr20m1280_get_mctrl(port);
		stats.counters.norun = port->icount.overrun;
		stats.counters.noflow = port->icount.buf_overrun;
		stats.counters.nframe = port->icount.frame;
		stats.counters.nparity = port->icount.parity;
		stats.counters.nbreak = port->icount.brk;
		stats.tiocoutq = uart_circ_chars_pending(&port->state->xmit);
		stats.estat = one->estat;
		spin_unlock(&port->lock);
		return copy_to_user((struct realport_hw_stats __user *) arg, &stats, sizeof(stats));
	}
	case DIGI_REALPORT_START_BREAK: {
		struct tty_struct *tty = port->state->port.tty;
		spin_lock(&port->lock);
		xr20m1280_stop_tx(port);
		one->estat |= RP_TXB;
		spin_unlock(&port->lock);
		tty->ops->wait_until_sent(tty, 0);
		xr20m1280_break_ctl(port, 1);
		return 0;
	}
	case DIGI_REALPORT_STOP_BREAK: {
		xr20m1280_break_ctl(port, 0);
		spin_lock(&port->lock);
		one->estat &= ~RP_TXB;
		xr20m1280_start_tx(port);
		spin_unlock(&port->lock);
		return 0;
	}
	case DIGI_RTS_TOGGLE_CONFIG: {
		struct serial_rts_toggle config;
		if (copy_from_user(&config, (struct serial_rts_toggle __user *) arg, sizeof(config)))
			return -EFAULT;
		spin_lock(&port->lock);
		if (one->rts_toggle.enable != config.enable) {
			one->config.flags |= XR20M1280_RECONF_RTS_TOGGLE;
			kthread_queue_work(&s->kworker, &one->reg_work);
		}
		one->rts_toggle = config;
		spin_unlock(&port->lock);
		return 0;
	}
	}
	return -ENOIOCTLCMD;
}

static void xr20m1280_throttle(struct uart_port *port)
{
	spin_lock(&port->lock);
	xr20m1280_throttle_ms(port);
	spin_unlock(&port->lock);
	if (I_IXOFF(port->state->port.tty)) {
		xr20m1280_send_xchar(port, STOP_CHAR(port->state->port.tty));
	}
}

static void xr20m1280_unthrottle(struct uart_port *port)
{
	spin_lock(&port->lock);
	xr20m1280_unthrottle_ms(port);
	spin_unlock(&port->lock);
	if (I_IXOFF(port->state->port.tty)) {
		xr20m1280_send_xchar(port, START_CHAR(port->state->port.tty));
	}
}

static const struct uart_ops xr20m1280_ops = {
	.tx_empty	= xr20m1280_tx_empty,
	.set_mctrl	= xr20m1280_set_mctrl,
	.get_mctrl	= xr20m1280_get_mctrl,
	.stop_tx	= xr20m1280_stop_tx,
	.start_tx	= xr20m1280_start_tx,
	.stop_rx	= xr20m1280_stop_rx,
	.break_ctl	= xr20m1280_break_ctl,
	.startup	= xr20m1280_startup,
	.shutdown	= xr20m1280_shutdown,
	.set_termios	= xr20m1280_set_termios,
	.type		= xr20m1280_type,
	.request_port	= xr20m1280_request_port,
	.release_port	= xr20m1280_null_void,
	.config_port	= xr20m1280_config_port,
	.verify_port	= xr20m1280_verify_port,
#ifdef CONFIG_PM
	.pm		= xr20m1280_pm,
#endif
	.ioctl		= xr20m1280_ioctl,
	.throttle	= xr20m1280_throttle,
	.unthrottle	= xr20m1280_unthrottle,
	.send_xchar	= xr20m1280_send_xchar,
	.set_ldisc	= xr20m1280_set_ldisc
};


static const char *uart_type(struct uart_port *port)
{
	const char *str = NULL;

	if (port->ops->type)
		str = port->ops->type(port);

	if (!str)
		str = "unknown";

	return str;
}

static inline void
xr20m1280_report_port(struct uart_driver *drv, struct uart_port *port)
{
	char address[64];

	switch (port->iotype) {
	case UPIO_PORT:
		snprintf(address, sizeof(address), "I/O");
		break;
	default:
		strlcpy(address, "*unknown*", sizeof(address));
		break;
	}

	pr_info("%s%s%s at %s (irq = %d, base_baud = %d) is a %s\n",
	       port->dev ? dev_name(port->dev) : "",
	       port->dev ? ": " : "",
	       port->name,
	       address, port->irq, port->uartclk / 16, uart_type(port));
}

static int xr20m1280_probe(struct device *dev,
			   const struct xr20m1280_devtype *devtype,
			   struct regmap *regmap, int irq, unsigned long flags)
{
	struct sched_param sched_param = { .sched_priority = MAX_RT_PRIO / 2 };
	unsigned long freq = 0, *pfreq = dev_get_platdata(dev);
	u32 uartclk = 0;
	int i, ret;
	struct xr20m1280_port *s;

	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	/* Alloc port structure */
	s = devm_kzalloc(dev, sizeof(*s) +
			 sizeof(struct xr20m1280_one) * devtype->nr_uart,
			 GFP_KERNEL);
	if (!s) {
		dev_err(dev, "Error allocating port structure\n");
		return -ENOMEM;
	}

	/* Always ask for fixed clock rate from a property. */
	device_property_read_u32(dev, "clock-frequency", &uartclk);

	s->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(s->clk)) {
		if (uartclk)
			freq = uartclk;
		if (pfreq)
			freq = *pfreq;
		if (freq)
			dev_dbg(dev, "Clock frequency: %luHz\n", freq);
		else
			return PTR_ERR(s->clk);
	} else {
		ret = clk_prepare_enable(s->clk);
		if (ret)
			return ret;

		freq = clk_get_rate(s->clk);
	}

	s->regmap = regmap;
	s->devtype = devtype;
	dev_set_drvdata(dev, s);
	mutex_init(&s->reg_mode_mutex);

	kthread_init_worker(&s->kworker);
	kthread_init_work(&s->irq_work, xr20m1280_ist);
	s->kworker_task = kthread_run(kthread_worker_fn, &s->kworker,
				      XR20M1280_NAME);
	if (IS_ERR(s->kworker_task)) {
		ret = PTR_ERR(s->kworker_task);
		goto out_clk;
	}
	sched_setscheduler(s->kworker_task, SCHED_FIFO, &sched_param);

#ifdef CONFIG_GPIOLIB
    if (devtype->nr_gpio) {
        /* Setup GPIO cotroller */
        s->gpio.owner            = THIS_MODULE;
        s->gpio.label            = XR20M1280_NAME;
        s->gpio.direction_input  = xr20m1280_gpio_direction_input;
        s->gpio.get              = xr20m1280_gpio_get;
        s->gpio.direction_output = xr20m1280_gpio_direction_output;
        s->gpio.set              = xr20m1280_gpio_set;
        s->gpio.base             = -1;
        s->gpio.ngpio            = devtype->nr_gpio;
        s->gpio.can_sleep        = 1;
        ret = gpiochip_add(&s->gpio);
        if (ret)
            goto out_clk;
    }
#endif

	/* reset device, purging any pending irq / data */
	/* TODO -- soft reset?  Just reset FIFOs? */
/*	regmap_write(s->regmap, XR20M1280_IOCONTROL_REG << XR20M1280_REG_SHIFT,
			XR20M1280_IOCONTROL_SRESET_BIT);*/

	for (i = 0; i < devtype->nr_uart; ++i) {
		s->p[i].line		= i;
		/* Initialize port data */
		s->p[i].port.dev	= dev;
		s->p[i].port.irq	= irq;
		s->p[i].port.type	= PORT_XR20M1280;
		s->p[i].port.fifosize	= XR20M1280_FIFO_SIZE;
		s->p[i].port.flags	= UPF_FIXED_TYPE | UPF_LOW_LATENCY;
		s->p[i].port.iotype	= UPIO_PORT;
		s->p[i].port.uartclk	= freq;
		s->p[i].port.rs485_config = xr20m1280_config_rs485;
		s->p[i].port.ops	= &xr20m1280_ops;
		s->p[i].port.line	= xr20m1280_alloc_line();
		if (s->p[i].port.line >= XR20M1280_MAX_DEVS) {
			ret = -ENOMEM;
			goto out_ports;
		}

		/* Disable all interrupts */
		xr20m1280_port_write(&s->p[i].port, XR20M1280_IER_REG, 0);

		/* Disable TX/RX */
		/* TODO -- SFR only if LCR != 0xBF && EFR[4] == 1 */
		xr20m1280_port_write(&s->p[i].port, XR20M1280_SFR_REG,
				     XR20M1280_SFR_DEFAULT |
				     XR20M1280_SFR_RXDISABLE_BIT |
				     XR20M1280_SFR_TXDISABLE_BIT);
		/* Initialize kthread work structs */
		kthread_init_work(&s->p[i].tx_work, xr20m1280_tx_proc);
		kthread_init_work(&s->p[i].stop_work, xr20m1280_stop_tx_proc);
		kthread_init_work(&s->p[i].reg_work, xr20m1280_reg_proc);
		kthread_init_work(&s->p[i].start_work, xr20m1280_start_proc);
		
		hrtimer_init(&s->p[i].stop_tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		hrtimer_init(&s->p[i].start_tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		s->p[i].stop_tx_timer.function = xr20m1280_queue_stop_tx;
		s->p[i].start_tx_timer.function = xr20m1280_queue_start_tx;

		/* Register port */
		uart_add_one_port(&xr20m1280_uart, &s->p[i].port);
		xr20m1280_report_port(&xr20m1280_uart, &s->p[i].port);

		/* Enable EFR */
		xr20m1280_port_write(&s->p[i].port, XR20M1280_LCR_REG,
				     XR20M1280_LCR_CONF_MODE_B);

		/* Enable write access to enhanced features */
		xr20m1280_port_write(&s->p[i].port, XR20M1280_EFR_REG,
				     XR20M1280_EFR_ENABLE_BIT);

		/* Restore access to general registers */
		xr20m1280_port_write(&s->p[i].port, XR20M1280_LCR_REG, 0x00);

		/* Go to suspend mode */
		xr20m1280_power(&s->p[i].port, 0);
	}

	/* Setup interrupt */
	ret = devm_request_irq(dev, irq, xr20m1280_irq,
			       flags, dev_name(dev), s);

	//dump_register(&s->p[0].port);
	if (!ret)
		return 0;

out_ports:
#ifdef CONFIG_GPIOLIB
    if (devtype->nr_gpio)
        gpiochip_remove(&s->gpio);
#endif
	for (i--; i >= 0; i--) {
		uart_remove_one_port(&xr20m1280_uart, &s->p[i].port);
		clear_bit(s->p[i].port.line, &xr20m1280_lines);
	}
	kthread_stop(s->kworker_task);

out_clk:
	if (!IS_ERR(s->clk))
		clk_disable_unprepare(s->clk);

	return ret;
}

static int xr20m1280_remove(struct device *dev)
{
	struct xr20m1280_port *s = dev_get_drvdata(dev);
	int i;

#ifdef CONFIG_GPIOLIB
    if (s->devtype->nr_gpio)
        gpiochip_remove(&s->gpio);
#endif

	for (i = 0; i < s->devtype->nr_uart; i++) {
		uart_remove_one_port(&xr20m1280_uart, &s->p[i].port);
		clear_bit(s->p[i].port.line, &xr20m1280_lines);
		xr20m1280_power(&s->p[i].port, 0);
	}

	kthread_flush_worker(&s->kworker);
	kthread_stop(s->kworker_task);

	if (!IS_ERR(s->clk))
		clk_disable_unprepare(s->clk);

	return 0;
}

static const struct of_device_id __maybe_unused xr20m1280_dt_ids[] = {
	{ .compatible = "nxp,xr20m1280",	.data = &xr20m1280_devtype, },
	{ }
};
MODULE_DEVICE_TABLE(of, xr20m1280_dt_ids);

static struct regmap_config regcfg = {
	.reg_bits = 7,
	.pad_bits = 1,
	.val_bits = 8,
	.cache_type = REGCACHE_NONE,
	.precious_reg = xr20m1280_regmap_precious,
};

#ifdef CONFIG_SERIAL_XR20M1280_SPI
static int xr20m1280_spi_probe(struct spi_device *spi)
{
	const struct xr20m1280_devtype *devtype;
	unsigned long flags = 0;
	struct regmap *regmap;
	int ret;

	pr_info("Entering probe...\n"); // JAMES
	/* Setup SPI bus */
	spi->bits_per_word	= 8;
	/* only supports mode 0 on XR20M1280 (TODO?) */
	spi->mode		= spi->mode ? : SPI_MODE_0;
	spi->max_speed_hz	= spi->max_speed_hz ? : 15000000;
	spi->rt			= true;
	ret = spi_setup(spi);
	if (ret)
		return ret;

	if (spi->dev.of_node) {
		//const struct of_device_id *of_id =
		//	of_match_device(xr20m1280_dt_ids, &spi->dev);
		//
		//if (!of_id)
		//	return -ENODEV;
		//
		//devtype = (struct xr20m1280_devtype *)of_id->data;
		devtype = device_get_match_data(&spi->dev);
		if (!devtype)
			return -ENODEV;
	} else {
		const struct spi_device_id *id_entry = spi_get_device_id(spi);

		devtype = (struct xr20m1280_devtype *)id_entry->driver_data;
		flags = IRQF_TRIGGER_FALLING;
	}

	regcfg.max_register = (0x7 << XR20M1280_REG_SHIFT) |
			      (devtype->nr_uart - 1);
	regmap = devm_regmap_init_spi(spi, &regcfg);

	pr_info("About to probe...\n"); // JAMES
	return xr20m1280_probe(&spi->dev, devtype, regmap, spi->irq, flags);
}

static void xr20m1280_spi_remove(struct spi_device *spi)
{
	xr20m1280_remove(&spi->dev);
	return;
}

static const struct spi_device_id xr20m1280_spi_id_table[] = {
	{ XR20M1280_NAME,	(kernel_ulong_t)&xr20m1280_devtype, },
	{ }
};

MODULE_DEVICE_TABLE(spi, xr20m1280_spi_id_table);

static struct spi_driver xr20m1280_spi_uart_driver = {
	.driver = {
		.name		= XR20M1280_NAME,
		.of_match_table	= xr20m1280_dt_ids,
	},
	.probe		= xr20m1280_spi_probe,
	.remove		= xr20m1280_spi_remove,
	.id_table	= xr20m1280_spi_id_table,
};

MODULE_ALIAS("spi:xr20m1280");
#endif

#ifdef CONFIG_SERIAL_XR20M1280_I2C
static int xr20m1280_i2c_probe(struct i2c_client *i2c,
			       const struct i2c_device_id *id)
{
	const struct xr20m1280_devtype *devtype;
	unsigned long flags = 0;
	struct regmap *regmap;

	if (i2c->dev.of_node) {
		devtype = device_get_match_data(&i2c->dev);
		if (!devtype)
			return -ENODEV;
	} else {
		devtype = (struct xr20m1280_devtype *)id->driver_data;
		flags = IRQF_TRIGGER_FALLING;
	}

	regcfg.max_register = (0x7 << XR20M1280_REG_SHIFT) |
			      (devtype->nr_uart - 1);
	regmap = devm_regmap_init_i2c(i2c, &regcfg);

	return xr20m1280_probe(&i2c->dev, devtype, regmap, i2c->irq, flags);
}

static int xr20m1280_i2c_remove(struct i2c_client *client)
{
	return xr20m1280_remove(&client->dev);
}

static const struct i2c_device_id xr20m1280_i2c_id_table[] = {
	{ XR20M1280_NAME,	(kernel_ulong_t)&xr20m1280_devtype, },
	{ }
};
MODULE_DEVICE_TABLE(i2c, xr20m1280_i2c_id_table);

static struct i2c_driver xr20m1280_i2c_uart_driver = {
	.driver = {
		.name		= XR20M1280_NAME,
		.of_match_table	= xr20m1280_dt_ids,
	},
	.probe		= xr20m1280_i2c_probe,
	.remove		= xr20m1280_i2c_remove,
	.id_table	= xr20m1280_i2c_id_table,
};

#endif

static int __init xr20m1280_init(void)
{
	int ret;

	pr_info("Entering init...\n"); // JAMES
	ret = uart_register_driver(&xr20m1280_uart);
	if (ret) {
		pr_err("Registering UART driver failed\n");
		return ret;
	}

	pr_info("After register driver...\n"); // JAMES
#ifdef CONFIG_SERIAL_XR20M1280_I2C
	ret = i2c_add_driver(&xr20m1280_i2c_uart_driver);
	if (ret < 0) {
		pr_err("failed to init xr20m1280 i2c --> %d\n", ret);
		goto err_i2c;
	}
#endif

#ifdef CONFIG_SERIAL_XR20M1280_SPI
	ret = spi_register_driver(&xr20m1280_spi_uart_driver);
	if (ret < 0) {
		pr_err("failed to init xr20m1280 spi --> %d\n", ret);
		goto err_spi;
	}
#endif
	pr_info("Exiting init...(%d)\n", ret); // JAMES
	return ret;

#ifdef CONFIG_SERIAL_XR20M1280_SPI
err_spi:
#endif
#ifdef CONFIG_SERIAL_XR20M1280_I2C
	i2c_del_driver(&xr20m1280_i2c_uart_driver);
err_i2c:
#endif
	uart_unregister_driver(&xr20m1280_uart);
	return ret;
}
module_init(xr20m1280_init);

static void __exit xr20m1280_exit(void)
{
#ifdef CONFIG_SERIAL_XR20M1280_I2C
	i2c_del_driver(&xr20m1280_i2c_uart_driver);
#endif

#ifdef CONFIG_SERIAL_XR20M1280_SPI
	spi_unregister_driver(&xr20m1280_spi_uart_driver);
#endif
	uart_unregister_driver(&xr20m1280_uart);
}
module_exit(xr20m1280_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Digi International, http://www.digi.com");
MODULE_DESCRIPTION("XR20M1280 serial driver");

