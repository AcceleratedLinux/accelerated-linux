/*
 * si321x.c
 *
 * Si321x ProSLIC: Simple SPI Interface to SLIC control registers.
 *                 ( SPI protocol emulated by GPIO )
 *
 * Linux: Character Device Driver Interface.
 *
 * Copyright (c) 2007  Arcturus Networks Inc.
 *	         by Oleksandr G Zhadan <www.ArcturusNetworks.com>
 *
 * All rights reserved.
 * 
 * This material is proprietary to Arcturus Networks Inc. and, in
 * addition to the above mentioned Copyright, may be subject to
 * protection under other intellectual property regimes, including
 * patents, trade secrets, designs and/or trademarks.
 *
 * Any use of this material for any purpose, except with an express
 * license from Arcturus Networks Inc. is strictly prohibited.
 * 
 * Arcturus Networks Inc. hereby grants release of this code to the
 * Linux kernel under the license specified as:
 * 	"GNU GENERAL PUBLIC LICENSE Version 2, June 1991"
 *
 */

#include <linux/init.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include <asm/cacheflush.h>

int si321x_spi_major = 126;

/* DEFINES */

/* uC53281 module SLIC GPIO pins */

#define MCF_PODR_QSPI		0xFC0A400A
#define MCF_PDDR_QSPI		0xFC0A401E
#define MCF_PPDSDR_QSPI		0xFC0A4032
#define MCF_PCLRR_QSPI		0xFC0A4046
#define MCF_PAR_QSPI		0xFC0A405A

#define MCF_PODR_PWM		0xFC0A4006
#define MCF_PDDR_PWM		0xFC0A401A
#define MCF_PPDSDR_PWM		0xFC0A402E
#define MCF_PCLRR_PWM		0xFC0A4042
#define MCF_PAR_PWM		0xFC0A4051

/* SLIC SPI Interface on GPIO pins */

#define SPI_CLK_CLEAR	 *(volatile unsigned char *)MCF_PCLRR_QSPI=0xFB
#define SPI_CLK_SET	 *(volatile unsigned char *)MCF_PPDSDR_QSPI=0x04

#define SPI_MOSI_CLEAR	 *(volatile unsigned char *)MCF_PCLRR_QSPI=0xFE
#define SPI_MOSI_SET	 *(volatile unsigned char *)MCF_PPDSDR_QSPI=0x01

#define SPI_CS_CLEAR	 *(volatile unsigned char *)MCF_PCLRR_QSPI=0xF7
#define SPI_CS_SET	 *(volatile unsigned char *)MCF_PPDSDR_QSPI=0x08

#define SPI_MISO_GET 	((*(volatile unsigned char *)MCF_PPDSDR_QSPI&2)>>1)

static void slic_reset(int level)
{
	if (level == 1)
		*(volatile unsigned char *)MCF_PPDSDR_PWM = 0x04;	/*  PWM1(reset) high */
	else if (level == 0)
		*(volatile unsigned char *)MCF_PCLRR_PWM = 0xFB;	/*  PWM1(reset) low */
}

static void slici_setup(void)
{
	/* slic reset line */
	*(volatile unsigned char *)MCF_PAR_PWM &= 0xFC;		/* PAR_PWM1 as GPIO */
	*(volatile unsigned char *)MCF_PDDR_PWM |= 0x04;	/* PAR_PWM1 as output */

	/* slic SPI lines */
	*(volatile unsigned short *)MCF_PAR_QSPI &= 0xF00F;	/* PAR_QSPI as GPIO */
	*(volatile unsigned char *)MCF_PDDR_QSPI |= 0x0D;	/* PAR_QSPI directions */

	*(volatile unsigned char *)MCF_PPDSDR_QSPI = 0x04;	/* CLK high */
	*(volatile unsigned char *)MCF_PPDSDR_QSPI = 0x08;	/* CS high */
}

enum exceptions {
	PROSLICiNSANE,
	TIMEoUTpOWERuP,
	TIMEoUTpOWERdOWN,
	POWERlEAK,
	TIPoRrINGgROUNDsHORT,
	POWERaLARMQ1,
	POWERaLARMQ2,
	POWERaLARMQ3,
	POWERaLARMQ4,
	POWERaLARMQ5,
	POWERaLARMQ6
};

#define	OPEN_DR64	 	0
#define	STANDARD_CAL_DR96	0x47	/* Calibrate common mode and differential DAC mode DAC + ILIM */
#define CAL_COMPLETE_DR96 	0	/* Value in dr96 after calibration is completed */
#define	STANDARD_CAL_DR97	0x18	/* Calibrations without the ADC and DAC offset and without common mode calibration. */
#define	BIT_CALCM_DR97		0x01	/* CALCM Common Mode Balance Calibration. */
#define MAX_CAL_PERIOD		800	/* The longest period in ms. for a calibration to complete. */
#define ENB2_DR23  		1<<2	/* enable interrupt for the balance Cal */

#define DISABLE_ALL_DR21 0
#define DISABLE_ALL_DR22 0
#define DISABLE_ALL_DR23 0

