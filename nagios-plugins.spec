%{!?custom:%global custom 0}

%define archive nagios-plugins

%if %custom
%define name %{archive}-custom
%else
%define name %{archive}
%endif

%define version 1.3.0
%define release alpha1
%define source http://nagiosplug.sourceforge.net/src/%{archive}-%{version}-%{release}.tar.gz

Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Source: %{source}
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}/lib/nagios/plugins
Packager: Karl DeBisschop <kdebisschop@users.sourceforge.net>
Vendor: Nagios Plugin Development Group
%if %custom
Obsoletes: nagios-plugins nagios-plugins-extras
%else
Obsoletes: nagios-plugins-custom
%endif
AutoReqProv: no
Summary: Host/service/network monitoring program plugins for Nagios
Group: Applications/System


%description

Nagios is a program that will monitor hosts and services on your
network, and to email or page you when a problem arises or is
resolved. Nagios runs on a unix server as a background or daemon
process, intermittently running checks on various services that you
specify. The actual service checks are performed by separate "plugin"
programs which return the status of the checks to Nagios.

This package contains the basic plugins necessary for use with the
Nagios package.  This package should install cleanly on almost any
RPM-based system.


%package extras
Summary: Plugins which depend on the presence of other packages
Group: Applications/System

%description extras

Nagios is a program that will monitor hosts and services on your
network, and to email or page you when a problem arises or is
resolved. Nagios runs on a unix server as a background or daemon
process, intermittently running checks on various services that you
specify. The actual service checks are performed by separate "plugin"
programs which return the status of the checks to Nagios.

This package contains plugins which use additional libraries or system
calls that are not installed on all systems.  As a result, most users
will need to install the '--nodeps' option when invoking `rpm`


%prep
%setup -q -n %{archive}-%{version}-%{release}


%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
--prefix=%{_prefix}/lib/nagios/plugins \
--libexecdir=%{_prefix}/lib/nagios/plugins \
--with-cgiurl=/nagios/cgi-bin
make


