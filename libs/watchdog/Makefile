SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS= -O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I/usr/local/include
LDFLAGS += -L /usr/local/lib -lorange

all: orange_watchdog_version.c orange_watchdog_version.o orange_watchdog 

LIB_OBJS = orange_watchdog.o orange_watchdog_version.o

orange_watchdog: $(LIB_OBJS)
	${CC} ${CFLAGS} ${LIB_OBJS} -shared -o orange_watchdog.so ${LDFLAGS}  

orange_watchdog_version.c:
	@sh orange_watchdog_version.sh

orange_watchdog_version.o:
	$(CC) $(CFLAGS) -c orange_watchdog_version.c
	@rm -rf orange_watchdog_version.c

distclean: clean
clean:
	rm -f *.o *.so orange_watchdog_version.c
	
#.c.o:
#	${CC} -fPIC $(CFLAGS) -c ${.IMPSRC} -o ${.TARGET}