#define	INIT_IR0		0x55C2	/* DTMF_ROW_0_PEAK */
#define	INIT_IR1		0x51E6	/* DTMF_ROW_0_PEAK */
#define	INIT_IR2		0x4B85	/* DTMF_ROW_1_PEAK */
#define	INIT_IR3		0x4937	/* DTMF_ROW2_PEAK */
#define	INIT_IR4		0x3333	/* DTMF_ROW3_PEAK */
#define	INIT_IR5		0x0202	/* DTMF_COL1_PEAK */
#define	INIT_IR6		0x0202	/* DTMF_FWD_TWIST */
#define	INIT_IR7		0x0198	/* DTMF_RVS_TWIST */
#define	INIT_IR8		0x0198	/* DTMF_ROW_RATIO */
#define	INIT_IR9		0x0611	/* DTMF_COL_RATIO */
#define	INIT_IR10		0x0202	/* DTMF_ROW_2ND_ARM */
#define	INIT_IR11		0x00E5	/* DTMF_COL_2ND_ARM */
#define	INIT_IR12		0x0A1C	/* DTMF_PWR_MIN_ */
#define	INIT_IR13		0x7b30	/* DTMF_OT_LIM_TRES */
#define	INIT_IR14		0x0063	/* OSC1_COEF */
#define	INIT_IR15		0x0000	/* OSC1X */
#define	INIT_IR16		0x7870	/* OSC1Y */
#define	INIT_IR17		0x007d	/* OSC2_COEF */
#define	INIT_IR18		0x0000	/* OSC2X */
#define	INIT_IR19		0x0000	/* OSC2Y */
#define	INIT_IR20		0x7EF0	/* RING_V_OFF */
#define	INIT_IR21		0x0160	/* RING_OSC */
#define	INIT_IR22		0x0000	/* RING_X */
#define	INIT_IR23		0x2000	/* RING_Y */
#define	INIT_IR24		0x2000	/* PULSE_ENVEL */
#define	INIT_IR25		0x0000	/* PULSE_X */
#define	INIT_IR26		0x4000	/* PULSE_Y */
#define	INIT_IR27		0x4000	/* RECV_DIGITAL_GAIN */
#define	INIT_IR28		0x1600	/* XMIT_DIGITAL_GAIN */
#define	INIT_IR29		0x3600	/* LOOP_CLOSE_TRES */
#define	INIT_IR30		0x1000	/* RING_TRIP_TRES */
#define	INIT_IR31		0x0200	/* COMMON_MIN_TRES */
#define	INIT_IR32		0x7c0	/* COMMON_MAX_TRES */
#define	INIT_IR33		0x2600	/* PWR_ALARM_Q1Q2 */
#define	INIT_IR34		0x1B80	/* PWR_ALARM_Q3Q4 */
#define	INIT_IR35		0x8000	/* PWR_ALARM_Q5Q6 */
#define	INIT_IR36		0x0320	/* LOOP_CLSRE_FlTER */
#define	INIT_IR37		0x08c	/* RING_TRIP_FILTER */
#define	INIT_IR38		0x0100	/* TERM_LP_POLE_Q1Q2 */
#define	INIT_IR39		0x010	/* TERM_LP_POLE_Q3Q4 */
#define	INIT_IR40		0x0C00	/* TERM_LP_POLE_Q5Q6 */
#define	INIT_IR41		0x0C00	/* CM_BIAS_RINGING */
#define	INIT_IR42		0x1000	/* DCDC_MIN_V */
#define	INIT_IR43		0x1000	/* LOOP_CLOSE_TRES Low */
#define	INIT_IR99		0x00DA	/* FSK 0 FREQ PARAM */
#define	INIT_IR100		0x6B60	/* FSK 0 AMPL PARAM */
#define	INIT_IR101		0x0074	/* FSK 1 FREQ PARAM */
#define	INIT_IR102		0x79C0	/* FSK 1 AMPl PARAM */
#define	INIT_IR103		0x1120	/* FSK 0to1 SCALER */
#define	INIT_IR104		0x3BE0	/* FSK 1to0 SCALER */
#define	INIT_IR97		0x0000	/* TRASMIT_FILTER */

#define ERRORCODE_LONGBALCAL -2;

