Name:          net-modules
Version:       0.2.0
Release:       1.custom
Summary:       Network isolation modules for Apache Mesos
License:       ASL 2.0
URL:           http://mesos.apache.org/

ExclusiveArch: i386

Source0:       netmodules.tar.gz
Source1:       isolation
Source2:       hooks

BuildRequires: python-devel
BuildRequires: gcc-c++
BuildRequires: curl-devel


%description
The first implementation in this repository showcases Apache Mesos using Project Calico as the networking solution.

%prep
%setup -q -n isolator


%build
./bootstrap

CPPFLAGS='-I/root/protobuf/protobuf-2.5.0/src/ -I/root/glog/glog-0.3.3/src/ -I/root/boost/boost-1.53.0/' ./configure --prefix=/usr --with-mesos=/
make -j 2

%install
%make_install

rm -f %{buildroot}%{_libdir}/mesos/*.la

mkdir -p %{buildroot}%{_sysconfdir}/mesos-slave
install %{SOURCE1} %{buildroot}%{_sysconfdir}/mesos-slave/
install %{SOURCE2} %{buildroot}%{_sysconfdir}/mesos-slave/

############################################
%files
%{_libdir}/mesos/libmesos_network_isolator*.so
%{_sysconfdir}/mesos-slave/isolation
%{_sysconfdir}/mesos-slave/hooks

%changelog
* Tue Feb 16 2016 Dan Osborne <daniel.osborne@metaswitch.com> - 0.2.0-1.custom
- Build netmoudules 0.2.0

* Tue Dec 22 2015 Dan Osborne <daniel.osborne@metaswitch.com> - 0.26-1.custom
- Build mesos 0.26.0

* Wed Oct 21 2015 Thibault Cohen <thibault.cohen@nuance.com> - 0.25.0-1.custom
- Build mesos 0.25.0
