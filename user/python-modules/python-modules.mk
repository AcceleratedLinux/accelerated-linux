PYTHON_VERSION = $(shell sed -rn 's/^PYTHON_VERSION += +(.*)$\/\1/p' $(ROOTDIR)/user/python/makefile)
PYTHON_VERSION_SHORT = $(shell echo $(PYTHON_VERSION) | cut -d'.' -f1,2)

# make sure we do not lean on system packages
export PYTHONNOUSERSITE = 1

# Requires that the build system has the same major/minor version of Python as
# was built for the target.
PYTHON_HOSTINSTALL := $(ROOTDIR)/user/python/build/Python-Hostinstall
PYTHON := PATH="$(PYTHON_HOSTINSTALL)/bin:$$PATH"; export PATH; python$(PYTHON_VERSION_SHORT)

PYTHON_BUILDDIR = $(ROOTDIR)/user/python/build/Python-$(PYTHON_VERSION)
SYSCONFIGDATA_MATCH = _sysconfigdata*.py
SYSCONFIGPATH = $(shell ls -1 $(ROOTDIR)/user/python/build/$(SYSCONFIGDATA_MATCH) 2> /dev/null | head -n1)
SYSCONFIGDATA = $(notdir $(SYSCONFIGPATH))

# Setup environment to find sysconfigdata module from the cross-compiled Python
export PYTHONPATH = $(ROOTDIR)/user/python-modules
export _PYTHON_PROJECT_BASE = $(PYTHON_BUILDDIR)
export _PYTHON_SYSCONFIGDATA_NAME = $(basename $(SYSCONFIGDATA))
