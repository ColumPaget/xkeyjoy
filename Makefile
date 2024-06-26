OBJ=X11.o evdev.o profile.o proc.o command_line.o config.o common.o
CFLAGS=-g -O2 -DHAVE_LINUX_INPUT_H -I/include -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DSTDC_HEADERS=1 -DHAVE_LIBX11=1 -DHAVE_X11=1 -DHAVE_XKBKEYCODETOKEYSYM=1 -DHAVE_LIBXTST=1 -DHAVE_LIBUSEFUL_5_LIBUSEFUL_H=1 -DHAVE_LIBUSEFUL5=1 -DHAVE_LIBUSEFUL_5=1
LDFLAGS= -L/lib
LIBS=-lUseful-5 -lXtst -lX11 
VERSION=5.1
prefix=/usr/local

all: $(OBJ) 
	$(CC) -oxkeyjoy -DVERSION=\"$(VERSION)\" $(CFLAGS) $(LDFLAGS) $(OBJ) main.c $(LIBS) 

libUseful-5/libUseful.a:
	$(MAKE) -C libUseful-5


X11.o: X11.c X11.h common.h
	$(CC) -c X11.c $(CFLAGS)

profile.o: profile.c profile.h
	$(CC) -c profile.c $(CFLAGS)

proc.o: proc.c proc.h
	$(CC) -c proc.c $(CFLAGS)

evdev.o: evdev.c evdev.h
	$(CC) -c evdev.c $(CFLAGS)

command_line.o: command_line.c command_line.h
	$(CC) -c command_line.c $(CFLAGS)

config.o: config.c config.h
	$(CC) -c config.c $(CFLAGS)

common.o: common.c common.h
	$(CC) -c common.c $(CFLAGS)

install:
	-mkdir -p $(DESTDIR)$(prefix)/bin
	-mkdir -p $(DESTDIR)$(prefix)/etc/xkeyjoy
	cp xkeyjoy $(DESTDIR)$(prefix)/bin
	cp profiles.conf $(DESTDIR)$(prefix)/etc/xkeyjoy

clean:
	rm -f *.o */*.o */*.so */*.a xkeyjoy

test:
	echo "No tests"