%install
make AM_INSTALL_PROGRAM_FLAGS="" DESTDIR=${RPM_BUILD_ROOT} install
install -d ${RPM_BUILD_ROOT}/etc/nagios
install -m 664 command.cfg ${RPM_BUILD_ROOT}/etc/nagios

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
%config(missingok,noreplace) /etc/nagios/command.cfg
%doc INSTALL README REQUIREMENTS COPYING ChangeLog command.cfg
%defattr(775,root,root)
%dir %{_prefix}/lib/nagios/plugins
%if %custom
%{_prefix}/lib/nagios/plugins/*
%else
%{_prefix}/lib/nagios/plugins/check_by_ssh
%{_prefix}/lib/nagios/plugins/check_breeze
%{_prefix}/lib/nagios/plugins/check_disk
%{_prefix}/lib/nagios/plugins/check_disk_smb
%{_prefix}/lib/nagios/plugins/check_dns
%{_prefix}/lib/nagios/plugins/check_dummy
%{_prefix}/lib/nagios/plugins/check_flexlm
%{_prefix}/lib/nagios/plugins/check_ftp
%{_prefix}/lib/nagios/plugins/check_http
%{_prefix}/lib/nagios/plugins/check_imap
%{_prefix}/lib/nagios/plugins/check_ircd
%{_prefix}/lib/nagios/plugins/check_load
%{_prefix}/lib/nagios/plugins/check_log
%{_prefix}/lib/nagios/plugins/check_mrtg
%{_prefix}/lib/nagios/plugins/check_mrtgtraf
%{_prefix}/lib/nagios/plugins/check_nagios
%{_prefix}/lib/nagios/plugins/check_nntp
%{_prefix}/lib/nagios/plugins/check_ntp
%{_prefix}/lib/nagios/plugins/check_nwstat
%{_prefix}/lib/nagios/plugins/check_oracle
%{_prefix}/lib/nagios/plugins/check_overcr
%{_prefix}/lib/nagios/plugins/check_ping
%{_prefix}/lib/nagios/plugins/check_pop
%{_prefix}/lib/nagios/plugins/check_procs
%{_prefix}/lib/nagios/plugins/check_real
%{_prefix}/lib/nagios/plugins/check_rpc
%{_prefix}/lib/nagios/plugins/check_sensors
%{_prefix}/lib/nagios/plugins/check_smtp
%{_prefix}/lib/nagios/plugins/check_ssh
%{_prefix}/lib/nagios/plugins/check_swap
%{_prefix}/lib/nagios/plugins/check_tcp
%{_prefix}/lib/nagios/plugins/check_time
%{_prefix}/lib/nagios/plugins/check_udp
%{_prefix}/lib/nagios/plugins/check_ups
%{_prefix}/lib/nagios/plugins/check_users
%{_prefix}/lib/nagios/plugins/check_vsz
%{_prefix}/lib/nagios/plugins/check_wave
%{_prefix}/lib/nagios/plugins/utils.pm
%{_prefix}/lib/nagios/plugins/utils.sh
%{_prefix}/lib/nagios/plugins/urlize
%endif

%if ! %custom
%files extras
%defattr(775,root,root)
%{_prefix}/lib/nagios/plugins/check_fping
%{_prefix}/lib/nagios/plugins/check_game
%{_prefix}/lib/nagios/plugins/check_ldap
%{_prefix}/lib/nagios/plugins/check_mysql
%{_prefix}/lib/nagios/plugins/check_pgsql
%{_prefix}/lib/nagios/plugins/check_radius
%{_prefix}/lib/nagios/plugins/check_snmp
%{_prefix}/lib/nagios/plugins/check_hpjd

%endif

%changelog
* Wed Jan 17 2001 Karl DeBisschop <karl@debisschop.net> (1.2.9-1)
- switch from /usr/libexec to /usr/lib because FHS has no libexec
- use 'custom' macro define to merge with nagios-plugins-custom spec
- add check_game to extras

* Mon Jun 26 2000 Karl DeBisschop <karl@debisschop.net>
- Release 1.2.8-4 (check_ping bug fix)
- use bzip2 insted of gzip for mandrake compatibility

* Thu Jun 22 2000 Karl DeBisschop <karl@debisschop.net>
- Release 1.2.8-3 (bug fixes)
- Add macros to spec where possible

* Fri Jun 16 2000 Karl DeBisschop <karl@debisschop.net>
- Release 1.2.8-2 (bug fixes)

* Fri Jun 09 2000 Karl DeBisschop <karl@debisschop.net>
- Release to 1.2.8

* Wed Jun 07 2000 Karl DeBisschop <karl@debisschop.net>
- Upgrade to 1.2.8pre7

* Sat Jun 03 2000 Karl DeBisschop <karl@debisschop.net>
- Upgraded to 1.2.8pre5
- use RPM_OPT_FALGS to set compiler options
- cahneg group to Applications/System

* Fri May 19 2000 Karl DeBisschop <karl@debisschop.net>
- Upgraded to 1.2.8pre3 (release-3)

* Mon Mar 20 2000 Karl DeBisschop <karl@debisschop.net>
- Upgraded to 1.2.8b2

* Tue Dec 14 1999 Adam Jacob <adam@cybertrails.com> (1.2.7-1cvs)
- Upgraded package from 1.2.6 to 1.2.7 from the latest CVS code
- Modified SPEC file to contain the proper build_root stuff. :)

* Tue Oct 19 1999 Mike McHenry <mmchen@minn.net> (1.2.6)
- Upgraded package from 1.2.4 to 1.2.6
- Resolved dependancy issue with libpq.so
- Added support for check_fping

* Fri Sep 03 1999 Mike McHenry <mmchen@minn.net> (1.2.4)
- Upgraded package from 1.2.2 to 1.2.4

* Mon Aug 16 1999 Mike McHenry <mmchen@minn.net> (1.2.2)
- First RPM build (1.2.2)
