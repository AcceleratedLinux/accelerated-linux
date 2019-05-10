#ifndef NETFLASH_ATECC_H
#define NETFLASH_ATECC_H

#include <stddef.h>
#include <stdint.h>

typedef struct  {
	int fd;
} atecc_ctx;

#define ATECC_STATUS_OK			0x00
#define ATECC_STATUS_VERIFY_FAILED	0x01
#define ATECC_STATUS_PARSE_ERROR	0x03
#define ATECC_STATUS_ECC_FAULT		0x05
#define ATECC_STATUS_EXECUTION_ERR	0x0F
#define ATECC_STATUS_WAKE_OK		0x11
#define ATECC_STATUS_WATCHDOG_SOON	0xEE
#define ATECC_STATUS_CRC_ERROR		0xFF

int atecc_init(atecc_ctx *ctx);
void atecc_close(atecc_ctx *ctx);
void calculate_crc(uint8_t length, const uint8_t *data, uint8_t *crc);
int check_crc(const uint8_t *response, uint8_t count);
int send_cmd(atecc_ctx *ctx, const uint8_t *buffer, uint8_t len, int sleep_time_ms,
	     uint8_t *ret_buffer, size_t rx_buffer_size);
int verify(atecc_ctx *ctx, uint8_t slot_id, const uint8_t *r, const uint8_t *s);
int nonce(atecc_ctx *ctx, const uint8_t *message);
int hmac(atecc_ctx *ctx, uint8_t slot_id,
	 const uint8_t *message, const uint8_t *hmac);
void wakeup_cryptochip(atecc_ctx *ctx);
void sleep_cryptochip(atecc_ctx *ctx);

#endif
