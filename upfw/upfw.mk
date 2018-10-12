######################################################################
## Filename:      upfw.mk
## Author:        MJX
## Created at:    2018-04-18
##
## Description:   build upfw app
## Copyright (C)  ABB(Genway)
##
######################################################################
#Source directory
APP_UPFW_SRC_DIR    := $(PUBLIC_APPSRC_DIR)/upfw/
#Target directory
APP_UPFW_TARGET_DIR := $(TOP_DIR)/temp/app/upfw
#Target name
APP_UPFW_TARGET_NAME:= upfw

up:
	@echo "====ABB(Genway)====> Build: $(APP_UPFW_TARGET_NAME)"
	mkdir  -p $(APP_UPFW_TARGET_DIR)
	$(MAKE) -C $(APP_UPFW_SRC_DIR) CC=$(TARGET_CC) CXX=$(TARGET_CXX) TARGET_DIR=$(APP_UPFW_TARGET_DIR) TARGET_NAME=$(APP_UPFW_TARGET_NAME)
	cp $(APP_UPFW_TARGET_DIR)/$(APP_UPFW_TARGET_NAME) $(PUBLIC_APP_DIR)/$(APP_UPFW_TARGET_NAME)
up-clean:
	@echo "====ABB(Genway)====> Clean: $(APP_UPFW_TARGET_NAME)"
	$(MAKE) -C $(APP_UPFW_SRC_DIR) clean TARGET_DIR=$(APP_UPFW_TARGET_DIR) TARGET_NAME=$(APP_UPFW_TARGET_NAME)
	rm $(PUBLIC_APP_DIR)/$(APP_UPFW_TARGET_NAME)
#Add to global: LIBS/CLEANS
SRV   += up
SRV_CLEANS += up-clean
