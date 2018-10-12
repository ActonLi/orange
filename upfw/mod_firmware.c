#include "mod_firmware.h"
#include "cfg_ini.h"
#include "common.h"
#include "events.h"
#include "firmware.h"
#include "json.h"
#include "lsrpc.h"
#include "mod_thread.h"
#include "module.h"
#include "orangelog.h"
#include "queue.h"
#include "remotesocket.h"
#include "var.h"
#include "tree.h"
#include "queue.h"
#include "watchdog_client.h"
#include "xml.h"
#include "xml_to_json.h"
#include <dirent.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>

#if 0
#define DEBUGP orangelog_log_info
#else
#define DEBUGP(format, args...)
#endif

#if 0
#define DEBUGP_TEST printf
#else
#define DEBUGP_TEST(format, args...)
#endif

#define UPFW_METHOD_NOTIFY "notifyUPFWProcess"
#define UPFW_METHOD_QUERY "queryUPFWProcess"

#define UPFW_METHOD_NOTIFY_ACK "notifyUPFWProcessAck"
#define UPFW_METHOD_QUERY_ACK "queryUPFWProcessAck"

#define UPFW_METHOD_QUERY_UPDATE_PROGRESS "queryUpdateProgress"
#define UPFW_METHOD_PUBLISH_UPDATE_PROGRESS "publishUpdateStatus"

#define UPFW_METHOD_QUERY_VERSION "queryFirmwareVersion"
#define UPFW_METHOD_REQUEST_UPDATE "requestUpdateFirmware"

#define UPFW_UPDATE_DEVICE_COUNT_MAX   8 

