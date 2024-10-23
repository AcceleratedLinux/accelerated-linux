#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#include "gpio.h"

static int gpio_get_input_fd_line(int fd, unsigned int line)
{
	struct gpio_v2_line_request req = {
		.offsets = { line },
		.consumer = "netflash",
		.config.flags = GPIO_V2_LINE_FLAG_INPUT,
		.num_lines = 1,
		.fd = -1, /* defensive init */
	};
	if (ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &req) == -1)
		return -1;

	struct gpio_v2_line_values values = { .mask = 1 };
	int ret = -1;
	if (ioctl(req.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values) == 0)
		ret = values.bits & 1;
	close(req.fd);
	return ret;
}

int gpio_get_input_by_line(const char *chip, unsigned int line)
{
	const char *chipdev;
	char path[PATH_MAX];
	if (*chip == '/') {
		chipdev = chip;
	} else {
		if (isdigit(*chip))
			snprintf(path, sizeof path, "/dev/gpiochip%s", chip);
		else
			snprintf(path, sizeof path, "/dev/%s", chip);
		chipdev = path;
	}
	int fd = open(chipdev, O_RDONLY);
	if (fd == -1)
		return -1;
	int ret = gpio_get_input_fd_line(fd, line);
	close(fd);
	return ret;
}

int gpio_get_input_by_name(const char *name)
{
	int ret = -1;
	for (unsigned chipno = 0; chipno < 10000; chipno++) {
		char path[sizeof "/dev/gpiochipXXXX"];
		snprintf(path, sizeof path, "/dev/gpiochip%u", chipno);
		int fd = open(path, O_RDONLY);
		if (fd == -1)
			break; /* no more chips */

		struct gpiochip_info chip_info;
		if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) == 0) {
			unsigned int line;
			for (line = 0; line < chip_info.lines; line++) {
				struct gpio_v2_line_info line_info = {
					.offset = line,
				};
				if (ioctl(fd, GPIO_V2_GET_LINEINFO_IOCTL,
					  &line_info) == 0 &&
				    strcmp(line_info.name, name) == 0)
				{
					ret = gpio_get_input_fd_line(fd, line);
					break;
				}
			}
		}
		close(fd);
		if (ret >= 0)
			break;
	}
	return ret;
}
