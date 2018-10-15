URL = https://www.snort.org/downloads/community/community-rules.tar.gz

FINALTARGET = patched
AUTOMAKE_ROMFS = snortrules_romfs
include $(ROOTDIR)/tools/automake.inc

snortrules_romfs:
	mkdir -p $(ROMFSDIR)/etc/snort/rules
	$(ROMFSINST) /etc/snort/snort.conf
	$(ROMFSINST) build/community-rules/sid-msg.map /etc/snort/
	$(ROMFSINST) build/community-rules/community.rules /etc/snort/rules/
