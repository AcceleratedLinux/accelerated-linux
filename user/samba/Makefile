FLTFLAGS = -s 0x8000

all:
	$(MAKE) -C source
clean:
	$(MAKE) -C source clean
romfs:
	$(MAKE) -C source romfs
ifdef CONFIG_USER_FLATFSD_ETC_CONFIG
	echo "[global]" > $(ROMFSDIR)/etc/default/smb.conf
else
	echo "[global]" > $(ROMFSDIR)/etc/smb.conf
endif
