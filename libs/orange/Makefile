SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS= -O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I/usr/local/include
LDFLAGS += -L /usr/local/lib -lrt -ldl

all: liborange 

LIB_OBJS = orange_log.o orange_file.o orange_utils.o \
		   log/orange_filelog.o log/orange_consolelog.o \
		   log/orange_syslog.o orange_daemon.o orange_options.o \
		   orange_mutex.o orange_sem.o orange_thread.o \
		   orange_module.o orange_elf.o

liborange: $(LIB_OBJS)
	${CC} ${CFLAGS} ${LIB_OBJS} -shared -o liborange.so ${LDFLAGS}  

distclean: clean
clean:
	rm -f *.o log/*.o *.so
	