#define	INIT_DR0	0X00	/* Serial Interface */
#define	INIT_DR1	0X3c	/* PCM Mode (16 bit linear) */
#define	INIT_DR2	0X00	/* PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB) */
#define	INIT_DR3	0x00	/* PCM TX Clock Slot High Byte */
#define	INIT_DR4	0x00	/* PCM RX Clock Slot Low Byte (1 PCLK cycle/LSB) */
#define	INIT_DR5	0x00	/* PCM RX Clock Slot High Byte */
#define	INIT_DR6	0x00	/* DIO Control (external battery operation, Si3211/12) */
#define	INIT_DR8	0X00	/* Loopbacks (digital loopback default) */
#define	INIT_DR9	0x00	/* Transmit and receive path gain and control */
#define	INIT_DR10	0X28	/* Initialization Two-wire impedance (600  and enabled) */
#define	INIT_DR11	0x33	/* Transhybrid Balance/Four-wire Return Loss */
#define	INIT_DR14	0X10	/* Powerdown Control 1 */
#define	INIT_DR15	0x00	/* Initialization Powerdown Control 2 */
#define	INIT_DR18	0xff	/* Normal Oper. Interrupt Register 1 (clear with 0xFF) */
#define	INIT_DR19	0xff	/* Normal Oper. Interrupt Register 2 (clear with 0xFF) */
#define	INIT_DR20	0xff	/* Normal Oper. Interrupt Register 3 (clear with 0xFF) */
#define	INIT_DR21	0xff	/* Interrupt Mask 1 */
#define	INIT_DR22	0xff	/* Initialization Interrupt Mask 2 */
#define	INIT_DR23	0xff	/* Initialization Interrupt Mask 3 */
#define	INIT_DR32	0x00	/* Oper. Oscillator 1 Controlùtone generation */
#define	INIT_DR33	0x00	/* Oper. Oscillator 2 Controlùtone generation */
#define	INIT_DR34	0X18	/* 34 0x22 0x00 Initialization Ringing Oscillator Control */
#define	INIT_DR35	0x00	/* Oper. Pulse Metering Oscillator Control */
#define	INIT_DR36	0x00	/* 36 0x24 0x00 Initialization OSC1 Active Low Byte (125 sSB) */
#define	INIT_DR37	0x00	/* 37 0x25 0x00 Initialization OSC1 Active High Byte (125 s/LSB) */
#define	INIT_DR38	0x00	/* 38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 s/LSB) */
#define	INIT_DR39	0x00	/* 39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 s/LSB) */
#define	INIT_DR40	0x00	/* 40 0x28 0x00 Initialization OSC2 Active Low Byte (125 sB) */
#define	INIT_DR41	0x00	/* 41 0x29 0x00 Initialization OSC2 Active High Byte (125 s/LSB) */
#define	INIT_DR42	0x00	/* 42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 s/LSB) */
#define	INIT_DR43	0x00	/* 43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 s/LSB) */
#define	INIT_DR44	0x00	/* 44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 s/LSB) */
#define	INIT_DR45	0x00	/* 45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 sSB) */
#define	INIT_DR46	0x00	/* 46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 s/LSB) */
#define	INIT_DR47	0x00	/* 47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 s/LSB) */
#define	INIT_DR48	0X80	/* 48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 s/LSB) */
#define	INIT_DR49	0X3E	/* 49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 s/LSB) */
#define	INIT_DR50	0X00	/* 50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 s/LSB) */
#define	INIT_DR51	0X7D	/* 51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 s/LSB) */
#define	INIT_DR52	0X00	/* 52 0x34 0x00 Normal Oper. FSK Data Bit */
#define	INIT_DR63	0X54	/* 63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval */
#define	INIT_DR64	0x00	/* 64 0x40 0x00 Normal Oper. Mode Byteùprimary control */
#define	INIT_DR65	0X61	/* 65 0x41 0x61 Initialization External Bipolar Transistor Settings */
#define	INIT_DR66	0X03	/* 66 0x42 0x03 Initialization Battery Control */
#define	INIT_DR67	0X1F	/* 67 0x43 0x1F Initialization Automatic/Manual Control */
#define	INIT_DR69	0X0C	/* 69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB) */
#define	INIT_DR70	0X0A	/* 70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB) */
#define	INIT_DR71	0X01	/* 71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB) */
#define	INIT_DR72	0X20	/* 72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB) */
#define	INIT_DR73	0X02	/* 73 0x49 0x02 Initialization Common Mode VoltageùVCM = û3 V(û1.5 V/LSB) */
#define	INIT_DR74	0X32	/* 74 0x4A 0x32 Initialization VBATH (ringing) = û75 V (û1.5 V/LSB) */
#define	INIT_DR75	0X10	/* 75 0x4B 0x10 Initialization VBATL (off-hook) = û24 V (TRACK = 0)(û1.5 V/LSB) */
#define	INIT_DR92	0x7f	/* 92 0x5C  7F Initialization DCûDC Converter PWM Period (61.035 ns/LSB) */
#define	INIT_DR93	0x14	/* 93 0x5D 0x14 0x19 Initialization DCûDC Converter Min. Off Time (61.035 ns/LSB) */
#define	INIT_DR96	0x00	/* 96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration) */
#define	INIT_DR97	0X1F	/* 97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96) */
#define	INIT_DR98	0X10	/* 98 0x62 0x10 Informative Calibration result (see data sheet) */
#define	INIT_DR99	0X10	/* 99 0x63 0x10 Informative Calibration result (see data sheet) */
#define	INIT_DR100	0X11	/* 100 0x64 0x11 Informative Calibration result (see data sheet) */
#define	INIT_DR101	0X11	/* 101 0x65 0x11 Informative Calibration result (see data sheet) */
#define	INIT_DR102	0x08	/* 102 0x66 0x08 Informative Calibration result (see data sheet) */
#define	INIT_DR103	0x88	/* 103 0x67 0x88 Informative Calibration result (see data sheet) */
#define	INIT_DR104	0x00	/* 104 0x68 0x00 Informative Calibration result (see data sheet) */
#define	INIT_DR105	0x00	/* 105 0x69 0x00 Informative Calibration result (see data sheet) */
#define	INIT_DR106	0x20	/* 106 0x6A 0x20 Informative Calibration result (see data sheet) */
#define	INIT_DR107	0x08	/* 107 0x6B 0x08 Informative Calibration result (see data sheet) */
#define	INIT_DR108	0xEB	/* 108 0x63 0x00 0xEB Initialization Feature enhancement register */
#define INIT_SI3210M_DR92 0x60	/*  92 0x60 Initialization DCûDC Converter PWM Period (61.035 ns/LSB) */
#define INIT_SI3210M_DR93 0x38	/*  92 0x60 Initialization DCûDC Converter PWM Period (61.035 ns/LSB) */

#define usleep udelay

#define IDA_LO	28
#define IDA_HI	29
#define IAA	30
#define	IAS	31

#define T_LOOPS 0x400

#if defined(T_LOOPS)
#define T_WAIT(t) { int _twait = t; do _twait--; while(_twait); }
#else
#define T_WAIT(t)
#endif

