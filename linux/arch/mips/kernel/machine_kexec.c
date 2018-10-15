/*
 * machine_kexec.c for kexec
 * Created by <nschichan@corp.free.fr> on Thu Oct 12 15:15:06 2006
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */
#include <linux/compiler.h>
#include <linux/kexec.h>
#include <linux/mm.h>
#include <linux/delay.h>

#include <asm/cacheflush.h>
#include <asm/page.h>

extern const unsigned char relocate_new_kernel[];
extern const size_t relocate_new_kernel_size;

extern unsigned long kexec_start_address;
extern unsigned long kexec_indirection_page;

int (*_machine_kexec_prepare)(struct kimage *) = NULL;
void (*_machine_kexec_shutdown)(void) = NULL;
void (*_machine_crash_shutdown)(struct pt_regs *regs) = NULL;
#ifdef CONFIG_SMP
void (*relocated_kexec_smp_wait) (void *);
atomic_t kexec_ready_to_reboot = ATOMIC_INIT(0);
atomic_t kexec_smp_reboot_waiting = ATOMIC_INIT(0);
void (*_crash_smp_send_stop)(void) = NULL;
#endif

extern unsigned long fw_arg0, fw_arg1, fw_arg2, fw_arg3;

static unsigned long reboot_code_buffer;

#define KEXEC_ARGS_MAGIC        "_KeXeC ArGs_"
#define MAX_COMMAND_LINE        2048
#define MAX_COMMAND_ARGS        32

struct args_page {
	char    magic[16];
	int     argc;
	char	*argvp;
	char    *argv[MAX_COMMAND_ARGS];
	char    argbuf[MAX_COMMAND_LINE];
};

static void set_args(void *pg)
{
	struct args_page *ap = pg;
	int i;

	kexec_args[0] = ap->argc;
	kexec_args[1] = (unsigned long) phys_to_virt((unsigned long) ap->argvp);

	/* Convert physical addresses to flat virtual addresses */
	for (i = 0; (i < ap->argc); i++)
		ap->argv[i] = phys_to_virt((unsigned long) ap->argv[i]);
}

static void scan_for_args(struct kimage *image)
{
	unsigned long entry;
	unsigned long *ptr;
	void *pg;

	/* Scan pages for real argc/argv list */
	for (ptr = &image->head; (entry = *ptr) && !(entry &IND_DONE);
	     ptr = (entry & IND_INDIRECTION) ? phys_to_virt(entry & PAGE_MASK) : ptr + 1) {
		if (*ptr & IND_SOURCE) {
			pg = phys_to_virt(*ptr & PAGE_MASK);
			if (memcmp(pg, KEXEC_ARGS_MAGIC, sizeof(KEXEC_ARGS_MAGIC)) == 0)
				set_args(pg);
		}
	}
}

static void kexec_image_info(const struct kimage *kimage)
{
	unsigned long i;

	pr_debug("kexec kimage info:\n");
	pr_debug("  type:        %d\n", kimage->type);
	pr_debug("  start:       %lx\n", kimage->start);
	pr_debug("  head:        %lx\n", kimage->head);
	pr_debug("  nr_segments: %lu\n", kimage->nr_segments);

	for (i = 0; i < kimage->nr_segments; i++) {
		pr_debug("    segment[%lu]: %016lx - %016lx, 0x%lx bytes, %lu pages\n",
			i,
			kimage->segment[i].mem,
			kimage->segment[i].mem + kimage->segment[i].memsz,
			(unsigned long)kimage->segment[i].memsz,
			(unsigned long)kimage->segment[i].memsz /  PAGE_SIZE);
	}
}

int
machine_kexec_prepare(struct kimage *kimage)
{
	kexec_args[0] = fw_arg0;
	kexec_args[1] = fw_arg1;
	kexec_args[2] = fw_arg2;
	kexec_args[3] = fw_arg3;
#ifdef CONFIG_SMP
	secondary_kexec_args[0] = fw_arg0;
	secondary_kexec_args[1] = fw_arg1;
	secondary_kexec_args[2] = 0;
	secondary_kexec_args[3] = 0;
#endif

	kexec_image_info(kimage);

	if (_machine_kexec_prepare)
		return _machine_kexec_prepare(kimage);
	return 0;
}

