SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS=-O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I/usr/local/include -I./
LDFLAGS += -L/usr/lib -L/lib -L/usr/local/lib -pthread

all: test 

OBJS = test.o

watchdog: $(OBJS)
	${CC} ${CFLAGS} ${OBJS} -o test ${LDFLAGS}  

distclean: clean
clean:
	rm -f *.o test
	
