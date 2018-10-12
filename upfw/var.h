#ifndef __FW_VAR_H_
#define __FW_VAR_H_

#include "watchdog_client.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "define.h"
#include "message.h"

#define DAEMON_NAME_SIZE 32
#define VERSION_LEN_MAX 16
#define SERIAL_MASK_LEN_MAX 16
#define FIRMWARE_NAME_LEN_MAX 64
#define XML_FILENAME_LEN_MAX 64
#define FILE_PATH_LEN_MAX 128
#define FWS_AVAILABLE_COUNT_MAX 32
#define CHECKING 1
#define OUT_PARAMETER_LEN_MAX 128
#define URL_FILE_LEN_MAX 512
#define CMD_LEN_MAX 1024
#define MAXBUF 65535

#define CHECK_TIMEOUT (25 * 60 * 60)

#define DAEMON_NAME "upfw"

#define UNIX_IPC_PATH "/tmp/test_unixsock"

#define UPDATE_DIR "/var/volatile/tmp"

#define UPDATE_URL "ftp://liyeqiang:1234abcd,A@192.168.0.23"
#define RSA_PUBKEY "/etc/rsa_public.pem"
#define DEFALT_JSON_FILE "myversion.json"

#define DEFALT_JSON_FILE_DIR "/home/wipap/frontend/firmwares"

#define FRONTEND_FILE_SUFFIX "frontend.json"
#define UPFW_FILE_SUFFIX "upfw.json"

#define JSON_LOAD_TYPE_FILE 1
#define JSON_LOAD_TYPE_STRING 2

typedef struct config_info {
	char* firmware_info;
	char* update_url;
	char* update_dir;
	char* public_key;
	int   daemon;
} config_info_t;

typedef struct firmware_info {
	char serial_mask[SERIAL_MASK_LEN_MAX];
	char version[VERSION_LEN_MAX];
	char min_version[VERSION_LEN_MAX];
	int  build_number;
	int  firmware_size;
	int  deviceid;

	char		  firmware_filename[FIRMWARE_NAME_LEN_MAX];
	char		  xml_filename[XML_FILENAME_LEN_MAX];
	unsigned char is_local;
	char*		  de;
	char*		  en;
} firmware_info_t;

typedef struct firmware_available {
	int					  fw_count;
	struct firmware_info* fw[FWS_AVAILABLE_COUNT_MAX];
} firmware_available_t;

extern U64				  last_check_timestamp;
extern int				  check_status;
extern struct config_info config;
extern int upfw_send_log(char* fmt, ...);

#endif
