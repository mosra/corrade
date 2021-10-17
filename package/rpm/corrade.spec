Name:           corrade
Version:        v2020.06
Release:        02a22de6%{?dist}
Summary:        Multiplatform plugin management and utility library

License:        MIT
URL:            https://magnum.graphics/corrade/
Source0:        %{name}-%{version}-%{release}.tar.gz

BuildRequires: gcc-c++
BuildRequires: cmake >= 3.4.0

%description
C++11/C++14 multiplatform utility library

%package devel
Summary:        Multiplatform plugin management and utility library
Requires:       %{name} = %{version}

%description devel
C++11/C++14 multiplatform utility library

%global debug_package %{nil}

%prep
%autosetup


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

%files
%{_prefix}/lib*/lib*.so.*
%doc COPYING

%files devel
%{_prefix}/bin/*
%{_prefix}/include/Corrade/*
%{_prefix}/lib*/lib*.so
%{_prefix}/share/cmake*
%doc COPYING


%changelog
* Sun Oct 17 2021 1b00 <1b00@pm.me> - v2020.06-02a22de6
- Script updated to current spec file format
* Wed Feb 08 2012 Vladimír Vondruš <mosra@centrum.cz> - 0.2-1
- Initial release.
