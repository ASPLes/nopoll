%define release_date %(date +"%a %b %d %Y")
%define nopoll_version %(cat VERSION)

Name:           nopoll
Version:        %{nopoll_version}
Release:        5%{?dist}
Summary:        WebSocket OpenSource implementation
Group:          System Environment/Libraries
License:        LGPLv2+ 
URL:            http://www.aspl.es/nopoll
Source:         %{name}-%{version}.tar.gz

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
/usr/lib64/libnopoll.a
/usr/lib64/libnopoll.so
/usr/lib64/libnopoll.so.0
/usr/lib64/libnopoll.so.0.0.0

# libnopoll0-dev package
%package -n libnopoll0-dev
Summary: WebSocket OpenSource implementation (devel)
Group: System Environment/Libraries
%description  -n libnopoll0-dev
noPoll is a WebSocket implementation designed to integreate well into
existing projects that needs support for WebSocket, even in the same
native port (devel packages)
%files -n libnopoll0-dev
/usr/include/nopoll/nopoll.h
/usr/include/nopoll/nopoll_config.h
/usr/include/nopoll/nopoll_conn.h
/usr/include/nopoll/nopoll_conn_opts.h
/usr/include/nopoll/nopoll_ctx.h
/usr/include/nopoll/nopoll_decl.h
/usr/include/nopoll/nopoll_handlers.h
/usr/include/nopoll/nopoll_io.h
/usr/include/nopoll/nopoll_listener.h
/usr/include/nopoll/nopoll_log.h
/usr/include/nopoll/nopoll_loop.h
/usr/include/nopoll/nopoll_msg.h
/usr/include/nopoll/nopoll_private.h
/usr/include/nopoll/nopoll_win32.h
/usr/lib64/pkgconfig/nopoll.pc

%changelog
%include rpm/SPECS/changelog.inc

