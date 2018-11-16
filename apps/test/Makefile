CFLAGS		 := -O2 -pipe -fPIC -Wall -Werror
INCLUDE      := -g -I${LIBS_DIR} -I${APPS_DIR}
LDFLAGS      := -L$(PUBLIC_LIB_DIR) -L/usr/lib -L/lib -L/usr/local/lib -lrt -ldl -lpthread -lorange

TARGET_TYPE:="exe"

include $(BUILD_COMMON_MK)
	
