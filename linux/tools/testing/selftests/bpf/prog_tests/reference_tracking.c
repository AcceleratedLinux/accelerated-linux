// SPDX-License-Identifier: GPL-2.0
#include <test_progs.h>

static int libbpf_debug_print(enum libbpf_print_level level,
			      const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG)
		return 0;

	return vfprintf(stderr, format, args);
}

void test_reference_tracking(void)
{
	const char *file = "./test_sk_lookup_kern.o";
	struct bpf_object *obj;
	struct bpf_program *prog;
	__u32 duration = 0;
	int err = 0;

	obj = bpf_object__open(file);
	if (IS_ERR(obj)) {
		error_cnt++;
		return;
	}

	bpf_object__for_each_program(prog, obj) {
		const char *title;

		/* Ignore .text sections */
		title = bpf_program__title(prog, false);
		if (strstr(title, ".text") != NULL)
			continue;

		bpf_program__set_type(prog, BPF_PROG_TYPE_SCHED_CLS);

		/* Expect verifier failure if test name has 'fail' */
		if (strstr(title, "fail") != NULL) {
			libbpf_set_print(NULL);
			err = !bpf_program__load(prog, "GPL", 0);
			libbpf_set_print(libbpf_debug_print);
		} else {
			err = bpf_program__load(prog, "GPL", 0);
		}
		CHECK(err, title, "\n");
	}
	bpf_object__close(obj);
}
