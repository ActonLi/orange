SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS= -O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I/usr/local/include
LDFLAGS += -L /usr/local/lib -lorange

all: orange_socket_version.c orange_socket_version.o orange_socket 

LIB_OBJS = orange_socket.o orange_socket_tcp.o orange_socket_udp.o \
		   orange_socket_un.o orange_socket_version.o

orange_socket: $(LIB_OBJS)
	${CC} ${CFLAGS} ${LIB_OBJS} -shared -o orange_socket.so ${LDFLAGS}  

orange_socket_version.c:
	@sh orange_socket_version.sh

orange_socket_version.o:
	$(CC) $(CFLAGS) -c orange_socket_version.c
	@rm -rf orange_socket_version.c

distclean: clean
clean:
	rm -f *.o *.so orange_socket_version.c
	
#.c.o:
#	${CC} -fPIC $(CFLAGS) -c ${.IMPSRC} -o ${.TARGET}
