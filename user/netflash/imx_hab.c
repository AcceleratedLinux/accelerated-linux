#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "imx_hab.h"
#include "util.h"

#define NVMEM_OTP_PATH		"/sys/bus/nvmem/devices/imx-ocotp0/nvmem"
#define OTP_TOTAL_SIZE_BYTES	512
#define OTP_WORDS_PER_BANK	8
#define OTP_BYTES_PER_WORD	4
#define HAB_ENABLED_OTP_BANK	0
#define HAB_ENABLED_OTP_WORD	6
#define HAB_ENABLED_OTP_MASK	0x2

int imx_hab_is_enabled()
{
	int ret = 1; /* Assume HAB is enabled on error */

	int fd = open(NVMEM_OTP_PATH, O_RDONLY);
	if (fd == -1) {
		notice("%s: %s", NVMEM_OTP_PATH, strerror(errno));
		goto out;
	}

	/*
	 * Reading only 1 byte fails with EINVAL, so read the whole OTP memory
	 */
	uint8_t buffer[OTP_TOTAL_SIZE_BYTES];
	int n = read(fd, buffer, sizeof(buffer));
	if (n != sizeof(buffer)) {
		notice("%s: %s", NVMEM_OTP_PATH, strerror(errno));
		goto out;
	}

	ret = buffer[
		HAB_ENABLED_OTP_BANK * OTP_WORDS_PER_BANK * OTP_BYTES_PER_WORD +
		HAB_ENABLED_OTP_WORD * OTP_BYTES_PER_WORD
	      ] & HAB_ENABLED_OTP_MASK;

out:
	if (fd != -1)
		close(fd);

	return ret;
}
