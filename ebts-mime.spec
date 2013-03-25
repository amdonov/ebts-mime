Name: ebts-mime          
Version: 0.1       
Release:        2%{?dist}
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
javac *.java
jar cvf ebts-mime.jar *.class

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
* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-2
- Corrected typo in install command (amdonov@gmail.com)

* Mon Mar 25 2013 Aaron Donovan <amdonov@gmail.com> 0.1-1
- new package built with tito


