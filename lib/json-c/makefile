
URL = https://s3.amazonaws.com/json-c_releases/releases/json-c-0.15.tar.gz
CONFVARS = ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes

ifdef CONFIG_DEFAULTS_LIBC_UCLIBC
# uClibc needs to add link of math library as well
CFLAGS += -lm
endif

include $(ROOTDIR)/tools/cmake.inc

