// SPDX-License-Identifier: GPL-2.0
#include "misc.h"

#define _SETUP
#include <asm/setup.h>	/* For COMMAND_LINE_SIZE */
#undef _SETUP

#ifdef CONFIG_CMDLINE_BOOL
static char builtin_cmdline[COMMAND_LINE_SIZE] = CONFIG_CMDLINE;
#ifndef CONFIG_CMDLINE_OVERRIDE
/* .bss section is not initialized yet */
static bool builtin_cmdline_inited __section(".data");
#endif
#endif

#include <asm/bootparam.h>

static unsigned long fs;
static inline void set_fs(unsigned long seg)
{
	fs = seg << 4;  /* shift it back */
}
typedef unsigned long addr_t;
static inline char rdfs8(addr_t addr)
{
	return *((char *)(fs + addr));
}
#include "../cmdline.c"
unsigned long get_cmd_line_ptr(void)
{
	unsigned long cmd_line_ptr = boot_params_ptr->hdr.cmd_line_ptr;

	cmd_line_ptr |= (u64)boot_params_ptr->ext_cmd_line_ptr << 32;

#ifdef CONFIG_CMDLINE_BOOL
#ifndef CONFIG_CMDLINE_OVERRIDE
	if (!builtin_cmdline_inited) {
		strlcat(builtin_cmdline, " ", COMMAND_LINE_SIZE);
		strlcat(builtin_cmdline, (char *)cmd_line_ptr,
			COMMAND_LINE_SIZE);

		builtin_cmdline_inited = true;
	}
#endif
	cmd_line_ptr = (unsigned long)builtin_cmdline;
#endif

	return cmd_line_ptr;
}
int cmdline_find_option(const char *option, char *buffer, int bufsize)
{
	return __cmdline_find_option(get_cmd_line_ptr(), option, buffer, bufsize);
}
int cmdline_find_option_bool(const char *option)
{
	return __cmdline_find_option_bool(get_cmd_line_ptr(), option);
}
