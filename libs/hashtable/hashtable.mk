HASHTABLE_SRC_DIR	:= $(LIBS_DIR)/hashtable

HASHTABLE_TARGET_DIR	:= $(TOP_DIR)/temp/libs/hashtable

HASHTABLE_TARGET_NAME	:= orange_hashtable.$(LIB_SO_SUBFIX)

hashtable: 
	cd ${HASHTABLE_SRC_DIR} && sh ./orange_hashtable_version.sh	
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(HASHTABLE_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(HASHTABLE_TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -C $(HASHTABLE_SRC_DIR) CC=$(TARGET_CC) TARGET_DIR=$(HASHTABLE_TARGET_DIR) TARGET_NAME=$(HASHTABLE_TARGET_NAME)
	cp $(HASHTABLE_TARGET_DIR)/$(HASHTABLE_TARGET_NAME) $(PUBLIC_LIB_DIR)/$(HASHTABLE_TARGET_NAME)
	rm ${HASHTABLE_SRC_DIR}/orange_hashtable_version.c

hashtable-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(HASHTABLE_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -C $(HASHTABLE_SRC_DIR) clean TARGET_DIR=$(HASHTABLE_TARGET_DIR) TARGET_NAME=$(HASHTABLE_TARGET_NAME)
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(HASHTABLE_TARGET_NAME)
	-$(MS_MAKEFILE_V)rm $(HASHTABLE_SRC_DIR)/orange_hashtable_version.c
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(HASHTABLE_TARGET_NAME) success $(NOCOLOR)\n"

LIBS   += hashtable
CLEANS += hashtable-clean


