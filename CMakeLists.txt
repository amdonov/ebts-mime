cmake_minimum_required (VERSION 2.6)
project (ebts-mime)
add_executable(ebts-mime main.c)
include_directories("/usr/include/gmime-2.6" "/usr/include/OpenEBTS" 
	"/usr/include/glib-2.0/" "/usr/lib64/glib-2.0/include/"
	"/usr/include/libxml2")
target_link_libraries(ebts-mime gmime-2.6 OpenEBTS xml2)
