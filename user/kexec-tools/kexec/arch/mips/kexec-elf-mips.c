/*
 * kexec-elf-mips.c - kexec Elf loader for mips
 * Copyright (C) 2007 Francesco Chiechi, Alessandro Rubini
 * Copyright (C) 2007 Tvblob s.r.l.
 *
 * derived from ../ppc/kexec-elf-ppc.c
 * Copyright (C) 2004 Albert Herranz
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <elf.h>
#include <boot/elf_boot.h>
#include <ip_checksum.h>
#include "../../kexec.h"
#include "../../kexec-elf.h"
#include "kexec-mips.h"
#include <arch/options.h>

static const int probe_debug = 0;

#define BOOTLOADER         "kexec"
#define MAX_COMMAND_LINE   256

#define OPT_APPEND	(OPT_ARCH_MAX+0)

int elf_mips_probe(const char *buf, off_t len)
{

	struct mem_ehdr ehdr;
	int result;
	result = build_elf_exec_info(buf, len, &ehdr, 0);
	if (result < 0) {
		goto out;
	}

	/* Verify the architecuture specific bits */
	if (ehdr.e_machine != EM_MIPS) {
		/* for a different architecture */
		if (probe_debug) {
			fprintf(stderr, "Not for this architecture.\n");
		}
		result = -1;
		goto out;
	}
	result = 0;
 out:
	free_elf_info(&ehdr);
	return result;
}

void elf_mips_usage(void)
{
	printf("    --command-line=STRING Set the kernel command line to "
			"STRING.\n"
	       "    --append=STRING       Set the kernel command line to "
			"STRING.\n");
}

int elf_mips_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
	struct mem_ehdr ehdr;
	int opt;
	const char *command_line;
	int command_line_len;
	static const struct option options[] = {
		KEXEC_ARCH_OPTIONS
		{"command-line", 1, 0, OPT_APPEND},
		{"append",       1, 0, OPT_APPEND},
		{0, 0, 0, 0},
	};

	static const char short_options[] = KEXEC_ARCH_OPT_STR "d";

	command_line = 0;
	while ((opt = getopt_long(argc, argv, short_options,
				  options, 0)) != -1) {
		switch (opt) {
		default:
			/* Ignore core options */
			if (opt < OPT_ARCH_MAX) {
				break;
			}
		case '?':
			usage();
			return -1;
		case OPT_APPEND:
			command_line = optarg;
			break;
		}
	}
	command_line_len = 0;
	if (command_line) {
		command_line_len = strlen(command_line) + 1;
	}

	/* Load the ELF executable */
	elf_exec_build_load(info, &ehdr, buf, len, 0);

	info->entry = (void *) virt_to_phys(ehdr.e_entry);

	return 0;
}