unsigned char readDirectReg(unsigned char reg)
{
	int i;
	volatile unsigned char tx_data;
	volatile unsigned char rx_data;

	local_irq_disable();

	SPI_CS_CLEAR;		/* CS to low */
	tx_data = reg | 0x80;
	for (i = 0; i < 8; i++) {
		SPI_CLK_CLEAR;	/* CLK Falling Edge */
		if ((tx_data & 0x80) == 0x80)
			SPI_MOSI_SET;	/* Tx line to 1 */
		else
			SPI_MOSI_CLEAR;	/* Tx line to 0 */
		tx_data <<= 1;	/* Prepare next bit to trasfer */
		T_WAIT(T_LOOPS);
		SPI_CLK_SET;	/* CLK Rising Edge  */
		T_WAIT(T_LOOPS);
	}

	SPI_CS_SET;		/* CS to high */
	T_WAIT(T_LOOPS);
	SPI_CS_CLEAR;		/* CS to low */

	rx_data = 0;
	for (i = 0; i < 8; i++) {
		SPI_CLK_CLEAR;	/* CLK Falling Edge */
		T_WAIT(T_LOOPS);
		rx_data += SPI_MISO_GET;	/* catch received bit (MSB first) */
		if (i < 7)
			rx_data <<= 1;	/* prepare space for the next bit */
		SPI_CLK_SET;	/* Rising Edge */
		T_WAIT(T_LOOPS);
	}

	SPI_CS_SET;		/* CS to high */

	local_irq_enable();

	return rx_data;
}

void writeDirectReg(unsigned char reg, unsigned char val)
{
	int i;
	volatile unsigned char tx_data;

	local_irq_disable();

	SPI_CS_CLEAR;		/* CS to low */
	tx_data = reg & 0x7F;
	for (i = 0; i < 8; i++) {
		SPI_CLK_CLEAR;	/* CLK Falling Edge */
		if ((tx_data & 0x80) == 0x80)
			SPI_MOSI_SET;	/* Tx line to 1 */
		else
			SPI_MOSI_CLEAR;	/* Tx line to 0 */
		tx_data <<= 1;	/* Prepare next bit to trasfer */
		T_WAIT(T_LOOPS);
		SPI_CLK_SET;	/* CLK Rising Edge  */
		T_WAIT(T_LOOPS);
	}

	SPI_CS_SET;		/* CS to high */
	T_WAIT(T_LOOPS);

	SPI_CS_CLEAR;		/* CS to low */
	tx_data = val;
	for (i = 0; i < 8; i++) {
		SPI_CLK_CLEAR;	/* CLK Falling Edge */
		if ((tx_data & 0x80) == 0x80)
			SPI_MOSI_SET;	/* Tx line to 1 */
		else
			SPI_MOSI_CLEAR;	/* Tx line to 0 */
		tx_data <<= 1;	/* Prepare next bit to trasfer */
		T_WAIT(T_LOOPS);
		SPI_CLK_SET;	/* CLK Rising Edge  */
		T_WAIT(T_LOOPS);
	}

	SPI_CS_SET;		/* CS to high */
	local_irq_enable();
}

void waitForIndirectReg(void)
{
	while (readDirectReg(IAS)) ;
}

unsigned short readIndirectReg(unsigned char reg)
{
	waitForIndirectReg();
	writeDirectReg(IAA, reg);
	waitForIndirectReg();
	return (readDirectReg(IDA_LO) | (readDirectReg(IDA_HI)) << 8);
}

void writeIndirectReg(unsigned char reg, unsigned short val)
{
	waitForIndirectReg();
	writeDirectReg(IDA_LO, (unsigned char)(val & 0xFF));
	writeDirectReg(IDA_HI, (unsigned char)((val & 0xFF00) >> 8));
	writeDirectReg(IAA, reg);
}

/******************************************************************************/
static unsigned char t;

const char *exceptionStrings[] = {
	" ProSLIC not communicating",
	"Time out during Power Up",
	"Time out during Power Down",
	"Power is Leaking; might be a short",
	"Tip or Ring Ground Short",
	"Too Many Q1 Power Alarms",
	"Too Many Q2 Power Alarms",
	"Too Many Q3 Power Alarms",
	"Too Many Q4 Power Alarms",
	"Too Many Q5 Power Alarms",
	"Too Many Q6 Power Alarms"
};

int exception(enum exceptions e)
{
	printk("ProSLIC:  %s\n", exceptionStrings[e]);
	return (0 - e);
}

int selfTest(void)
{
	if (readDirectReg(8) != 2)
		return (exception(PROSLICiNSANE));
	if (readDirectReg(64) != 0)
		return (exception(PROSLICiNSANE));
	if (readDirectReg(11) != 0x33)
		return (exception(PROSLICiNSANE));
	return 0;
}

unsigned char loopStatus(void)
{
	return (readDirectReg(68) & 0x3);
}

void goActive(void)
{
	writeDirectReg(64, 1);	/* LOOP STATE REGISTER SET TO ACTIVE */
	/* Active works for on-hook and off-hook see spec. */
	/* The phone hook-switch sets the off-hook and on-hook substate */
	usleep(10000);
}

void clearInterrupts(void)
{
	writeDirectReg(18, INIT_DR18);
	writeDirectReg(19, INIT_DR19);
	writeDirectReg(20, INIT_DR20);
}

void disable_interrupts(void)
{
	writeDirectReg(21, 0);
	writeDirectReg(22, 0);
	writeDirectReg(23, 0);
}

