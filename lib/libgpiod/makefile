URL = https://mirrors.edge.kernel.org/pub/software/libs/libgpiod/libgpiod-2.0.tar.xz
CONFVARS = ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes

MAKEVARS=

ifdef CONFIG_LIB_LIBGPIOD_TOOLS
CONFOPTS += --enable-tools
endif

# Note that for libgpiod-1.6 and earlier,
# .linux/config needs to contain CONFIG_GPIO_CDEV_V1=y
# for gpioinfo to work.

include $(ROOTDIR)/tools/automake.inc
