LIB_A_SUBFIX := a
LIB_SO_SUBFIX := so
LIB_PREFIX := lib

PUBLIC_APP_DIR  := $(TOP_DIR)

PUBLIC_LIB_DIR  := $(TOP_DIR)/build/lib

BUILD_COMMON_MK	:= $(TOP_DIR)/build/common.mk

export LIB_PREFIX
export BUILD_COMMON_MK
export LIB_A_SUBFIX
export LIB_SO_SUBFIX
export PUBLIC_APP_DIR

export PUBLIC_LIB_DIR

MS_MAKEFILE_V		= @
MS_MAKE_PARA		= -s
MS_MAKE_JOBS		= -j$(shell cat /proc/cpuinfo | grep processor | wc -l)

GREEN			= \\033[2;32m
LIGHTGREEN		= \\033[1;32m
GREENU			= \\033[4;32m
NOCOLOR			= \\033[0;39m
BLUE			= \\033[2;34m
LIGHTBLUE		= \\033[1;34m
RED			= \\033[0;31m
WARNING			= \\033[0;33m

SUCCESS			= $(LIGHTGREEN)
ORANGE_COMPILE_PRX		= ====ORANGE====> MAKE:
ORANGE_CLEAN_PRX		= ====ORANGE====> CLEAN:
ORANGE_CONFIG_PRX		= ====ORANGE====> CONFIG:

COMPILE_P		= $(NOCOLOR)$(ORANGE_COMPILE_PRX)
COMPILE_SUCCESS_P	= $(SUCCESS)$(ORANGE_COMPILE_PRX)
CLEAN_P			= $(NOCOLOR)$(ORANGE_CLEAN_PRX)
CLEAN_SUCCESS_P		= $(SUCCESS)$(ORANGE_CLEAN_PRX)
CONFIG_P		= $(LIGHTBLUE)$(ORANGE_CONFIG_PRX)
CONFIG_SUCCESS_P	= $(SUCCESS)$(ORANGE_CONFIG_PRX)

include $(TOP_DIR)/apps/watchdog/watchdog.mk
include $(TOP_DIR)/libs/orange/orange.mk
include $(TOP_DIR)/libs/socket/socket.mk
include $(TOP_DIR)/libs/timer/timer.mk
include $(TOP_DIR)/libs/watchdog/watchdog.mk
include $(TOP_DIR)/libs/event/event.mk
include $(TOP_DIR)/libs/thread_pool/thread_pool.mk

.PHONY: appsclean distclean libs apps all
