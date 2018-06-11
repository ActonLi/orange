## Filename:      testapp.makefile
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
#INCLUDE         := -I$(PUBLIC_INCLUDE_DIR) -I$(PUBLIC_LIBSRC_DIR)/json/inc -I$(PUBLIC_LIBSRC_DIR)/osa/inc -I$(PUBLIC_LIBSRC_DIR)/network/inc -I$(PUBLIC_LIBSRC_DIR)/localsocket/inc -I./src -I./inc -I./inc/curl -I./inc/nopoll -I./inc/openssl -I./inc/rtp_rtcp -I./inc/sip -I./inc/sqlite -I./inc/strophe

INCLUDE         :=  -g -I$(PUBLIC_INCLUDE_DIR) -I$(PUBLIC_INCLUDE_DIR)/libxml2 -I$(PUBLIC_INCLUDE_DIR)/onvif -I$(PUBLIC_LIBSRC_DIR)/osa/inc -I$(PUBLIC_LIBSRC_DIR)/network/inc -I$(PUBLIC_LIBSRC_DIR)/json/inc -I$(PUBLIC_LIBSRC_DIR)/database/inc -I$(PUBLIC_LIBSRC_DIR)/cfgfile/inc -I$(PUBLIC_LIBSRC_DIR)/localsocket/inc -I./src -I./inc -I./inc/curl -I./inc/nopoll -I./inc/openssl -I./inc/rtp_rtcp -I./inc/sip -I./inc/sqlite -I./inc/strophe 

#Include librarys
LDFLAGS         := -L$(PUBLIC_LIB_DIR)  -lnetwork -lalgorithm -lcfgfile -losa -llocalsocket -ljson -lssl -lcurl -lcrypto -luuid -ldatabase -pthread -lxml2 -lresolv -lstrophe 
#Target type("exe"-program "so"-shared library "a"-static library)
TARGET_TYPE:="exe"


#include common mk files
include $(BUILD_COMMON_MK)

