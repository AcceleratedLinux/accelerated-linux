%define name nmap
%define version 4.20
%define release 1
%define prefix /usr

# To not build the frontend, add:
#   --define "buildfe 0"
# ...to the rpm build command-line
# To build a static rpm, add:
# --define "static 1"

%if "%{buildfe}" != "0"
%define buildfe 1
%endif

Summary: Network exploration tool and security scanner
Name: %{name}
Version: %{version}
Release: %{release}
Epoch: 2
License: http://www.insecure.org/nmap/man/man-legal.html
Group: Applications/System
Source0: http://www.insecure.org/nmap/dist/%{name}-%{version}.tgz
URL: http://www.insecure.org/nmap/
BuildRoot: %{_tmppath}/%{name}-root
# RPM can't be relocatable until I stop storing path info in the binary
# Prefix: %{prefix}

%description
Nmap is a utility for network exploration or security auditing. It
supports ping scanning (determine which hosts are up), many port
scanning techniques, version detection (determine service protocols
and application versions listening behind ports), and TCP/IP
fingerprinting (remote host OS or device identification). Nmap also
offers flexible target and port specification, decoy/stealth scanning,
sunRPC scanning, and more. Most Unix and Windows platforms are
supported in both GUI and commandline modes. Several popular handheld
devices are also supported, including the Sharp Zaurus and the iPAQ.

%if "%{buildfe}" == "1"
%package frontend
Summary: Gtk+ frontend for nmap
Group: Applications/System
Requires: nmap, gtk2
BuildPreReq: gtk2-devel
Version: %{version}
%description frontend
This package includes nmapfe, a Gtk+ frontend for nmap. The nmap package must
be installed before installing nmap-frontend.
%endif

%prep
%setup -q

%build
export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{prefix} --mandir=%{prefix}/share/man --without-openssl
%if "%{static}" == "1"
make static
%else
make
%endif

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} mandir=$RPM_BUILD_ROOT%{prefix}/share/man install

mkdir -p $RPM_BUILD_ROOT%{prefix}/share/applications

strip $RPM_BUILD_ROOT%{prefix}/bin/* || :
gzip $RPM_BUILD_ROOT%{prefix}/share/man/man1/* || :

%if "%{buildfe}" == "1"
%post frontend
%endif

%if "%{buildfe}" == "1"
%postun frontend
%endif

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files 
%defattr(-,root,root)
%doc COPYING
%doc docs/README
%doc docs/nmap.usage.txt
%{prefix}/bin/nmap
%{prefix}/share/nmap
%{prefix}/share/man/man1/nmap.1.gz

%if "%{buildfe}" == "1"
%files frontend
%defattr(-,root,root)
%{prefix}/bin/nmapfe
%{prefix}/bin/xnmap
%{prefix}/share/applications/nmapfe.desktop
%{prefix}/share/man/man1/xnmap.1.gz
%{prefix}/share/man/man1/nmapfe.1.gz
%endif

%changelog

* Sat Sep 01 2004 Stephane Loeuillet (stephane.loeuillet(a)tiscali.fr)
- Place .desktop file under ${prefix}/share/applications rather than
  ${prefix}/share/gnome/apps/Utilities

* Mon Dec 16 2002 Matthieu Verbert (mve(a)zurich.ibm.com)
- Place man pages under ${prefix}/share/man rather than ${prefix}/man

* Fri Jun 01 2001 GOMEZ Henri (hgomez(a)slib.fr)
- Patch which checks that $RPM_BUILD_ROOT is not "/" before rm'ing it.

* Tue Mar 06 2001 Ben Reed <ben(a)opennms.org>
- changed spec to handle not building the frontend

* Thu Dec 30 1999 Fyodor (fyodor(a)insecure.org)
- Updated description
- Eliminated source1 (nmapfe.desktop) directive and simply packaged it with Nmap
- Fixed nmap distribution URL (source0)
- Added this .rpm to base Nmap distribution

* Mon Dec 13 1999 Tim Powers <timp(a)redhat.com>
- based on origional spec file from
	http://www.insecure.org/nmap/index.html#download
- general cleanups, removed lots of commenrts since it made the spec hard to
	read
- changed group to Applications/System
- quiet setup
- no need to create dirs in the install section, "make
	prefix=$RPM_BUILD_ROOT&{prefix} install" does this.
- using defined %{prefix}, %{version} etc. for easier/quicker maint.
- added docs
- gzip man pages
- strip after files have been installed into buildroot
- created separate package for the frontend so that Gtk+ isn't needed for the
	CLI nmap 
- not using -f in files section anymore, no need for it since there aren't that
	many files/dirs
- added desktop entry for gnome

* Sun Jan 10 1999 Fyodor (fyodor(a)insecure.org)
- Merged in spec file sent in by Ian Macdonald <ianmacd(a)xs4all.nl>

* Tue Dec 29 1998 Fyodor (fyodor(a)insecure.org)
- Made some changes, and merged in another .spec file sent in
  by Oren Tirosh <oren(a)hishome.net>

* Mon Dec 21 1998 Riku Meskanen (mesrik(a)cc.jyu.fi)
- initial build for RH 5.x
