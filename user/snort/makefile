SNORT = snort-2.9.8.2
URL = https://www.snort.org/downloads/archive/snort/$(SNORT).tar.gz

CONFOPTS += --with-libpcre-includes=$(STAGEDIR)/include
CONFOPTS += --with-libpcre-libraries=$(STAGEDIR)/lib

MAKEVARS = 
$(SNORT)_ROMFS = snort_romfs

include $(ROOTDIR)/tools/automake.inc

snort_romfs:
	mkdir -p $(ROMFSDIR)/etc/snort/preproc_rules
	$(ROMFSINST) build/$(SNORT)/etc/classification.config /etc/snort/
	$(ROMFSINST) build/$(SNORT)/etc/reference.config /etc/snort/
	$(ROMFSINST) build/$(SNORT)/etc/threshold.conf /etc/snort/
	$(ROMFSINST) build/$(SNORT)/etc/gen-msg.map /etc/snort/
	$(ROMFSINST) build/$(SNORT)/etc/unicode.map /etc/snort/
	$(ROMFSINST) build/$(SNORT)/preproc_rules/decoder.rules /etc/snort/preproc_rules/
	$(ROMFSINST) build/$(SNORT)/preproc_rules/preprocessor.rules /etc/snort/preproc_rules/
	$(ROMFSINST) build/$(SNORT)/preproc_rules/sensitive-data.rules /etc/snort/preproc_rules/

