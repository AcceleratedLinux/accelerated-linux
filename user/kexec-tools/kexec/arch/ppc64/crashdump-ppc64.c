/*
 * kexec: Linux boots Linux
 *
 * Created by: R Sharada (sharada@in.ibm.com)
 * Copyright (C) IBM Corporation, 2005. All rights reserved
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <elf.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../../kexec.h"
#include "../../kexec-elf.h"
#include "../../kexec-syscall.h"
#include "../../crashdump.h"
#include "kexec-ppc64.h"
#include "crashdump-ppc64.h"

static struct crash_elf_info elf_info64 =
{
	class: ELFCLASS64,
	data: ELFDATA2MSB,
	machine: EM_PPC64,
	backup_src_start: BACKUP_SRC_START,
	backup_src_end: BACKUP_SRC_END,
	page_offset: PAGE_OFFSET,
	lowmem_limit: MAXMEM,
};

static struct crash_elf_info elf_info32 =
{
	class: ELFCLASS32,
	data: ELFDATA2MSB,
	machine: EM_PPC64,
	backup_src_start: BACKUP_SRC_START,
	backup_src_end: BACKUP_SRC_END,
	page_offset: PAGE_OFFSET,
	lowmem_limit: MAXMEM,
};

extern struct arch_options_t arch_options;

/* Stores a sorted list of RAM memory ranges for which to create elf headers.
 * A separate program header is created for backup region
 */
static struct memory_range *crash_memory_range = NULL;

/* Define a variable to replace the CRASH_MAX_MEMORY_RANGES macro */
static int crash_max_memory_ranges;

/*
 * Used to save various memory ranges/regions needed for the captured
 * kernel to boot. (lime memmap= option in other archs)
 */
mem_rgns_t usablemem_rgns = {0, NULL};

/*
 * To store the memory size of the first kernel and this value will be
 * passed to the second kernel as command line (savemaxmem=xM).
 * The second kernel will be calculated saved_max_pfn based on this
 * variable.
 * Since we are creating/using usable-memory property, there is no way
 * we can determine the RAM size unless parsing the device-tree/memoy@/reg
 * property in the kernel.
 */
uint64_t saved_max_mem = 0;

/* Reads the appropriate file and retrieves the SYSTEM RAM regions for whom to
 * create Elf headers. Keeping it separate from get_memory_ranges() as
 * requirements are different in the case of normal kexec and crashdumps.
 *
 * Normal kexec needs to look at all of available physical memory irrespective
 * of the fact how much of it is being used by currently running kernel.
 * Crashdumps need to have access to memory regions actually being used by
 * running  kernel. Expecting a different file/data structure than /proc/iomem
 * to look into down the line. May be something like /proc/kernelmem or may
 * be zone data structures exported from kernel.
 */
