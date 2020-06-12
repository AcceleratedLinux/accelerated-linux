# you will need these packages to build for 32 bit systems
#
# sudo apt install libc6-dev:i386 libstdc++-7-dev:i386
# sudo apt-get install gcc-multilib g++-multilib
# sudo apt install libssl-dev:i386 zlib1g-dev:i386

NODE_VERSION = v10.16.0
URL = https://nodejs.org/dist/$(NODE_VERSION)/node-$(NODE_VERSION).tar.gz
NODEFCONF = true

NODE_CPU = $(subst powerpc,ppc,$(subst aarch64,arm64,$(subst x86_64,x64,$(subst i386,ia32,$(ARCH)))))

node-$(NODE_VERSION)_CONFOPTS += --dest-cpu=$(NODE_CPU)
node-$(NODE_VERSION)_CONFOPTS += --cross-compiling
node-$(NODE_VERSION)_CONFOPTS += --without-snapshot
node-$(NODE_VERSION)_CONFOPTS += --dest-os=linux
node-$(NODE_VERSION)_CONFOPTS += --prefix=/
node-$(NODE_VERSION)_CONFOPTS += --shared-zlib
node-$(NODE_VERSION)_CONFOPTS += --shared-openssl
node-$(NODE_VERSION)_CONFOPTS += --without-intl
ifneq ($(findstring arm,$(ARCH)),)
node-$(NODE_VERSION)_CONFOPTS += --with-arm-fpu=vfpv3
endif

node-$(NODE_VERSION)_ROMFS     = node_romfs

LDFLAGS += -latomic

CONFVARS += CXX="$(CXX) $(CFLAGS) $(CXXFLAGS)" CC="$(CC) $(CFLAGS)" DESTCPU=$(NODE_CPU) CC_host="$(HOSTCC)" CXX_host="$(HOSTCXX)"

include $(ROOTDIR)/tools/automake.inc

node_romfs:
	$(ROMFSINST) -p 444 /etc/npmrc
	$(ROMFSINST) -s /opt/node /.config