void
machine_kexec_cleanup(struct kimage *kimage)
{
}

void
machine_shutdown(void)
{
	if (_machine_kexec_shutdown)
		_machine_kexec_shutdown();
}

void
machine_crash_shutdown(struct pt_regs *regs)
{
	if (_machine_crash_shutdown)
		_machine_crash_shutdown(regs);
	else
		default_machine_crash_shutdown(regs);
}

typedef void (*noretfun_t)(void) __noreturn;

void kexec_start_new_kernel(unsigned long reboot_code_buffer)
{
	local_irq_disable();
	printk(KERN_EMERG "Bye ...\n");
	__flush_cache_all();
    	((noretfun_t) reboot_code_buffer)();
}

#ifdef CONFIG_SMP
/*
 * All CPU cores will enter here ready to start new kernel. We only start
 * the new kernel proper on the boot CPU (that is CPU 0). All the others
 * spin waiting (but they have to spin in the relocated startup code).
 */
void
machine_kexec_smp(void *info)
{
	int ncpus;

	local_irq_disable();

	/* All secondary CPU's wait for reboot on primary CPU */
	if (smp_processor_id() != 0) {
		atomic_inc(&kexec_smp_reboot_waiting);
		((noretfun_t) relocated_kexec_smp_wait)();
	}

	ncpus = num_online_cpus() - 1;
	while (atomic_read(&kexec_smp_reboot_waiting) < ncpus)
		;

	/* Ready to kexec on the primary boot CPU now */
	kexec_start_new_kernel(reboot_code_buffer);
}
#endif

void
machine_kexec(struct kimage *image)
{
	unsigned long entry;
	unsigned long *ptr;

	reboot_code_buffer =
	  (unsigned long)page_address(image->control_code_page);

	kexec_start_address =
		(unsigned long) phys_to_virt(image->start);

	if (image->type == KEXEC_TYPE_DEFAULT) {
		kexec_indirection_page =
			(unsigned long) phys_to_virt(image->head & PAGE_MASK);
	} else {
		kexec_indirection_page = (unsigned long)&image->head;
	}

	memcpy((void*)reboot_code_buffer, relocate_new_kernel,
	       relocate_new_kernel_size);

	scan_for_args(image);

	/*
	 * The generic kexec code builds a page list with physical
	 * addresses. they are directly accessible through KSEG0 (or
	 * CKSEG0 or XPHYS if on 64bit system), hence the
	 * phys_to_virt() call.
	 */
	for (ptr = &image->head; (entry = *ptr) && !(entry &IND_DONE);
	     ptr = (entry & IND_INDIRECTION) ?
	       phys_to_virt(entry & PAGE_MASK) : ptr + 1) {
		if (*ptr & IND_SOURCE || *ptr & IND_INDIRECTION ||
		    *ptr & IND_DESTINATION)
			*ptr = (unsigned long) phys_to_virt(*ptr);
	}

	printk(KERN_EMERG "Will call new kernel at %08lx\n", image->start);

#ifdef CONFIG_SMP
	/* All secondary cpus now may jump to kexec_wait cycle */
	relocated_kexec_smp_wait = (void *)(reboot_code_buffer +
		(kexec_smp_wait - relocate_new_kernel));
	smp_wmb();
	atomic_set(&kexec_ready_to_reboot,1);

	smp_call_function(machine_kexec_smp, NULL, 0);
	machine_kexec_smp(NULL);
#else
	kexec_start_new_kernel(reboot_code_buffer);
#endif
}

/* crashkernel=[13]size at addr specifies the location to reserve for
 * a crash kernel.  By reserving this memory we guarantee
 * that linux never sets it up as a DMA target.
 * Useful for holding code to do something appropriate
 * after a kernel panic.
 */
static int __init parse_crashkernel_cmdline(char *arg)
{
	unsigned long size, base;
	size = memparse(arg, &arg);
	if (*arg == '@') {
		base = memparse(arg+1, &arg);
		/* FIXME: Do I want a sanity check
		 * to validate the memory range?
		 */
		crashk_res.start = base;
		crashk_res.end   = base + size - 1;
	}
	return 0;
}
early_param("crashkernel", parse_crashkernel_cmdline);