static int get_crash_memory_ranges(struct memory_range **range, int *ranges)
{

	int memory_ranges = 0;
	char device_tree[256] = "/proc/device-tree/";
	char fname[256];
	char buf[MAXBYTES];
	DIR *dir, *dmem;
	FILE *file;
	struct dirent *dentry, *mentry;
	int i, n, crash_rng_len = 0;
	unsigned long long start, end, cstart, cend;
	int page_size;

	crash_max_memory_ranges = max_memory_ranges + 6;
	crash_rng_len = sizeof(struct memory_range) * crash_max_memory_ranges;

	crash_memory_range = (struct memory_range *) malloc(crash_rng_len);
	if (!crash_memory_range) {
		fprintf(stderr, "Allocation for crash memory range failed\n");
		return -1;
	}
	memset(crash_memory_range, 0, crash_rng_len);

	/* create a separate program header for the backup region */
	crash_memory_range[0].start = BACKUP_SRC_START;
	crash_memory_range[0].end = BACKUP_SRC_END + 1;
	crash_memory_range[0].type = RANGE_RAM;
	memory_ranges++;

	if ((dir = opendir(device_tree)) == NULL) {
		perror(device_tree);
		goto err;
	}
	while ((dentry = readdir(dir)) != NULL) {
		if (strncmp(dentry->d_name, "memory@", 7) &&
			strcmp(dentry->d_name, "memory"))
			continue;
		strcpy(fname, device_tree);
		strcat(fname, dentry->d_name);
		if ((dmem = opendir(fname)) == NULL) {
			perror(fname);
			closedir(dir);
			goto err;
		}
		while ((mentry = readdir(dmem)) != NULL) {
			if (strcmp(mentry->d_name, "reg"))
				continue;
			strcat(fname, "/reg");
			if ((file = fopen(fname, "r")) == NULL) {
				perror(fname);
				closedir(dmem);
				closedir(dir);
				goto err;
			}
			if ((n = fread(buf, 1, MAXBYTES, file)) < 0) {
				perror(fname);
				fclose(file);
				closedir(dmem);
				closedir(dir);
				goto err;
			}
			if (memory_ranges >= (max_memory_ranges + 1)) {
				/* No space to insert another element. */
				fprintf(stderr,
					"Error: Number of crash memory ranges"
					" excedeed the max limit\n");
				goto err;
			}

			start = ((unsigned long long *)buf)[0];
			end = start + ((unsigned long long *)buf)[1];
			if (start == 0 && end >= (BACKUP_SRC_END + 1))
				start = BACKUP_SRC_END + 1;

			cstart = crash_base;
			cend = crash_base + crash_size;
			/*
			 * Exclude the region that lies within crashkernel
			 */
			if (cstart < end && cend > start) {
				if (start < cstart && end > cend) {
					crash_memory_range[memory_ranges].start = start;
					crash_memory_range[memory_ranges].end = cstart;
					crash_memory_range[memory_ranges].type = RANGE_RAM;
					memory_ranges++;
					crash_memory_range[memory_ranges].start = cend;
					crash_memory_range[memory_ranges].end = end;
					crash_memory_range[memory_ranges].type = RANGE_RAM;
					memory_ranges++;
				} else if (start < cstart) {
					crash_memory_range[memory_ranges].start = start;
					crash_memory_range[memory_ranges].end = cstart;
					crash_memory_range[memory_ranges].type = RANGE_RAM;
					memory_ranges++;
				} else if (end > cend){
					crash_memory_range[memory_ranges].start = cend;
					crash_memory_range[memory_ranges].end = end;
					crash_memory_range[memory_ranges].type = RANGE_RAM;
					memory_ranges++;
				}
			} else {
				crash_memory_range[memory_ranges].start = start;
				crash_memory_range[memory_ranges].end  = end;
				crash_memory_range[memory_ranges].type = RANGE_RAM;
				memory_ranges++;
			}
			fclose(file);
		}
		closedir(dmem);
	}
	closedir(dir);

	/*
	 * If RTAS region is overlapped with crashkernel, need to create ELF
	 * Program header for the overlapped memory.
	 */
	if (crash_base < rtas_base + rtas_size &&
		rtas_base < crash_base + crash_size) {
		page_size = getpagesize();
		cstart = rtas_base;
		cend = rtas_base + rtas_size;
		if (cstart < crash_base)
			cstart = crash_base;
		if (cend > crash_base + crash_size)
			cend = crash_base + crash_size;
		/*
		 * The rtas section created here is formed by reading rtas-base
		 * and rtas-size from /proc/device-tree/rtas.  Unfortunately
		 * rtas-size is not required to be a multiple of PAGE_SIZE
		 * The remainder of the page it ends on is just garbage, and is
		 * safe to read, its just not accounted in rtas-size.  Since
		 * we're creating an elf section here though, lets round it up
		 * to the next page size boundary though, so makedumpfile can
		 * read it safely without going south on us.
		 */
		cend = (cend + page_size - 1) & (~(page_size - 1));

		crash_memory_range[memory_ranges].start = cstart;
		crash_memory_range[memory_ranges++].end = cend;
	}
	/*
	 * Can not trust the memory regions order that we read from
	 * device-tree. Hence, get the MAX end value.
	 */
	for (i = 0; i < memory_ranges; i++)
		if (saved_max_mem < crash_memory_range[i].end)
			saved_max_mem = crash_memory_range[i].end;

	*range = crash_memory_range;
	*ranges = memory_ranges;
#if DEBUG
	int j;
	printf("CRASH MEMORY RANGES\n");
	for(j = 0; j < *ranges; j++) {
		start = crash_memory_range[j].start;
		end = crash_memory_range[j].end;
		fprintf(stderr, "%016Lx-%016Lx\n", start, end);
	}
#endif
	return 0;

err:
	if (crash_memory_range)
		free(crash_memory_range);
	return -1;
}

/* Converts unsigned long to ascii string. */
static void ultoa(uint64_t i, char *str)
{
	int j = 0, k;
	char tmp;

	do {
		str[j++] = i % 10 + '0';
	} while ((i /=10) > 0);
	str[j] = '\0';

	/* Reverse the string. */
	for (j = 0, k = strlen(str) - 1; j < k; j++, k--) {
		tmp = str[k];
		str[k] = str[j];
		str[j] = tmp;
	}
}

static int add_cmdline_param(char *cmdline, uint64_t addr, char *cmdstr,
				char *byte)
{
	int cmdlen, len, align = 1024;
	char str[COMMAND_LINE_SIZE], *ptr;

	/* Passing in =xxxK / =xxxM format. Saves space required in cmdline.*/
	switch (byte[0]) {
		case 'K':
			if (addr%align)
				return -1;
			addr = addr/align;
			break;
		case 'M':
			addr = addr/(align *align);
			break;
	}
	ptr = str;
	strcpy(str, cmdstr);
	ptr += strlen(str);
	ultoa(addr, ptr);
	strcat(str, byte);
	len = strlen(str);
	cmdlen = strlen(cmdline) + len;
	if (cmdlen > (COMMAND_LINE_SIZE - 1))
		die("Command line overflow\n");
	strcat(cmdline, str);
#if DEBUG
	fprintf(stderr, "Command line after adding elfcorehdr: %s\n", cmdline);
#endif
	return 0;
}

