export ACL_LICENSE = Qualcomm License

all:

romfs:
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_COMPEX -d vendor_file/Compex/WLE3002HX_2G.bin /lib/firmware/ath11k/QCN9074/hw1.0/compex-2400mhz.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_COMPEX -d vendor_file/Compex/WLE3002HX_5G.bin /lib/firmware/ath11k/QCN9074/hw1.0/compex-5000mhz.bin

	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_QCN9074 -d vendor_file/SparkLan/QCN9074/amss.bin /lib/firmware/ath11k/QCN9074/hw1.0/amss.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_QCN9074 -d vendor_file/SparkLan/QCN9074/m3.bin /lib/firmware/ath11k/QCN9074/hw1.0/m3.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_QCN9074 -d vendor_file/SparkLan/QCN9074/board.bin /lib/firmware/ath11k/QCN9074/hw1.0/sparklan-board.bin

#	Legacy 6855 SparkLan driver 3/3/2022
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/amss.bin /lib/firmware/ath11k/WCN6855/hw2.1/amss.bin
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/m3.bin /lib/firmware/ath11k/WCN6855/hw2.1/m3.bin
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/board.bin /lib/firmware/ath11k/WCN6855/hw2.1/board.bin
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/regdb.bin /lib/firmware/ath11k/WCN6855/hw2.1/regdb.bin

#	Reuse legacy board.bin and custom regdb.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/board.bin /lib/firmware/ath11k/WCN6855/hw2.0/board.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/WCN6855/regdb.bin /lib/firmware/ath11k/WCN6855/hw2.0/regdb.bin

#	r00017.9 6855 4/25/2023
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/firmware_update/r00017.9/WCN6855/hw2.0/amss.bin /lib/firmware/ath11k/WCN6855/hw2.0/amss.bin
#	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/firmware_update/r00017.9/WCN6855/hw2.0/m3.bin /lib/firmware/ath11k/WCN6855/hw2.0/m3.bin

#	r00018.2 6855 9/7/2023
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/firmware_update/r00018.2/WCN6855/hw2.0/amss.bin /lib/firmware/ath11k/WCN6855/hw2.0/amss.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d vendor_file/SparkLan/firmware_update/r00018.2/WCN6855/hw2.0/m3.bin /lib/firmware/ath11k/WCN6855/hw2.0/m3.bin

#	Symbolic link hw2.1 to hw2.0 as with standard ath11k
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -s /lib/firmware/ath11k/WCN6855/hw2.0 /lib/firmware/ath11k/WCN6855/hw2.1

#	The QCA206x driver will be replacing the ATH11K driver for the WCN6855
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d qca_firmware/bdwlang.elf /lib/firmware/bdwlang.elf
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d qca_firmware/regdb.bin /lib/firmware/regdb.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d qca_firmware/amss.bin /lib/firmware/amss.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -d qca_firmware/m3.bin /lib/firmware/m3.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_SPARKLAN_WCN6855 -f qca_firmware/wlan /lib/firmware/wlan

	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_VOXMICRO -d vendor_file/Voxmicro/amss.bin /lib/firmware/ath11k/QCA6390/hw2.0/amss.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_VOXMICRO -d vendor_file/Voxmicro/m3.bin /lib/firmware/ath11k/QCA6390/hw2.0/m3.bin
	$(ROMFSINST) -e CONFIG_USER_ATH11K_FIRMWARE_VOXMICRO -d vendor_file/Voxmicro/board-2.bin /lib/firmware/ath11k/QCA6390/hw2.0/board-2.bin

clean:
	$(ROMFSINST) -R /lib/firmware/ath11k
	$(ROMFSINST) -R /lib/firmware/bdwlang.elf
	$(ROMFSINST) -R /lib/firmware/regdb.bin
	$(ROMFSINST) -R /lib/firmware/amss.bin
	$(ROMFSINST) -R /lib/firmware/m3.bin
	$(ROMFSINST) -R /lib/firmware/wlan
