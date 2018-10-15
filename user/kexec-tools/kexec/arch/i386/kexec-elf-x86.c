/*
 * kexec: Linux boots Linux
 *
 * Copyright (C) 2003-2005  Eric Biederman (ebiederm@xmission.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <elf.h>
#include <x86/x86-linux.h>
#include "../../kexec.h"
#include "../../kexec-syscall.h"
#include "../../kexec-elf.h"
#include "../../kexec-elf-boot.h"
#include "x86-linux-setup.h"
#include "kexec-x86.h"
#include "crashdump-x86.h"
#include <arch/options.h>

static const int probe_debug = 0;

int elf_x86_probe(const char *buf, off_t len)
{
	
	struct mem_ehdr ehdr;
	int result;
	result = build_elf_exec_info(buf, len, &ehdr, 0);
	if (result < 0) {
		if (probe_debug) {
			fprintf(stderr, "Not an ELF executable\n");
		}
		goto out;
	}

	/* Verify the architecuture specific bits */
	if ((ehdr.e_machine != EM_386) && (ehdr.e_machine != EM_486)) {
		/* for a different architecture */
		if (probe_debug) {
			fprintf(stderr, "Not x86_64 ELF executable\n");
		}
		result = -1;
		goto out;
	}
	result = 0;
 out:
	free_elf_info(&ehdr);
	return result;
}

void elf_x86_usage(void)
{
	printf(	"    --command-line=STRING Set the kernel command line to STRING\n"
		"    --append=STRING       Set the kernel command line to STRING\n"
		"    --reuse-cmdline       Use kernel command line from running system.\n"
		"    --initrd=FILE         Use FILE as the kernel's initial ramdisk.\n"
		"    --ramdisk=FILE        Use FILE as the kernel's initial ramdisk.\n"
		"    --args-linux          Pass linux kernel style options\n"
		"    --args-elf            Pass elf boot notes\n"
		);
	
	
}

int elf_x86_load(int argc, char **argv, const char *buf, off_t len, 
	struct kexec_info *info)
{
	struct mem_ehdr ehdr;
	const char *command_line;
	char *modified_cmdline;
	int command_line_len;
	int modified_cmdline_len;
	const char *ramdisk;
	unsigned long entry, max_addr;
	int arg_style;
#define ARG_STYLE_ELF   0
#define ARG_STYLE_LINUX 1
#define ARG_STYLE_NONE  2
	int opt;
#define OPT_APPEND		(OPT_ARCH_MAX+0)
#define OPT_REUSE_CMDLINE	(OPT_ARCH_MAX+1)
#define OPT_RAMDISK		(OPT_ARCH_MAX+2)
#define OPT_ARGS_ELF    	(OPT_ARCH_MAX+3)
#define OPT_ARGS_LINUX  	(OPT_ARCH_MAX+4)
#define OPT_ARGS_NONE   	(OPT_ARCH_MAX+5)

	static const struct option options[] = {
		KEXEC_ARCH_OPTIONS
		{ "command-line",	1, NULL, OPT_APPEND },
		{ "append",		1, NULL, OPT_APPEND },
		{ "reuse-cmdline",	1, NULL, OPT_REUSE_CMDLINE },
		{ "initrd",		1, NULL, OPT_RAMDISK },
		{ "ramdisk",		1, NULL, OPT_RAMDISK },
		{ "args-elf",		0, NULL, OPT_ARGS_ELF },
		{ "args-linux",		0, NULL, OPT_ARGS_LINUX },
		{ "args-none",		0, NULL, OPT_ARGS_NONE },
		{ 0, 			0, NULL, 0 },
	};

	static const char short_options[] = KEXEC_OPT_STR "";

	/*
	 * Parse the command line arguments
	 */
	arg_style = ARG_STYLE_ELF;
	command_line = 0;
	modified_cmdline = 0;
	modified_cmdline_len = 0;
	ramdisk = 0;
	while((opt = getopt_long(argc, argv, short_options, options, 0)) != -1) {
		switch(opt) {
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
		case OPT_REUSE_CMDLINE:
			command_line = get_command_line();
			break;
		case OPT_RAMDISK:
			ramdisk = optarg;
			break;
		case OPT_ARGS_ELF: 
			arg_style = ARG_STYLE_ELF;
			break;
		case OPT_ARGS_LINUX:
			arg_style = ARG_STYLE_LINUX;
			break;
		case OPT_ARGS_NONE:
#ifdef __i386__
			arg_style = ARG_STYLE_NONE;
#else
			die("--args-none only works on arch i386\n");
#endif
			break;
		}
	}
	command_line_len = 0;
	if (command_line) {
		command_line_len = strlen(command_line) +1;
	}

