/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright 2023 Digi International Inc.
 */

#ifndef MFD_MCU_TX40_COMM_H_
#define MFD_MCU_TX40_COMM_H_

/****** Special commands ******/
#define MCU_CMD_RESET			0x10
#define MCU_CMD_TO_BLOADER		0x11
#define MCU_CMD_SET_POWEROFF		0x12
#define MCU_CMD_FACTORY_RESET		0x13
#define MCU_CMD_TEST_EVENT		0x14
#define MCU_CMD_SET_COLD_REBOOT		0x15

/****** Get commands ******/
#define MCU_CMD_GET_VERSION		0x20
#define MCU_CMD_GET_IO_INFO		0x21
#define MCU_CMD_GET_EVENTS		0x22
#define MCU_CMD_GET_VIN_STUP_SHDN	0x23
#define MCU_CMD_GET_IGN_OFF_DELAY	0x24
#define MCU_CMD_GET_IGN_ON_DELAY	0x25
#define MCU_CMD_GET_TEMP_IGN_OFF_DELAY	0x26
#define MCU_CMD_GET_PWR_LED		0x27
#define MCU_CMD_GET_WAKEUP_SRC		0x28
#define MCU_CMD_GET_BLOADER_VERSION	0x29
#define MCU_CMD_GET_PWR_LED_COLOR	0x2A
#define MCU_CMD_GET_WDT_TIMEOUT		0x2B

/****** Set commands ******/
//#define MCU_CMD_SET_MCU_INT		0x80	/* Obsolete */
#define MCU_CMD_SET_VIN_STUP_SHDN	0x81
#define MCU_CMD_SET_IGN_OFF_DELAY	0x82
#define MCU_CMD_SET_IGN_ON_DELAY	0x83
#define MCU_CMD_SET_TEMP_IGN_OFF_DELAY	0x84
#define MCU_CMD_SET_PWR_LED		0x85
#define MCU_CMD_CLEAR_EVENTS		0x86
#define MCU_CMD_SET_PWR_LED_COLOR	0x87
#define MCU_CMD_SET_WDT_TIMEOUT		0x88

/* Return values from MCU */
enum mcu_status {
	MCU_STATUS_OK, 			/* Success */
	MCU_STATUS_INVALID_LENGTH,	/* Invalid length */
	MCU_STATUS_INVALID_CMD,		/* Invalid command */
	MCU_STATUS_INVALID_MAGIC,	/* Invalid magic */
	MCU_STATUS_INVALID_VALUE,	/* Invalid value */
	MCU_STATUS_OVERRUN,		/* Overrun */
	MCU_STATUS_INVALID_ADDR,	/* Invalid address */
	MCU_STATUS_INVALID_CHECKSUM,	/* Invalid checksum */

	MCU_STATUS_LAST,
};

struct mcu_pkt_dummy {
	u8 dummy[0];
} __packed;

struct mcu_pkt_magic {
	u16 magic;
} __packed;

struct mcu_pkt_sec {
	u16 sec;
} __packed;

struct mcu_pkt_vin {
	u16 stup_mv;
	u16 shdn_mv;
} __packed;

struct mcu_pkt_led {
	u16 delay_off;
	u16 delay_on;
} __packed;

enum mcu_wakeup_src {
	MCU_WAKEUP_SRC_NONE,
	MCU_WAKEUP_SRC_IGNITION,
	MCU_WAKEUP_SRC_RTC,
	MCU_WAKEUP_SRC_ACCELEROMETER,
	MCU_WAKEUP_SRC_SOFT_REBOOT,
	MCU_WAKEUP_SRC_WDT,

	MCU_WAKEUP_SRC_MAX
};
struct mcu_pkt_wakeup_src {
	u8 src;		/* See enum mcu_wakeup_src */
} __packed;

struct mcu_pkt_io_info {
	u16 vin;
	struct {
		u8 rtc_int : 1;
		u8 acc_int : 1;
		u8 ign_sense : 1;
		u8 mcu_int_state : 1;
		u8 wd_trig : 1;
	};
} __packed;

union mcu_pkt_events {
#define MCU_EVENT_IGN_PWDN	(1 << 0)
#define MCU_EVENT_VIN_LOW	(1 << 1)
#define MCU_EVENT_VIN_CRITICAL	(1 << 2)
#define MCU_EVENT_RTC_INT	(1 << 3)
#define MCU_EVENT_ACC_INT	(1 << 4)
#define MCU_EVENT_TEST_EVENT	(1 << 5)
	u16 events;
	struct {
		u8 ign_pwdn : 1;
		u8 vin_low : 1;
		u8 vin_critical : 1;
		u8 rtc_int : 1;
		u8 acc_int : 1;
		u8 test_event : 1;
	} __packed;
};

