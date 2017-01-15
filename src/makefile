all:
	i686-w64-mingw32.static-gcc icon.o resource.c main.c -o stupid-backup.exe -mwindows `i686-w64-mingw32.static-pkg-config --cflags --libs gio-2.0`
	strip stupid-backup.exe
clean:
	rm ../stupid-backup.exe
