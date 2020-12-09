URL := https://sourceforge.net/projects/cryptopp/files/cryptopp/5.6.1/cryptopp561.zip
AUTOMAKE_ROMFS = cryptopp_romfs
include $(ROOTDIR)/tools/automake.inc

cryptopp_romfs:
	$(ROMFSINST) build/cryptopp561-install/lib/libcryptopp.so /lib/libcryptopp.so