int calibrate(void)
{
	unsigned char x, y, progress = 0;	/* progress contains individual bits for the Tip and Ring Calibrations */
	unsigned char DRvalue;
	int timeOut, nCalComplete, i = 0;

	writeDirectReg(21, DISABLE_ALL_DR21);
	writeDirectReg(22, DISABLE_ALL_DR22);
	writeDirectReg(23, DISABLE_ALL_DR23);
	writeDirectReg(64, OPEN_DR64);

	writeDirectReg(97, STANDARD_CAL_DR97);	/* Calibrations without the ADC and DAC offset and without common mode calibration. */
	writeDirectReg(96, STANDARD_CAL_DR96);	/* Calibrate common mode and differential DAC mode DAC + ILIM */

	do {
		DRvalue = readDirectReg(96);
		nCalComplete = DRvalue == CAL_COMPLETE_DR96;	/*  When Calibration completes DR 96 will be zero */
		timeOut = i++ > MAX_CAL_PERIOD;	
		usleep(10000);
	} while (nCalComplete && !timeOut);

	if (timeOut)
		return (0 - timeOut);

	usleep(100000);
	writeIndirectReg(88, 0);
	writeIndirectReg(89, 0);
	writeIndirectReg(90, 0);
	writeIndirectReg(91, 0);
	writeIndirectReg(92, 0);
	writeIndirectReg(93, 0);

	writeDirectReg(98, 0x10);	/* This is necessary if the calibration occurs other than at reset time */
	writeDirectReg(99, 0x10);

	for (i = 0x1f; i > 0; i--) {
		writeDirectReg(98, i);
		usleep(400000);
		if ((readDirectReg(88)) == 0) {
			progress |= 1;
			x = i;
			break;
		}
	}

	for (i = 0x1f; i > 0; i--) {
		writeDirectReg(99, i);
		usleep(400000);
		if ((readDirectReg(89)) == 0) {
			progress |= 2;
			y = i;
			break;
		}
	}

	goActive();

	if (loopStatus() & 4)
		return ERRORCODE_LONGBALCAL;

	writeDirectReg(64, OPEN_DR64);

	writeDirectReg(23, ENB2_DR23);		/* enable interrupt for the balance Cal */
	writeDirectReg(97, BIT_CALCM_DR97);	/* this is a singular calibration bit for longitudinal calibration */
	writeDirectReg(96, 0x40);

	readDirectReg(96);

	writeDirectReg(21, INIT_DR21);
	writeDirectReg(22, INIT_DR22);
	writeDirectReg(23, INIT_DR23);

	return (0);
}

