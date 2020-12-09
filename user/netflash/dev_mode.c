/*****************************************************************************
* Copyright 2020, Digi International Inc.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>

#include "dev_mode.h"
#include "util.h"

#if defined(CONFIG_DEFAULTS_DIGI_CONNECTIT_MINI) || \
    defined(CONFIG_DEFAULTS_DIGI_EX12) || \
    defined(CONFIG_DEFAULTS_DIGI_IX10) || \
    defined(CONFIG_DEFAULTS_DIGI_IX15) || \
    defined(CONFIG_DEFAULTS_DIGI_IX20)
#include <unistd.h>

#define DEVKEY_GPIO "/sys/class/gpio/gpio86/value"
#endif

#if defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ1)
#define DEVKEY_GPIO "/sys/class/gpio/gpio455/value"
#endif

#if defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ2) || \
    defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ4)
#define DEVKEY_GPIO "/sys/class/gpio/gpio425/value"
#endif

#if defined(CONFIG_DEFAULTS_DIGI_IX14)
#include <linux/gpio.h>
#include <string.h>
#include <sys/ioctl.h>

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
	const int byte_offset =
		HAB_ENABLED_OTP_BANK * OTP_WORDS_PER_BANK * OTP_BYTES_PER_WORD +
		HAB_ENABLED_OTP_WORD * OTP_BYTES_PER_WORD;
	int fd = open(NVMEM_OTP_PATH, O_RDONLY);

	if (fd < 0)
		goto close;

	/*
	 * Reading only 1 byte fails with -EINVAL, so read the whole OTP memory
	 */
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

int is_development_key_allowed() {
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
int is_development_key_allowed()
{
	char *s;
	int ret;

	s = arg_from_kernel_cmdline("dev_mode=");
	if (s == NULL)
		return 0;

	ret = (*s == '1');

	free(s);
	return ret;
}

#elif defined(CONFIG_DEFAULTS_DIGI_CONNECTITXX)

int is_development_key_allowed()
{
	FILE *fp = popen("nexcom-tool -d | grep -q 'System Mode.*engineer'",
			 "w");

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
      defined(CONFIG_DEFAULTS_DIGI_IX20) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ1) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ2) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ4)

int is_development_key_allowed() {
	int fd = open(DEVKEY_GPIO, O_RDONLY);
	char value = 0;

	if (fd >= 0) {
		read(fd, &value, 1);
		close(fd);
	}

	return value == '1';
}

#else
int is_development_key_allowed()
{
	/* No method defined, assume dev-mode is not allowed */
	return 0;
}
#endif
