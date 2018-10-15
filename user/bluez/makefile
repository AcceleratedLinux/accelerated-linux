VERSION = 5.46
URL = http://www.kernel.org/pub/linux/bluetooth/bluez-$(VERSION).tar.xz

CONFOPTS += --disable-systemd

ifdef CONFIG_USER_BLUEZ_BLUETOOTHCTRL
CONFOPTS += --enable-client
else
CONFOPTS += --disable-client
endif

ifdef CONFIG_USER_BLUEZ_TOOLS
CONFOPTS += --enable-tools
else
CONFOPTS += --disable-tools
endif

ifdef CONFIG_USER_BLUEZ_TOOLS_DEPRECATED
CONFOPTS += --enable-deprecated
else
CONFOPTS += --disable-deprecated
endif

ifdef CONFIG_USER_BLUEZ_BTMON
CONFOPTS += --enable-monitor
else
CONFOPTS += --disable-monitor
endif

ifdef CONFIG_USER_BLUEZ_LIBBLUETOOTH
CONFOPTS += --enable-library
else
CONFOPTS += --disable-library
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_SAP
CONFOPTS += --enable-sap
else
CONFOPTS += --disable-sap
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_A2DP
CONFOPTS += --enable-a2dp
else
CONFOPTS += --disable-a2dp
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_AVRCP
CONFOPTS += --enable-avrcp
else
CONFOPTS += --disable-avrcp
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_NETWORK
CONFOPTS += --enable-network
else
CONFOPTS += --disable-network
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_HID
CONFOPTS += --enable-hid
else
CONFOPTS += --disable-hid
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_HOG
CONFOPTS += --enable-hog
else
CONFOPTS += --disable-hog
endif

ifdef CONFIG_USER_BLUEZ_PROFILE_HEALTH
CONFOPTS += --enable-health
else
CONFOPTS += --disable-health
endif

ifdef CONFIG_USER_BLUEZ_CUPS
CONFOPTS += --enable-cups
else
CONFOPTS += --disable-cups
endif

ifdef CONFIG_USER_BLUEZ_MIDI
CONFOPTS += --enable-midi
else
CONFOPTS += --disable-midi
endif

ifdef CONFIG_USER_BLUEZ_OBEX
CONFOPTS += --enable-obex
else
CONFOPTS += --disable-obex
endif

ifdef CONFIG_USER_BLUEZ_NFC
CONFOPTS += --enable-nfc
else
CONFOPTS += --disable-nfc
endif

ifdef CONFIG_USER_UDEV
CONFOPTS += --enable-udev
else
CONFOPTS += --disable-udev
endif

CONFOPTS += --localstatedir /etc/config/ble

# Android compatability wordexp and readline are stubs only.
ifndef CONFIG_UCLIBC_HAS_WORDEXP
# Compatability wordexp limits functionality of bluetoothctl (dbus GATT
# commands), btmgmt, and obexctl
CFLAGS += -idirafter $(ROOTDIR)/user/bluez/build/bluez-$(VERSION)/android/compat
endif
ifndef CONFIG_USER_READLINE
# Compatability readline severely limits functionality of bluetoothctl, btmgmt,
# and obexctl. If you don't have readline, don't bother building those.
CFLAGS += -idirafter $(ROOTDIR)/user/bluez/build/bluez-$(VERSION)/android/compat
endif

ROMFS = extra_romfs

include $(ROOTDIR)/tools/automake.inc

extra_romfs:
	$(ROMFSINST) -e CONFIG_USER_BLUEZ_TOOLS build/bluez-$(VERSION)/tools/btmgmt /bin/btmgmt
