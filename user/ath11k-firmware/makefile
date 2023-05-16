export ACL_LICENSE = Qualcomm License

all:

romfs:
	$(ROMFSINST) -d vendor_file/Compex/amss.bin /lib/firmware/ath11k/compex/amss.bin
	$(ROMFSINST) -d vendor_file/Compex/m3.bin /lib/firmware/ath11k/compex/m3.bin
	$(ROMFSINST) -d vendor_file/Compex/WLE3002HX_2G.bin /lib/firmware/ath11k/compex/board-2400mhz.bin
	$(ROMFSINST) -d vendor_file/Compex/WLE3002HX_5G.bin /lib/firmware/ath11k/compex/board-5000mhz.bin
	$(ROMFSINST) -d vendor_file/SparkLan/amss.bin /lib/firmware/ath11k/sparklan/amss.bin
	$(ROMFSINST) -d vendor_file/SparkLan/m3.bin /lib/firmware/ath11k/sparklan/m3.bin
	$(ROMFSINST) -d vendor_file/SparkLan/board.bin /lib/firmware/ath11k/sparklan/board.bin

clean: