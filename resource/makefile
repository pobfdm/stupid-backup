all:
	glib-compile-resources --generate-source resource.xml
	mv resource.c ../src/resource.c
	i686-w64-mingw32.static-windres winresource.rc -o ../src/icon.o
	
