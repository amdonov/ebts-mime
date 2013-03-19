cmake_minimum_required (VERSION 2.6)
project (ebts-mime)
include_directories("/usr/include/gmime-2.6"
	"/usr/include/glib-2.0/" "/usr/lib64/glib-2.0/include/"
	"/usr/include/libxml2" "/usr/java/latest/include"
	"/usr/java/latest/include/linux")
add_executable(ebts-mime main.c ebts.c fswatch.c)
target_link_libraries(ebts-mime gmime-2.6 OpenEBTS xml2 glib-2.0 gthread-2.0)
FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})
SWIG_ADD_MODULE(libebts java ebts.i ebts.c)
SWIG_LINK_LIBRARIES(libebts gmime-2.6 OpenEBTS xml2)
