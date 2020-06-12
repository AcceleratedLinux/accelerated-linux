#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <config/autoconf.h>

#include "atecc.h"
#include "crypto_atmel.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "sha256.h"
#include "util.h"

#define SQUASHFS_SIZE_OFFSET 	0x28
#define UIMAGE_HDR_SIZE_OFFSET	0x0C
#define UIMAGE_HDR_SIZE		0x40

#define SHA256_BLOCKSIZE	1024

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define CRYPTOCHIP_RETRIES 	5

#if defined(CONFIG_DEFAULTS_DIGI_CONNECTIT_MINI) || \
    defined(CONFIG_DEFAULTS_DIGI_EX12) || \
    defined(CONFIG_DEFAULTS_DIGI_IX10) || \
    defined(CONFIG_DEFAULTS_DIGI_IX15) || \
    defined(CONFIG_DEFAULTS_DIGI_IX20)
#define DEVKEY_GPIO "/sys/class/gpio/gpio86/value"
#endif

#if defined(CONFIG_DEFAULTS_DIGI_IX14)
#define SECJUMPER_GPIOCHIP_PATH	"/dev/gpiochip3"
#define SECJUMPER_GPIO_OFFSET	25
#define SECJUMPER_ASSERTED_VAL	1
#define GPIO_CONSUMER_NAME	"netflash-sec-jumper"

#define NVMEM_OTP_PATH		"/sys/bus/nvmem/devices/imx-ocotp0/nvmem"
#define OTP_TOTAL_SIZE_BYTES	512
#define OTP_WORDS_PER_BANK	8
#define OTP_BYTES_PER_WORD	4
#define HAB_ENABLED_OTP_BANK	0
#define HAB_ENABLED_OTP_WORD	6
#define HAB_ENABLED_OTP_MASK	0x2
#endif

static uint32_t get_blobsize()
{
	uint32_t squashfs_size;
	uint32_t uimage_size;

	/* Read and round SquashFS size */
	if (fb_seek_set(SQUASHFS_SIZE_OFFSET))
		return 0;

	if (fb_read(&squashfs_size, sizeof(squashfs_size)) != sizeof(squashfs_size))
		return 0;

	squashfs_size = (squashfs_size + 0xfff) & 0xfffff000;

#ifndef CONFIG_USER_NETFLASH_ATECC508A_EMBEDDED_KERNEL
	/* Read uImage size */
	if (fb_seek_set(squashfs_size + UIMAGE_HDR_SIZE_OFFSET))
		return 0;

	if (fb_read(&uimage_size, sizeof(uimage_size)) != sizeof(uimage_size))
		return 0;

	uimage_size = htonl(uimage_size) + UIMAGE_HDR_SIZE;
#else
	uimage_size = 0;
#endif

	return squashfs_size + uimage_size;
}

static void compute_sha256(uint32_t size, unsigned char *sha256)
{
	struct sha256_ctx ctx;
	uint8_t buffer[SHA256_BLOCKSIZE];
	int remaining = size;
	int read;

	fb_seek_set(0);

	sha256_init_ctx(&ctx);
	while (remaining > 0) {
		read = fb_read(buffer, MIN(remaining, SHA256_BLOCKSIZE));
		sha256_process_bytes(buffer, read, &ctx);
		remaining -= read;
	}

	sha256_finish_ctx(&ctx, sha256);
}


static int signature_valid(atecc_ctx *ctx, int key_slot,
			   const unsigned char *sha256, const atmel_signature_t *signature)
{
	int ret;
#if defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_ECDSA)
	ret = verify(ctx, key_slot, sha256, signature->r, signature->s);
#elif defined(CONFIG_USER_NETFLASH_ATECC508A_ALG_HMAC)
	ret = hmac(ctx, key_slot, sha256, signature->hmac);
#endif

	return ret;
}

#ifdef CONFIG_DEFAULTS_DIGI_IX14
static int is_seckey_jumper_closed() {
	int ret;
	int jumper_closed = 0;
	struct gpiohandle_request req;
	struct gpiohandle_data data;
	int fd = open(SECJUMPER_GPIOCHIP_PATH, 0);

	if (fd < 0)
		goto gpio_error;

	req.lineoffsets[0] = SECJUMPER_GPIO_OFFSET;
	req.lines = 1;
	req.flags = GPIOHANDLE_REQUEST_INPUT;
	strcpy(req.consumer_label, GPIO_CONSUMER_NAME);
	ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret)
		goto gpio_error;

	ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (ret)
		goto gpio_error;

	jumper_closed = (data.values[0] == SECJUMPER_ASSERTED_VAL);

gpio_error:
	if (fd >= 0)
		close(fd);

	if (fd < 0 || ret) {
		notice("security jumper could not be read, assuming open\n");
		jumper_closed = 0;
	}

	return jumper_closed;
}

