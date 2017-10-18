%define _topdir         %(realpath $PWD)
%define name            lsvmtools
%define release         1
%define version         1.0.0
%define prefix          /opt/lsvmtools-%{version}
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot: %{buildroot}
Summary: LSVMTools
License: MIT
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.gz
Prefix: %{prefix}
Group: Development/Tools

%description
The LSVMTools program for shielding VMs for Hyper-V.

%prep
%setup -q
%build
./configure
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%doc %attr(0644,root,root) /opt/lsvmtools-1.0.0/VERSION
%doc %attr(0644,root,root) /opt/lsvmtools-1.0.0/LICENSE

%attr(0755,root,root) /opt/lsvmtools-1.0.0/lsvmprep
%attr(0755,root,root) /opt/lsvmtools-1.0.0/build/bin/lsvmtool
%attr(0755,root,root) /opt/lsvmtools-1.0.0/dbxupdate.bin
%attr(0755,root,root) /opt/lsvmtools-1.0.0/lsvmload/lsvmload.efi
%attr(0755,root,root) /opt/lsvmtools-1.0.0/sanity

%attr(0644,root,root) /opt/lsvmtools-1.0.0/policy/*.policy

%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/*
%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/centos/*
%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/centos/*
%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/redhat/*
%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/sles/*
%attr(0755,root,root) /opt/lsvmtools-1.0.0/scripts/ubuntu/*