/* Loads additional segments in case of a panic kernel is being loaded.
 * One segment for backup region, another segment for storing elf headers
 * for crash memory image.
 */
int load_crashdump_segments(struct kexec_info *info, char* mod_cmdline,
				uint64_t max_addr, unsigned long min_base)
{
	void *tmp;
	unsigned long sz;
	uint64_t elfcorehdr;
	int nr_ranges, align = 1024, i;
	unsigned long long end;
	struct memory_range *mem_range;

	if (get_crash_memory_ranges(&mem_range, &nr_ranges) < 0)
		return -1;

	/* Create a backup region segment to store backup data*/
	sz = (BACKUP_SRC_SIZE + align - 1) & ~(align - 1);
	tmp = xmalloc(sz);
	memset(tmp, 0, sz);
	info->backup_start = add_buffer(info, tmp, sz, sz, align,
					0, max_addr, 1);
	reserve(info->backup_start, sz);

	/* On ppc64 memory ranges in device-tree is denoted as start
	 * and size rather than start and end, as is the case with
	 * other architectures like i386 . Because of this when loading
	 * the memory ranges in crashdump-elf.c the filesz calculation
	 * [ end - start + 1 ] goes for a toss.
	 *
	 * To be in sync with other archs adjust the end value for
	 * every crash memory range before calling the generic function
	 */

	for (i = 0; i < nr_ranges; i++) {
		end = crash_memory_range[i].end - 1;
		crash_memory_range[i].end = end;
	}


	/* Create elf header segment and store crash image data. */
	if (arch_options.core_header_type == CORE_TYPE_ELF64) {
		if (crash_create_elf64_headers(info, &elf_info64,
					       crash_memory_range, nr_ranges,
					       &tmp, &sz,
					       ELF_CORE_HEADER_ALIGN) < 0)
			return -1;
	}
	else {
		if (crash_create_elf32_headers(info, &elf_info32,
					       crash_memory_range, nr_ranges,
					       &tmp, &sz,
					       ELF_CORE_HEADER_ALIGN) < 0)
			return -1;
	}

	elfcorehdr = add_buffer(info, tmp, sz, sz, align, min_base,
				max_addr, 1);
	reserve(elfcorehdr, sz);
	/* modify and store the cmdline in a global array. This is later
	 * read by flatten_device_tree and modified if required
	 */
	add_cmdline_param(mod_cmdline, elfcorehdr, " elfcorehdr=", "K");
	add_cmdline_param(mod_cmdline, saved_max_mem, " savemaxmem=", "M");
	return 0;
}

/*
 * Used to save various memory regions needed for the captured kernel.
 */

void add_usable_mem_rgns(unsigned long long base, unsigned long long size)
{
	int i;
	unsigned long long end = base + size;
	unsigned long long ustart, uend;

	base = _ALIGN_DOWN(base, getpagesize());
	end = _ALIGN_UP(end, getpagesize());

	for (i=0; i < usablemem_rgns.size; i++) {
		ustart = usablemem_rgns.ranges[i].start;
		uend = usablemem_rgns.ranges[i].end;
		if (base < uend && end > ustart) {
			if ((base >= ustart) && (end <= uend))
				return;
			if (base < ustart && end > uend) {
				usablemem_rgns.ranges[i].start = base;
				usablemem_rgns.ranges[i].end = end;
				return;
			} else if (base < ustart) {
				usablemem_rgns.ranges[i].start = base;
				return;
			} else if (end > uend){
				usablemem_rgns.ranges[i].end = end;
				return;
			}
		}
	}
	usablemem_rgns.ranges[usablemem_rgns.size].start = base;
	usablemem_rgns.ranges[usablemem_rgns.size++].end = end;

#ifdef DEBUG
	fprintf(stderr, "usable memory rgns size:%u base:%llx size:%llx\n",
		usablemem_rgns.size, base, size);
#endif
}

int is_crashkernel_mem_reserved(void)
{
	int fd;

	fd = open("/proc/device-tree/chosen/linux,crashkernel-base", O_RDONLY);
	if (fd < 0)
		return 0;
	close(fd);
	return 1;
}

#if 0
static int sort_regions(mem_rgns_t *rgn)
{
	int i, j;
	unsigned long long tstart, tend;
	for (i = 0; i < rgn->size; i++) {
		for (j = 0; j < rgn->size - i - 1; j++) {
			if (rgn->ranges[j].start > rgn->ranges[j+1].start) {
				tstart = rgn->ranges[j].start;
				tend = rgn->ranges[j].end;
				rgn->ranges[j].start = rgn->ranges[j+1].start;
				rgn->ranges[j].end = rgn->ranges[j+1].end;
				rgn->ranges[j+1].start = tstart;
				rgn->ranges[j+1].end = tend;
			}
		}
	}
	return 0;

}
#endif