static int is_hab_enabled() {
	int hab_enabled = 1;
	uint8_t buffer[OTP_TOTAL_SIZE_BYTES];
	int ret = -1;
	const int byte_offset = HAB_ENABLED_OTP_BANK * OTP_WORDS_PER_BANK * OTP_BYTES_PER_WORD +
				HAB_ENABLED_OTP_WORD * OTP_BYTES_PER_WORD;
	int fd = open(NVMEM_OTP_PATH, O_RDONLY);

	if (fd < 0)
		goto close;

	/* Reading only 1 byte fails with -EINVAL, so read the whole OTP memory */
	ret = read(fd, buffer, sizeof(buffer));
	if (ret <= 0)
		goto close;

	hab_enabled = (buffer[byte_offset] & HAB_ENABLED_OTP_MASK);

close:
	close(fd);
	if (ret <= 0) {
		notice("HAB status could not be read, assuming enabled\n");
		hab_enabled = 1;
	}

	return hab_enabled;
}

static int is_development_key_allowed() {
	int secjumper_closed = is_seckey_jumper_closed();
	int hab_enabled = is_hab_enabled();

	if (secjumper_closed || !hab_enabled) {
		notice("development keys allowed (sec jumper: %s, hab: %s)",
		       secjumper_closed ? "closed" : "open",
			hab_enabled ? "enabled" : "disabled");
		return 1;
	}

	return 0;
}

#elif defined(CONFIG_USER_NETFLASH_DEVMODE_KERNEL_CMDLINE)
static int is_development_key_allowed()
{
	int allowed = 0;
	FILE *f;
	size_t len;
	size_t n = 0;
	char *line = NULL;
	char *s;

	f = fopen("/proc/cmdline", "r");
	if (f) {
		if ((len = getline(&line, &n, f)) != -1) {
			s = strtok(line, " \n");
			while (s != NULL) {
				if (strncmp(s, "dev_mode=", 9) == 0) {
					allowed = (s[9] == '1');
					notice("development keys allowed");
					break;
				}
				s = strtok(NULL, " \n");
			}
			free(line);
		}
		fclose(f);
	}

	return allowed;
}

#elif defined(CONFIG_DEFAULTS_DIGI_CONNECTITXX)

static int is_development_key_allowed()
{
	int stat;
	FILE *fp = popen("nexcom-tool -d | grep -q 'System Mode.*engineer'", "w");

	if (fp) {
		int stat = pclose(fp);
		/* shell exits 0 on success */
		return WEXITSTATUS(stat) ? 0 : 1;
	}

	return 0;
}

#elif defined(CONFIG_DEFAULTS_DIGI_CONNECTIT_MINI) || \
      defined(CONFIG_DEFAULTS_DIGI_EX12) || \
      defined(CONFIG_DEFAULTS_DIGI_IX10) || \
      defined(CONFIG_DEFAULTS_DIGI_IX15) || \
      defined(CONFIG_DEFAULTS_DIGI_IX20)

static int is_development_key_allowed() {
	int fd = open(DEVKEY_GPIO, O_RDONLY);
	char value = 0;
	
	if (fd >= 0) {
		read(fd, &value, 1);
		close(fd);
	}

	return value == '1';
}

#else
static int is_development_key_allowed()
{
	/* No method defined, assume dev-mode is not allowed */
	return 0;
}
#endif

void check_crypto_atmel()
{
	atmel_signature_t signature;
	unsigned char sha256[32];
	int retry, ret = -255;
	atecc_ctx ctx;
	uint32_t blobsize = get_blobsize();
	int key_index = CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT;

	if (blobsize == 0) {
		error("signature not found (blob parse error)");
		exit(BAD_CRYPT_LEN);
	}

	/* Read atmel signature */
	fb_seek_set(blobsize);
	if (fb_read(&signature, sizeof(signature)) != sizeof(signature)) {
		error("signature not found (unexpected length)");
		exit(BAD_CRYPT_LEN);
	}

	if (htonl(signature.magic_hdr) != ATMEL_SIGN_MAGIC) {
		error("signature not found (bad magic)");
		exit(BAD_CRYPT_MAGIC);
	}

	compute_sha256(blobsize, sha256);

	for (retry = 0; retry < CRYPTOCHIP_RETRIES; retry++) {
		/* If this is not the first attempt, reset the cryptochip */
		if (retry > 0) {
			notice("retrying authentication... (%d/%d)", retry + 1, CRYPTOCHIP_RETRIES);
			atecc_close(&ctx);
			/* Wait 2s ( > t_WATCHDOG) */
			usleep(2 * 1000 * 1000);
		}

		if (atecc_init(&ctx)) {
			ret = CRYPTOCHIP_IO_ERROR;
			continue;
		}

		/* Authenticate signature */
		ret = signature_valid(&ctx, key_index, sha256, &signature);
		if (ret == ATECC_STATUS_OK) {
			break;
		} else if (ret == ATECC_STATUS_VERIFY_FAILED) {
			if (key_index == CONFIG_USER_NETFLASH_ATECC508A_PRODUCTION_KEY_SLOT && is_development_key_allowed()) {
				key_index = CONFIG_USER_NETFLASH_ATECC508A_DEVELOPMENT_KEY_SLOT;
				retry = -1;
				atecc_close(&ctx);
				continue;
			}

			error("invalid signature");
			ret = BAD_CRYPT_SIGN;
			break;
		} else {
			error("authentication failed (%d, %d)", ret, errno);
			ret = CRYPTOCHIP_IO_ERROR;
			continue;
		}
	}

	atecc_close(&ctx);

	if (ret)
		exit(ret);
	else
		notice("authentication successful");
}