struct mcu_pkt_version {
	u16 version;
} __packed;

union mcu_pkt_color {
	u8 raw;
	struct {
		u8 r : 1;
		u8 g : 1;
		u8 b : 1;
	};
} __packed;

struct mcu_pkt_wdt_timeout {
	uint8_t timeout;
} __packed;

#define MCU_TX_MAX_SIZE		32
#define MCU_TX_HDR_SIZE		2

struct mcu_tx_pkt {
	u8 length;		/* Packet length, including this byte */
	u8 cmd;
	union {
		u8 data[MCU_TX_MAX_SIZE - MCU_TX_HDR_SIZE];

#define MCU_MAGIC_RESET		0x8C91
		struct mcu_pkt_magic reset;

#define MCU_MAGIC_TO_BLOADER	0x73A8
		struct mcu_pkt_magic to_bloader;

#define MCU_MAGIC_SET_POWEROFF	0xADEC
		struct mcu_pkt_magic set_poweroff;

#define MCU_FACTORY_RESET_MAGIC	0x75D0
		struct mcu_pkt_magic factory_reset;

		struct mcu_pkt_dummy test_event;

#define MCU_MAGIC_SET_COLD_REBOOT	0x87EF
		struct mcu_pkt_magic set_cold_reboot;

		struct mcu_pkt_dummy get_version;
		struct mcu_pkt_dummy get_io_info;
		struct mcu_pkt_dummy get_events;
		struct mcu_pkt_dummy get_vin_stup_shdn;
		struct mcu_pkt_dummy get_ign_off_delay;
		struct mcu_pkt_dummy get_ign_on_delay;
		struct mcu_pkt_dummy get_temp_ign_off_delay;
		struct mcu_pkt_dummy get_pwr_led;
		struct mcu_pkt_dummy get_wakeup_src;
		struct mcu_pkt_dummy get_bloader_version;
		struct mcu_pkt_dummy get_pwr_led_color;
		struct mcu_pkt_dummy get_wdt_timeout;

		struct {
			u8 enabled;
		} __packed set_mcu_int;

		struct mcu_pkt_vin set_vin_stup_shdn;

		struct mcu_pkt_sec set_ign_off_delay;
		struct mcu_pkt_sec set_ign_on_delay;
		struct mcu_pkt_sec set_temp_ign_off_delay;
		struct mcu_pkt_led set_pwr_led;
		union mcu_pkt_events clear_events;
		union mcu_pkt_color set_pwr_led_color;
		struct mcu_pkt_wdt_timeout set_wdt_timeout;
	};
} __packed;

#define MCU_RX_MAX_SIZE		32
#define MCU_RX_HDR_SIZE		2

struct mcu_rx_pkt {
	u8 length;
	u8 status;		/* mcu_status_t */
	union {
		u8 data[MCU_RX_MAX_SIZE - MCU_RX_HDR_SIZE];

		struct mcu_pkt_dummy reset;
		struct mcu_pkt_dummy to_bloader;
		struct mcu_pkt_dummy set_poweroff;
		struct mcu_pkt_dummy factory_reset;
		struct mcu_pkt_dummy test_event;
		struct mcu_pkt_dummy set_cold_reboot;

		struct mcu_pkt_version get_version;
		struct mcu_pkt_io_info get_io_info;
		union mcu_pkt_events get_events;
		struct mcu_pkt_vin get_vin_stup_shdn;
		struct mcu_pkt_sec get_ign_off_delay;
		struct mcu_pkt_sec get_ign_on_delay;
		struct mcu_pkt_sec get_ign_temp_off_delay;
		struct mcu_pkt_led get_pwr_led;
		struct mcu_pkt_wakeup_src get_wakeup_src;
		struct mcu_pkt_version get_bloader_version;
		union mcu_pkt_color get_pwr_led_color;
		struct mcu_pkt_wdt_timeout get_wdt_timeout;

		struct mcu_pkt_dummy set_mcu_int;
		struct mcu_pkt_dummy set_vin_stup_shdn;
		struct mcu_pkt_dummy set_ign_off_delay;
		struct mcu_pkt_dummy set_ign_on_delay;
		struct mcu_pkt_dummy set_temp_ign_off_delay;
		struct mcu_pkt_dummy set_pwr_led;
		struct mcu_pkt_dummy clear_events;
		struct mcu_pkt_dummy set_pwr_led_color;
		struct mcu_pkt_dummy set_wdt_timeout;
	};
} __packed;

#endif /* MFD_MCU_TX40_COMM_H_ */