static void fw_query_firmware_version(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_update_firmware(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_notify_upfw(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_query_upfw(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_notify_upfw_ack(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_query_upfw_ack(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
static void fw_query_update_progress(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);

const STRLSRPCType fm_appRPCFun[] = {
	{UPFW_METHOD_QUERY_VERSION, fw_query_firmware_version},
	{UPFW_METHOD_REQUEST_UPDATE, fw_update_firmware},
	{UPFW_METHOD_NOTIFY, fw_notify_upfw},
	{UPFW_METHOD_QUERY, fw_query_upfw},
	{UPFW_METHOD_NOTIFY_ACK, fw_notify_upfw_ack},
	{UPFW_METHOD_QUERY_ACK, fw_query_upfw_ack},
	{UPFW_METHOD_QUERY_UPDATE_PROGRESS, fw_query_update_progress}
};

typedef struct upfw_query_status {
	int status;
	int percentage;
	int result;
	U64 timestamp;
} upfw_query_status_t;

#define FILE_NAME_LEN_MAX 1024
#define SERIAL_NUMBER_LEN_MAX 32

typedef struct upfw_update_status {
	int  session_id;
	int  result;
	int  status;
	int  step;
	int  percentage;
	int  need_query;
	int  need_send;
	int  file_size;
	char file_name[FILE_NAME_LEN_MAX];
	U64  timestamp;
} upfw_update_status_t;

typedef struct upfw_update_cmp {
    int session_id;
} upfw_update_cmp_t;

typedef struct upfw_update_entry {
    int session_id;

    U8  update_type;  /* 0: download + update, 1: download + notification, 2: notification */
    int filesize;
    int software_id;
    char wipap_sn[SERIAL_NUMBER_LEN_MAX];
    char serial_number[SERIAL_NUMBER_LEN_MAX];
    char filename[128];

	RB_ENTRY(upfw_update_entry) entry;
	TAILQ_ENTRY(upfw_update_entry) list;
} upfw_update_entry_t; 

static inline int __upfw_update_entry_cmp(struct upfw_update_entry* a, struct upfw_update_entry* b);
RB_HEAD(upfw_update_tree, upfw_update_entry);
RB_PROTOTYPE(upfw_update_tree, upfw_update_entry, entry, __upfw_update_entry_cmp);
RB_GENERATE(upfw_update_tree, upfw_update_entry, entry, __upfw_update_entry_cmp);

typedef struct upfw_update_data {
    TAILQ_HEAD(upfw_update_head, upfw_update_entry) upfw_list_head;
    struct upfw_update_tree upfw_rb_tree;
    U8 is_updating;
    int g_session_id;
} upfw_update_data_t; 

struct upfw_update_data g_upfw_update;

static struct upfw_update_status current_update_status;
static struct upfw_query_status  current_query_status;

U32 localperiod = 1000;

struct device_key_list frontend_key = {.key_count = 4,
									   .key_info  = {
										   {"serialMask", cJSON_String}, {"version", cJSON_String}, {"i", cJSON_Number}, {"softwareId", cJSON_Number},
									   }};

struct device_key_list upfw_key = {.key_count = 6,
								   .key_info  = {
									   {"serialMask", cJSON_String},
									   {"updateType", cJSON_Number},
									   {"version", cJSON_String},
									   {"fileName", cJSON_String},
									   {"i", cJSON_Number},
									   {"fileSize", cJSON_Number},
									   {"softwareId", cJSON_Number},
								   }};


static inline int __upfw_update_entry_cmp(struct upfw_update_entry* a, struct upfw_update_entry* b)
{
    return (a->session_id - b->session_id);
}

static struct upfw_update_entry* __upfw_update_entry_find(int session_id) 
{
    struct upfw_update_cmp cmp;

    cmp.session_id = session_id;

    return RB_FIND(upfw_update_tree, &g_upfw_update.upfw_rb_tree, (struct upfw_update_entry*)&cmp);
}

static void __upfw_update_entry_free(struct upfw_update_entry* entry) 
{
    if (entry) {
        free(entry);
    }

    return;
}

static struct upfw_update_entry* __upfw_update_entry_alloc(void)
{
    struct upfw_update_entry* entry = NULL;

    entry = malloc(sizeof(struct upfw_update_entry));
    if (entry) {
        memset(entry, 0, sizeof(struct upfw_update_entry));
    }

    return entry;
}

static void __fm_setperiod(U32 period)
{
	localperiod = period;
}

int __fw_readline(char* line, int size, FILE* fp)
{
	int length = 0;

	if (fgets(line, size, fp) != NULL) {
		length = strlen(line);
		if (line[length - 1] == '\n' && line[length - 2] == '\r') {
			line[length - 2] = '\0';
			length -= 2;
		} else {
			line[length - 1] = '\0';
			length--;
		}
	}

	return length;
}

typedef struct df_info {
	char filesystem[512];
	int  blocks;
	int  used;
	int  available;
	char use_percentage[8];
	char mount_on[128];
} df_info_t;

static int __fw_get_update_dir_available_space(char* update_dir, int* available_size)
{
	int			   ret = -1;
	FILE*		   fp  = NULL;
	char		   buf[1024];
	struct df_info df;
	int			   i;
	int			   tmp_len = 0;
	char*		   p_dir;

	fp = popen("df", "r");
	if (NULL == fp) {
		goto exit;
	}

	while (__fw_readline(buf, 1024, fp) > 0) {
		if (NULL != strstr(buf, "Filesystem")) {
			continue;
		}

		memset(&df, 0, sizeof(df));
		sscanf(buf, "%s %d %d %d %s %s", df.filesystem, &df.blocks, &df.used, &df.available, df.use_percentage, df.mount_on);

		i	 = 0;
		p_dir = update_dir;
		while (*p_dir != '\0' || df.mount_on[i] != '\0') {
			if (*p_dir != df.mount_on[i]) {
				break;
			}
			i++;
			p_dir++;
		}
		if (i > tmp_len) {
			tmp_len			= i;
			*available_size = df.available;
		}
	}

	ret = 0;
exit:
	if (fp) {
		fclose(fp);
	}

	return ret;
}

static int __fw_append_device_info(void* data, void* arg)
{
	cJSON*				versions = arg;
	struct device_info* info	 = data;
	cJSON* __attribute__((unused)) version;
	cJSON* version_item;
	cJSON* __attribute__((unused)) hardwareversion;
	cJSON* hardwareversion_item;

	hardwareversion_item = cJSON_CreateObject();
	version_item		 = cJSON_CreateObject();
	hardwareversion		 = cJSON_CreateArray();
	version				 = cJSON_CreateArray();

	cJSON_AddStringToObject(hardwareversion_item, "minVersion", info->min_version);
	cJSON_AddStringToObject(hardwareversion_item, "maxVersion", info->min_version);

	cJSON_AddItemToArray(hardwareversion, hardwareversion_item);

	cJSON_AddItemToObject(version_item, "hardwareversion", hardwareversion);
	cJSON_AddNumberToObject(version_item, "i", info->index);
	cJSON_AddStringToObject(version_item, "version", info->version);
	cJSON_AddNumberToObject(version_item, "build", info->build_number);

	cJSON_AddItemToArray(versions, version_item);

	DEBUGP("index: %d, version: %s, serialMask: %s\n", info->index, info->version, info->serial_mask);

	return 0;
}

static int __fw_get_upgradeable_device_info(int software_id, char* serial_mask, cJSON* out)
{
	char   file_path[256];
	cJSON* versions = cJSON_CreateArray();

	snprintf(file_path, sizeof(file_path), "%s/%s.xml", config.update_dir, serial_mask);

	pase_xml_callback(file_path, serial_mask, __fw_append_device_info, (void*) versions);

	snprintf(file_path, sizeof(file_path), "%s_%s", serial_mask, FRONTEND_FILE_SUFFIX);
	cJSON_AddNumberToObject(out, "softwareId", software_id);
	cJSON_AddStringToObject(out, "descriptionfile", file_path);
	cJSON_AddStringToObject(out, "serialMask", serial_mask);
	cJSON_AddItemToObject(out, "versions", versions);

	return 0;
}

static int __fw_make_mount_dir(char* mount_dir)
{
	char cmd[256];
	int  ret = 0;

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	if (mount_dir == NULL) {
		printf("\n%s mount_dir is null\n", __func__);
		goto exit;
	}

	ret = system("cd / ");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	snprintf(cmd, sizeof(cmd), "mkdir -p %s", mount_dir);
	ret = system(cmd);
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

exit:
	return ret;
}

static void __fw_unmount_mount_dir(char* mount_dir)
{
	char cmd[256];
	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	snprintf(cmd, sizeof(cmd), "umount %s 2>>/dev/null", mount_dir);
	system(cmd);

	return;
}

static int __fw_mount_update_partion(char* mount_dir)
{
	char cmd[256];
	int  ret = 0;
	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	snprintf(cmd, sizeof(cmd), "mount -t ubifs ubi1:update %s", mount_dir);

	ret = system(cmd);
	ret = firmware_get_system_return_value(ret);

	return ret;
}

static int __fw_format_update_partion_and_mount(char* mount_dir)
{
	int ret = 0;

	DEBUGP("\n============%s Enter==========\n", __func__);
	__fw_unmount_mount_dir(mount_dir);
	ret = system("ubidetach /dev/ubi_ctrl -m 10  2>>/dev/null");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = system("flash_erase /dev/mtd10 0 0");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = system("ubiformat /dev/mtd10");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = system("ubiattach /dev/ubi_ctrl -m 10 -d 1");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = system("ubimkvol /dev/ubi1  -N update -m");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = __fw_mount_update_partion(mount_dir);

exit:
	return ret;
}

static void __fw_clean_update_partion(char* mount_dir)
{
	int  ret = -1;
	char cmd[256];

	DEBUGP("============%s Enter==========\n", __func__);
	snprintf(cmd, sizeof(cmd), "rm -fr %s/*", mount_dir);
	ret = system(cmd);
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = system("sync");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

exit:
	DEBUGP("%s:rm all the contents in %s \n", __func__, mount_dir);
	return;
}

static void __fw_check_and_write_available_size(void)
{
	char  available_path[128] = {0};
	char  path[128]			  = {0};
	char* p_update_dir		  = config.update_dir;
	char* up_dir			  = strrchr(config.update_dir, '/');
	int   available_size	  = 0;
	char  as_str[32];
	FILE* fp = NULL;

	if (NULL == up_dir) {
		goto exit;
	}

	__fw_get_update_dir_available_space(config.update_dir, &available_size);

	memcpy(available_path, p_update_dir, up_dir - p_update_dir);
	snprintf(path, 128, "%s/available_space/config", available_path);
	if (-1 == access(path, F_OK)) {
		up_dir = strrchr(path, '/');
		memcpy(available_path, path, up_dir - path);
		__fw_make_mount_dir(available_path);
		snprintf(as_str, 32, "%d", available_size);
		fp = fopen(path, "w+");
		if (fp) {
			fwrite(as_str, 1, strlen(as_str), fp);
		}
	} else {
		fp = fopen(path, "r");
		if (NULL == fp) {
			goto exit;
		}
		memset(as_str, 0, 32);
		fread(as_str, 1, 32, fp);
		if (atoi(as_str) < available_size) {
			fclose(fp);
			fp = fopen(path, "w+");
			if (fp) {
				snprintf(as_str, 32, "%d", available_size);
				fwrite(as_str, 1, strlen(as_str), fp);
			}
		}
	}

exit:
	if (NULL != fp) {
		fclose(fp);
	}
	return;
}

static int __fw_mount_update(char* mount_dir)
{
	int ret = -1;

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	ret = __fw_make_mount_dir(mount_dir);
	if (ret != 0) {
		printf("%s:mkdir -p %s -->result = %d\n", __func__, mount_dir, ret);
		goto exit;
	}

	__fw_unmount_mount_dir(mount_dir);

	system("ubidetach /dev/ubi_ctrl -m 10 2>>/dev/null");

	ret = system("ubiattach /dev/ubi_ctrl -m 10 -d 1");
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		printf("%s:%d error: %d.\n", __func__, __LINE__, ret);
		goto exit;
	}

	ret = __fw_mount_update_partion(mount_dir);
	if (ret != 0) {
		ret = __fw_format_update_partion_and_mount(mount_dir);
	}

	if (ret == 0) {
		__fw_clean_update_partion(mount_dir);
	}

	DEBUGP("%s to %s  result = %d\n", __func__, mount_dir, ret);

exit:
	return ret;
}

static int __fw_update_desap_copy_image(char* filename)
{
	char file_path[512];
	char cmd[1024];
	int  status = -1;

	snprintf(file_path, sizeof(file_path), "%s/%s", config.update_dir, filename);

	DEBUGP("%s:%d file_path: %s\n", __func__, __LINE__, file_path);

	status = __fw_mount_update("/update");
	if (0 != status) {
		DEBUGP("%s:%d mount error.\n", __func__, __LINE__);
		goto exit;
	}
	// status = system(MOUNT_UPDATE_SCRIPT);
	status = firmware_get_system_return_value(status);
	if (0 == status) {
		DEBUGP("%s:%d cmd: cp %s /update/\n", __func__, __LINE__, file_path);
		snprintf(cmd, sizeof(cmd), "cp %s /update/", file_path);
		status = system(cmd);
		status = firmware_get_system_return_value(status);
		DEBUGP("%s:%d cmd: cp %s /update/ end\n", __func__, __LINE__, file_path);
		system("umount /update");
	}

exit:
	return status;
}

int __fw_get_firmware_info(int index, char* version, char* serial_mask, char* json_file, char* firmware_name, int* software_id, int* file_size, U8* update_type)
{
	int	ret			= -1;
	cJSON* item			= NULL;
	cJSON* version_json = NULL;
	cJSON* root			= NULL;
	char   tmp_serial_mask[SERIAL_MASK_LEN_MAX];
	char   tmp_version[VERSION_LEN_MAX];
	int	tmp_index = -1;

	if (TRUE != read_jsonfile(json_file, &root) || root == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	version_json = root->child;
	if (version_json == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	while (version_json) {
		item = cJSON_GetObjectItem(version_json, "i");
		if (item && item->type == cJSON_Number) {
			tmp_index = item->valueint;
		}
		if (index != tmp_index) {
			version_json = version_json->next;
			continue;
		}

		item = cJSON_GetObjectItem(version_json, "version");
		if (item && item->type == cJSON_String) {
			memset(tmp_version, 0, VERSION_LEN_MAX);
			snprintf(tmp_version, sizeof(tmp_version), "%s", item->valuestring);
		}
		if (0 != memcmp(tmp_version, version, VERSION_LEN_MAX)) {
			version_json = version_json->next;
			continue;
		}

		item = cJSON_GetObjectItem(version_json, "serialMask");
		if (item && item->type == cJSON_String) {
			memset(tmp_serial_mask, 0, SERIAL_MASK_LEN_MAX);
			snprintf(tmp_serial_mask, sizeof(tmp_serial_mask), "%s", item->valuestring);
		}
		if (0 != memcmp(tmp_serial_mask, serial_mask, SERIAL_MASK_LEN_MAX)) {
			version_json = version_json->next;
			continue;
		}

		DEBUGP("%s:%d index: %d, version: %s, serial_mask: %s\n", __func__, __LINE__, tmp_index, tmp_version, tmp_serial_mask);

		item = cJSON_GetObjectItem(version_json, "fileSize");
		if (item && item->type == cJSON_Number) {
			*file_size = item->valueint;
		}

		item = cJSON_GetObjectItem(version_json, "fileName");
		if (item && item->type == cJSON_String) {
			snprintf(firmware_name, FIRMWARE_NAME_LEN_MAX, "%s", item->valuestring);
		}
		item = cJSON_GetObjectItem(version_json, "softwareId");
		if (item && item->type == cJSON_Number) {
			*software_id = item->valueint;
		}

		item = cJSON_GetObjectItem(version_json, "updateType");
		if (item && item->type == cJSON_Number) {
			*update_type = item->valueint;
		}

		ret = 0;

		break;

		version_json = version_json->next;
	}

exit:
	return ret;
}

static int __fw_update_wipap(int session_id, char* pc_project_name, U8 update, U8 auto_reboot, U8 update_source, char* filename_to_ramdisk, char* filename)
{
	char  bin_file[256];
	U8	buffer[512];
	U16   image_name_len = 0;
	FILE* fp			 = NULL;
	int   ret			 = -1;

	DEBUGP("%s:%d update: %d, auto_reboot: %d, filename: %s\n", __func__, __LINE__, update, auto_reboot, filename);

	int status = __fw_update_desap_copy_image(filename);
	if (0 != status) {
		goto exit;
	}
	current_update_status.percentage = 75;

	snprintf(bin_file, sizeof(bin_file), "/%s", filename);
	image_name_len = strlen(bin_file);
	if (image_name_len >= 256) {
		goto exit;
	}

	system("rm -fr /usr/tmp/tmp.bin");
	system("mkdir -p /usr/tmp");

	fp = fopen("/usr/tmp/tmp.bin", "w");
	if (NULL == fp) {
		goto exit;
	}

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = update;		   /* update flag */
	buffer[1] = auto_reboot;   /* auto reboot flag */
	buffer[2] = update_source; /* update source flag */
	buffer[3] = (image_name_len >> 8) & 0x00ff;
	buffer[4] = image_name_len & 0x00ff;

	memcpy(buffer + 5, bin_file, image_name_len);

	DEBUGP("%s:%d buf0: %0x, buf1: %0x, buf2: %0x, buf3: %0x, buf4: %0x, "
		   "bin_file: %s, len: %d\n",
		   __func__, __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], bin_file, image_name_len);

	fwrite(buffer, 512, 1, fp);
	fclose(fp);

	system("flash_erase /dev/mtd6 0 0");
	system("nandwrite /dev/mtd6 -p /usr/tmp/tmp.bin");

	if (NULL != pc_project_name && (NULL != strstr(pc_project_name, "HGI16") || NULL != strstr(pc_project_name, "HGM53"))) {
		// fnMcuRebootCPU();
	}

	ret = 0;
exit:
	if (ret != 0) {
		current_update_status.result = 1;
	}
	current_update_status.status = UPFW_UPDATE_STATUS_FINISHED;
	current_update_status.step   = UPFW_UPDATE_STEP_FINISHED;

	if (ret == 0) {
		system("reboot");
	}

	return ret;
}

static int __fw_update_cctv(char* filename)
{
	return 0;
}

static int __fw_update_ac(int session_id, int software_id, char* wipap, char* sn, char* filename)
{
	cJSON* root		= NULL;
	char*  response = NULL;
	U32	len;
	char   file_path[512];
	int	ret = -1;

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	if (NULL == wipap || sn == NULL || filename == NULL) {
		goto exit;
	}

	root = cJSON_CreateObject();
	if (NULL == root) {
		goto exit;
	}

	snprintf(file_path, sizeof(file_path), "%s/%s", config.update_dir, filename);

	cJSON_AddStringToObject(root, "url", file_path);
	cJSON_AddNumberToObject(root, "softwareId", software_id);
	cJSON_AddNumberToObject(root, "sessionId", session_id);
	cJSON_AddStringToObject(root, "serialNumber", sn);
	cJSON_AddStringToObject(root, "wipap", wipap);

	len = ls_generate_msg(UPFW_METHOD_NOTIFY, root, &response);

	char* string = cJSON_Print(root);
	if (string) {
		printf("%s:%d response: %s\n", __func__, __LINE__, string);
		free(string);
	}

	remotesocket_sendmsg(SOCK_AC_ID, SOCK_UPFW_ID, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) response, len);
	if (response) {
		free(response);
	}

	cJSON_Delete(root);

	DEBUGP("%s:%d end\n", __func__, __LINE__);

	ret = 0;
exit:
	return ret;
}

static int __fw_update_write_update_flag(char* filename, char* update_state)
{
	FILE* fp = NULL;

	fp = fopen(filename, "w");
	if (fp != NULL) {
		fprintf(fp, "%s\n", update_state);
		fclose(fp);
		return 0;
	}

	return -1;
}

#define PATH_UPFW_PROC_CONFIG_INI "/home/wipap/upfw/upfw.ini"
#define PATH_UPFW_PROC_NAME_KEY "proc_name"
#define PATH_UPFW_PROC_DIR_KEY "proc_dir"
#define PATH_UPFW_PROC_PARAMS_KEY "proc_params"
#define PROC_UPDATE_SUFFIX ".updating"

static int __fw_update_proc(int software_id, int session_id, char* filename)
{
	char cmd[512];
	int  ret = -1;
	int  status;
	char software_section[32] = {0};
	char g_proc_dir[128]	  = {0};
	char g_proc_name[64]	  = {0};
	char g_proc_params[64]	= {0};

	if (NULL == strstr(filename, ".zip")) {
		goto exit;
	}

	snprintf(software_section, 32, "softwareId_%d", software_id);
	ret = GetValueFromEtcFile(PATH_UPFW_PROC_CONFIG_INI, software_section, PATH_UPFW_PROC_DIR_KEY, g_proc_dir, sizeof(g_proc_dir) - 1);
	if (0 != ret) {
		goto exit;
	}
	ret = GetValueFromEtcFile(PATH_UPFW_PROC_CONFIG_INI, software_section, PATH_UPFW_PROC_NAME_KEY, g_proc_name, sizeof(g_proc_name) - 1);
	if (0 != ret) {
		goto exit;
	}
	DEBUGP("%s:%d begin to update backend.\n", __func__, __LINE__);

	watchdog_client_pause(SOCK_UPFW_ID, SOCK_WATCHDOG_ID, g_proc_name);

	snprintf(cmd, 512, "killall %s", g_proc_name);
	status = system(cmd);
	status = firmware_get_system_return_value(status);

	chdir(WIPAP_DIR);
	snprintf(cmd, 512, "tar zcf %s.tar.bz2 %s", g_proc_name, g_proc_name);
	status = system(cmd);
	status = firmware_get_system_return_value(status);

	snprintf(cmd, 512, "%s/%s%s", g_proc_dir, g_proc_name, PROC_UPDATE_SUFFIX);
	__fw_update_write_update_flag(cmd, "updating");

	snprintf(cmd, 512, "cd %s && rm -fr `ls | egrep -v %s%s`", g_proc_dir, g_proc_name, PROC_UPDATE_SUFFIX);
	status = system(cmd);
	status = firmware_get_system_return_value(status);

	snprintf(cmd, 512, "unzip %s/%s -d %s", config.update_dir, filename, WIPAP_DIR);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (status != 0) {
		orangelog_log_error("%s:%d unzip error.\n", __func__, __LINE__);
		snprintf(cmd, 512, "cd %s && rm -fr `ls | egrep -v backend.status`", g_proc_dir);
		status = system(cmd);
		status = firmware_get_system_return_value(status);
		snprintf(cmd, 512, "tar xf %s/%s.tar.bz2 -C %s", WIPAP_DIR, g_proc_name, WIPAP_DIR);
		status = system(cmd);
		status = firmware_get_system_return_value(status);
		if (status == 0) {
			snprintf(cmd, 512, "rm -fr %s/%s%s", g_proc_dir, g_proc_name, PROC_UPDATE_SUFFIX);
			status = system(cmd);
		}
	}

	snprintf(cmd, 512, "chmod 755 %s/%s", g_proc_dir, g_proc_name);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (0 != status) {
		orangelog_log_error("%s:%d start error.\n", __func__, __LINE__);
	}

	GetValueFromEtcFile(PATH_UPFW_PROC_CONFIG_INI, software_section, PATH_UPFW_PROC_PARAMS_KEY, g_proc_params, sizeof(g_proc_params) - 1);
	snprintf(cmd, 512, "cd %s && ./%s %s &", g_proc_dir, g_proc_name, g_proc_params);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (0 != status) {
		orangelog_log_error("%s:%d start error.\n", __func__, __LINE__);
	}

	DEBUGP("%s:%d.\n", __func__, __LINE__);
	snprintf(cmd, 512, "ps | grep \"./%s\" | grep -v grep", g_proc_name);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (0 == status) {
		snprintf(cmd, 512, "rm -fr %s/%s.tar.bz2", WIPAP_DIR, g_proc_name);
		status = system(cmd);
		status = firmware_get_system_return_value(status);
		snprintf(cmd, 512, "rm -fr %s/%s%s", g_proc_dir, g_proc_name, PROC_UPDATE_SUFFIX);
		status = system(cmd);
		orangelog_log_info("%s update successful.\n", g_proc_name);
	}

	watchdog_client_resume(SOCK_UPFW_ID, SOCK_WATCHDOG_ID, g_proc_name);

	DEBUGP("%s:%d.\n", __func__, __LINE__);

	ret = 0;
exit:
	snprintf(cmd, 512, "rm -fr %s/%s", config.update_dir, filename);
	status = system(cmd);
	if (ret != 0) {
		current_update_status.result = 1;
	}
	DEBUGP("%s:%d begin to update backend.\n", __func__, __LINE__);
	current_update_status.status = UPFW_UPDATE_STATUS_FINISHED;
	current_update_status.step   = UPFW_UPDATE_STEP_FINISHED;
	return ret;
}

static int __fw_update_frontend(int session_id, char* filename)
{
	char cmd[512];
	int  ret	= -1;
	int  status = 0;

	orangelog_log_info("start updating frontend.\n");

	if (NULL == strstr(filename, ".zip")) {
		orangelog_log_info("frontend filename: %s error.\n", filename);
		goto exit;
	}

	chdir(WIPAP_DIR);
	snprintf(cmd, 512, "tar zcf frontend.tar.bz2 frontend");
	status = system(cmd);
	status = firmware_get_system_return_value(status);

	snprintf(cmd, 512, "rm -fr %s", FRONTEND_DIR);
	status = system(cmd);
	status = firmware_get_system_return_value(status);

	snprintf(cmd, sizeof(cmd), "unzip %s/%s -d %s", config.update_dir, filename, WIPAP_DIR);

	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (status != 0) {
		snprintf(cmd, 512, "tar xf %s/frontend.tar.bz2 -C %s", WIPAP_DIR, WIPAP_DIR);
		status = system(cmd);
		status = firmware_get_system_return_value(status);
		orangelog_log_info("update frontend error\n");
	} else {
		orangelog_log_info("update frontend suc\n");
	}

	ret = 0;
exit:

	snprintf(cmd, 512, "rm -fr %s/frontend.tar.bz2", WIPAP_DIR);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	snprintf(cmd, 512, "rm -fr %s/%s", config.update_dir, filename);
	status = system(cmd);
	status = firmware_get_system_return_value(status);
	if (ret != 0) {
		current_update_status.result = 1;
	}
	current_update_status.status = UPFW_UPDATE_STATUS_FINISHED;
	current_update_status.step   = UPFW_UPDATE_STEP_FINISHED;
	return ret;
}

static int __fw_get_file_size(char* file_name)
{
	FILE* fp		= NULL;
	char  buf[1024] = {0};
	int   size		= -1;

	snprintf(buf, 1024, "stat %s | grep Size", file_name);

	fp = popen(buf, "r");
	if (fp) {
		memset(buf, 0, 1024);
		fread(buf, 1024, 1, fp);
		char* size_p = strstr(buf, "Size:");
		if (size_p) {
			size_p += strlen("Size:");
			if (*size_p == ' ') {
				size_p++;
			}
			size = atoi(size_p);
		}
		pclose(fp);
	}
	return size;
}

static void __fw_calculate_file_download_speed(void)
{
	return;
	if (current_update_status.step != UPFW_UPDATE_STEP_DOWNLOADING) {
		return;
	}

	int current_size = __fw_get_file_size(current_update_status.file_name);
	if (current_size > 0) {
		current_update_status.percentage = (current_size / current_update_status.file_size) * 50;
	}

	return;
}

static int __fw_update_firmware_process(int session_id, int software_id, char* wipap, char* sn, char* filename)
{
	int ret = -1;

	if ((wipap != NULL) && (sn != NULL) && (strcmp(wipap, sn) == 0)) {
		switch (software_id) {
			case SOFTWARE_ID_ALL:
				break;
			case SOFTWARE_ID_WIPAP:
				ret = __fw_update_wipap(session_id, NULL, 1, 1, 1, NULL, filename);
				break;
			case SOFTWARE_ID_FRONTEND:
				ret = __fw_update_frontend(session_id, filename);
				break;
			case SOFTWARE_ID_AC_ROOTER:
			case SOFTWARE_ID_AC_NODE:
				ret = __fw_update_ac(session_id, software_id, wipap, sn, filename);
				break;
			case SOFTWARE_ID_CCTV:
				ret = __fw_update_cctv(filename);
				break;
			default:
				__fw_update_proc(software_id, session_id, filename);
				break;
		}
	} else {
	}

	return ret;
}

static int __fw_update_cmp_suffix(char* string, char* suffix)
{
	char* p_string = NULL;
	char* p_suffix = NULL;
	int   len1	 = strlen(string);
	int   len2	 = strlen(suffix);

	if (len1 < len2) {
		return -1;
	}

	DEBUGP("%s:%d src: %s, suffix: %s\n", __func__, __LINE__, string, suffix);

	p_string = string + len1 - 1;
	p_suffix = suffix + len2 - 1;

	while (1) {
		if (*p_suffix != *p_string) {
			return -1;
		}
		if (p_suffix == suffix) {
			break;
		}
		p_string--;
		p_suffix--;
	}

	return 0;
}

static int __fw_update_local_upload_get_file_name(char* dir, char* xml, char* xml_sign, char* image, char* image_sign)
{
	DIR*		   p_dir;
	struct dirent* d;

	p_dir = opendir(dir);
	if (!p_dir) {
		return -1;
	}

	while ((d = readdir(p_dir)) != NULL) {
		if (*(d->d_name) == '.') {
			continue;
		}
		DEBUGP("%s:%d d_name: %s\n", __func__, __LINE__, d->d_name);

		if (0 == __fw_update_cmp_suffix(d->d_name, ".xml")) {
			sprintf(xml, "%s", d->d_name);
		} else if (0 == __fw_update_cmp_suffix(d->d_name, ".xml.sign")) {
			sprintf(xml_sign, "%s", d->d_name);
		} else if (0 == __fw_update_cmp_suffix(d->d_name, ".img.sign") || 0 == __fw_update_cmp_suffix(d->d_name, ".zip.sign")) {
			sprintf(image_sign, "%s", d->d_name);
		} else if (0 == __fw_update_cmp_suffix(d->d_name, ".img") || 0 == __fw_update_cmp_suffix(d->d_name, ".zip")) {
			sprintf(image, "%s", d->d_name);
		}
	}

	DEBUGP("%s:%d xmlsign: %s, xml: %s, imgsign: %s, image: %s\n", __func__, __LINE__, xml_sign, xml, image_sign, image);
	if (*xml == '\0' || *image_sign == '\0' || *image == '\0') {
		return -1;
	}

	closedir(p_dir);

	return 0;
}

static int __fw_udpate_local_check(char* xml, char* sign)
{
	char dir[512];

	snprintf(dir, 512, "%s/tmp", config.update_dir);

	return firmware_check_signature_ex(dir, xml, sign);
}

/*
static int __fw_udpate_local_cmp_md5(char* md5, char* filename)
{
	FILE* fp		= NULL;
	char  cmd[1024] = {0};
	int   ret		= -1;

	snprintf(cmd, 1024, "md5sum %s/tmp/%s", config.update_dir, filename);

	fp = popen(cmd, "r");
	if (fp) {
		fread(cmd, 1024, 1, fp);
		if (0 == strcmp(cmd, md5)) {
			ret = 0;
		}
		fclose(fp);
	}

	return ret;
}
*/

static int __fw_update_local_pase_xml(char* xml, char* image, char* sn, int* software_id, char* image_name)
{
	int  ret = -1;
	char file_name[512];

	snprintf(file_name, 512, "%s/tmp/%s", config.update_dir, xml);
	assert(file_name);

	xmlDocPtr  doc;
	xmlNodePtr cur;
	xmlChar*   attribute;

	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	doc = xmlParseFile(file_name);
	if (doc == NULL) {
		orangelog_log_error("%s:%d xmlParseFile failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		orangelog_log_error("%s:%d xmlDocGetRootElement failed\n", __func__, __LINE__);
		goto exit;
	}

	if ((xmlStrcmp(cur->name, (const xmlChar*) "updates"))) {
		orangelog_log_error("%s:%d xmlStrcmp failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "update"))) {
			attribute = xmlGetProp(cur, (const xmlChar*) "serialMask");
			if (attribute) {
				if (0 != strncmp((char*) attribute, sn, 3)) {
					continue;
				}
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "fileName");
			if (attribute) {
				snprintf(image_name, FIRMWARE_NAME_LEN_MAX, "%s", attribute);
			}

			/*
			attribute = xmlGetProp(cur, (const xmlChar*) "fileMD5");
			if (attribute) {
				if (0 != __fw_udpate_local_cmp_md5((char*) attribute, image)) {
					continue;
				}
			}
			*/

			attribute = xmlGetProp(cur, (const xmlChar*) "softwareId");
			if (attribute) {
				*software_id = atoi((char*) attribute);
				ret			 = 0;
				break;
			}
		}
		cur = cur->next;
	}

exit:
	if (*image_name == '\0') {
		ret = -1;
	}
	DEBUGP("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	if (doc) {
		xmlFreeDoc(doc);
	}
	return ret;
}

static int __fw_update_local_upload_firmware(char* url, char* sn, int* software_id, char* filename)
{
	int  ret = -1;
	char unzip_dir[512];
	char cmd[1024];
	char xml[512];
	char xml_sign[512];
	char image[512];
	char image_sign[512];

	DEBUGP("%s:%d \n", __func__, __LINE__);
	if (url == NULL || software_id == NULL || sn == NULL || filename == NULL) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	snprintf(unzip_dir, 512, "%s/tmp", config.update_dir);
	ret = __fw_make_mount_dir(unzip_dir);
	if (ret != 0) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}
	__fw_clean_update_partion(unzip_dir);

	snprintf(cmd, 1024, "unzip %s/%s -d %s", config.update_dir, url, unzip_dir);
	ret = system(cmd);
	ret = firmware_get_system_return_value(ret);
	if (ret != 0) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	ret = __fw_update_local_upload_get_file_name(unzip_dir, xml, xml_sign, image, image_sign);
	if (ret != 0) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	ret = __fw_udpate_local_check(xml, xml_sign);
	if (ret != 0) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	ret = __fw_udpate_local_check(image, image_sign);
	if (ret != 0) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	ret = __fw_update_local_pase_xml(xml, image, sn, software_id, filename);
	if (ret != 0) {
		orangelog_log_error("%s:%d pars xml error\n", __func__, __LINE__);
	}

	if (ret == 0) {
		snprintf(cmd, 1024, "mv %s/%s %s", unzip_dir, filename, config.update_dir);
		ret = system(cmd);
		ret = firmware_get_system_return_value(ret);
	}

exit:
	snprintf(cmd, 1024, "rm -fr %s/%s", config.update_dir, url);
	system(cmd);
	snprintf(cmd, 1024, "rm -fr %s/tmp/%s", config.update_dir, xml);
	system(cmd);
	snprintf(cmd, 1024, "rm -fr %s/tmp/%s", config.update_dir, xml_sign);
	system(cmd);
	snprintf(cmd, 1024, "rm -fr %s/tmp/%s", config.update_dir, image_sign);
	system(cmd);
	DEBUGP("%s:%d softwareid: %d\n", __func__, __LINE__, *software_id);
	return ret;
}

static int __fw_update_parse_request_item(cJSON* data)
{
    int ret = -1;
    int index = 0;
    cJSON* item = NULL;
    char   serial_mask[SERIAL_MASK_LEN_MAX]			= "";
    char   version[VERSION_LEN_MAX]					= "";
    char   file_path[256]							= "";
    char   serial_number_prefix[8]					= "";
    struct upfw_update_entry* entry = NULL;

    if (NULL == data) {
        return -1;
    }

    entry = __upfw_update_entry_alloc();
    if (NULL == entry) {
        return -1;
    }

	item = cJSON_GetObjectItem(data, "serialNumber");
	if (item != NULL && item->valuestring != NULL) {
        memcpy(entry->serial_number, item->valuestring, MIN(strlen(item->valuestring), SERIAL_NUMBER_LEN_MAX));
	}

    item = cJSON_GetObjectItem(data, "i");
    if (NULL == item || item->type != cJSON_Number) {
        goto exit;
    }

    index = item->valueint;

    item = cJSON_GetObjectItem(data, "version");
    if (NULL == item || item->type != cJSON_String) {
        goto exit;
    }

    snprintf(version, sizeof(version), "%s", item->valuestring);

    memcpy(serial_number_prefix, entry->serial_number, 4);
    serial_number_prefix[4] = '\0';
    snprintf(serial_mask, sizeof(serial_mask), "%sFFFFFFFF", serial_number_prefix);

    snprintf(file_path, sizeof(file_path), "%s/%s_%s", config.update_dir, serial_mask, UPFW_FILE_SUFFIX);
    ret = __fw_get_firmware_info(index, version, serial_mask, file_path, entry->filename, &(entry->software_id), &(entry->filesize), &(entry->update_type)); 
    if (0 != ret) {
        goto exit;
    }

    entry->session_id = (++g_upfw_update.g_session_id);
    RB_INSERT(upfw_update_tree, &g_upfw_update.upfw_rb_tree, entry);
	TAILQ_INSERT_TAIL(&g_upfw_update.upfw_list_head, entry, list);

    ret = 0;
exit:
    if (0 != ret) {
        __upfw_update_entry_free(entry);
    }
    return ret;
}

static int __fw_update_parse_request(char *attributes, int *type) 
{
    int ret         = -1;
	cJSON* data		= NULL;
	cJSON* root		    = NULL;
	cJSON* root_item		= NULL;
	cJSON* item		= NULL;

	if (attributes == NULL) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	orangelog_log_error("%s:%d attributes: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (data == NULL) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	item = cJSON_GetObjectItem(data, "type");
	if ((NULL != item) && (item->type == cJSON_Number)) {
        *type = item->valueint;
    } else {
        *type = 1;
    }

    if (*type == 0) {
        ret = 0;
        goto exit;
    }

	root = cJSON_GetObjectItem(data, "data");
	if (NULL == root) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

    root_item = root->child;

    while(root_item) {
        ret = __fw_update_parse_request_item(root_item);
        root_item = root_item->next;
    }

exit:
	return ret;
}

static void __fw_update_remote_update_process_entry(struct upfw_update_entry* entry) 
{
    int available_size = 0;

    if (entry->update_type == 0 || entry->update_type == 1) {
		ret				   = __fw_get_update_dir_available_space(config.update_dir, &available_size);
		if (entry->filesize >= available_size || ret != 0) {
			goto exit;
		}

		ret = fimware_download_file_by_curl(config.update_url, config.update_dir, entry->filename, NULL, &(current_update_status.percentage));
		if (ret != 0) {
			goto exit;
		}

    }

exit:

    return;
}

static void __fw_update_remote_update_process(STRThreadParamType* args, U32 threadid, U32 semid)
{
    struct upfw_update_entry* entry = NULL;
    struct upfw_update_entry* tmp = NULL;

	TAILQ_FOREACH_SAFE(entry, &(g_upfw_update.upfw_list_head), list, tmp) {
        __fw_update_remote_update_process_entry(entry);
    }

    return;
}

static void __fw_update_local_upload_process(STRThreadParamType* args, U32 threadid, U32 semid)
{
	char* attributes	   = (char*) args->params;
	cJSON* data										= NULL;
	cJSON* root										= NULL;
	cJSON* item										= NULL;
	char   firmware_filename[FIRMWARE_NAME_LEN_MAX] = "";
	int	software_id								= -1;
	int	ret										= -1;
    int session_id = (++(g_upfw_update.g_session_id));
	char *wipap = NULL, *sn = NULL;

	if (attributes == NULL) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	orangelog_log_error("%s:%d attributes: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (data == NULL) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	item  = cJSON_GetObjectItem(data, "wipap");
	if (item != NULL) {
		wipap = item->valuestring;
	}

	root = cJSON_GetObjectItem(data, "data");
	if (NULL == root) {
		orangelog_log_error("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	item = cJSON_GetObjectItem(root, "serialNumber");
	if (item != NULL) {
		sn = item->valuestring;
		orangelog_log_error("%s:%d sn: %s\n", __func__, __LINE__, sn);
	}

    item = cJSON_GetObjectItem(root, "url");
    if ((NULL == item) || (item->valuestring == NULL)) {
        goto exit;
    }
    ret = __fw_update_local_upload_firmware(item->valuestring, sn, &software_id, firmware_filename);
    if (ret != 0) {
        orangelog_log_error("%s:%d \n", __func__, __LINE__);
        goto exit;
    }

    ret = __fw_update_firmware_process(session_id, software_id, wipap, sn, firmware_filename);

exit:
	return ;
}

static void __fw_query_firmware_version(STRThreadParamType* args, U32 threadid, U32 semid)
{
	cJSON* data		   = NULL;
	cJSON* item		   = NULL;
	cJSON* device_type = NULL;
	cJSON* out		   = NULL;

	char* language		   = NULL;
	char* sm			   = NULL;
	char* attributes	   = (char*) args->params;
	char* msg			   = NULL;
	char* signature_suffix = ".sign";

	char xml_filename[256];
	char xml_filename_path[256];
	char frontend_json_filename_path[256];
	char upfw_json_filename_path[256];
	int  len;
	int  ret;
	int  software_id;

	current_query_status.status = UPFW_QUERY_STATUS_RUNNING;

	DEBUGP("%s:%d begin json: %s\n", __func__, __LINE__, attributes);
	DEBUGP("%s:%d begin json: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (NULL == data) {
		goto exit;
	}

	item = cJSON_GetObjectItem(data, "language");
	if (NULL == item || item->type != cJSON_String) {
		goto exit;
	}
	language = item->valuestring;

	item = cJSON_GetObjectItem(data, "deviceType");
	if (NULL == item || item->type != cJSON_Array) {
		goto exit;
	}

	device_type = item->child;
	if (device_type == NULL) {
		goto exit;
	}

	out = cJSON_CreateArray();
	if (NULL == out) {
		goto exit;
	}

	while (device_type) {
		item = cJSON_GetObjectItem(device_type, "deviceTypeId");
		if (NULL == item || item->type != cJSON_Number) {
			device_type = device_type->next;
			continue;
		}
		software_id = item->valueint;

		item = cJSON_GetObjectItem(device_type, "serialMask");
		if (NULL == item || item->type != cJSON_String) {
			device_type = device_type->next;
			continue;
		}

		sm = item->valuestring;

		snprintf(xml_filename, sizeof(xml_filename), "%s.xml", sm);

		snprintf(xml_filename_path, sizeof(xml_filename_path), "%s/%s.xml", config.update_dir, sm);

		snprintf(frontend_json_filename_path, sizeof(frontend_json_filename_path), "%s/%s_%s", DEFALT_JSON_FILE_DIR, sm, FRONTEND_FILE_SUFFIX);

		snprintf(upfw_json_filename_path, sizeof(upfw_json_filename_path), "%s/%s_%s", config.update_dir, sm, UPFW_FILE_SUFFIX);

		ret = fimware_download_file_by_curl(config.update_url, config.update_dir, xml_filename, NULL, &(current_query_status.percentage));
		if (ret != 0) {
			device_type = device_type->next;
			continue;
		}

		ret = fimware_download_file_by_curl(config.update_url, config.update_dir, xml_filename, signature_suffix, &(current_query_status.percentage));
		if (ret != 0) {
			device_type = device_type->next;
			continue;
		}

		ret = firmware_check_signature(config.update_dir, xml_filename, signature_suffix);
		if (ret != 0) {
			device_type = device_type->next;
			continue;
		}

		xml_to_json(NULL, &upfw_key, software_id, sm, xml_filename_path, upfw_json_filename_path);

		xml_to_json(language, &frontend_key, software_id, sm, xml_filename_path, frontend_json_filename_path);

		item = cJSON_CreateObject();
		if (NULL == item) {
			break;
		}

		__fw_get_upgradeable_device_info(software_id, sm, item);
		cJSON_AddItemToArray(out, item);
		firmware_remove_file(config.update_dir, xml_filename, signature_suffix);
		device_type = device_type->next;
	}

	len = ls_generate_msg("updateFirmwareInfo", out, &msg);
	if (len <= 0) {
		goto exit;
	}
	remotesocket_sendmsg(SOCK_BACKEND_ID, SOCK_UPFW_ID, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, len);

exit:
	if (msg) {
		free(msg);
	}

	if (out) {
		cJSON_Delete(out);
	}
	if (data) {
		cJSON_Delete(data);
	}
	current_query_status.status = UPFW_QUERY_STATUS_IDLE;
	return;
}

static void __fw_query_update_status(int session_id)
{
	char*  msg  = NULL;
	cJSON* root = cJSON_CreateObject();
	if (NULL == root) {
		goto exit;
	}
	cJSON_AddNumberToObject(root, "sessionId", session_id);

	int len = ls_generate_msg(UPFW_METHOD_QUERY, root, &msg);
	if (len <= 0) {
		goto exit;
	}

	remotesocket_sendmsg(SOCK_AC_ID, SOCK_UPFW_ID, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, len);
exit:
	if (root) {
		cJSON_Delete(root);
	}
	if (msg) {
		free(msg);
	}
	return;
}

static void __fw_send_update_status(void)
{
	cJSON* root		= NULL;
	char*  response = NULL;
	U32	len;

	root = cJSON_CreateObject();
	if (NULL == root) {
		return;
	}

	cJSON_AddNumberToObject(root, "sessionId", current_update_status.session_id);
	cJSON_AddNumberToObject(root, "status", current_update_status.status);
	cJSON_AddNumberToObject(root, "result", current_update_status.result);
	cJSON_AddNumberToObject(root, "percentage", current_update_status.percentage);

	len = ls_generate_msg(UPFW_METHOD_NOTIFY, root, &response);

	remotesocket_sendmsg(SOCK_BACKEND_ID, SOCK_UPFW_ID, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) response, len);
	if (response) {
		free(response);
	}

	cJSON_Delete(root);
}

static void fw_query_firmware_version(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	if (current_query_status.status != UPFW_QUERY_STATUS_IDLE) {
		return;
	}

	char* attributes = ls_getParamString(params, 0);

	thread_request(EVENT_TYPE_REMOTESOCKET, attributes, strlen(attributes), 50, __fw_query_firmware_version);
	return;
}

static void fw_update_firmware(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
    int ret = -1;
    int type = 0;
	orangelog_log_info("%s:%d start update firmware.\n", __func__, __LINE__);

    if (g_upfw_update.is_updating) {
        return;
    }

    g_upfw_update.is_updating = 1;

	char* attributes = ls_getParamString(params, 0);
    ret = __fw_update_parse_request(attributes, &type);
    if (0 == ret) {
        if (0 == type) {
            thread_request(EVENT_TYPE_REMOTESOCKET, attributes, strlen(attributes), 50, __fw_update_local_upload_process);
        }
        else {
            thread_request(EVENT_TYPE_REMOTESOCKET, attributes, strlen(attributes), 50, __fw_update_remote_update_process);
        }
    }

	return;
}

static void fw_notify_upfw(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	return;
}

static void fw_query_upfw(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	char*  attributes = ls_getParamString(params, 0);
	cJSON* data		  = NULL;
	cJSON* item		  = NULL;
	int	session_id = -1;

	if (attributes == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}
	DEBUGP("%s:%d attributes: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (data == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	item = cJSON_GetObjectItem(data, "sessionId");
	if (item && item->type == cJSON_Number) {
		session_id = item->valueint;
	}

	if (session_id == current_update_status.session_id && current_update_status.status != UPFW_UPDATE_STATUS_IDLE) {
		current_update_status.need_send = 1;
	}

exit:
	if (data) {
		cJSON_Delete(data);
	}
	return;
}

static void fw_notify_upfw_ack(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	char* attributes = ls_getParamString(params, 0);

	cJSON* data		  = NULL;
	cJSON* item		  = NULL;
	cJSON* root		  = NULL;
	char*  msg		  = NULL;
	int	session_id = 0;
	int	result	 = -1;

	if (attributes == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}
	DEBUGP("%s:%d attributes: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (data == NULL) {
		printf("%s:%d \n", __func__, __LINE__);
		goto exit;
	}

	root = cJSON_CreateObject();
	if (NULL == root) {
		goto exit;
	}

	item = cJSON_GetObjectItem(data, "result");
	if (item && item->type == cJSON_Number) {
		result = item->valueint;
	}

	item = cJSON_GetObjectItem(data, "sessionId");
	if (item && item->type == cJSON_Number) {
		session_id = item->valueint;
	}

	if (result != 0) {
		goto exit;
	}

	cJSON_AddNumberToObject(root, "sessionId", session_id);

	int len = ls_generate_msg(UPFW_METHOD_QUERY, root, &msg);
	if (len <= 0) {
		goto exit;
	}

	remotesocket_sendmsg(SOCK_AC_ID, SOCK_UPFW_ID, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, len);
exit:
	if (data) {
		cJSON_Delete(data);
	}
	if (root) {
		cJSON_Delete(root);
	}
	if (msg) {
		free(msg);
	}
	return;
}

static void fw_query_upfw_ack(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	char*  attributes = ls_getParamString(params, 0);
	cJSON* data		  = NULL;
	cJSON* item		  = NULL;

	DEBUGP("%s:%d attributes: %s\n", __func__, __LINE__, attributes);

	data = cJSON_Parse(attributes);
	if (data == NULL) {
		goto exit;
	}

	item = cJSON_GetObjectItem(data, "sessionId");
	if (item != NULL && item->type == cJSON_Number) {
		current_update_status.session_id = item->valueint;
	}

	item = cJSON_GetObjectItem(data, "status");
	if (item != NULL && item->type == cJSON_Number) {
		current_update_status.status = item->valueint;
	}

	item = cJSON_GetObjectItem(data, "percentage");
	if (item != NULL && item->type == cJSON_Number) {
		current_update_status.percentage = 50 + item->valueint / 2;
		DEBUGP("percentag: %d\n", item->valueint);
	}

	item = cJSON_GetObjectItem(data, "result");
	if (item != NULL && item->type == cJSON_Number) {
		current_update_status.result = item->valueint;
	}

	DEBUGP("%s:%d session_id: %d, result: %d, staus: %d, percentag: %d\n", __func__, __LINE__, current_update_status.session_id, current_update_status.result,
		   current_update_status.status, current_update_status.percentage);

	if (current_update_status.status == UPFW_UPDATE_STATUS_RUNNING) {
		current_update_status.need_query = 1;
	}
	if (current_update_status.status == UPFW_UPDATE_STATUS_FINISHED) {
		current_update_status.need_query = 0;
		current_update_status.step		 = UPFW_UPDATE_STEP_FINISHED;
	}

exit:
	if (data) {
		cJSON_Delete(data);
	}
	return;
}

static void fw_query_update_progress(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
    return;
}

static void fm_init(void)
{
	ls_init(fm_appRPCFun, 6);

    g_upfw_update.g_session_id = 0;
    g_upfw_update.is_updating = 0;
	RB_INIT(&g_upfw_update.upfw_rb_tree);
    TAILQ_INIT(&g_upfw_update.upfw_list_head);

	IniGlobalInit(1);
	LoadIniData(PATH_UPFW_PROC_CONFIG_INI);
	return;
}

static void fm_uninit(void)
{
	return;
}

static void fm_realtime(U32 semid, U32 moduleid, STREventElemType* event)
{
	U64 current_timestamp = time(NULL);
	watchdog_client_feed(SOCK_UPFW_ID, SOCK_WATCHDOG_ID, "upfw");

	if (NULL != event) {
		event_clearflags(semid, event, moduleid);
	}

#if 0
	int  i;
	char xxxx[1024] =
		"Here you can find the help you need when configuring or installing Prosody. If you can't find the answer to your question, try the search box at the top of the page. If you are still stuck, come and ask us for help!";

	for (i = 0; i < 16; i++) {
		upfw_send_log("%s:%d i: %d, semid: %u, b: %u, buf: %s\n", __func__, __LINE__, i, semid, b, xxxx);
	}
#endif

	static int test_ac = 1;
	if (0 == test_ac) {
		__fw_update_ac(1, 4, "ABB7FFFFFFFF", "ABB7FFFFFFFF", "saltofirmw_0149_0010.txt");
		test_ac = 1;
	}

	static int test_query_version = 0;
	if (test_query_version) {
		STRThreadParamType query_version;

		query_version.params = strdup("{\"language\":\"en\", \"deviceType\":[{\"deviceTypeId\":1, \"serialMask\": \"ABB7FFFFFFFF\"}]}");
		__fw_query_firmware_version(&query_version, 0, 0);
		free(query_version.params);

		test_query_version = 0;
	}

	if (current_timestamp % 20 == 0) {
		__fw_check_and_write_available_size();
	}

	if (current_update_status.need_query == 1) {
		__fw_query_update_status(current_update_status.session_id);
		current_update_status.need_query = 0;
	}

	if (current_update_status.timestamp != 0 && (current_update_status.timestamp + UPFW_UPDATE_TIMEOUT < current_timestamp)) {
		current_update_status.status	= UPFW_UPDATE_STATUS_IDLE;
		current_update_status.step		= UPFW_UPDATE_STEP_NONE;
		current_update_status.timestamp = 0;
	}

	if (current_update_status.status > UPFW_UPDATE_STATUS_IDLE && current_update_status.status <= UPFW_UPDATE_STATUS_FINISHED) {
		__fw_calculate_file_download_speed();

		if (current_update_status.need_send == 1) {
			__fw_send_update_status();
			current_update_status.need_send = 0;
		}

		if (current_update_status.status == UPFW_UPDATE_STATUS_FINISHED) {
			current_update_status.status	= UPFW_UPDATE_STATUS_IDLE;
			current_update_status.step		= UPFW_UPDATE_STEP_NONE;
			current_update_status.timestamp = 0;
		}
		__fm_setperiod(500);
	} else {
		__fm_setperiod(1000);
	}

	return;
}

static void fm_clear(void)
{
	return;
}

static void fm_setlocaltime(U64 time)
{
	return;
}

static U32 fm_getperiod(void)
{
	return (localperiod);
}

MODULE_INFO(MD_UPFM_NAME, MD_UPFM_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0), fm_init, fm_realtime, fm_clear, fm_uninit, fm_getperiod,
			fm_setlocaltime);
