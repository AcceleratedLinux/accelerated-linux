PYTHON_VERSION = $(shell sed -rn 's/^PYTHON_VERSION += +(.*)$\/\1/p' $(ROOTDIR)/user/python/makefile)
PYTHON_VERSION_SHORT = $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

# make sure we do not lean on system packages
export PYTHONNOUSERSITE = 1

# Requires that the build system has the same major/minor version of Python as
# was built for the target.
PYTHON_HOSTINSTALL := $(ROOTDIR)/user/python/build/Python-Hostinstall
PYTHON := PATH="$(PYTHON_HOSTINSTALL)/bin:$$PATH"; export PATH; python$(PYTHON_VERSION_SHORT)

PYTHON_BUILDDIR = $(ROOTDIR)/user/python/build/Python-$(PYTHON_VERSION)
PYTHON_INSTALLDIR = $(ROOTDIR)/user/python/build/Python-$(PYTHON_VERSION)-install/usr/lib/python$(PYTHON_VERSION_SHORT)
SYSCONFIGDATA_MATCH = _sysconfigdata*.py
SYSCONFIGPATH = $(shell find $(PYTHON_INSTALLDIR) -name $(SYSCONFIGDATA_MATCH) 2> /dev/null | head)
SYSCONFIGDATA = $(notdir $(SYSCONFIGPATH))

# Setup environment to find sysconfigdata module from the cross-compiled Python
export PYTHONPATH = $(ROOTDIR)/user/python-modules
export _PYTHON_PROJECT_BASE = $(PYTHON_BUILDDIR)
export _PYTHON_SYSCONFIGDATA_NAME = $(basename $(SYSCONFIGDATA))

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl
CPPFLAGS += -I$(STAGEDIR)/include -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_SETUPTOOLS

SETUPTOOLS_VERSION = 41.0.1
SETUPTOOLS_PKG = setuptools-$(SETUPTOOLS_VERSION)
$(SETUPTOOLS_PKG)_URL = https://github.com/pypa/setuptools/archive/v$(SETUPTOOLS_VERSION).tar.gz
$(SETUPTOOLS_PKG)_DOWNLOADNAME = $(SETUPTOOLS_PKG).tar.gz

# currently we can only hostinstall non compiled modules,  a little more
# work will be needed if we need to compile modules for the build python
$(SETUPTOOLS_PKG)_HOSTINSTALL = 1

AUTOMAKE_y += $(SETUPTOOLS_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PIP

PIP_VERSION = 19.1.1
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
$(XBEE_LIB_PKG)_VERSION = 2971725bb227694c6ca76bea0c5f95459ba19c4a
$(XBEE_LIB_PKG)_METHOD = git
$(XBEE_LIB_PKG)_URL = https://github.com/digidotcom/xbee-python.git

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
$(SRP_PKG)_URL = https://files.pythonhosted.org/packages/9b/9f/04743397ed89136223966a721ec3d96065ce58b1f62aafa6fbae78830309/$(SRP_PKG).tar.gz

AUTOMAKE_y += $(SRP_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON_MODULES_PAHO_MQTT

PAHO_MQTT_VERSION = 1.5.0
PAHO_MQTT_PKG = paho.mqtt.python-$(PAHO_MQTT_VERSION)
$(PAHO_MQTT_PKG)_URL = https://github.com/eclipse/paho.mqtt.python/archive/v$(PAHO_MQTT_VERSION).tar.gz
$(PAHO_MQTT_PKG)_DOWNLOADNAME = $(PAHO_MQTT_PKG).tar.gz

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

build/%-pkg_install: build/%-pkg_build
	@echo
	@echo "Installing $* $(PYTHON) module with setup.py ..."
	$(AT)rm -rf build/$*-install
	$(AT)# install host version (non compiled)
	$(AT)if [ "$($*_HOSTINSTALL)" = 1 ]; then \
		cd build/$*; \
		$(PYTHON) setup.py install --root=$(PYTHON_HOSTINSTALL) --prefix= || exit 1; \
	fi
	$(AT)cd build/$*; $(PYTHON) setup.py install --root=../$*-install --prefix=/usr
	$(AT)# --executable and -e should be able to update the python path but
	$(AT)# it does nto work.  Seems to be because setup.py is not listing
	$(AT)# package scripts,  so do our own
	$(AT)grep -rl '#!$(PYTHON_HOSTINSTALL)' build/$*-install | while read t; do \
		echo "Patching python path in '$t' ..."; \
		sed -i "1s|^#!$(PYTHON_HOSTINSTALL).*$$|#!/usr/bin/python$(PYTHON_VERSION_SHORT)|" "$$t"; \
	done

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
	echo "/usr/lib/python$(PYTHON_VERSION_SHORT)/dist-packages" > $(ROMFSDIR)/usr/lib/python$(PYTHON_VERSION_SHORT)/site-packages/modules.pth
romfs: pth
