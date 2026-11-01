Name:           escala-notas
Version:        1.0.1
Release:        1%{?dist}
Summary:        Escala de notas chilena de 1,0 a 7,0 para calcular notas escolares
License:        GPL-3.0-or-later
URL:            https://github.com/navarretenoir/escalanotas
Source0:        https://github.com/navarretenoir/escalanotas/archive/refs/tags/v%{version}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  gcc make pkgconfig(gtk4) pkgconfig(webkitgtk-6.0) pkgconfig(json-glib-1.0)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

%description
Calculates the Chilean 1.0-7.0 grade scale for any exigence and score, grades for a whole class with
average and distribution, and exports to PDF, Excel, Word, CSV and image. Works offline.
Interface in Spanish.

%prep
%autosetup -n escalanotas-%{version}

%build
rm -f desktop/escala-notas; make -C desktop UI_DIR=%{_datadir}/escala-notas/ui EXTRA_CFLAGS="%{optflags} %{build_ldflags}"

%install
D=%{buildroot}
install -Dm755 desktop/escala-notas "%{buildroot}%{_prefix}/bin/escala-notas"
install -d "%{buildroot}%{_prefix}/share/escala-notas"; cp -r ui "%{buildroot}%{_prefix}/share/escala-notas/"; rm -f "%{buildroot}%{_prefix}/share/escala-notas/ui/"*.bak*
install -Dm644 linux/io.github.navarretenoir.EscalaNotas.desktop "%{buildroot}%{_prefix}/share/applications/io.github.navarretenoir.EscalaNotas.desktop"
install -Dm644 linux/io.github.navarretenoir.EscalaNotas.metainfo.xml "%{buildroot}%{_prefix}/share/metainfo/io.github.navarretenoir.EscalaNotas.metainfo.xml"
install -Dm644 linux/io.github.navarretenoir.EscalaNotas.svg "%{buildroot}%{_prefix}/share/icons/hicolor/scalable/apps/io.github.navarretenoir.EscalaNotas.svg"

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/io.github.navarretenoir.EscalaNotas.desktop
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/io.github.navarretenoir.EscalaNotas.metainfo.xml || :

%files
%license LICENSE
%doc README.md
%{_bindir}/*
%{_datadir}/escala-notas
%{_datadir}/applications/io.github.navarretenoir.EscalaNotas.desktop
%{_datadir}/metainfo/io.github.navarretenoir.EscalaNotas.metainfo.xml
%{_datadir}/icons/hicolor/scalable/apps/io.github.navarretenoir.EscalaNotas.svg


%changelog
* Sun Nov 01 2026 Javiera Navarrete <navarrete.noir@gmail.com> - 1.0.1-1
- Paquete inicial desde el código fuente.
