/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright 2019 Digi International Inc.
 */

#ifndef MFD_MCU_TX54_COMM_H_
#define MFD_MCU_TX54_COMM_H_

/****** Set commands ******/
/* Reset MCU */
#define MCU_CMD_SET_RESET		0x0F00
/* Ask MCU to power off */
#define MCU_CMD_SET_POWER		0x0F01
/* Update MCU FW */
#define MCU_CMD_TO_BLOADER		0x0F02
/* Set startup voltage threshold. Range: 0 - 1023, 0: no threshold; 1023: 36V */
#define MCU_CMD_SET_VOLTAGE_THRES_STUP	0x0F03
/* Set ignition sense power on delay time (in seconds) */
#define MCU_CMD_SET_IGN_PWR_ON_DELAY	0x0F04
/* Set ignition sense power off delay time (in seconds) */
#define MCU_CMD_SET_IGN_PWR_OFF_DELAY	0x0F05
/* Set MCU_INT pin */
#define MCU_CMD_SET_MCU_INT		0x0F06
/* Set power led */
#define MCU_CMD_SET_PWR_LED		0x0F07
/*
 * Set shutdown voltage threshold. Range: 0 - 1023, 0: no threshold; 1023: 36V
 */
#define MCU_CMD_SET_VOLTAGE_THRES_SHDN	0x0F08
/* Factory reset */
#define MCU_CMD_SET_FACTORY_DEFAULTS	0x0F09
/* Set power button lock */
#define MCU_CMD_SET_PWR_BUTTON_LOCK	0x0F0A

/****** Get commands ******/
/* Get MCU status */
#define MCU_CMD_GET_STATUS		0x0F80
/* Get MCU firmware version */
#define MCU_CMD_GET_VERSION		0x0F81
/* Get wakeup source */
#define MCU_CMD_GET_WAKEUP_SRC		0x0F82
/*
 * Get events (when we received interrupt from MCU). MCU interrupt will be
 * cleared by reading this
 */
#define MCU_CMD_GET_EVENTS		0x0F83
/*
 * Get startup voltage threshold. Range: 0 - 1023, 0: no threshold; 1023: 36V
 */
#define MCU_CMD_GET_VOLTAGE_THRES_STUP	0x0F84
/* Get ignition sense power on delay time (in seconds) */
#define MCU_CMD_GET_IGN_PWR_ON_DELAY	0x0F85
/* Get ignition sense power off delay time (in seconds) */
#define MCU_CMD_GET_IGN_PWR_OFF_DELAY	0x0F86
/*
 * Get shutdown voltage threshold. Range: 0 - 1023, 0: no threshold; 1023: 36V
 */
#define MCU_CMD_GET_VOLTAGE_THRES_SHDN	0x0F87
/* Get power button lock status */
#define MCU_CMD_GET_PWR_BUTTON_LOCK	0x0F88

#define MCU_VERSION_LEN		18

enum mcu_led_mode {
	MCU_LED_MODE_OFF = 0,
	MCU_LED_MODE_ON,
	MCU_LED_MODE_BLINKING_SLOW,
	MCU_LED_MODE_BLINKING_FAST,
	MCU_LED_MODE_BLINKING_FASTER,

	MCU_LED_MODE_MAX,
};

/* Return values from MCU */
enum mcu_status {
	/* Special */
	MCU_STATUS_LINE_KEPT_LOW = 0,	/* Special: I2C line kept low */

	MCU_STATUS_SUCCESS = 1,		/* Success */
	MCU_STATUS_BUSY,		/* Busy */
	MCU_STATUS_INVALID_PACKET,	/* Invalid packet */
	MCU_STATUS_PARAM_ERROR,		/* Parameter error */
	MCU_STATUS_CMD_NOT_FOUND,	/* Invalid command */

	/* Special */
	MCU_STATUS_NO_RESP = 0xFF,	/* Special: no response from chip */
};

enum mcu_power_state {
	POWER_STATE_ON,
	POWER_STATE_OFF,
};

/* Magic key to put into bootloader mode */
#define MCU_BLOADER_MAGIC_KEY		0xA55A

/* Factory defaults magic */
#define MCU_FACTORY_DEF_MAGIC_KEY	0x0001

/* Ignition power off delay max. value */
#define MCU_IGN_PWR_OFF_DELAY_MAX	64800
/* Ignition power on delay max. value */
#define MCU_IGN_PWR_ON_DELAY_MAX	64800

struct mcu_tx_set_cmd {
	uint16_t	param;
} __packed;

/* Max value of the power button lock */
#define MCU_PWR_BUTTON_LOCK_MAX               3

union mcu_tx_set_pwr_b_lock {
	uint16_t		raw;
	struct {
		/* Short button press event disabled */
		uint16_t	short_press_disabled:1;
		/* Long button press event disabled */
		uint16_t	long_press_disabled:1;
	};
};

struct mcu_tx_get_cmd {
	uint8_t		dummy[0];
} __packed;

#define MCU_TX_MAX_SIZE		32
#define MCU_TX_HDR_SIZE		3

struct mcu_tx_pkt {
	uint8_t		len;		/* Packet length, including this byte */
	uint16_t	cmd;
	union {
		uint8_t			data[MCU_TX_MAX_SIZE - MCU_TX_HDR_SIZE];

