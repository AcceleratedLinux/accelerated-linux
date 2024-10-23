URL      = https://github.com/mstpd/mstpd.git
VERSION  = c216c5da74c8a5aec4ad1ed4a0b37a7016acb320
METHOD   = git

AUTORECONF = ./autogen.sh
AUTOMAKE_ROMFS = mstpd_romfs
INSTALLDIR = build/mstpd-install

CONFOPTS =

CFLAGS   += -I$(ROOTDIR)/staging/include

include $(ROOTDIR)/tools/automake.inc

mstpd_romfs:
	$(ROMFSINST) $(INSTALLDIR)/sbin/mstpd /sbin
	$(ROMFSINST) $(INSTALLDIR)/sbin/mstpctl /sbin
	$(ROMFSINST) bridge-stp /sbin
