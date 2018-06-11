ifeq ($(TOOL_PREFIX),)
TOOL_PREFIX=/opt/poky/1.6.1/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
endif

TARGET=ipgw

CC=$(TOOL_PREFIX)gcc
CPP=$(TOOL_PREFIX)g++
AR=$(TOOL_PREFIX)ar
LD=$(TOOL_PREFIX)gcc
STRIP=$(TOOL_PREFIX)strip

WARNING_OPT= -Wcast-qual -Wstrict-prototypes -Winline
CC_OPTS= -c -Wall $(WARNING_OPT) -fPIC 
#CC_OPTS+=-g -rdynamic -fstack-check

INCLUDE= -I./inc  -I../../../component/abb_lib/inc/  -I ../../../third_party/inc/ 

LD_OPTS = -L ../../../component/abb_lib/lib/imx6/   -L ../../../third_party/lib/imx6 -L./
LD_OPTS +=  -lwdg_interface -losa -llocalsocket -lcallhistory -lnetwork -lalgorithm -laudio_interface -lvideointerface -lsetting_interface  -lcfgfile -ldevicemanager
LD_OPTS +=  -lsip -lrtp -lssl -lportalclient  -lcurl -luuid  -ljson  -lnopoll  -lcrypto -lssl -lm
LD_OPTS +=  -lpthread -ldl

FILES=$(subst ./, , $(foreach dir,.,$(wildcard $(dir)/*.c))) 
OBJS=$(subst .c,.o, $(FILES) )
vpath %.o $(OBJ_DIR) 

target: $(OBJS)
	$(LD) $(LD_OPTS) -o $(TARGET) $^ 2>&1 

.c.o:
	@echo  $(MODULE): Compiling $< 
	$(CC) $(CC_OPTS) $(DEFINE) $(INCLUDE) -o $@ $< 2>&1
	@echo

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	LD_OPTS = 
.PHONY: target clean
