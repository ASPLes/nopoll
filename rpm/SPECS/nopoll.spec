%define nopoll_version %(cat VERSION)

Name:           nopoll
Version:        %{nopoll_version}
Release:        5%{?dist}
Summary:        WebSocket OpenSource implementation
Group:          System Environment/Libraries
License:        LGPLv2+ 
URL:            http://www.aspl.es/nopoll
Source:         %{name}-%{version}.tar.gz
# %_topdir 	/usr/src/libaxl/rpm
# BuildRequires:  libidn-devel
# BuildRequires:  krb5-devel
# BuildRequires:  libntlm-devel
# BuildRequires:  pkgconfig

%define debug_package %{nil}

%description
noPoll is a WebSocket implementation designed to integreate well into
existing projects that needs support for WebSocket, even in the same
native port.

%prep
%setup -q

%build
%configure --prefix=/usr --sysconfdir=/etc
make clean
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} INSTALL='install -p'
find %{buildroot} -name '*.la' -exec rm -f {} ';'
# %find_lang %{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

# %files -f %{name}.lang
%doc AUTHORS COPYING NEWS README THANKS
# %{_libdir}/libaxl.so.*

# %files devel
# %doc COPYING
# %{_includedir}/axl*
# %{_libdir}/libaxl.so
# %{_libdir}/pkgconfig/axl.pc

# libnopoll0 package
%package -n libnopoll0
Summary: WebSocket OpenSource implementation
Group: System Environment/Libraries
%description  -n libnopoll0
noPoll is a WebSocket implementation designed to integreate well into
existing projects that needs support for WebSocket, even in the same
native port.
%files -n libnopoll0

# libnopoll0-dev package
%package -n libnopoll0-dev
Summary: WebSocket OpenSource implementation (devel)
Group: System Environment/Libraries
%description  -n libnopoll0-dev
noPoll is a WebSocket implementation designed to integreate well into
existing projects that needs support for WebSocket, even in the same
native port (devel packages)
%files -n libnopoll0-dev

%changelog
* Sun Aug 17 2014 Francis Brosnan Blázquez <francis@aspl.es> - (cat VERSION)
- New upstream release