void initializeDirectRegisters(void)
{
	writeDirectReg(1, INIT_DR1);		/*0X28        PCM Mode */

	writeDirectReg(2, 0x01 );		/*0X00        PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB) */
	writeDirectReg(3, INIT_DR3);		/*0x00        PCM TX Clock Slot High Byte */
	writeDirectReg(4, 0x01 );		/*0x00        PCM RX Clock Slot Low Byte (1 PCL) */
	writeDirectReg(5, INIT_DR5);		/*0x00        PCM RX Clock Slot High Byte */
	writeDirectReg(8, INIT_DR8);		/*0X00        Loopbacks (digital loopback default) */
	writeDirectReg(9, INIT_DR9);		/*0x00        Transmit and receive path gain and control */
	writeDirectReg(10, INIT_DR10);		/*0X28        Initialization Two-wire impedance (600  and enabled) */
	writeDirectReg(11, INIT_DR11);		/*0x33        Transhybrid Balance/Four-wire Return Loss */
	writeDirectReg(18, INIT_DR18);		/*0xff        Normal Oper. Interrupt Register 1 (clear with 0xFF) */
	writeDirectReg(19, INIT_DR19);		/*0xff        Normal Oper. Interrupt Register 2 (clear with 0xFF) */
	writeDirectReg(20, INIT_DR20);		/*0xff        Normal Oper. Interrupt Register 3 (clear with 0xFF) */
	writeDirectReg(21, INIT_DR21);		/*0xff        Interrupt Mask 1 */
	writeDirectReg(22, INIT_DR22);		/*0xff        Initialization Interrupt Mask 2 */
	writeDirectReg(23, INIT_DR23);		/*0xff         Initialization Interrupt Mask 3 */
	writeDirectReg(32, INIT_DR32);		/*0x00        Oper. Oscillator 1 Controltone generation */
	writeDirectReg(33, INIT_DR33);		/*0x00        Oper. Oscillator 2 Controltone generation */
	writeDirectReg(34, INIT_DR34);		/*0X18        34 0x22 0x00 Initialization Ringing Oscillator Control */
	writeDirectReg(35, INIT_DR35);		/*0x00        Oper. Pulse Metering Oscillator Control */
	writeDirectReg(36, INIT_DR36);		/*0x00        36 0x24 0x00 Initialization OSC1 Active Low Byte (125 ‘s/LSB) */
	writeDirectReg(37, INIT_DR37);		/*0x00        37 0x25 0x00 Initialization OSC1 Active High Byte (125 ‘s/LSB) */
	writeDirectReg(38, INIT_DR38);		/*0x00        38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 ‘s/LSB) */
	writeDirectReg(39, INIT_DR39);		/*0x00        39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 ‘s/LSB) */
	writeDirectReg(40, INIT_DR40);		/*0x00        40 0x28 0x00 Initialization OSC2 Active Low Byte (125 ‘s/LSB) */
	writeDirectReg(41, INIT_DR41);		/*0x00        41 0x29 0x00 Initialization OSC2 Active High Byte (125 ‘s/LSB) */
	writeDirectReg(42, INIT_DR42);		/*0x00        42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 ‘s/LSB) */
	writeDirectReg(43, INIT_DR43);		/*0x00        43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 ‘s/LSB) */
	writeDirectReg(44, INIT_DR44);		/*0x00        44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 ‘s/LSB) */
	writeDirectReg(45, INIT_DR45);		/*0x00        45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 ‘s/LSB) */
	writeDirectReg(46, INIT_DR46);		/*0x00        46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 ‘s/LSB) */
	writeDirectReg(47, INIT_DR47);		/*0x00        47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 ‘s/LSB) */
	writeDirectReg(48, INIT_DR48);		/*0X80        48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 ‘s/LSB) */
	writeDirectReg(49, INIT_DR49);		/*0X3E        49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 ‘s/LSB) */
	writeDirectReg(50, INIT_DR50);		/*0X00        50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 ‘s/LSB) */
	writeDirectReg(51, INIT_DR51);		/*0X7D        51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 ‘s/LSB) */
	writeDirectReg(52, INIT_DR52);		/*0X00        52 0x34 0x00 Normal Oper. FSK Data Bit */
	writeDirectReg(63, INIT_DR63);		/*0X54        63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval */
	writeDirectReg(64, INIT_DR64);		/*0x00        64 0x40 0x00 Normal Oper. Mode Byteùprimary control */
	writeDirectReg(65, INIT_DR65);		/*0X61        65 0x41 0x61 Initialization External Bipolar Transistor Settings */
	writeDirectReg(66, INIT_DR66);		/*0X03        66 0x42 0x03 Initialization Battery Control */
	writeDirectReg(67, INIT_DR67);		/*0X1F        67 0x43 0x1F Initialization Automatic/Manual Control */
	writeDirectReg(69, INIT_DR69);		/*0X0C        69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB) */
	writeDirectReg(70, INIT_DR70);		/*0X0A        70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB) */
	writeDirectReg(71, INIT_DR71);		/*0X01        71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB) */
	writeDirectReg(72, INIT_DR72);		/*0X20        72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB) */
	writeDirectReg(73, INIT_DR73);		/*0X02        73 0x49 0x02 Initialization Common Mode VoltageVCM = 3 V(1.5 V/LSB) */
	writeDirectReg(74, INIT_DR74);		/*0X32        74 0x4A 0x32 Initialization VBATH (ringing) = 75 V (1.5 V/LSB) */
	writeDirectReg(75, INIT_DR75);		/*0X10        75 0x4B 0x10 Initialization VBATL (off-hook) = 24 V (TRACK = 0)(1.5 V/LSB) */

	if (t != 3)
		writeDirectReg(92, INIT_DR92);	/*0x7f        92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB) */
	else
		writeDirectReg(92, INIT_SI3210M_DR92);	/*0x7f        92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB) */

	writeDirectReg(93, INIT_DR93);		/*0x14        93 0x5D 0x14 0x19 Initialization DCDC Converter Min. Off Time (61.035 ns/LSB) */
	writeDirectReg(96, INIT_DR96);		/*0x00        96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration) */
	writeDirectReg(97, INIT_DR97);		/*0X1F        97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96) */
	writeDirectReg(98, INIT_DR98);		/*0X10        98 0x62 0x10 Informative Calibration result (see data sheet) */
	writeDirectReg(99, INIT_DR99);		/*0X10        99 0x63 0x10 Informative Calibration result (see data sheet) */
	writeDirectReg(100, INIT_DR100);	/*0X11        100 0x64 0x11 Informative Calibration result (see data sheet) */
	writeDirectReg(101, INIT_DR101);	/*0X11        101 0x65 0x11 Informative Calibration result (see data sheet) */
	writeDirectReg(102, INIT_DR102);	/*0x08        102 0x66 0x08 Informative Calibration result (see data sheet) */
	writeDirectReg(103, INIT_DR103);	/*0x88        103 0x67 0x88 Informative Calibration result (see data sheet) */
	writeDirectReg(104, INIT_DR104);	/*0x00        104 0x68 0x00 Informative Calibration result (see data sheet) */
	writeDirectReg(105, INIT_DR105);	/*0x00        105 0x69 0x00 Informative Calibration result (see data sheet) */
	writeDirectReg(106, INIT_DR106);	/*0x20        106 0x6A 0x20 Informative Calibration result (see data sheet) */
	writeDirectReg(107, INIT_DR107);	/*0x08        107 0x6B 0x08 Informative Calibration result (see data sheet) */
	writeDirectReg(108, INIT_DR108);	/*0xEB        108 0x63 0x00 0xEB Initialization Feature enhancement register */
}

