include $(ROOTDIR)/tools/base.inc
include $(ROOTDIR)/tools/download.inc

ifndef NAME
NAME = FactoryTest
endif
DATE = $(shell date -u +%Y%m%d)
DIR = $(shell basename $$(pwd))
COM = $(ROOTDIR)/prop/factory/factory-cfg/common-files
ZIPFILENAME = $(DIR)-$(NAME)-$(DATE).zip
INSTALL = build/$(DIR)-install

SSH = $(COM)/enable_shell.expect $(COM)/enable_factory_reboot.expect $(COM)/wait_ssh.expect
MT = $(COM)/mt/factory.sh $(COM)/dynamicip.sh $(COM)/configure_test_station_auto.txt $(COM)/configure_test_station_manual.txt
TOOLS = $(COM)/tools.sh

$(foreach item,$(DL),$(eval $(call download_pkg_def,$(item))))
$(foreach item,$(DL),$(eval $(call download_dep_default,$(item))))
DOWNLOADED_FILES = $(foreach item,$(DL),downloads/$($(item)_DOWNLOADNAME))
DOWNLOADED_FILES += $(foreach item,$(PKG_y),build/$($(item)_SUBDIR))

$(foreach item,$(CP_FILES),$(/bin/cp $(COM)/$(item) build/))

all: $(INSTALL)/$(ZIPFILENAME)

$(INSTALL)/$(ZIPFILENAME): $(DOWNLOADED_FILES) $(CP_FILES) $(MAKEFILE_LIST)
	rm -rf $(INSTALL)
	mkdir -p $(INSTALL)
	/bin/cp -r $(DOWNLOADED_FILES) $(CP_FILES) $(INSTALL)
	if [ -f $(DIR).cfg ]; then \
		/bin/cp $(DIR).cfg $(INSTALL) ;\
		sed -i "1iinfo --------------------------------------------------------------" $(INSTALL)/$(DIR).cfg ;\
		sed -i "2iinfo Release: $(ZIPFILENAME)" $(INSTALL)/$(DIR).cfg ;\
		sed -i "3iinfo Version: $(shell $(ROOTDIR)/bin/version)" $(INSTALL)/$(DIR).cfg ;\
		sed -i "4isystem -d echo Station Serial: \$$(fw_printenv -n serial)" $(INSTALL)/$(DIR).cfg ;\
		sed -i "5isystem -d echo Station Hostname: \$$(hostname)" $(INSTALL)/$(DIR).cfg ;\
		sed -i "6iinfo --------------------------------------------------------------" $(INSTALL)/$(DIR).cfg ;\
	fi
	if [ -f test.sh ]; then \
		/bin/cp test.sh $(INSTALL)/ ;\
	fi
ifdef IMAGE
	cd $(INSTALL); mv $(IMAGE) image.bin
	cd $(INSTALL); touch $(IMAGE)
endif
ifdef IMAGE2
	cd $(INSTALL); mv $(IMAGE2) image2.bin
	cd $(INSTALL); touch $(IMAGE2)
endif
	cd $(INSTALL); zip -r $(ZIPFILENAME) .
	@echo ============================
	@echo Created Factorytest zipfile:
	@echo $(shell pwd)/$(INSTALL)/$(ZIPFILENAME)
	@echo ============================

clean:
	rm -rf downloads build

