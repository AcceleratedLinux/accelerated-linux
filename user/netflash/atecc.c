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
#include <config/autoconf.h>

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
 * | HMAC     | 13 ms           | 23 ms          |
 * +----------+-----------------+----------------+
 *
 * Use the maximum execution times. Note that waiting too much
 * may cause the cryptochip to automatically enter sleep mode
 * (see section 5.3.1), so these waiting times need to be tight.
 */
#define NONCE_WAIT_TIME_MS 	7
#define VERIFY_WAIT_TIME_MS	58
#define HMAC_WAIT_TIME_MS       23

#ifndef CONFIG_USER_NETFLASH_ATECC508A_I2C_ADDRESS
# error "ERROR: I2C address is not defined"
#endif
#define ATECC508_I2C_ADDRESS	CONFIG_USER_NETFLASH_ATECC508A_I2C_ADDRESS
#ifndef CONFIG_USER_NETFLASH_ATECC508A_I2C_BUS
# error "ERROR: I2C bus is not defined"
#endif
#define STRINGIFY(x)	#x
#define MKSTRING(x)	STRINGIFY(x)
#define I2C_BUS_PATH		"/dev/i2c-" MKSTRING(CONFIG_USER_NETFLASH_ATECC508A_I2C_BUS)

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

/*
 * some targets have a lesser SMBus implementation,  work around that
 */

#undef SMBUS
#if defined(CONFIG_DEFAULTS_DIGI_CONNECTITXX)
#define SMBUS 1
#endif

#ifdef SMBUS

#include <i2c/smbus.h>

#define SPLIT_TRANSACTION 1
#define TRANSACTION_LEN_MAX 25

static ssize_t sbus_read(int fd, uint8_t *buf, size_t count)
{
	__s32 retval;
	retval = i2c_smbus_read_i2c_block_data(fd, 0x0, 32, buf);
	if (retval != 32) {
		printf("Error in reading smbus\n");
	}
	return (ssize_t) *buf;
}

static ssize_t sbus_write(int fd, const uint8_t *buf, size_t count)
{
	__s32 retval;
	retval = i2c_smbus_write_i2c_block_data(fd, *buf, count - 1, buf + 1);
	if (retval != 0) {
		printf("Error in sending smbus Buffer\n");
		return -1;
	}
	return count;
}

# ifdef SPLIT_TRANSACTION
#  ifndef TRANSACTION_LEN_MAX
#   error "Specify max. transaction length!"
#  endif
#  if TRANSACTION_LEN_MAX < 2
#   error "Max. transaction length should be at least 2!"
#  endif

static ssize_t i2c_read(int fd, void *buf, size_t count)
{
	ssize_t retval = 0;
	uint8_t *ptr = buf;

	/* If read length is more than TRANSACTION_LEN_MAX, split up package */
	while (count > 0) {
		size_t c = (count > TRANSACTION_LEN_MAX) ?
			TRANSACTION_LEN_MAX : count;

		ssize_t ret = sbus_read(fd, ptr, c);
		if (ret < 0)
			return ret;

		retval += ret;

		if (ret != c)
			return retval;

		count -= c;
		ptr += c;
	}

	return retval;
}

static ssize_t i2c_write(int fd, const void *buf, size_t count)
{
	ssize_t retval = 0;
	uint8_t send_buf[TRANSACTION_LEN_MAX];
	const uint8_t *ptr;

	/* If write length is more than TRANSACTION_LEN_MAX, split up package */

	if (count == 0)
		return 0;

	if (count == 1) /* special case */
		return i2c_smbus_write_byte(fd, * (unsigned char *) buf);

	if (count <= TRANSACTION_LEN_MAX) /* easy case */
		return sbus_write(fd, buf, count);

	/* split transaction up to MAX chunks */

	ptr = buf;

	/* Each transaction should start with the word address. We'll reuse the
	 * pre-filled one */
	send_buf[0] = *ptr;

	/* Skip the pre-filled word address, we'll add ours */
	ptr++;
	count--;

	while (count > 0) {
		/* Max. is: TRANSACTION_LEN_MAX - COMMAND_BYTE */
		size_t c = (count > (TRANSACTION_LEN_MAX - 1)) ?
			(TRANSACTION_LEN_MAX - 1) : count;
		ssize_t ret;

		/* Copy data, but keep word address */
		memcpy(&send_buf[1], ptr, c);

		ret = sbus_write(fd, send_buf, c + 1);
		if (ret < 0) {
			retval = ret;
			goto err;
		}

		if (ret != (c + 1))
			goto err;

		/* Do not count the word address, will be added once at the
		 * end */
		retval += c;

		count -= c;
		ptr += c;
	}

	/* Add +1 because of the word address */
	retval++;

err:
	return retval;
}

