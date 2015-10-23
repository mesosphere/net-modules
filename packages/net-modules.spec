Name:          net-modules
Version:       0.25
Release:       1.custom
Summary:       Network isolation modules for Apache Mesos
License:       ASL 2.0
URL:           http://mesos.apache.org/

ExclusiveArch: x86_64

Source0:       https://github.com/mesosphere/net-modules/archive/integration/%{version}.zip

BuildRequires: libtool
BuildRequires: python-devel
BuildRequires: gcc-c++
BuildRequires: glog-devel
BuildRequires: gflags-devel
BuildRequires: boost-devel
BuildRequires: protobuf-devel
BuildRequires: curl-devel
BuildRequires: subversion-devel


%description
The first implementation in this repository showcases Apache Mesos using Project Calico as the networking solution.

%prep
%setup -q -n net-modules-integration-0.25


%build

cd isolator
./bootstrap
%configure --with-mesos=/usr/include/mesos/ --with-protobuf=/usr
make -j 4

%install
ls -R %{buildroot}
echo %{buildroot} 
mkdir -p %{buildroot}/opt/net-modules
cp -v isolator/.libs/* %{buildroot}/opt/net-modules/

############################################
%files
/opt/net-modules/*

%changelog
* Wed Oct 21 2015 Thibault Cohen <thibault.cohen@nuance.com> - 0.25.0-1.custom
- Build mesos 0.25.0
