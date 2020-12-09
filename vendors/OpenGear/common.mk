# -*- makefile -*-
#
# 	common.mk -- common initialisation tasks on romfs for all targets
#	pchunt@opengear.com
#

.PHONY: romfs
romfs:
	$(ROMFSINST) -s /etc/config/bash_profile /.profile

	$(ROMFSINST) -s /var/tmp /tmp
ifndef CONFIG_USER_UDEV
	$(ROMFSINST) -s /var/tmp/log /dev/log
endif
	$(ROMFSINST) -s /etc/config/passwd /etc/passwd
	$(ROMFSINST) -s /etc/config/group /etc/group
	$(ROMFSINST) -s /etc/config/shadow /etc/shadow
	$(ROMFSINST) -s /etc/config/gshadow /etc/gshadow
	$(ROMFSINST) -s /etc/config/pam.d /etc/pam.d
	$(ROMFSINST) -s /etc/config/TZ /etc/TZ
	$(ROMFSINST) -s /etc/config /etc/Wireless
	$(ROMFSINST) -s /etc/config/gai.conf /etc/gai.conf
	$(ROMFSINST) -s /var/run /run 			# for udev
	$(ROMFSINST) -A "inet:" -a "inet:unknown:/bin/inetd" /etc/inittab
	$(ROMFSINST) -A "fltd:" -a "fltd:unknown:/bin/flatfsd" /etc/inittab

	# Setup correct permissions on /etc/ files
	chmod 660 $(ROMFSDIR)/etc/default/config.xml

	# Make standard scripts executable
	if [ -d $(ROMFSDIR)/etc/scripts ] ; then \
		chmod g-w,o-w $(ROMFSDIR)/etc/scripts; \
		find $(ROMFSDIR)/etc/scripts -type f -print0 | xargs -r0 chmod 555; \
	fi

	# fix up permissions for scripts directory -- ssh doesn't like it being
	# group or world writable
	chmod g-w,o-w $(ROMFSDIR)/etc/

.PHONY: romfs.post
romfs.post::

	# Write the timestamp to /etc/version last
	echo "$(VERSIONSTR) -- $(shell date -d "$(BUILDTIMESTAMP)" -u)" > $(ROMFSDIR)/etc/version
