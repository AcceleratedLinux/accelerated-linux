URL = https://mirrors.edge.kernel.org/pub/software/network/tftp/tftp-hpa/tftp-hpa-5.2.tar.xz

CONFOPTS += --without-tcpwrappers
CONFOPTS += --disable-doc

MAKEVARS = INSTALLROOT=`pwd`/build/tftp-hpa-5.2-install

include $(ROOTDIR)/tools/automake.inc

romfs:
	$(ROMFSINST) -a "tftp dgram udp wait root /sbin/in.tftpd /factory -s" /etc/inetd.conf

