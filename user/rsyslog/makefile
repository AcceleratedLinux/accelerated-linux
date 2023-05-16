
RSYSLOG        = rsyslog-8.33.0
URL            = http://www.rsyslog.com/files/download/rsyslog/$(RSYSLOG).tar.gz
DOWNLOADHASH   = 2df39d91baddb75c575aa03525cd9d4d20aad75011c6d6d25ef773ac26ff5c12

$(RSYSLOG)_ROMFS = rsyslog_romfs

# override using jlibtool and use packages libtool
MAKEVARS = 

CONFOPTS += --enable-libgcrypt=no
CONFOPTS += --enable-liblogging-stdlog=no

include $(ROOTDIR)/tools/automake.inc

rsyslog_romfs:
	$(ROMFSINST) -p 0644 config/rsyslog.conf /etc/default/rsyslog.conf
	$(ROMFSINST) -a "slog:unknown:/sbin/rsyslogd -n -f /etc/config/rsyslog.conf" /etc/default/inittab.d/slog
	rm -f $(ROMFSDIR)/lib/rsyslog/*.la