	/* Need to append some command line parameters internally in case of
	 * taking crash dumps.
	 */
	if (info->kexec_flags & KEXEC_ON_CRASH) {
		modified_cmdline = xmalloc(COMMAND_LINE_SIZE);
		memset((void *)modified_cmdline, 0, COMMAND_LINE_SIZE);
		if (command_line) {
			strncpy(modified_cmdline, command_line,
						COMMAND_LINE_SIZE);
			modified_cmdline[COMMAND_LINE_SIZE - 1] = '\0';
		}
		modified_cmdline_len = strlen(modified_cmdline);
	}

	/* Load the ELF executable */
	elf_exec_build_load(info, &ehdr, buf, len, 0);

	entry = ehdr.e_entry;
	max_addr = elf_max_addr(&ehdr);

	/* Do we want arguments? */
	if (arg_style != ARG_STYLE_NONE) {
		/* Load the setup code */
		elf_rel_build_load(info, &info->rhdr, (char *) purgatory, purgatory_size,
			0, ULONG_MAX, 1, 0);
	}
	if (arg_style == ARG_STYLE_NONE) {
		info->entry = (void *)entry;

	}
	else if (arg_style == ARG_STYLE_ELF) {
		unsigned long note_base;
		struct entry32_regs regs;
		uint32_t arg1, arg2;

		/* Setup the ELF boot notes */
		note_base = elf_boot_notes(info, max_addr,
			(unsigned char *) command_line, command_line_len);

		/* Initialize the stack arguments */
		arg2 = 0; /* No return address */
		arg1 = note_base;
		elf_rel_set_symbol(&info->rhdr, "stack_arg32_1", &arg1, sizeof(arg1));
		elf_rel_set_symbol(&info->rhdr, "stack_arg32_2", &arg2, sizeof(arg2));
		
		/* Initialize the registers */
		elf_rel_get_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));
		regs.eip = entry;       /* The entry point */
		regs.esp = elf_rel_get_addr(&info->rhdr, "stack_arg32_2");
		elf_rel_set_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));

		if (ramdisk) {
			die("Ramdisks not supported with generic elf arguments");
		}
	}
	else if (arg_style == ARG_STYLE_LINUX) {
		struct x86_linux_faked_param_header *hdr;
		unsigned long param_base;
		const unsigned char *ramdisk_buf;
		off_t ramdisk_length;
		struct entry32_regs regs;
		int rc = 0;

		/* Get the linux parameter header */
		hdr = xmalloc(sizeof(*hdr));

		/* Hack: With some ld versions, vmlinux program headers show
		 * a gap of two pages between bss segment and data segment
		 * but effectively kernel considers it as bss segment and
		 * overwrites the any data placed there. Hence bloat the
		 * memsz of parameter segment to 16K to avoid being placed
		 * in such gaps.
		 * This is a makeshift solution until it is fixed in kernel
		 */
		param_base = add_buffer(info, hdr, sizeof(*hdr), 16*1024,
			16, 0, max_addr, 1);

		/* Initialize the parameter header */
		memset(hdr, 0, sizeof(*hdr));
		init_linux_parameters(&hdr->hdr);

		/* Add a ramdisk to the current image */
		ramdisk_buf = NULL;
		ramdisk_length = 0;
		if (ramdisk) {
			ramdisk_buf = (unsigned char *) slurp_file(ramdisk, &ramdisk_length);
		}

		/* If panic kernel is being loaded, additional segments need
		 * to be created. */
		if (info->kexec_flags & KEXEC_ON_CRASH) {
			rc = load_crashdump_segments(info, modified_cmdline,
						max_addr, 0);
			if (rc < 0)
				return -1;
			/* Use new command line. */
			command_line = modified_cmdline;
			command_line_len = strlen(modified_cmdline) + 1;
		}

		/* Tell the kernel what is going on */
		setup_linux_bootloader_parameters(info, &hdr->hdr, param_base, 
			offsetof(struct x86_linux_faked_param_header, command_line),
			command_line, command_line_len,
			ramdisk_buf, ramdisk_length);

		/* Fill in the information bios calls would usually provide */
		setup_linux_system_parameters(&hdr->hdr, info->kexec_flags);

		/* Initialize the registers */
		elf_rel_get_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));
		regs.ebx = 0;		/* Bootstrap processor */
		regs.esi = param_base;	/* Pointer to the parameters */
		regs.eip = entry;	/* The entry point */
		regs.esp = elf_rel_get_addr(&info->rhdr, "stack_end"); /* Stack, unused */
		elf_rel_set_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));
	}
	else {
		die("Unknown argument style\n");
	}
	return 0;
}
