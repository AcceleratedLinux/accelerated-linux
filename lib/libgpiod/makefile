URL = https://mirrors.edge.kernel.org/pub/software/libs/libgpiod/libgpiod-1.4.tar.gz
CONFVARS = ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes

MAKEVARS=

ifdef CONFIG_LIB_LIBGPIOD_TOOLS
CONFOPTS += --enable-tools
endif

include $(ROOTDIR)/tools/automake.inc
