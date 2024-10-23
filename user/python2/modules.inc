# make sure we do not lean on system packages
export PYTHONNOUSERSITE = 1

# Requires that the build system has the same major/minor version of Python as
# was built for the target.
PYTHON := PATH="$(PYTHON_HOSTINSTALL)/bin:$$PATH"; export PATH; python$(PYTHON_VERSION_SHORT)

PYTHON_BUILDDIR = $(CURDIR)/build/Python-$(PYTHON_VERSION)
PYTHON_INSTALLDIR = $(PYTHON_INSTALLDIR_ROOT)/$(INSTALL_PREFIX)/lib/python$(PYTHON_VERSION_SHORT)

# Absolute paths to staging so setup.py can correctly detect module
# dependencies like zlib and ssl
CPPFLAGS += -I$(STAGEDIR)/include -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/lib -L$(STAGEDIR)/usr/lib

PYTHON2_MODULES_y =

###############################################################################
ifeq ($(or $(CONFIG_USER_PYTHON2_MODULES_SETUPTOOLS),$(CONFIG_USER_PYTHON2_MODULES_SETUPTOOLS_HOST)),y)

SETUPTOOLS_VERSION = 44.1.1
SETUPTOOLS_PKG = setuptools-$(SETUPTOOLS_VERSION)
$(SETUPTOOLS_PKG)_URL = https://github.com/pypa/setuptools/archive/v$(SETUPTOOLS_VERSION).tar.gz
$(SETUPTOOLS_PKG)_DOWNLOADNAME = $(SETUPTOOLS_PKG).tar.gz

# currently we can only hostinstall non compiled modules,  a little more
# work will be needed if we need to compile modules for the build python
$(SETUPTOOLS_PKG)_HOSTINSTALL = 1
ifndef CONFIG_USER_PYTHON2_MODULES_SETUPTOOLS
$(SETUPTOOLS_PKG)_NOTARGETINSTALL = 1
endif

PYTHON2_MODULES_y += $(SETUPTOOLS_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON2_MODULES_PIP

PIP_VERSION = 20.3.4
PIP_PKG = pip-$(PIP_VERSION)
$(PIP_PKG)_URL = https://github.com/pypa/pip/archive/$(PIP_VERSION).tar.gz
$(PIP_PKG)_DOWNLOADNAME = $(PIP_PKG).tar.gz
$(PIP_PKG)_DEP = $(SETUPTOOLS_PKG)

PYTHON2_MODULES_y += $(PIP_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON2_MODULES_SIX

SIX_VERSION = 1.16.0
SIX_PKG = six-$(SIX_VERSION)
$(SIX_PKG)_URL = https://github.com/benjaminp/six/archive/$(SIX_VERSION).tar.gz
$(SIX_PKG)_DOWNLOADNAME = $(SIX_PKG).tar.gz

PYTHON2_MODULES_y += $(SIX_PKG)
endif

###############################################################################
ifdef CONFIG_USER_PYTHON2_MODULES_PAHO_MQTT

PAHO_MQTT_VERSION = 1.6.1
PAHO_MQTT_PKG = paho.mqtt.python-$(PAHO_MQTT_VERSION)
$(PAHO_MQTT_PKG)_URL = https://github.com/eclipse/paho.mqtt.python/archive/v$(PAHO_MQTT_VERSION).tar.gz
$(PAHO_MQTT_PKG)_DOWNLOADNAME = $(PAHO_MQTT_PKG).tar.gz
$(PAHO_MQTT_PKG)_DEP = $(SETUPTOOLS_PKG)

PYTHON2_MODULES_y += $(PAHO_MQTT_PKG)
endif

###############################################################################
PYTHON2_MODULES_INSTALLED_TARGET =
define python2_module_def
  $(1)_FINALTARGET = pkg_install
  PYTHON2_MODULES_INSTALLED_TARGET += build/$(1)-pkg_install
endef
$(foreach m,$(PYTHON2_MODULES_y),$(eval $(call python2_module_def,$(m))))

PKG_y += $(PYTHON2_MODULES_y)

build/%-pkg_build: build/%-patched build/Python-$(PYTHON_VERSION)-installed
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
		$(PYTHON) setup.py install --root=$(PYTHON_INSTALLDIR_ROOT) --prefix=/$(INSTALL_PREFIX) || exit 1; \
		grep -rl '^#!$(PYTHON_HOSTINSTALL)' $(PYTHON_INSTALLDIR_ROOT) | while read t; do \
			echo "Patching python path in '$$t' ..."; \
			sed -i "1s|^#!$(PYTHON_HOSTINSTALL).*$$|#!/$(INSTALL_PREFIX)/bin/python$(PYTHON_VERSION_SHORT)|" "$$t"; \
		done; \
	fi

	@touch $@

# vim: set ft=make
