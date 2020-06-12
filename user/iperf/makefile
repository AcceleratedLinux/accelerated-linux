# Use latest tagged release on Github as of Jun 2020
URL = https://github.com/esnet/iperf.git
METHOD = git
VERSION = 3.7
iperf_PKG = iperf-$(VERSION)
MAKEVARS= 

# disable-threads was set on the original iperf2 makefile. disable-profiling was added because
# of compiler errors on iperf3 related to the feature and the x86_64 compiler. See https://github.com/esnet/iperf/issues/337
# for details.
CONFOPTS += --disable-threads --disable-profiling
LDFLAGS += -lm

include $(ROOTDIR)/tools/automake.inc

