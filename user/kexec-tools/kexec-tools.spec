Summary: Load one kernel from another
Name: kexec-tools
Version: 2.0.0
Release: 0
Copyright: GPL
Group: Development/Tools
Source0:%{name}-%{version}.tar.gz
Packager: Eric Biederman <ebiederman@xmission.com>
BuildRoot: %{_tmppath}/%{name}

%description
/sbin/kexec is a user space utiltity for loading another kernel
and asking the currently running kernel to do something with it.
A currently running kernel may be asked to start the loaded
kernel on reboot, or to start the loaded kernel after it panics.

The panic case is useful for having an intact kernel for writing
crash dumps.  But other uses may be imagined.

%prep
%setup -q -n %{name}-%{version}

%build
%configure
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%{_sbindir}/kexec
%doc News
%doc COPYING
%doc TODO
#%{_mandir}/man8/kexec.8.gz

%changelog
* Tue Dec 16 2004 Eric Biederman <ebiederman@lnxi.com>
- kexec-tools initialy packaged as an rpm. 
