Name: corrade
URL: https://magnum.graphics/corrade/
Version: 0.2
Release: 1
License: MIT
%if %{defined suse_version}
Group: System/Libraries
%else
Group: System Environment/Libraries
%endif
Source: https://github.com/mosra/%{name}/tarball/v%{version}/%{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: gcc-c++
BuildRequires: cmake >= 2.6.0

Summary: Multiplatform plugin management and utility library

%description
Provides debugging, portability, configuration, resource management and
filesystem utilites and plugin management with dependency handling.

%package devel
%if %{defined suse_version}
Group: Development/Libraries/C and C++
%else
Group: Development/Libraries
%endif
Summary: Corrade development files
Requires: %{name} = %{version}

%description devel
Headers and tools needed for developing with Corrade.

%prep
%setup -q -n mosra-corrade-87ac3ee

%build
mkdir build && cd build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \
    -DCMAKE_BUILD_TYPE=Release
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
cd build
make DESTDIR=$RPM_BUILD_ROOT install
strip $RPM_BUILD_ROOT/%{_prefix}/lib*/* $RPM_BUILD_ROOT/%{_prefix}/bin/*

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_prefix}/lib*/*.so.*
%doc COPYING COPYING.LESSER

%files devel
%defattr(-,root,root,-)
%{_prefix}/bin/corrade-rc
%{_prefix}/include/Corrade
%{_prefix}/share/*/Modules
%{_prefix}/lib*/*.so
%doc COPYING COPYING.LESSER

%changelog
* Wed Feb 08 2012 Vladimír Vondruš <mosra@centrum.cz> - 0.2-1
- Initial release.
