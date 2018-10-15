#include "atecc.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_BUFFER 		256
#define CRC_SIZE		2
#define NONCE_SIZE		32

#define SLEEP_WRITES		5
/*
 * From Section 9.1.3: (...) and Execution Times, table 9-4
 *
 * +----------+-----------------+----------------+
 * | Command  | Typ. Exec. Time | Max. Exec.Time |
 * +----------+-----------------+----------------+
 * | Nonce    | 0.1 ms          | 7ms            |
 * | Verify   | 38 ms           | 58 ms          |
 * +----------+-----------------+----------------+
 *
 * Use the maximum execution times. Note that waiting too much
 * may cause the cryptochip to automatically enter sleep mode
 * (see section 5.3.1), so these waiting times need to be tight.
 */
#define NONCE_WAIT_TIME_MS 	7
#define VERIFY_WAIT_TIME_MS	58

#define ATECC508_I2C_ADDRESS	0x60
#define I2C_BUS_PATH		"/dev/i2c-0"

/* ATECC Word Address Values */
#define ATECC_WAV_RESET			0x00
#define ATECC_WAV_SLEEP			0x01
#define ATECC_WAV_IDLE			0x02
#define ATECC_WAV_COMMAND		0x03

/* ATECC OP codes */
#define ATECC_OP_CHECKMAC		0x28
#define ATECC_OP_COUNTER		0x24
#define ATECC_OP_DERIVEKEY		0x1C
#define ATECC_OP_ECDH			0x43
#define ATECC_OP_GENDIG			0x15
#define ATECC_OP_GENKEY			0x40
#define ATECC_OP_HMAC			0x11
#define ATECC_OP_INFO			0x30
#define ATECC_OP_LOCK			0x17
#define ATECC_OP_MAC			0x08
#define ATECC_OP_NONCE			0x16
#define ATECC_OP_PAUSE			0x01
#define ATECC_OP_PRIVWRITE		0x46
#define ATECC_OP_RANDOM			0x1B
#define ATECC_OP_READ			0x02
#define ATECC_OP_SIGN			0x41
#define ATECC_OP_SHA			0x47
#define ATECC_OP_UPDATEEXTRA		0x20
#define ATECC_OP_VERIFY			0x45
#define ATECC_OP_WRITE			0x12

void wakeup_cryptochip(atecc_ctx *ctx) {
	const uint8_t zero[] = { 0 };
	int i;

	for (i = 0; i < SLEEP_WRITES; i++)
		write(ctx->fd, zero, sizeof(zero));
}

void sleep_cryptochip(atecc_ctx *ctx) {
	int i;
	const uint8_t buf[] = { ATECC_WAV_SLEEP };

	for (i = 0; i < SLEEP_WRITES; i++)
		if (write(ctx->fd, buf, sizeof(buf)) == sizeof(buf))
			break;

}

int atecc_init(atecc_ctx *ctx)
{
	int err;

	ctx->fd = open(I2C_BUS_PATH, O_RDWR);
	if (ctx->fd < 0)
		return ctx->fd;

	err = ioctl(ctx->fd, I2C_SLAVE, ATECC508_I2C_ADDRESS);
	if (err < 0) {
		close(ctx->fd);
		ctx->fd = -1;
		return err;
	}

	wakeup_cryptochip(ctx);

	return 0;
}

void atecc_close(atecc_ctx *ctx)
{
	sleep_cryptochip(ctx);
	close(ctx->fd);
	ctx->fd = -1;
}

void calculate_crc(uint8_t length, const uint8_t *data, uint8_t *crc)
{
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
			data_bit = (data[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;

			crc_register <<= 1;

			if ((data_bit ^ crc_bit) != 0)
				crc_register ^= polynom;
		}
	}

	crc[0] = (uint8_t) (crc_register & 0x00FF);
	crc[1] = (uint8_t) (crc_register >> 8);
}

int check_crc(const uint8_t *response, uint8_t count)
{
	uint8_t crc[CRC_SIZE];

	count -= CRC_SIZE;
	calculate_crc(count, response, crc);

	return !(crc[0] == response[count] && crc[1] == response[count + 1]);
}

int send_cmd(atecc_ctx *ctx, const uint8_t *data_buffer, uint8_t data_len,
	     int sleep_time_ms, uint8_t *rx_buffer, size_t rx_buffer_size)
{
	int ret;
	int tx_len;
	int rx_len;
	uint8_t tx_buffer[MAX_BUFFER];

	/*
	 * I2C command structure:
	 * - Header (1 byte)
	 * - Length (1 byte)
	 * - Data   ('data_len' bytes)
	 * - CRC    (2 bytes)
	 */
	tx_len = 1 + 1 + data_len + 2;
	if (tx_len >= MAX_BUFFER)
		return -E2BIG;

	tx_buffer[0] = ATECC_WAV_COMMAND;
	tx_buffer[1] = data_len + 2 + 1;
	memcpy(tx_buffer + 2, data_buffer, sizeof(tx_buffer) - 2);
	calculate_crc(data_len + 1, tx_buffer + 1, &tx_buffer[data_len + 2]);

	ret = write(ctx->fd, tx_buffer, tx_len);
	if (ret != tx_len) {
		/* For positive ret (incomplete reads) report IO error */
		return ret > 0 ? -EIO : ret;
	}

	// Wait for the cryptochip to process the command
	usleep(sleep_time_ms * 1000);

	// Read response length
	ret = read(ctx->fd, rx_buffer, 1);
	if (ret <= 0)
		return ret;

	rx_len = rx_buffer[0];
	if (rx_len >= rx_buffer_size)
		return -E2BIG;

	// Read the rest of the response
	ret = read(ctx->fd, rx_buffer + 1, rx_len - 1);
	if (ret != rx_len - 1) {
		/* For positive ret (incomplete reads) report IO error */
		return ret > 0 ? -EIO : ret;
	}

	if (check_crc(rx_buffer, rx_len))
		return -EIO;

	return 0;
}

/*
 * return value:
 *  < 0: IO error
 *    0: Verification OK
 *  > 0: Verification FAILED (see 'atecc_parse_status')
 */
int verify(atecc_ctx *ctx, uint8_t slot_id, 
	       const uint8_t *r, const uint8_t *s)
{
	int ret;
	uint8_t rx_buff[MAX_BUFFER];
	uint8_t cmd[4 + 32 + 32] = {
		ATECC_OP_VERIFY,
		0x00, 		/* stored mode        */
		slot_id,	/* key slot (2 bytes) */
		0x00
		/* 32 bytes for r */
		/* 32 bytes for s */
	};

	memcpy(cmd + 4, r, 32);
	memcpy(cmd + 4 + 32, s, 32);

	ret = send_cmd(ctx, cmd, sizeof(cmd), VERIFY_WAIT_TIME_MS, rx_buff, sizeof(rx_buff));
	if (ret < 0)
		return ret;

	return rx_buff[1];
}

/*
 * return value:
 *  < 0: IO error
 *    0: OK
 */
int nonce(atecc_ctx *ctx, const uint8_t *message)
{
	uint8_t rx_buff[MAX_BUFFER];
	uint8_t cmd[4 + NONCE_SIZE] = {
		ATECC_OP_NONCE,
		0x03, 		/* Param1: Pass-through mode */
		0x00,		/* Param2: (2 bytes)         */
		0x00		/*         no TempKey usage  */
	};

	memcpy(cmd + 4, message, NONCE_SIZE);

	return send_cmd(ctx, cmd, sizeof(cmd), NONCE_WAIT_TIME_MS, rx_buff, sizeof(rx_buff));
}
