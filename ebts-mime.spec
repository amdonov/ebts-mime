Name: ebts-mime          
Version: 0.1       
Release:        4%{?dist}
Summary: C library and Java wrapper for converting EBTS files to MIME files     

License: Booz Allen Hamilton       
Source0: %{name}-%{version}.tar.gz       

BuildRequires: cmake NBIS glib2-devel OpenEBTS
Requires: java glib2 OpenEBTS      

%description


%prep
%setup -q


%build
cmake . -DCMAKE_INSTALL_PREFIX=$RPM_BUILD_ROOT
make
javac -d . *.java
jar cvf ebts-mime.jar com

%install
rm -rf $RPM_BUILD_ROOT
make install


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_datadir}/*
%{_libdir}/*
%{_bindir}/*
%doc

%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-4
- Added init and shutdown methods, moved Java classes to com.bah.biometrics
  package (amdonov@gmail.com)
- Merge branch 'master' of github.com:amdonov/ebts-mime (amdonov@gmail.com)
- Initial commit (amdonov@gmail.com)

* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-3
- Changed install location of ebt-mime jar file. (amdonov@gmail.com)

* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-2
- Corrected typo in install command (amdonov@gmail.com)

* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-1
- new package built with tito


