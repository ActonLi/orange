######################################################################
## Filename:      PortalClient.mk
## Author:        MJX
## Created at:    2017-07-12
##
## Description:   build PortalClient app
## Copyright (C)  ABB(Genway)
##
######################################################################
#Source directory
APP_PC_SRC_DIR    := $(PUBLIC_APPSRC_DIR)/portalclient/
#Target directory
APP_PC_TARGET_DIR := $(TOP_DIR)/temp/app/portalclient
#Target name
APP_PC_TARGET_NAME:= portalclient

portal:
	@echo "====ABB(Genway)====> Build: $(APP_PC_TARGET_NAME)"
	mkdir  -p $(APP_PC_TARGET_DIR)
	$(MAKE) -C $(APP_PC_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(APP_PC_TARGET_DIR) TARGET_NAME=$(APP_PC_TARGET_NAME)
	cp $(APP_PC_TARGET_DIR)/$(APP_PC_TARGET_NAME) $(PUBLIC_APP_DIR)/$(APP_PC_TARGET_NAME)
portal-clean:
	@echo "====ABB(Genway)====> Clean: $(APP_PC_TARGET_NAME)"
	$(MAKE) -C $(APP_PC_SRC_DIR) clean TARGET_DIR=$(APP_PC_TARGET_DIR) TARGET_NAME=$(APP_PC_TARGET_NAME)
	rm $(PUBLIC_APP_DIR)/$(APP_PC_TARGET_NAME)
#Add to global: LIBS/CLEANS
SRV   += portal
SRV_CLEANS += portal-clean
