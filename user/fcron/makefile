FCRON_VERSION = 3.2.1
CONFVARS = ac_cv_func_memcmp_working=yes
CONFOPTS = --disable-checks
CONFOPTS += --disable-option-checking
CONFOPTS += --with-answer-all=no
CONFOPTS += --with-pam=no
CONFOPTS += --with-fcrondyn=no
CONFOPTS += --with-db2man=no
CONFOPTS += --with-selinux=no
CONFOPTS += --with-run-non-privileged=yes
CONFOPTS += --with-sendmail="/bin/true"
CONFOPTS += --with-editor="/bin/vi"
CONFOPTS += --with-spooldir="/etc/config/fcron"
CONFOPTS += --with-username=root
CONFOPTS += --with-groupname=root

fcron-$(FCRON_VERSION).src_INSTALL = install-staged

BUILDDIR = fcron-$(FCRON_VERSION)

URL = http://fcron.free.fr/archives/fcron-$(FCRON_VERSION).src.tar.gz

include $(ROOTDIR)/tools/automake.inc
