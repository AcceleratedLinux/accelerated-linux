#ifndef CRASHDUMP_H
#define CRASHDUMP_H

extern int get_crash_notes_per_cpu(int cpu, uint64_t *addr, uint64_t *len);
extern int get_kernel_vmcoreinfo(uint64_t *addr, uint64_t *len);
extern int get_xen_vmcoreinfo(uint64_t *addr, uint64_t *len);

/* Need to find a better way to determine per cpu notes section size. */
#define MAX_NOTE_BYTES		1024
/* Expecting ELF headers to fit in 4K. Increase it if you need more. */
#define KCORE_ELF_HEADERS_SIZE  4096
/* The address of the ELF header is passed to the secondary kernel
 * using the kernel command line option memmap=nnn.
 * The smallest unit the kernel accepts is in kilobytes,
 * so we need to make sure the ELF header is aligned to 1024.
 */
#define ELF_CORE_HEADER_ALIGN   1024

/* structure passed to crash_create_elf32/64_headers() */

struct crash_elf_info {
	unsigned long class;
	unsigned long data;
	unsigned long machine;

	unsigned long backup_src_start;
	unsigned long backup_src_end;

	unsigned long page_offset;
	unsigned long lowmem_limit;

	int (*get_note_info)(int cpu, uint64_t *addr, uint64_t *len);
};

int crash_create_elf32_headers(struct kexec_info *info,
			       struct crash_elf_info *elf_info,
			       struct memory_range *range, int ranges,
			       void **buf, unsigned long *size,
			       unsigned long align);

int crash_create_elf64_headers(struct kexec_info *info,
			       struct crash_elf_info *elf_info,
			       struct memory_range *range, int ranges,
			       void **buf, unsigned long *size,
			       unsigned long align);

unsigned long crash_architecture(struct crash_elf_info *elf_info);

int xen_present(void);
unsigned long xen_architecture(struct crash_elf_info *elf_info);
int xen_get_nr_phys_cpus(void);
int xen_get_note(int cpu, uint64_t *addr, uint64_t *len);

#endif /* CRASHDUMP_H */
