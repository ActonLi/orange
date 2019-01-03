#
#  Makefile for dwarfdump
#  This is made very simple so it should work with
#  any 'make'.
#  The Makefile does assume that libdwarf is at ../libdwarf
#  from the dwarfdump2 source directory.
#

srcdir =	.


prefix =	/usr/local
exec_prefix =	${prefix}
bindir =	$(exec_prefix)/bin
libdir =	$(exec_prefix)/lib
mandir =        $(exec_prefix)/share/man
man1dir =       $(mandir)/man1


INSTALL =	/usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA =	${INSTALL} -m 644
DATAROOT =      ${prefix}/share
SHELL =		/bin/sh
CC =		arm-poky-linux-gnueabi-gcc  -march=armv7-a -mthumb-interwork -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/opt/poky/1.6.1/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
AR =		arm-poky-linux-gnueabi-ar
ARFLAGS =	@ARFLAGS@
RM =		rm
RANLIB =	arm-poky-linux-gnueabi-ranlib
DEFS =		-DHAVE_CONFIG_H
# ../libdwarf gets us to local headers and a libdwarf 
# archive, usually, so we assume it.
DIRINC =        $(srcdir)/../libdwarf 
dwfwall =       
dwfsanitize =   
dwfzlib =       -lz
LIBS =		 -L../libdwarf -ldwarf -lelf $(dwfzlib)
INCLUDES =	-I. -I$(srcdir) -I../libdwarf -I$(srcdir)/../libdwarf
CFLAGS =	$(PREINCS)    -O2 -pipe -g -feliminate-unused-debug-types $(INCLUDES) \
                $(dwfwall) $(dwfsanitize) -DCONFPREFIX=${libdir} $(POSTINCS)
LDFLAGS =	$(PRELIBS)  $(dwfsanitize) $(LIBS) $(POSTLIBS)
#VG =            valgrind --leak-check=full
VG =


INSTALL = cp

binprefix =

all: simplereader frame1

simplereader: $(srcdir)/simplereader.c
	$(CC) $(CFLAGS) $(srcdir)/simplereader.c -o simplereader $(LDFLAGS)
frame1: $(srcdir)/frame1.c
	$(CC) $(CFLAGS) $(srcdir)/frame1.c -o frame1 $(LDFLAGS)

install: all
	echo do no install

uninstall:
	echo do no uninstall

clean:
	rm -f *.o
	rm -f frame1
	rm -f simplereader
	rm -f *~

distclean: clean
	rm -f config.log config.h config.cache config.status 
	rm -rf autom4te.cache
	rm -rf Makefile

shar:
	@echo "shar not set up yet"
dist:
	@echo "dist not set up yet"
