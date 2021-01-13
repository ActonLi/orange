SHELL=/bin/sh
PREFIX=/usr/local/

CFLAGS=-O2 -pipe -fPIC -fno-strict-aliasing -fno-common \
			   -Wall -Werror -Wno-pointer-sign -Wno-unused-parameter \
			   -Wredundant-decls -Wnested-externs -Wstrict-prototypes \
			   -Wmissing-prototypes -Wpointer-arith -Winline -Wcast-qual \
			   -Wundef -g -std=gnu9x -fstack-protector

CFLAGS += -I./ \
		  -I/opt/arm-linux-gcc-4.8.3/arm-none-linux-gnueabi/libc/usr/include \

LDFLAGS += -L/opt/arm-linux-gcc-4.8.3/arm-none-linux-gnueabi/libc/lib \
		   -L/opt/arm-linux-gcc-4.8.3/arm-none-linux-gnueabi/libc/usr/lib \
		   -lpthread \
		   -lm -ldl

CC     = arm-none-linux-gnueabi-gcc

all: paraset 

OBJS = paraset.o cJSON.o 

paraset: $(OBJS)
	${CC} ${CFLAGS} ${OBJS} -o paraset ${LDFLAGS}  

distclean: clean
clean:
	rm -f *.o paraset
