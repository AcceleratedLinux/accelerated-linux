/*
 *  kexec-multiboot-x86.c
 *
 *  (partial) multiboot support for kexec.  Only supports ELF32 
 *  kernels, and a subset of the multiboot info page options 
 *  (i.e. enough to boot the Xen hypervisor).
 * 
 *  TODO:  
 *    - smarter allocation of new segments
 *    - proper support for the MULTIBOOT_VIDEO_MODE bit
 *    - support for the MULTIBOOT_AOUT_KLUDGE bit
 *
 *
 *  Copyright (C) 2003  Tim Deegan (tjd21 at cl.cam.ac.uk)
 * 
 *  Parts based on GNU GRUB, Copyright (C) 2000  Free Software Foundation, Inc
 *  Parts copied from kexec-elf32-x86.c, written by Eric Biederman
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
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
#include <boot/elf_boot.h>
#include <ip_checksum.h>
#include "../../kexec.h"
#include "../../kexec-elf.h"
#include "kexec-x86.h"
#include <arch/options.h>

/* From GNU GRUB */
#include <x86/mb_header.h>
#include <x86/mb_info.h>

/* Static storage */
static char headerbuf[MULTIBOOT_SEARCH];
static struct multiboot_header *mbh = NULL;

#define MIN(_x,_y) (((_x)<=(_y))?(_x):(_y))


int multiboot_x86_probe(const char *buf, off_t buf_len)
/* Is it a good idea to try booting this file? */
{
	int i, len;
	/* First of all, check that this is an ELF file */
	if ((i=elf_x86_probe(buf, buf_len)) < 0) {
		return i;
	}
	/* Now look for a multiboot header in the first 8KB */
	len = MULTIBOOT_SEARCH;
	if (len > buf_len) {
		len = buf_len;
	}
	memcpy(headerbuf, buf, len);
	if (len < 12) {
		/* Short file */
		return -1;
	}
	for (i = 0; i <= (len - 12); i += 4)
	{
		/* Search for a multiboot header */
		mbh = (struct multiboot_header *)(headerbuf + i);
		if (mbh->magic != MULTIBOOT_MAGIC 
		    || ((mbh->magic+mbh->flags+mbh->checksum) & 0xffffffff))
		{
			/* Not a multiboot header */
			continue;
		}
		if (mbh->flags & MULTIBOOT_AOUT_KLUDGE) {
			/* Requires options we don't support */
			fprintf(stderr, 
				"Found a multiboot header, but it uses "
				"a non-ELF header layout,\n"
				"and I can't do that (yet).  Sorry.\n");
			return -1;
		} 
		if (mbh->flags & MULTIBOOT_UNSUPPORTED) {
			/* Requires options we don't support */
			fprintf(stderr, 
				"Found a multiboot header, but it "
				"requires multiboot options that I\n"
				"don't understand.  Sorry.\n");
			return -1;
		} 
		if (mbh->flags & MULTIBOOT_VIDEO_MODE) { 
			/* Asked for screen mode information */
			/* XXX carry on regardless */
			fprintf(stderr, 
				"BEWARE!  Found a multiboot header which asks "
				"for screen mode information.\n"
				"BEWARE!  I am NOT supplying screen mode "
                                "information, but loading it regardless.\n");
			
		}
		/* Bootable */
		return 0;
	}
	/* Not multiboot */
	return -1;
}


void multiboot_x86_usage(void)
/* Multiboot-specific options */
{
	printf("    --command-line=STRING        Set the kernel command line to STRING.\n");
	printf("    --reuse-cmdline       	 Use kernel command line from running system.\n");
	printf("    --module=\"MOD arg1 arg2...\"  Load module MOD with command-line \"arg1...\"\n");
	printf("                                 (can be used multiple times).\n");
}

