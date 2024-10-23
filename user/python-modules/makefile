include python-modules.mk

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl
CPPFLAGS += -I$(STAGEDIR)/include -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib

###############################################################################
ifeq ($(or $(CONFIG_USER_PYTHON_MODULES_SETUPTOOLS),$(CONFIG_USER_PYTHON_MODULES_SETUPTOOLS_HOST)),y)

SETUPTOOLS_VERSION = 41.0.1
SETUPTOOLS_PKG = setuptools-$(SETUPTOOLS_VERSION)
$(SETUPTOOLS_PKG)_URL = https://github.com/pypa/setuptools/archive/v$(SETUPTOOLS_VERSION).tar.gz
$(SETUPTOOLS_PKG)_DOWNLOADNAME = $(SETUPTOOLS_PKG).tar.gz
$(SETUPTOOLS_PKG)_PATCHES += 0001-update-deprecated-html-parser.patch
$(SETUPTOOLS_PKG)_PATCHES += 0002-Remove-bdist_wininst-Filter-out-distutils-deprecation.patch

# currently we can only hostinstall non compiled modules,  a little more
# work will be needed if we need to compile modules for the build python
$(SETUPTOOLS_PKG)_HOSTINSTALL = 1
ifndef CONFIG_USER_PYTHON_MODULES_SETUPTOOLS
$(SETUPTOOLS_PKG)_NOTARGETINSTALL = 1
endif

