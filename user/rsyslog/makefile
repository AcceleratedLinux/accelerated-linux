
RSYSLOG        = rsyslog-8.2304.0
URL            = https://www.rsyslog.com/files/download/rsyslog/$(RSYSLOG).tar.gz
DOWNLOADHASH   = d090e90283eb4b80de8b43e5ffc6e4b59c4e3970f2aa91e63beef0a11720d74d

$(RSYSLOG)_ROMFS = rsyslog_romfs

MAKEVARS = 
LDFLAGS += -lz

CONFOPTS += --enable-libgcrypt=no
CONFOPTS += --enable-liblogging-stdlog=no

include $(ROOTDIR)/tools/automake.inc

rsyslog_romfs:
	$(ROMFSINST) -p 0644 config/rsyslog.conf /etc/default/rsyslog.conf
	$(ROMFSINST) -d -a "slog:unknown:/sbin/rsyslogd -n -f /etc/config/rsyslog.conf" /etc/default/inittab.d/slog
	rm -f $(ROMFSDIR)/lib/rsyslog/*.la

