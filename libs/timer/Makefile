SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS= -O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I/usr/local/include
LDFLAGS += -L /usr/local/lib -lorange

all: orange_timer_version.c orange_timer_version.o orange_timer 

LIB_OBJS = orange_timer.o orange_timer_version.o

orange_timer: $(LIB_OBJS)
	${CC} ${CFLAGS} ${LIB_OBJS} -shared -o orange_timer.so ${LDFLAGS}  

orange_timer_version.c:
	@sh orange_timer_version.sh

orange_timer_version.o:
	$(CC) $(CFLAGS) -c orange_timer_version.c
	@rm -rf orange_timer_version.c

distclean: clean
clean:
	rm -f *.o *.so orange_timer_version.c
	
#.c.o:
#	${CC} -fPIC $(CFLAGS) -c ${.IMPSRC} -o ${.TARGET}