		/* param: 0 or 1 */
		struct mcu_tx_set_cmd		set_mcu_int;
		/* param: mcu_power_state_t */
		struct mcu_tx_set_cmd		set_power;
		/* param: mcu_led_mode_t */
		struct mcu_tx_set_cmd		set_pwr_led;
		struct mcu_tx_set_cmd		set_voltage_thres_stup;
		struct mcu_tx_set_cmd		set_voltage_thres_shdn;
		struct mcu_tx_set_cmd		set_ign_pwr_on_delay;
		struct mcu_tx_set_cmd		set_ign_pwr_off_delay;
		struct mcu_tx_set_cmd		set_factory_defaults;
		union mcu_tx_set_pwr_b_lock	set_pwr_button_lock;
		struct mcu_tx_set_cmd		to_bloader;

		struct mcu_tx_get_cmd		get_status;
		struct mcu_tx_get_cmd		get_version;
		struct mcu_tx_get_cmd		get_wakeup_src;
		struct mcu_tx_get_cmd		get_events;
		struct mcu_tx_get_cmd		get_voltage_thres_stup;
		struct mcu_tx_get_cmd		get_voltage_thres_shdn;
		struct mcu_tx_get_cmd		get_ign_pwr_on_delay;
		struct mcu_tx_get_cmd		get_ign_pwr_off_delay;
		struct mcu_tx_get_cmd		get_pwr_button_lock;
	};
} __packed;

struct mcu_rx_set_status {
	uint8_t		dummy[0];
} __packed;

struct mcu_inputs {
	/* 1 if RTC interrupt is asserted */
	uint8_t		rtc_int:1;
	/* 1 if accelerometer interrupt is asserted */
	uint8_t		acc_int:1;
	/* 1 if wake button is pressed */
	uint8_t		wake_btn:1;
	/* 1 if ignition is ON */
	uint8_t		ign_sense:1;
} __packed;

struct mcu_rx_get_status {
	/* System state: mcu_sys_state_t */
	uint8_t			sys_state;
	/* Battery voltage */
	uint16_t		battery;
	/* Current input states */
	struct mcu_inputs	inputs;
} __packed;

struct mcu_rx_get_version {
	char		version[MCU_VERSION_LEN];
} __packed;

enum mcu_wakeup_src {
	MCU_WAKEUP_SRC_NONE = 0,
	MCU_WAKEUP_SRC_IGNITION,
	MCU_WAKEUP_SRC_BUTTON,
	MCU_WAKEUP_SRC_ACC,
	MCU_WAKEUP_SRC_RTC,

	MCU_WAKEUP_SRC_MAX,
};

struct mcu_rx_get_wakeup_src {
	/* Last wakeup source: enum mcu_wakeup_src */
	uint8_t		wakeup_src;
} __packed;

union mcu_rx_get_events {
	uint32_t	events;
	struct {
		/* Button power down request */
		uint8_t	button_pwdn_req:1;
		/* Ignition down power down request */
		uint8_t	ign_pwdn_req:1;
		/* Power failure */
		uint8_t	power_failure:1;
		/* 1 if RTC interrupt happened */
		uint8_t	rtc_int:1;
		/* 1 if accelerometer interrupt happened */
		uint8_t	acc_int:1;
	};
};

union mcu_rx_get_pwr_b_lock {
	uint8_t         raw;
	struct {
		/* Short button press event disabled */
		uint8_t short_press_disabled:1;
		/* Long button press event disabled */
		uint8_t long_press_disabled:1;
	};
};

struct mcu_rx_get_uint16 {
	uint16_t	value;
} __packed;

#define MCU_RX_MAX_SIZE		32

struct mcu_rx_pkt {
	uint8_t		status;		/* mcu_status_t */
	union {
		uint8_t				data[MCU_RX_MAX_SIZE - 1];
		struct mcu_rx_set_status	set_mcu_int;
		struct mcu_rx_set_status	set_power;
		struct mcu_rx_set_status	set_pwr_led;
		struct mcu_rx_set_status	set_voltage_thres_stup;
		struct mcu_rx_set_status	set_voltage_thres_shdn;
		struct mcu_rx_set_status	set_ign_pwr_on_delay;
		struct mcu_rx_set_status	set_ign_pwr_off_delay;
		struct mcu_rx_set_status	set_factory_defaults;
		struct mcu_rx_set_status	set_pwr_button_lock;
		struct mcu_rx_set_status	to_bloader;

		struct mcu_rx_get_status	get_status;
		struct mcu_rx_get_version	get_version;
		struct mcu_rx_get_wakeup_src	get_wakeup_src;
		union mcu_rx_get_events		get_events;
		struct mcu_rx_get_uint16	get_voltage_thres_stup;
		struct mcu_rx_get_uint16	get_voltage_thres_shdn;
		struct mcu_rx_get_uint16	get_ign_pwr_on_delay;
		struct mcu_rx_get_uint16	get_ign_pwr_off_delay;
		union mcu_rx_get_pwr_b_lock	get_pwr_button_lock;
	};
} __packed;


#endif /* MFD_MCU_TX54_COMM_H_ */
