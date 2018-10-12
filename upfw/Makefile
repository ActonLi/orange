## Filename:      test.makefile
##
## Author:        MJX
##
## Created at:    2017-07-12
##
## Description:   testapp
##
## Copyright (C)  ABB(Genway)
##
######################################################################
#
CFLAGS		 := -DREMOTE_SOCKET_TEST -O2 -pipe -fPIC -Wall -Werror
#Include directory
INCLUDE         :=  -g -I$(PUBLIC_INCLUDE_DIR) -I$(PUBLIC_INCLUDE_DIR)/libxml2 -I$(PUBLIC_INCLUDE_DIR)/onvif -I$(PUBLIC_LIBSRC_DIR)/orangelog/inc -I$(PUBLIC_LIBSRC_DIR)/osa/inc -I$(PUBLIC_LIBSRC_DIR)/network/inc -I$(PUBLIC_LIBSRC_DIR)/json/inc -I$(PUBLIC_LIBSRC_DIR)/database/inc -I$(PUBLIC_LIBSRC_DIR)/cfgfile/inc -I/$(PUBLIC_INCLUDE_DIR)/curl 

#Include librarys
LDFLAGS         := -L$(PUBLIC_LIB_DIR) -lcurl -lovfcli -ldatabase -ljson  -losa -lnetwork -lalgorithm -lcfgfile -pthread -lm -ldl -lxml2 -lorangelog
#Target type("exe"-program "so"-shared library "a"-static library)
TARGET_TYPE:="exe"


#include common mk files
include $(BUILD_COMMON_MK)