void initializeIndirectRegisters(void)
{
	writeIndirectReg(0, INIT_IR0);		/*      0x55C2  DTMF_ROW_0_PEAK */
	writeIndirectReg(1, INIT_IR1);		/*      0x51E6  DTMF_ROW_1_PEAK */
	writeIndirectReg(2, INIT_IR2);		/*      0x4B85  DTMF_ROW2_PEAK */
	writeIndirectReg(3, INIT_IR3);		/*      0x4937  DTMF_ROW3_PEAK */
	writeIndirectReg(4, INIT_IR4);		/*      0x3333  DTMF_COL1_PEAK */
	writeIndirectReg(5, INIT_IR5);		/*      0x0202  DTMF_FWD_TWIST */
	writeIndirectReg(6, INIT_IR6);		/*      0x0202  DTMF_RVS_TWIST */
	writeIndirectReg(7, INIT_IR7);		/*      0x0198  DTMF_ROW_RATIO */
	writeIndirectReg(8, INIT_IR8);		/*      0x0198  DTMF_COL_RATIO */
	writeIndirectReg(9, INIT_IR9);		/*      0x0611  DTMF_ROW_2ND_ARM */
	writeIndirectReg(10, INIT_IR10);	/*      0x0202  DTMF_COL_2ND_ARM */
	writeIndirectReg(11, INIT_IR11);	/*      0x00E5  DTMF_PWR_MIN_ */
	writeIndirectReg(12, INIT_IR12);	/*      0x0A1C  DTMF_OT_LIM_TRES */
	writeIndirectReg(13, INIT_IR13);	/*      0x7b30  OSC1_COEF */
	writeIndirectReg(14, INIT_IR14);	/*      0x0063  OSC1X */
	writeIndirectReg(15, INIT_IR15);	/*      0x0000  OSC1Y */
	writeIndirectReg(16, INIT_IR16);	/*      0x7870  OSC2_COEF */
	writeIndirectReg(17, INIT_IR17);	/*      0x007d  OSC2X */
	writeIndirectReg(18, INIT_IR18);	/*      0x0000  OSC2Y */
	writeIndirectReg(19, INIT_IR19);	/*      0x0000  RING_V_OFF */
	writeIndirectReg(20, INIT_IR20);	/*      0x7EF0  RING_OSC */
	writeIndirectReg(21, INIT_IR21);	/*      0x0160  RING_X */
	writeIndirectReg(22, INIT_IR22);	/*      0x0000  RING_Y */
	writeIndirectReg(23, INIT_IR23);	/*      0x2000  PULSE_ENVEL */
	writeIndirectReg(24, INIT_IR24);	/*      0x2000  PULSE_X */
	writeIndirectReg(25, INIT_IR25);	/*      0x0000  PULSE_Y */
	writeIndirectReg(26, INIT_IR26);	/*      0x4000  RECV_DIGITAL_GAIN */
	writeIndirectReg(27, INIT_IR27);	/*      0x4000  XMIT_DIGITAL_GAIN */
	writeIndirectReg(28, INIT_IR28);	/*      0x1000  LOOP_CLOSE_TRES */
	writeIndirectReg(29, INIT_IR29);	/*      0x3600  RING_TRIP_TRES */
	writeIndirectReg(30, INIT_IR30);	/*      0x1000  COMMON_MIN_TRES */
	writeIndirectReg(31, INIT_IR31);	/*      0x0200  COMMON_MAX_TRES */
	writeIndirectReg(32, INIT_IR32);	/*      0x7c0   PWR_ALARM_Q1Q2 */
	writeIndirectReg(33, INIT_IR33);	/*      0x2600  PWR_ALARM_Q3Q4 */
	writeIndirectReg(34, INIT_IR34);	/*      0x1B80  PWR_ALARM_Q5Q6 */
	writeIndirectReg(35, INIT_IR35);	/*      0x8000  LOOP_CLSRE_FlTER */
	writeIndirectReg(36, INIT_IR36);	/*      0x0320  RING_TRIP_FILTER */
	writeIndirectReg(37, INIT_IR37);	/*      0x08c   TERM_LP_POLE_Q1Q2 */
	writeIndirectReg(38, INIT_IR38);	/*      0x0100  TERM_LP_POLE_Q3Q4 */
	writeIndirectReg(39, INIT_IR39);	/*      0x0010  TERM_LP_POLE_Q5Q6 */
	writeIndirectReg(40, INIT_IR40);	/*      0x0C00  CM_BIAS_RINGING */
	writeIndirectReg(41, INIT_IR41);	/*      0x0C00  DCDC_MIN_V */
	writeIndirectReg(43, INIT_IR43);	/*      0x1000  LOOP_CLOSE_TRES Low */
	writeIndirectReg(99, INIT_IR99);	/*      0x00DA  FSK 0 FREQ PARAM */
	writeIndirectReg(100, INIT_IR100);	/*      0x6B60  FSK 0 AMPL PARAM */
	writeIndirectReg(101, INIT_IR101);	/*      0x0074  FSK 1 FREQ PARAM */
	writeIndirectReg(102, INIT_IR102);	/*      0x79C0  FSK 1 AMPl PARAM */
	writeIndirectReg(103, INIT_IR103);	/*      0x1120  FSK 0to1 SCALER */
	writeIndirectReg(104, INIT_IR104);	/*      0x3BE0  FSK 1to0 SCALER */
	writeIndirectReg(97, INIT_IR97);	/*      0x0000  TRASMIT_FILTER */
}

int powerLeakTest(void)
{
	unsigned char vBat;

	writeDirectReg(64, 0);
	writeDirectReg(14, 0x10);
	usleep(1000);				/* 1 second (in miliseconds) */
	if ((vBat = readDirectReg(82)) < 0x4)	/* 6 volts */
		return (exception(POWERlEAK));

	return vBat;
}

int powerUp(void)
{
	unsigned char vBat;
	int i = 0;

	if (t == 3)		/* M version correction */
	{
		writeDirectReg(92, INIT_SI3210M_DR92);	/* M version */
		writeDirectReg(93, INIT_SI3210M_DR93);	/* M version */
	} else {
		writeDirectReg(93, 0x19);
		writeDirectReg(92, 0xff);	/* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW */
	}

	writeDirectReg(14, 0);			/* Engage the DC-DC converter */
	while ((vBat = readDirectReg(82)) < 0x76)	/* JHD was 0xc0 */
	{
		usleep(1000);
		++i;
		if (i > 200)
			return (exception(TIMEoUTpOWERuP));
	}
	if (t == 3)				/* M version correction */
		writeDirectReg(92, 0X80 + INIT_SI3210M_DR92);	/* M version */
	else
		writeDirectReg(93, 0x99);	/* DC-DC Calibration  */

	while (0x80 & readDirectReg(93)) ;	/* Wait for DC-DC Calibration to complete */

	return vBat;
}

