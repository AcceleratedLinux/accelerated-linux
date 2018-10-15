%define tinyproxy_confdir %{_sysconfdir}/tinyproxy
%define tinyproxy_datadir %{_datadir}/tinyproxy

Name:           tinyproxy
Version:        1.6.4
Release:        2%{?dist}
Summary:        A small, efficient HTTP/SSL proxy daemon

Group:          System Environment/Daemons
License:        GPLv2+
URL:            http://tinyproxy.banu.com/
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Source0:        http://files.banu.com/pub/tinyproxy/1.6/%{name}-%{version}.tar.gz
Source1:        %{name}.init
Source2:        %{name}.conf

Requires(post):     chkconfig
Requires(preun):    chkconfig
Requires(preun):    initscripts

%description
tinyproxy is a small, efficient HTTP/SSL proxy daemon released under the
GNU General Public License (GPL).  tinyproxy is very useful in a small
network setting, where a larger proxy like Squid would either be too
resource intensive, or a security risk.  

%prep
%setup -q


%build
%configure --with-config=%{tinyproxy_confdir}/%{name}.conf
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install-exec DESTDIR=%{buildroot}

# The default 'make install' installs too many items, so we trim it down
# and install manually
%{__install} -p -D -m 0755 %{SOURCE1} %{buildroot}%{_initrddir}/%{name}
%{__install} -p -D -m 0644 %{SOURCE2} %{buildroot}%{tinyproxy_confdir}/%{name}.conf
%{__install} -d -m 0755 %{buildroot}%{tinyproxy_datadir}
%{__install} -p -D -m 0644 ./doc/%{name}.8 %{buildroot}%{_mandir}/man8/%{name}.8

for htmlfile in $(find ./doc/ -type f -name '*.html')  
do
    %{__install} -p -m 0644 $htmlfile %{buildroot}%{tinyproxy_datadir}
done

%clean
rm -rf %{buildroot}


%post
/sbin/chkconfig --add %{name}
    

%preun
if [ $1 = 0 ]; then
    /sbin/service %{name} stop >/dev/null 2>&1
    /sbin/chkconfig --del %{name}
fi  
    

%postun
if [ "$1" -ge "1" ]; then
    /sbin/service %{name} condrestart > /dev/null 2>&1 || :
fi  
 


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING README doc/*.txt
%{_sbindir}/%{name}
%{_mandir}/man8/%{name}.8.gz
%{_initrddir}/%{name}
%dir %{tinyproxy_datadir}
%dir %{tinyproxy_datadir}/*
%dir %{tinyproxy_confdir}
%config(noreplace) %{tinyproxy_confdir}/%{name}.conf

%changelog
* Wed Apr 16 2008 Jeremy Hinegardner <jeremy at hinegardner dot org> - 1.6.3-2
- fix spec review issues
- fix initscript

* Sun Mar 09 2008 Jeremy Hinegardner <jeremy at hinegardner dot org> - 1.6.3-1
- Initial rpm configuration