int multiboot_x86_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
/* Marshal up a multiboot-style kernel */
{
	struct multiboot_info *mbi;
	void   *mbi_buf;
	struct mod_list *modp;
	unsigned long freespace;
	unsigned long long mem_lower = 0, mem_upper = 0;
	struct mem_ehdr ehdr;
	unsigned long mbi_base;
	struct entry32_regs regs;
	size_t mbi_bytes, mbi_offset;
	const char *command_line=NULL;
	char *imagename, *cp;
	struct memory_range *range;
	int ranges;
	struct AddrRangeDesc *mmap;
	int command_line_len;
	int i;
	int opt;
	int modules, mod_command_line_space;
#define OPT_CL  		(OPT_ARCH_MAX+0)
#define OPT_REUSE_CMDLINE	(OPT_ARCH_MAX+1)
#define OPT_MOD 		(OPT_ARCH_MAX+2)
#define OPT_VGA 		(OPT_ARCH_MAX+3)
	static const struct option options[] = {
		KEXEC_ARCH_OPTIONS
		{ "command-line",		1, 0, OPT_CL },
		{ "append",			1, 0, OPT_CL },
		{ "reuse-cmdline",		1, 0, OPT_REUSE_CMDLINE },
		{ "module",			1, 0, OPT_MOD },
		{ 0, 				0, 0, 0 },
	};
	static const char short_options[] = KEXEC_ARCH_OPT_STR "";
	
	/* Probe for the MB header if it's not already found */
	if (mbh == NULL && multiboot_x86_probe(buf, len) != 1) 
	{
		fprintf(stderr, "Cannot find a loadable multiboot header.\n");
		return -1;
	}

	
	/* Parse the command line */
	command_line = "";
	command_line_len = 0;
	modules = 0;
	mod_command_line_space = 0;
	while((opt = getopt_long(argc, argv, short_options, options, 0)) != -1)
	{
		switch(opt) {
		default:
			/* Ignore core options */
			if (opt < OPT_ARCH_MAX) {
				break;
			}
		case '?':
			usage();
			return -1;
		case OPT_CL:
			command_line = optarg;
			break;
		case OPT_REUSE_CMDLINE:
			command_line = get_command_line();
			break;
		case OPT_MOD:
			modules++;
			mod_command_line_space += strlen(optarg) + 1;
			break;
		}
	}
	imagename = argv[optind];
	command_line_len = strlen(command_line) + strlen(imagename) + 2;


	
	/* Load the ELF executable */
	elf_exec_build_load(info, &ehdr, buf, len, 0);

	/* Load the setup code */
	elf_rel_build_load(info, &info->rhdr, (char *) purgatory, purgatory_size, 0,
				ULONG_MAX, 1, 0);
	
	/* The first segment will contain the multiboot headers:
	 * =============
	 * multiboot information (mbi)
	 * -------------
	 * kernel command line
	 * -------------
	 * bootloader name
	 * -------------
	 * module information entries
	 * -------------
	 * module command lines
	 * ==============
	 */
	mbi_bytes = (sizeof(*mbi) + command_line_len 
		     + strlen (BOOTLOADER " " BOOTLOADER_VERSION) + 1
		     + 3) & ~3;
	mbi_buf = xmalloc(mbi_bytes);
	mbi = mbi_buf;
	memset(mbi, 0, sizeof(*mbi));
	sprintf(((char *)mbi) + sizeof(*mbi), "%s %s",
		imagename, command_line);
	sprintf(((char *)mbi) + sizeof(*mbi) + command_line_len, "%s",
		BOOTLOADER " " BOOTLOADER_VERSION);
	mbi->flags = MB_INFO_CMDLINE | MB_INFO_BOOT_LOADER_NAME;
	/* We'll relocate these to absolute addresses later. For now,
	 * all addresses within the first segment are relative to the
	 * start of the MBI. */
	mbi->cmdline = sizeof(*mbi); 
	mbi->boot_loader_name = sizeof(*mbi) + command_line_len; 

	/* Memory map */
	if ((get_memory_ranges(&range, &ranges, info->kexec_flags) < 0)
			|| ranges == 0) {
		fprintf(stderr, "Cannot get memory information\n");
		return -1;
	}
	mmap = xmalloc(ranges * sizeof(*mmap));
	for (i=0; i<ranges; i++) {
		unsigned long long length;
		length = range[i].end - range[i].start;
		/* Translate bzImage mmap to multiboot-speak */
		mmap[i].size = sizeof(mmap[i]) - 4;
		mmap[i].base_addr_low  = range[i].start & 0xffffffff;
		mmap[i].base_addr_high  = range[i].start >> 32;
		mmap[i].length_low     = length & 0xffffffff;
		mmap[i].length_high    = length >> 32;
		if (range[i].type == RANGE_RAM) {
			mmap[i].Type = 1; /* RAM */
			/* Is this the "low" memory? */
			if ((range[i].start == 0)
			    && (range[i].end > mem_lower))
				mem_lower = range[i].end;
			/* Is this the "high" memory? */
			if ((range[i].start <= 0x100000)
			    && (range[i].end > mem_upper + 0x100000))
				mem_upper = range[i].end - 0x100000;
		}
		else
		mmap[i].Type = 0xbad;  /* Not RAM */
	}


	if (mbh->flags & MULTIBOOT_MEMORY_INFO) { 
		/* Provide a copy of the memory map to the kernel */

		mbi->flags |= MB_INFO_MEMORY | MB_INFO_MEM_MAP;
		
		freespace = add_buffer(info,
			mmap, ranges * sizeof(*mmap), ranges * sizeof(*mmap),
			4, 0, 0xFFFFFFFFUL, 1);

		mbi->mmap_addr   = freespace;
		mbi->mmap_length = ranges * sizeof(*mmap);

		/* For kernels that care naught for fancy memory maps
		 * and just want the size of low and high memory */
		mbi->mem_lower = MIN(mem_lower>>10, 0xffffffff);
		mbi->mem_upper = MIN(mem_upper>>10, 0xffffffff);
			
		/* done */
	}


	/* Load modules */
	if (modules) {
		char *mod_filename, *mod_command_line, *mod_clp, *buf;
		off_t mod_size;

		/* We'll relocate this to an absolute address later */
		mbi->mods_addr = mbi_bytes;
		mbi->mods_count = 0;
		mbi->flags |= MB_INFO_MODS;
		
		/* Add room for the module descriptors to the MBI buffer */
		mbi_bytes += (sizeof(*modp) * modules)
			+ mod_command_line_space;
		mbi_buf = xrealloc(mbi_buf, mbi_bytes);

		/* mbi might have moved */
		mbi = mbi_buf;
		/* module descriptors go in the newly added space */
		modp = ((void *)mbi) + mbi->mods_addr; 
		/* module command lines go after the descriptors */
		mod_clp = ((void *)modp) + (sizeof(*modp) * modules);
		
		/* Go back and parse the module command lines */
		optind = opterr = 1;
		while((opt = getopt_long(argc, argv, 
					 short_options, options, 0)) != -1)
		{
			if (opt != OPT_MOD) continue;

			/* Split module filename from command line */
			mod_command_line = mod_filename = optarg;
			if ((cp = strchr(mod_filename, ' ')) != NULL) {
				/* See as I discard the 'const' modifier */
				*cp = '\0';
			}

			/* Load the module */
			buf = slurp_decompress_file(mod_filename, &mod_size);
			
			if (cp != NULL) *cp = ' ';

			/* Pick the next aligned spot to load it in */
			freespace = add_buffer(info,
				buf, mod_size, mod_size,
				getpagesize(), 0, 0xffffffffUL, 1);

			/* Add the module command line */
			sprintf(mod_clp, "%s", mod_command_line);

			modp->mod_start = freespace;
			modp->mod_end   = freespace + mod_size;
			modp->cmdline   = (void *)mod_clp - (void *)mbi;
			modp->pad       = 0;

			/* Done */
			mbi->mods_count++;
			mod_clp += strlen(mod_clp) + 1;
			modp++;
		}
		
	}


	/* Find a place for the MBI to live */
	if (sort_segments(info) < 0) {
                return -1;
        }
	mbi_base = add_buffer(info,
		mbi_buf, mbi_bytes, mbi_bytes, 4, 0, 0xFFFFFFFFUL, 1);
		
	/* Relocate offsets in the MBI to absolute addresses */
	mbi_offset = mbi_base;
	modp = ((void *)mbi) + mbi->mods_addr;
	for (i=0; i<mbi->mods_count; i++) {
		modp[i].cmdline += mbi_offset;
	}
	mbi->mods_addr += mbi_offset;
	mbi->cmdline += mbi_offset;
	mbi->boot_loader_name += mbi_offset;

	/* Specify the initial CPU state and copy the setup code */
	elf_rel_get_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));
	regs.eax = 0x2BADB002;
	regs.ebx = mbi_offset;
	regs.eip = ehdr.e_entry;
	elf_rel_set_symbol(&info->rhdr, "entry32_regs", &regs, sizeof(regs));

	return 0;
}

/*
 *  EOF (kexec-multiboot-x86.c)
 */

