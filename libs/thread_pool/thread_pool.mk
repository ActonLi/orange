THREAD_POOL_SRC_DIR	:= $(LIBS_DIR)/thread_pool

THREAD_POOL_TARGET_DIR	:= $(TOP_DIR)/temp/libs/thread_pool

THREAD_POOL_TARGET_NAME	:= orange_thread_pool.$(LIB_SO_SUBFIX)

thread_pool: 
	cd ${THREAD_POOL_SRC_DIR} && sh ./orange_thread_pool_version.sh	
	$(MS_MAKEFILE_V)printf "$(COMPILE_P) $(THREAD_POOL_TARGET_NAME) $(NOCOLOR)\n"
	-$(MS_MAKEFILE_V)mkdir -p $(THREAD_POOL_TARGET_DIR)
	$(MS_MAKEFILE_V)$(MAKE) -C $(THREAD_POOL_SRC_DIR) CC=$(TARGET_CC) TARGET_DIR=$(THREAD_POOL_TARGET_DIR) TARGET_NAME=$(THREAD_POOL_TARGET_NAME)
	cp $(THREAD_POOL_TARGET_DIR)/$(THREAD_POOL_TARGET_NAME) $(PUBLIC_LIB_DIR)/$(THREAD_POOL_TARGET_NAME)
	rm ${THREAD_POOL_SRC_DIR}/orange_thread_pool_version.c

thread_pool-clean:
	$(MS_MAKEFILE_V)printf "$(CLEAN_P) $(THREAD_POOL_TARGET_NAME) $(NOCOLOR)\n"
	$(MS_MAKEFILE_V)$(MAKE) -C $(THREAD_POOL_SRC_DIR) clean TARGET_DIR=$(THREAD_POOL_TARGET_DIR) TARGET_NAME=$(THREAD_POOL_TARGET_NAME)
	-$(MS_MAKEFILE_V)rm -rf $(PUBLIC_LIB_DIR)/$(THREAD_POOL_TARGET_NAME)
	$(MS_MAKEFILE_V)printf  "$(COMPILE_SUCCESS_P) $(THREAD_POOL_TARGET_NAME) success $(NOCOLOR)\n"

LIBS   += thread_pool
CLEANS += thread_pool-clean


