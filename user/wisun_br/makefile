# The Makefile for a DAL build of the Wi-SUN Linux Border Router

# NOTE: We are currently pointing to a specific commit.
#       However, the intent is to follow main/master until
#       such time as we can't, or don't want to.
URL         = https://github.com/SiliconLabs/wisun-br-linux.git
METHOD      = git
VERSION     = 8a90810

NINJA ?= ninja
NINJAOPTS ?= -j$(HOST_NCPU)
NINJAOPTS += $(if $(V:0=),-v)

CMAKE_PREFIX_PATH += $(ROOTDIR)/lib/mbedtls/build/mbedtls

# Define Cargo target
# i.e. mips-unknown-linux-musl target for mips-linux-musl
CARGO_TARGET := $(shell echo "$(CONFIGURE_TOOL)" | sed 's/-/-unknown-/1')

CMAKE_OPTS += -G Ninja -DRUST_TARGET=$(CARGO_TARGET) -DRUST_CARGO_EXTRA_FLAGS="--config=./cargo-target.config" .

include $(ROOTDIR)/tools/cmake.inc

# Check and install the target.
build/%-rustcheck: build/%-patched
	$(AT)echo CARGO_TARGET is $(CARGO_TARGET)
	$(AT)if ! rustup target list | grep installed | grep -q $(CARGO_TARGET); then \
		echo "Installing rust target $(CARGO_TARGET)..."; \
		rustup target add $(CARGO_TARGET); \
	fi
	$(AT)touch $@

build/wisun-br-linux-$(CONFIGTARGET): build/wisun-br-linux-rustcheck

# Override this target:
# NOTE: The Wi-SUN Linux Border Router package does a two stage build.
#       It initially requires you to do a cmake build which generates
#       a ninja buildable package.
#       After the cmake is done, you can build the result with Ninja.
build/%-built: build/%-$(CONFIGTARGET)
	@echo "Building $* ..."
	$(AT)$(BUILDVARS) $($*_BUILDVARS) \
		$(UCFRONT_ENV) $(NINJA) $(NINJAOPTS) $(MAKEVARS) $($*_MAKEVARS) -C build/$($*_BUILDDIR)
	$(AT)touch $@

# Override this target:
# NOTE: The normal install cmake script that gets built by the wisun-br package
#       assumes that we are installing local, (tries to create a new user, etc),
#       Since the package has just a few things we need, we can just pluck
#       out just the things we want/need.
build/%-installed: build/%-$(BUILDTARGET)
	@echo "Installing $* ..."
	$(AT)mkdir -p build/$*-install
	install -d build/$*-install/bin/
	install build/$*/wsbrd build/$*/wsbrd_cli build/$*/wsbrd-fwup build/$*-install/bin/
	install -d build/$*-install/share/dbus-1/system-services
	install build/$*/misc/com.silabs.Wisun.BorderRouter.service \
		build/$*-install/share/dbus-1/system-services/com.silabs.Wisun.BorderRouter.service
	$(AT)touch $@
