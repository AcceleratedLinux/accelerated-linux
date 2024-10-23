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
#include <unistd.h>

#include "dev_mode.h"
#include "imx_hab.h"
#include "gpio.h"
#include "util.h"

#if defined(CONFIG_DEFAULTS_DIGI_IX14)
#define SECJUMPER_GPIOCHIP_PATH	"/dev/gpiochip3"
#define SECJUMPER_GPIO_OFFSET	25
#define SECJUMPER_ASSERTED_VAL	1
#endif

#ifdef CONFIG_DEFAULTS_DIGI_IX14
static int is_seckey_jumper_closed() {
	int jumper_closed = 0;
	int val = gpio_get_input_by_line(SECJUMPER_GPIOCHIP_PATH,
				         SECJUMPER_GPIO_OFFSET);
	if (val == -1)
		notice("security jumper could not be read, assuming open");
	else
		jumper_closed = (val == SECJUMPER_ASSERTED_VAL);
	return jumper_closed;
}

int is_development_key_allowed() {
	int secjumper_closed = is_seckey_jumper_closed();
	int hab_enabled = imx_hab_is_enabled();

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
      defined(CONFIG_DEFAULTS_DIGI_IX30) || \
      defined(CONFIG_DEFAULTS_DIGI_IX40) || \
      defined(CONFIG_DEFAULTS_DIGI_EX50) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ1) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ2) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ4) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ4WS) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ8) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ8IO) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ8M) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ8IOS) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ8W) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ16) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ16M) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ32) || \
      defined(CONFIG_DEFAULTS_DIGI_CONNECTEZ32M) || \
      defined(CONFIG_DEFAULTS_DIGI_ANYWHEREUSB2) || \
      0

int is_development_key_allowed()
{
	int value;

	value = gpio_get_input_by_name("developer");
	if (value != -1)
		return value == 1;

	value = gpio_get_input_by_name("developer_");
	if (value != -1)
		return value == 0; /* ACTIVE_LOW */

	return 0;
}

/*
 * EX50 "DV" revision boards have a broken switch setup and always return "0".
 */

#else
int is_development_key_allowed()
{
	/* No method defined, assume dev-mode is not allowed */
	return 0;
}
#endif
