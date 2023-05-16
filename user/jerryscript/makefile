URL     = https://github.com/jerryscript-project/jerryscript.git
METHOD  = git
VERSION = v2.4.0
CMAKE_OPTS = -DENABLE_LTO=OFF

ifeq ($(CONFIG_DEFAULTS_LIBC_UCLIBC)$(CONFIG_DEFAULTS_LIBC_UCLIBC_NG),y)
CFLAGS += -D_GNU_SOURCE
endif

include $(ROOTDIR)/tools/cmake.inc