AUTOMAKE_y += $(SETUPTOOLS_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PIP

PIP_VERSION = 21.3
PIP_PKG = pip-$(PIP_VERSION)
$(PIP_PKG)_URL = https://github.com/pypa/pip/archive/$(PIP_VERSION).tar.gz
$(PIP_PKG)_DOWNLOADNAME = $(PIP_PKG).tar.gz
$(PIP_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(PIP_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_SIX

SIX_VERSION = 1.12.0
SIX_PKG = six-$(SIX_VERSION)
$(SIX_PKG)_URL = https://github.com/benjaminp/six/archive/$(SIX_VERSION).tar.gz
$(SIX_PKG)_DOWNLOADNAME = $(SIX_PKG).tar.gz

AUTOMAKE_y += $(SIX_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_CFFI

CFFI_VERSION = 1.12.3
CFFI_PKG = cffi-$(CFFI_VERSION)
$(CFFI_PKG)_URL = https://files.pythonhosted.org/packages/93/1a/ab8c62b5838722f29f3daffcc8d4bd61844aa9b5f437341cc890ceee483b/$(CFFI_PKG).tar.gz

AUTOMAKE_y += $(CFFI_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PYSERIAL

PYSERIAL_VERSION = 3.4
PYSERIAL_PKG = pyserial-$(PYSERIAL_VERSION)
$(PYSERIAL_PKG)_URL = https://github.com/pyserial/pyserial/archive/v$(PYSERIAL_VERSION).tar.gz
$(PYSERIAL_PKG)_DOWNLOADNAME = $(PYSERIAL_PKG).tar.gz

AUTOMAKE_y += $(PYSERIAL_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PYSERIAL_ASYNCIO

PYSERIAL_ASYNCIO_VERSION = 0.4
PYSERIAL_ASYNCIO_PKG = pyserial-asyncio-$(PYSERIAL_ASYNCIO_VERSION)
$(PYSERIAL_ASYNCIO_PKG)_URL = https://github.com/pyserial/pyserial-asyncio/archive/v$(PYSERIAL_ASYNCIO_VERSION).tar.gz
$(PYSERIAL_ASYNCIO_PKG)_DOWNLOADNAME = $(PYSERIAL_ASYNCIO_PKG).tar.gz

AUTOMAKE_y += $(PYSERIAL_ASYNCIO_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PYMODBUS

PYMODBUS_VERSION = 2.3.0
PYMODBUS_PKG = pymodbus-$(PYMODBUS_VERSION)
$(PYMODBUS_PKG)_URL = https://github.com/riptideio/pymodbus/archive/v$(PYMODBUS_VERSION).tar.gz
$(PYMODBUS_PKG)_DOWNLOADNAME = $(PYMODBUS_PKG).tar.gz

AUTOMAKE_y += $(PYMODBUS_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_BCRYPT

BCRYPT_VERSION = 3.1.6
BCRYPT_PKG = bcrypt-$(BCRYPT_VERSION)
$(BCRYPT_PKG)_URL = https://github.com/pyca/bcrypt/archive/$(BCRYPT_VERSION).tar.gz
$(BCRYPT_PKG)_DOWNLOADNAME = $(BCRYPT_PKG).tar.gz
$(BCRYPT_PKG)_DEP = $(CFFI_PKG) $(SIX_PKG)

AUTOMAKE_y += $(BCRYPT_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PEXPECT

PEXPECT_VERSION = 4.7.0
PEXPECT_PKG = pexpect-$(PEXPECT_VERSION)
$(PEXPECT_PKG)_URL = https://files.pythonhosted.org/packages/1c/b1/362a0d4235496cb42c33d1d8732b5e2c607b0129ad5fdd76f5a583b9fcb3/$(PEXPECT_PKG).tar.gz

AUTOMAKE_y += $(PEXPECT_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PTYPROCESS

PTYPROCESS_VERSION = 0.6.0
PTYPROCESS_PKG = ptyprocess-$(PTYPROCESS_VERSION)
$(PTYPROCESS_PKG)_URL = https://files.pythonhosted.org/packages/7d/2d/e4b8733cf79b7309d84c9081a4ab558c89d8c89da5961bf4ddb050ca1ce0/$(PTYPROCESS_PKG).tar.gz

AUTOMAKE_y += $(PTYPROCESS_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_DIGI_XBEE_LIB

XBEE_LIB_PKG = python-xbee
$(XBEE_LIB_PKG)_VERSION = cbd2cfe5ed18ff217cb97c565463be7879055be6
$(XBEE_LIB_PKG)_METHOD = git
$(XBEE_LIB_PKG)_URL = https://github.com/digidotcom/xbee-python.git
$(XBEE_LIB_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(XBEE_LIB_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_AWS_SDK

AWS_LIB_PKG = aws-iot-device-sdk-python
$(AWS_LIB_PKG)_VERSION = v1.4.7
$(AWS_LIB_PKG)_METHOD = git
$(AWS_LIB_PKG)_URL = https://github.com/aws/aws-iot-device-sdk-python.git

AUTOMAKE_y += $(AWS_LIB_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_SRP

SRP_VERSION = 1.0.14
SRP_PKG = srp-$(SRP_VERSION)
$(SRP_PKG)_URL = https://pypi.io/packages/source/s/srp/$(SRP_PKG).tar.gz
$(SRP_PKG)_DOWNLOADHASH = 34448d381f59783e88041eb7e049861279bd26e1c88578bc15e9fdcd2bacd3b1

AUTOMAKE_y += $(SRP_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PAHO_MQTT

PAHO_MQTT_VERSION = 1.6.1
PAHO_MQTT_PKG = paho.mqtt.python-$(PAHO_MQTT_VERSION)
$(PAHO_MQTT_PKG)_URL = https://github.com/eclipse/paho.mqtt.python/archive/v$(PAHO_MQTT_VERSION).tar.gz
$(PAHO_MQTT_PKG)_DOWNLOADNAME = $(PAHO_MQTT_PKG).tar.gz
$(PAHO_MQTT_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(PAHO_MQTT_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_HID

HID_PKG = hid
$(HID_PKG)_VERSION = ec92d96dab18a23f66d2ebcf853b38f47d258545
$(HID_PKG)_METHOD = git
$(HID_PKG)_URL = https://github.com/apmorton/pyhidapi.git
$(HID_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(HID_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_PYSOCKS

PYSOCKS_VERSION = 1.7.0
PYSOCKS_PKG = PySocks-$(PYSOCKS_VERSION)
$(PYSOCKS_PKG)_URL = https://github.com/Anorov/PySocks/archive/$(PYSOCKS_VERSION).tar.gz
$(PYSOCKS_PKG)_DOWNLOADNAME = $(PYSOCKS_PKG).tar.gz
$(PYSOCKS_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(PYSOCKS_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_CHARDET

CHARDET_VERSION = 3.0.4
CHARDET_PKG = chardet-$(CHARDET_VERSION)
$(CHARDET_PKG)_URL = https://github.com/chardet/chardet/archive/$(CHARDET_VERSION).tar.gz
$(CHARDET_PKG)_DOWNLOADNAME = $(CHARDET_PKG).tar.gz
$(CHARDET_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(CHARDET_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_CERTIFI

CERTIFI_VERSION = 2020.06.20
CERTIFI_PKG = python-certifi-$(CERTIFI_VERSION)
$(CERTIFI_PKG)_URL = https://github.com/certifi/python-certifi/archive/$(CERTIFI_VERSION).tar.gz
$(CERTIFI_PKG)_DOWNLOADNAME = $(CERTIFI_PKG).tar.gz
$(CERTIFI_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(CERTIFI_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_URLLIB3

URLLIB3_VERSION = 1.25.10
URLLIB3_PKG = urllib3-$(URLLIB3_VERSION)
$(URLLIB3_PKG)_URL = https://github.com/urllib3/urllib3/archive/$(URLLIB3_VERSION).tar.gz
$(URLLIB3_PKG)_DOWNLOADNAME = $(URLLIB3_PKG).tar.gz
$(URLLIB3_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(URLLIB3_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_IDNA

IDNA_VERSION = 2.10
IDNA_PKG = idna-$(IDNA_VERSION)
$(IDNA_PKG)_URL = https://github.com/kjd/idna/archive/v$(IDNA_VERSION).tar.gz
$(IDNA_PKG)_DOWNLOADNAME = $(IDNA_PKG).tar.gz
$(IDNA_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(IDNA_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_REQUESTS

REQUESTS_VERSION = 2.24.0
REQUESTS_PKG = requests-$(REQUESTS_VERSION)
$(REQUESTS_PKG)_URL = https://github.com/psf/requests/archive/v$(REQUESTS_VERSION).tar.gz
$(REQUESTS_PKG)_DOWNLOADNAME = $(REQUESTS_PKG).tar.gz
$(REQUESTS_PKG)_DEP = $(SETUPTOOLS_PKG) $(CHARDET_PKG) $(CERTIFI_PKG) \
		    = $(URLLIB3_PKG) $(IDNA_PKG)

AUTOMAKE_y += $(REQUESTS_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_REQUESTS_UNIXSOCKET

REQUESTS_UNIXSOCKET_VERSION = 0.1.5
REQUESTS_UNIXSOCKET_PKG = requests-unixsocket-$(REQUESTS_UNIXSOCKET_VERSION)
$(REQUESTS_UNIXSOCKET_PKG)_URL = https://github.com/msabramo/requests-unixsocket/archive/$(REQUESTS_UNIXSOCKET_VERSION).tar.gz
$(REQUESTS_UNIXSOCKET_PKG)_DOWNLOADNAME = $(REQUESTS_UNIXSOCKET_PKG).tar.gz
$(REQUESTS_UNIXSOCKET_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(REQUESTS_UNIXSOCKET_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_JANUS

JANUS_VERSION = 0.5.0
JANUS_PKG = janus-$(JANUS_VERSION)
$(JANUS_PKG)_URL = https://github.com/aio-libs/janus/archive/v$(JANUS_VERSION).tar.gz
$(JANUS_PKG)_DOWNLOADNAME = $(JANUS_PKG).tar.gz
$(JANUS_PKG)_DEP = $(SETUPTOOLS_PKG)

AUTOMAKE_y += $(JANUS_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_AZURE_IOT_DEVICE

AZURE_IOT_DEVICE_VERSION = 2.1.4
AZURE_IOT_DEVICE_DATE = 2020-07-02
AZURE_IOT_DEVICE_PKG = azure-iot-device-$(AZURE_IOT_DEVICE_VERSION)
$(AZURE_IOT_DEVICE_PKG)_URL = https://github.com/Azure/azure-iot-sdk-python/releases/download/$(AZURE_IOT_DEVICE_DATE)/azure-iot-device-$(AZURE_IOT_DEVICE_VERSION).tar.gz
$(AZURE_IOT_DEVICE_PKG)_DEP = $(SETUPTOOLS_PKG) $(SIX_PKG) $(PYSOCKS_PKG) \
			      $(REQUESTS_PKG) $(URLLIB3_PKG)

AUTOMAKE_y += $(AZURE_IOT_DEVICE_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_TACACS_PLUS

TACACS_PLUS_VERSION = 2.6
TACACS_PLUS_PKG = tacacs_plus-$(TACACS_PLUS_VERSION)
$(TACACS_PLUS_PKG)_URL = https://github.com/ansible/tacacs_plus/archive/refs/tags/$(TACACS_PLUS_VERSION).tar.gz
$(TACACS_PLUS_PKG)_DEP = $(SETUPTOOLS_PKG) $(SIX_PKG)

AUTOMAKE_y += $(TACACS_PLUS_PKG)
endif

###############################################################################

ifdef CONFIG_USER_PYTHON_MODULES_UBUS

PYTHON_UBUS_VERSION = 0.1.2
PYTHON_UBUS_PKG = ubus-$(PYTHON_UBUS_VERSION)
$(PYTHON_UBUS_PKG)_URL = https://pypi.io/packages/source/u/ubus/$(PYTHON_UBUS_PKG).tar.gz
$(PYTHON_UBUS_PKG)_DOWNLOADHASH = 4dc4ef0fbcc8abb7a2354691475a58ff3eb015f1bab3150750729f7f657dd440
$(PYTHON_UBUS_PKG)_DEP = $(SETUPTOOLS_PKG)
$(PYTHON_UBUS_PKG)_PATCHES += 0003-ubus-fix-memory-leak-in-objects-method.patch

AUTOMAKE_y += $(PYTHON_UBUS_PKG)
endif

###############################################################################
FINALTARGET = pkg_install

include $(ROOTDIR)/tools/automake.inc

build/%-pkg_build: build/%-patched $(SYSCONFIGDATA)
	@echo
	@echo "Building $* $(PYTHON) module with setup.py ..."
	$(AT)if [ -e build/$*/bootstrap.py ]; then \
		cd build/$*; \
		$(PYTHON) bootstrap.py; \
	fi
	$(AT)cd build/$*; $($*_BUILDENV) $(PYTHON) setup.py build $($*_BUILDVARS)
	@touch $@

#
# Patching python path (shebang) in the installed python scripts at the end.
#
# setup.py '--executable' and '-e' should be able to update the python path but
# it does not work. Seems to be because setup.py is not listing package scripts,
# so do our own.
#
build/%-pkg_install: build/%-pkg_build
	@echo
	@echo "Installing $* $(PYTHON) module with setup.py ..."
	$(AT)rm -rf build/$*-install
	$(AT)# install host version (non compiled)
	$(AT)if [ "$($*_HOSTINSTALL)" = 1 ]; then \
		cd build/$*; \
		$(PYTHON) setup.py install --root=$(PYTHON_HOSTINSTALL) --prefix= || exit 1; \
	fi
	$(AT)if [ "$($*_NOTARGETINSTALL)" != 1 ]; then \
		cd build/$*; \
		$(PYTHON) setup.py install --root=../$*-install --prefix=/usr || exit 1; \
		grep -rl '#!$(PYTHON_HOSTINSTALL)' ../$*-install | while read t; do \
			echo "Patching python path in '$$t' ..."; \
			sed -i "1s|^#!$(PYTHON_HOSTINSTALL).*$$|#!/usr/bin/python$(PYTHON_VERSION_SHORT)|" "$$t"; \
		done; \
		find ../$*-install -iname "*.exe" -delete; \
	fi

	@touch $@

# Copy the target Python's sysconfigdata module to here so we can PYTHONPATH to
# it without any of the other modules in the Python install directory.
$(SYSCONFIGDATA): $(SYSCONFIGPATH)
	cp -a $< $@

.PHONY: clean_sysconfigdata
clean_sysconfigdata:
	rm -f $(SYSCONFIGDATA_MATCH)
clean: clean_sysconfigdata

# The modules are installed into dist-packages in the romfs which is not
# included in the Python path by default, so include it.
.PHONY: pth
pth:
ifdef CONFIG_USER_PYTHON
	echo "/usr/lib/python$(PYTHON_VERSION_SHORT)/dist-packages" > $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/site-packages/modules.pth
endif

PYTHON_LIVE_IMAGE_DIR = $(STAGEDIR)/live_images/python/rootfs

.PHONY: python-modules_romfs
python-modules_romfs:
ifdef CONFIG_USER_PYTHON_LIVE_IMAGE
	mkdir -p $(PYTHON_LIVE_IMAGE_DIR)/usr/lib
	cp -r $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT) $(PYTHON_LIVE_IMAGE_DIR)/usr/lib
	rm -fr $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/*
endif

romfs: pth python-modules_romfs
