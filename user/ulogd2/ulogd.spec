Summary: ulogd - The userspace logging daemon for netfilter
Name: ulogd
Version: 2.00beta
Release: 1gm
License: GPL
Group: Network
Source: http://ftp.netfilter.org/pub/ulogd/%{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Packager: Harald Welte <laforge@netfilter.org>
BuildRequires: MySQL-devel postgresql-devel libpcap-devel libnfnetlink libnetfilter_conntrack libnetfilter_log
#BuildRequires: mysql-devel

%package mysql
Summary: MySQL output plugin for ulogd-2.x
Group: Network

%package pgsql
Summary: PostgreSQL output plugin for ulogd-2.x
Group: Network

%package pcap
Summary: libpcap output plugin for ulogd-2.x
Group: Network

%package nflog
Summary: netfilter_log input plugin for ulogd-2.x
Group: Network

%package ctnl
Summary: netfilter_conntrack input plugin for ulogd-2.x
Group: Network

%description
ulogd is an universal logging daemon for the ULOG target of netfilter, the
Linux 2.4 firewalling subsystem. ulogd is able to log packets in variuos
formats to different targets (text files, databases, etc..). It has an
easy-to-use plugin interface to add new protocols and new output targets.

%description mysql
ulogd-mysql is a MySQL output plugin for ulogd. It enables logging of
firewall information into a MySQL database.

%description pgsql
ulogd-mysql is a PostgreSQL output plugin for ulogd. It enables logging of
firewall information into a PostgreSQL database.

%prep
%setup

%build
%configure --with-mysql=/usr/lib/mysql --with-pgsql=/usr/lib/postgresql
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{_sysconfdir}
mkdir -p %{buildroot}/%{_libdir}/ulogd
mkdir -p %{buildroot}/%{_sbindir}/sbin
mkdir -p %{buildroot}/%{_mandir}/man8
make DESTDIR=%{buildroot} install

mkdir -p %{buildroot}/%{_sysconfdir}/rc.d/init.d
install ulogd.init %{buildroot}/%{_sysconfdir}/rc.d/init.d/ulogd
install ulogd.8 %{buildroot}/%{_mandir}/man8/ulogd.8
		
%clean
rm -rf %{buildroot}

%files
%defattr(0644,root,root,0755)
%attr(0755,root,root) %{_sbindir}/ulogd
%{_sysconfdir}/ulogd.conf
%{_sysconfdir}/rc.d/init.d/ulogd
%{_mandir}/man8/*
%dir %{_libdir}/ulogd
%{_libdir}/ulogd/ulogd_BASE.so
%{_libdir}/ulogd/ulogd_LOCAL.so
%{_libdir}/ulogd/ulogd_LOGEMU.so
%{_libdir}/ulogd/ulogd_OPRINT.so
%{_libdir}/ulogd/ulogd_PWSNIFF.so
%{_libdir}/ulogd/ulogd_PCAP.so
%doc COPYING AUTHORS README
%doc doc/ulogd.txt doc/ulogd.a4.ps doc/ulogd.html

%files mysql
%defattr(0644,root,root,0755)
%{_libdir}/ulogd/ulogd_MYSQL.so

%files pgsql
%defattr(0644,root,root,0755)
%{_libdir}/ulogd/ulogd_PGSQL.so

%changelog
* Sat Aug 25 2003 Harald Welte <laforge@gnumonks.org>
+ ulogd-1.00-1gm
- updated to 1.01 release
- add ulogd.8 manpage

* Wed Mar 05 2003 Harald Welte <laforge@gnumonks.org>
+ ulogd-1.00-1gm
- updated to 1.00 release

* Mon Sep 24 2001 Harald Welte <laforge@conectiva.com>
+ ulogd-0.97-1cl
- updatd to 0.97 release (to fix endless-one-packet-loop bug)

* Sun Jun 17 2001 Harald Welte <laforge@conectiva.com>
+ ulogd-0.96-2cl
- updated to 0.96 final release
- use ulogd.init from within source tgz

* Sun May 20 2001 Harald Welte <laforge@conectiva.com>
+ ulogd-0.96-1cl
- Initial conectiva package
- cleaned up SPEC file
- created mysql subpackage

* Sun Nov 19 2000 Harald Welte <laforge@gnumonks.org>
- Initial RPM package for ulogd-0.9.
