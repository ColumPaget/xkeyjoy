OBJ=X11.o evdev.o profile.o proc.o common.o
CFLAGS=-g -O2 -DHAVE_LINUX_INPUT_H -I/usr/X11R7/include
LDFLAGS= -L/usr/X11R7/lib
LIBS=-lX11 
LIBUSEFUL=libUseful/libUseful.a
VERSION=1.0
prefix=/usr/local

all: $(OBJ) $(LIBUSEFUL)
	gcc -oxkeyjoy -DVERSION=\"$(VERSION)\" $(CFLAGS) $(LDFLAGS) $(OBJ) main.c $(LIBS) $(LIBUSEFUL)

$(LIBUSEFUL):
	$(MAKE) -C libUseful


X11.o: X11.c X11.h common.h
	gcc -c X11.c $(CFLAGS)

profile.o: profile.c profile.h
	gcc -c profile.c $(CFLAGS)

proc.o: proc.c proc.h
	gcc -c proc.c $(CFLAGS)

evdev.o: evdev.c evdev.h
	gcc -c evdev.c $(CFLAGS)

common.o: common.c common.h
	gcc -c common.c $(CFLAGS)

install:
	-mkdir -p $(DESTDIR)$(prefix)/bin
	-mkdir -p $(DESTDIR)$(prefix)/etc/xkeyjoy
	cp xkeyjoy $(DESTDIR)$(prefix)/bin
	cp profiles.conf $(DESTDIR)$(prefix)/etc/xkeyjoy

clean:
	rm -f *.o */*.o */*.so */*.a

test:
	echo "No tests"
