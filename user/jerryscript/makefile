URL     = https://github.com/jerryscript-project/jerryscript.git
METHOD  = git
VERSION = 2dbb6f79efa08767ce44fbc8e3f26e80649295d7
CMAKE_OPTS = -DENABLE_LTO=OFF

ifeq ($(CONFIG_DEFAULTS_LIBC_UCLIBC)$(CONFIG_DEFAULTS_LIBC_UCLIBC_NG),y)
CFLAGS += -D_GNU_SOURCE
endif

include $(ROOTDIR)/tools/cmake.inc
