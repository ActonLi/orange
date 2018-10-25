AllDirs := $(shell ls -R | grep '^\./.*:$$' | awk '{gsub(":","");print}') .  

ALL_SRC_FILES	:= $(foreach n,$(AllDirs) , $(wildcard $(n)/*.c)) 
ALL_CPP_FILES	:= $(foreach n,$(AllDirs) , $(wildcard $(n)/*.cpp)) 

ALL_TARGET_OBJS	:= $(patsubst %.c,$(TARGET_DIR)/%.o,$(ALL_SRC_FILES))
ALL_TARGET_OBJS	+= $(patsubst %.cpp,$(TARGET_DIR)/%.o,$(ALL_CPP_FILES))

DEPEND_FILE		:= $(TARGET_DIR)/MAKEFILE.DEPEND

ifeq ($(TARGET_NAME),)
TARGET_NAME		:=oooxxxx
endif

STRIP_FLAG		:= n

ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -fPIC#-Wall -marm -march=armv6k -mtune=arm1136j-s -mlittle-endian -msoft-float
else
CFLAGS			+= -fPIC#-g -Wall -marm -march=armv6k -mtune=arm1136j-s -mlittle-endian -msoft-float
endif

ifeq ($(TARGET_TYPE),"so")
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -s
endif
MK_DYNAMIC		:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET  		:= $(MK_DYNAMIC)
MK_TARGET  		:  $(MK_TARGET)
endif
ifeq ($(TARGET_TYPE),"dll")
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -s
endif
MK_DYNAMIC		:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET  		:= $(MK_DYNAMIC)
MK_TARGET  		:  $(MK_TARGET)
endif
ifeq ($(TARGET_TYPE),"exe")
ifeq ($(STRIP_FLAG),y)
CFLAGS			+= -s
endif
MK_EXE			:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET  		:= $(MK_EXE)
MK_TARGET  		:  $(MK_TARGET)
endif

ifeq ($(TARGET_TYPE),"a")
MK_STATIC		:= $(TARGET_DIR)/$(TARGET_NAME)
MK_TARGET		:= $(MK_STATIC)
MK_TARGET		:  $(MK_TARGET)
endif

#include dependencies
ifneq ($(MAKECMDGOALS), clean)
#include $(DEPEND_FILE)#delete for make warning.
endif

$(MK_DYNAMIC): $(ALL_TARGET_OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ $(LDFLAGS)

$(MK_STATIC): $(ALL_TARGET_OBJS)
	$(AR) rcs $@ $^

$(MK_EXE): $(ALL_TARGET_OBJS)
ifeq ($(ALL_CPP_FILES), )
	$(CC) $(INCLUDE) $(CFLAGS) -o $@ $^ $(LDFLAGS)
else
	$(CXX) $(INCLUDE) $(CFLAGS) -o $@ $^ $(LDFLAGS)
endif

$(TARGET_DIR)/%.o:%.c
	@[ ! -d $(dir $@) ] & mkdir -p $(dir $@)
	$(CC) $(INCLUDE) $(CFLAGS) -o $@ -c $<

$(TARGET_DIR)/%.o:%.cpp
	@[ ! -d $(dir $@) ] & mkdir -p $(dir $@)
	$(CXX) $(INCLUDE) $(CFLAGS) -o $@ -c $<

depend:
	@set -e; \
	$(CC) $(INCLUDE) -E -MM $(ALL_SRC_FILES) | sed 's,\(.*\)\.o[ :]*,$(TARGET_DIR)/\1.o $@:,g'>$(DEPEND_FILE);

clean:
	-rm -rf $(TARGET_DIR)/
