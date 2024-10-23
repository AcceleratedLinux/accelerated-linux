
URL = https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.42.tar.bz2
ifneq ($(findstring gnueabi,$(CC)),)
# fix arm-unknown-linux-hnu to be arm-unknown-linux-gnueabi
CONFIGURE_OPTS += --host=$(TARGETARCH)-gnueabi
endif
MAKEVARS = 

include $(ROOTDIR)/tools/automake.inc