int si321x_init(void)
{
	int ret = 0;

	usleep(1000000);
	slici_setup();
	usleep(250000);
	slic_reset(1);
	usleep(150000);

	ret = selfTest();
	if (ret < 0)
		return ret;

	printk("ProSLIC: Id=0x%x found\n", readDirectReg(0));

	t = (readDirectReg(0) & 0x30) >> 4;	/* chip type */

	initializeIndirectRegisters();

	if (t == 0) {
		writeDirectReg(67, 0x17);	/* Make VBat switch not automatic */
		writeDirectReg(66, 1);		/* Q7 should be set to OFF for si3210 */
	}

	if ((readDirectReg(0) & 0x0f) <= 2)	/* version */
		writeDirectReg(73, 4);		/* set common mode voltage to 6 volts */

	/* Do Flush durring powerUp and calibrate */
	if (t == 0 || t == 3) {		/* Si3210 or Si3210M */
		ret = powerUp();	/* Turn on the DC-DC converter and verify voltage. */
		if (ret < 0)
			return ret;
		ret = powerLeakTest();	/* Check for power leaks */
		if (ret < 0)
			return ret;
		ret = powerUp();	/* Turn on the DC-DC converter again */
		if (ret < 0)
			return ret;
	}

	initializeDirectRegisters();

	writeDirectReg(67, 0x0F);
	writeDirectReg(73, 4);

	writeIndirectReg(20, 0x7F00);	/* set RCO = 20Hz Freq  */

#if defined(CONFIG_SH_SI3210_RING92)
	writeDirectReg(74, 0x3f);	/* set VBATH = 94.5 V */
	writeIndirectReg(21, 0x01EE);	/* set RNGX = 92 V */
#else
	writeDirectReg(74, 0x35);	/* set VBATH = 79.5 V */
	writeIndirectReg(21, 0x0177);	/* set RNGX = 75 V */
#endif

	ret = calibrate();
	if (ret < 0)
		return ret;

	clearInterrupts();
	disable_interrupts();

	goActive();
	return ret;
}

#define IOC_GET_DIRECTREG 	1
#define IOC_SET_DIRECTREG 	2
#define IOC_GET_IDIRECTREG	3
#define IOC_SET_IDIRECTREG	4
#define IOC_GET_SLICSTAT	5

static int
si321x_spi_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		 unsigned long arg)
{
	int ret = 0;
	volatile unsigned char regtmp = 0;

	static int same_dtmf;

	switch (cmd) {
	case IOC_GET_DIRECTREG:
		ret = (int)readDirectReg((unsigned char)arg);
		break;
	case IOC_SET_DIRECTREG:
		writeDirectReg((arg & 0xFFFF0000) >> 16, arg & 0x0FFFF);
		break;
	case IOC_GET_IDIRECTREG:
		ret = (int)readIndirectReg((unsigned char)arg);
		break;
	case IOC_SET_IDIRECTREG:
		writeIndirectReg((arg & 0xFFFF0000) >> 16, arg & 0x0FFFF);
		break;
	case IOC_GET_SLICSTAT:
		regtmp = readDirectReg(68);	/* on/off hook */

		flush_cache_all();

		ret |= ( (unsigned int)regtmp << 8 );
		
		printk("...r68=0x%x...", regtmp);
		
		if ( regtmp & 0x3 ) {
			regtmp = readDirectReg(24);	/* DTMF */
			printk("...r24=0x%x...", regtmp);
			if (!same_dtmf) {
				if ((regtmp & 0x10) == 0x10) {
					same_dtmf = 1;
					ret |= regtmp;
				}
			} else if ((regtmp & 0x10) == 0)
				same_dtmf = 0;

			regtmp = readDirectReg(19);	/* ring trip */
			printk("...r19=0x%x...", regtmp);
			if (regtmp & 0x01) {
				ret |= 0x00010000;
				writeDirectReg(19, regtmp);
			}
		}
		
		printk("...ret=0x%x...\n", ret);
		
		break;
	}
	return ret;
}

static int si321x_spi_read(struct file *filp, char *buf, size_t count,
			       loff_t * f_pos)
{
	ssize_t ret;

	local_irq_disable();
	ret = (ssize_t) readDirectReg((unsigned char)count);
	local_irq_enable();

	return ret;
}

static int si321x_spi_write(struct file *filp, const char *buf,
				size_t count, loff_t * f_pos)
{
	unsigned char tmp;

	copy_from_user(&tmp, buf, 1);

	local_irq_disable();
	writeDirectReg((unsigned char)count, tmp);
	local_irq_enable();

	return count;
}

static int si321x_spi_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int si321x_spi_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const static struct file_operations si321x_spi_fops = {
      read:			si321x_spi_read,
      write:			si321x_spi_write,
      open:			si321x_spi_open,
      ioctl:			si321x_spi_ioctl,
      release:			si321x_spi_release
};

int si321x_spi_init(void)
{
	int result;

	result =
	    register_chrdev(si321x_spi_major, "si321x_spi", &si321x_spi_fops);
	if (result < 0) {
		printk("si321x_spi: cannot obtain major number %d\n",
		       si321x_spi_major);
		return result;
	}

	return 0;
}

void si321x_spi_exit(void)
{
	unregister_chrdev(si321x_spi_major, "si321x_spi");
}

module_init(si321x_spi_init);
module_exit(si321x_spi_exit);

MODULE_AUTHOR
    ("Oleksandr Zhadan, Arcturus Networks Inc. <www.arcturusnetworks.com>");
MODULE_DESCRIPTION
    ("Simple SPI Interface to Si3210 ProSLIC device control registers");
MODULE_LICENSE("GPL");