# else
#  define i2c_read(fd, buf, count)	sbus_read(fd, buf, count)
#  define i2c_write(fd, buf, count)	sbus_write(fd, buf, count)
# endif /* SPLIT_TRANSACTION */

#else

# define i2c_write(fd,buf,n) write(fd,buf,n)
# define i2c_read(fd,buf,n) read(fd,buf,n)

#endif /* SMBUS */


void wakeup_cryptochip(atecc_ctx *ctx) {
	const uint8_t zero[] = { 0 };
	int i;

	for (i = 0; i < SLEEP_WRITES; i++)
		(void) i2c_write(ctx->fd, zero, sizeof(zero));
}

void sleep_cryptochip(atecc_ctx *ctx) {
	int i;
	const uint8_t buf[] = { ATECC_WAV_SLEEP };

	for (i = 0; i < SLEEP_WRITES; i++)
		if (i2c_write(ctx->fd, buf, sizeof(buf)) == sizeof(buf))
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

	ret = i2c_write(ctx->fd, tx_buffer, tx_len);
	if (ret != tx_len) {
		/* For positive ret (incomplete reads) report IO error */
		return ret > 0 ? -EIO : ret;
	}

	// Wait for the cryptochip to process the command
	usleep(sleep_time_ms * 1000);

	// Read response length
	ret = i2c_read(ctx->fd, rx_buffer, 1);
	if (ret <= 0)
		return ret;

	rx_len = rx_buffer[0];
	if (rx_len >= rx_buffer_size)
		return -E2BIG;

#ifndef SMBUS
	// Read the rest of the response
	ret = i2c_read(ctx->fd, rx_buffer + 1, rx_len - 1);
	if (ret != rx_len - 1) {
		/* For positive ret (incomplete reads) report IO error */
		return ret > 0 ? -EIO : ret;
	}
#endif

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
static int __verify(atecc_ctx *ctx, uint8_t slot_id,
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

	ret = send_cmd(ctx, cmd, sizeof(cmd), VERIFY_WAIT_TIME_MS, rx_buff,
		       sizeof(rx_buff));
	if (ret < 0)
		return ret;

	return rx_buff[1];
}

/*
 * return value:
 *  < 0: IO error
 *    0: OK
 */
static int nonce(atecc_ctx *ctx, const uint8_t *message)
{
	uint8_t rx_buff[MAX_BUFFER];
	uint8_t cmd[4 + NONCE_SIZE] = {
		ATECC_OP_NONCE,
		0x03, 		/* Param1: Pass-through mode */
		0x00,		/* Param2: (2 bytes)         */
		0x00		/*         no TempKey usage  */
	};

	memcpy(cmd + 4, message, NONCE_SIZE);

	return send_cmd(ctx, cmd, sizeof(cmd), NONCE_WAIT_TIME_MS, rx_buff,
			sizeof(rx_buff));
}

int verify(atecc_ctx *ctx, uint8_t slot_id, const uint8_t *message,
	   const uint8_t *r, const uint8_t *s)
{
	int ret;

	ret = nonce(ctx, message);
	if (!ret)
		ret = __verify(ctx, slot_id, r, s);

	return ret;
}

/*
 * return value:
 *  < 0: IO error
 *    0: Verification OK
 *  > 0: Verification FAILED (see 'atecc_parse_status')
 */
int hmac(atecc_ctx *ctx, uint8_t slot_id,
	 const uint8_t *message, const uint8_t *hmac)
{
	int ret;
	uint8_t rx_buff[MAX_BUFFER];
	uint8_t cmd[4 + 32 + 32 + 13] = {
		ATECC_OP_CHECKMAC,
		0x00, 		/* mode: sources: ClientChal and Slot[SlotID] */
		slot_id,	/* key slot (2 bytes) */
		0x00
		/* 32 bytes for digest */
		/* 32 bytes for signature */
		/* 13 bytes for other data (not used, should be zero) */
	};

	/* Copy digest */
	memcpy(cmd + 4, message, 32);
	/* Copy signature */
	memcpy(cmd + 4 + 32, hmac, 32);
	/* Other data is 0 */
	memset(cmd + 4 + 32 + 32, 0, 13);

	ret = send_cmd(ctx, cmd, sizeof(cmd), HMAC_WAIT_TIME_MS,
		       rx_buff, sizeof(rx_buff));
	if (ret < 0)
		return ret;

	return rx_buff[1];
}
