
METHOD      = git
URL         = https://github.com/shellinabox/shellinabox.git
VERSION     = 4f0ecc31ac6f985e0dd3f5a52cbfc0e9251f6361

CONFOPTS    = --disable-pam --disable-runtime-loading
CFLAGS     += -Dupdwtmp=failed
AUTORECONF  = autoreconf -i

ifdef CONFIG_DEFAULTS_LIBC_MUSL
# Musl does not fully support any of the glibc utmp functions
CONFOPTS   += --disable-utmp
endif

include $(ROOTDIR)/tools/automake.inc

