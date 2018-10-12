#include "firmware.h"
#include "curl.h"
#include "json.h"
#include "module.h"
#include "orangelog.h"
#include "remotesocket.h"
#include "xml.h"

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

static BOOL firmware_send_msg(U32 msgid, char* msg, int length)
{
	char*			 response;
	BOOL			 res;
	U32				 llen;
	MSG_DATA_HEADER* h;

	llen		   = (length + MSG_HEADER_LENTH);
	response	   = (char*) malloc(llen);
	h			   = (MSG_DATA_HEADER*) response;
	h->reserved[0] = 0;
	h->reserved[1] = 0;
	memcpy(h->srcID, SOCK_UPFW_ID, MSG_ID_SIZE);
	memcpy(h->dstID, SOCK_UPFW_ID, MSG_ID_SIZE);
	h->funcCode   = 0x01;
	h->operCode   = 0x02;
	h->dataLen[3] = (U8)(llen & 0xff);
	h->dataLen[2] = (U8)((llen >> 8) & 0xff);
	h->dataLen[1] = (U8)((llen >> 16) & 0xff);
	h->dataLen[0] = (U8)((llen >> 24) & 0xff);
	memcpy(response + MSG_HEADER_LENTH, msg, length);
	msgid = msgid | 0x80000000;
	res   = remotesocket_sendack((char*) h->dstID, (U8*) response, llen, msgid);

	free(response);

	return res;
}

static int firmware_send_upgradeable_firmwares_to_backend(U32 semid, struct firmware_available* upgradeable_firmwares)
{
	int	ret = -1;
	int	i;
	cJSON* root;
	cJSON* data;
	cJSON* item;
	char*  json_string = NULL;

	root = cJSON_CreateObject();
	if (root == NULL) {
		goto exit;
	}

	data = cJSON_CreateArray();
	if (data == NULL) {
		goto exit;
	}

	cJSON_AddStringToObject(root, "method", "GetUpgradeableFirmwares");
	cJSON_AddStringToObject(root, "jid", "1");
	cJSON_AddStringToObject(root, "queryid", "1");

	for (i = 0; i < upgradeable_firmwares->fw_count; i++) {
		item = cJSON_CreateObject();
		if (item == NULL) {
			goto exit;
		}

		ret = structure_to_json_object(upgradeable_firmwares->fw[i], item);
		if (ret == 0) {
			cJSON_AddItemToArray(data, item);
		}
	}

	cJSON_AddItemToObject(root, "data", data);
	json_string = cJSON_Print(root);
	firmware_send_msg(semid, json_string, strlen(json_string));

	ret = 0;
exit:
	if (json_string) {
		free(json_string);
	}
	if (root) {
		cJSON_Delete(root);
		json_string = cJSON_Print(root);
	}
	return ret;
}

int firmware_get_system_return_value(int status)
{
	int ret = -1;

	if (-1 == status) {
		DEBUGP("system error\n");
	} else {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				DEBUGP("run successfully\n");
				ret = 0;
			} else {
				DEBUGP("run failed\n");
			}
		} else {
			DEBUGP("exit code\n");
		}
	}

	return ret;
}

int firmware_check_signature_ex(char* update_dir, char* filename, char* sign_filename)
{
	int  status = -1;
	char cmd[CMD_LEN_MAX];

	snprintf(cmd, CMD_LEN_MAX, "/usr/local/ssl/bin/openssl dgst -verify /etc/rsa_public.pem -sha1 -signature "
							   "%s/%s %s/%s 2>&1 > /dev/null",
			 update_dir, sign_filename, update_dir, filename);

	status = system(cmd);

	status = firmware_get_system_return_value(status);

	DEBUGP("%s:%d cmd: %s end ret: %d\n", __func__, __LINE__, cmd, status);

	return status;
}

int firmware_check_signature(char* update_dir, char* filename, char* sign_suffix)
{
	int  status = -1;
	char cmd[CMD_LEN_MAX];

	snprintf(cmd, CMD_LEN_MAX, "openssl dgst -verify /etc/rsa_public.pem -sha1 -signature "
							   "%s/%s%s %s/%s 2>&1 > /dev/null",
			 update_dir, filename, sign_suffix, update_dir, filename);

	status = system(cmd);

	status = firmware_get_system_return_value(status);

	DEBUGP("%s:%d cmd: %s end ret: %d\n", __func__, __LINE__, cmd, status);

	return status;
}

static int firmware_get_file_size(int fd, unsigned int* size)
{
	int			ret = EINVAL;
	struct stat sb;

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	if (fd < 0 || size == NULL) {
		*size = 0;
		goto exit;
	}

	memset(&sb, 0, sizeof(struct stat));

	ret = fstat(fd, &sb);
	if (ret != 0) {
		*size = 0;
		goto exit;
	}
	*size = sb.st_size;

exit:
	DEBUGP("%s:%d end, ret: %d\n", __func__, __LINE__, ret);
	return ret;
}

static int firmware_read_whole_file(char* filename, char** buf)
{
	int			 ret	   = -1, fd;
	unsigned int read_size = 0;

	fd = open(filename, O_RDONLY);
	if (fd <= 0) {
		goto exit;
	}

	ret = firmware_get_file_size(fd, &read_size);
	if (ret == 0 && read_size > 0) {
		*buf = malloc(read_size);
		if (*buf == NULL) {
			ret = ENOMEM;
			goto exit;
		}
		memset(*buf, 0, read_size);
		ret = read(fd, *buf, read_size);
		if (ret == read_size) {
			ret = 0;
		}
	}

exit:
	if (fd > 0) {
		close(fd);
	}
	DEBUGP("%s:%d end ret: %d, buf: %s\n", __func__, __LINE__, ret, *buf);
	return ret;
}

double firmware_get_remote_file_size(const char* url)
{
	double downloadFileLenth = 0;

	CURL* handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_HEADER, 1);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
	// curl_easy_setopt(handle, CURLOPT_PROXY, getenv("http_proxy"));
	if (curl_easy_perform(handle) == CURLE_OK) {
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
	} else {
		downloadFileLenth = -1;
	}

	curl_easy_cleanup(handle);
	return downloadFileLenth;
}

static size_t firmware_curl_download_write(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	int fd = *(int*) userdata;

	size_t written = write(fd, ptr, size * nmemb);

	return written;
}

typedef struct curl_progressdata {
	long local_file_size;
	int* percentage;
} curl_progressdata_t;

static int firmware_xferinfo(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpLoad, curl_off_t nowUpLoaded)
{
	int						  tmp			 = 0;
	int						  tmp_percentage = 0;
	struct curl_progressdata* prog_data		 = (struct curl_progressdata*) ptr;
	DEBUGP("total: %lld now: %lld\n", totalToDownload, nowDownloaded);
	if (totalToDownload > 0) {
		tmp = (int) (100 * (nowDownloaded + (curl_off_t) prog_data->local_file_size) / (totalToDownload + (curl_off_t) prog_data->local_file_size));
	}
	tmp_percentage = tmp / 2;
	if (tmp_percentage == 50 && nowDownloaded < totalToDownload) {
		tmp_percentage = 49;
	}
	*(prog_data->percentage) = tmp_percentage;
	orangelog_log_info("[download progress ============================%d%%]\r", tmp);

	return 0;
}

static int firmware_download_bycurl_process(CURL* handle, int* fd, char* url, int timeout, int* percentage)
{
	int						 ret = -1;
	struct curl_progressdata prog_data;

	prog_data.percentage = percentage;
	firmware_get_file_size(*fd, (unsigned int*) &(prog_data.local_file_size));

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(handle, CURLOPT_RESUME_FROM, prog_data.local_file_size);

	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, firmware_curl_download_write);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*) fd);

	curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, firmware_xferinfo);
	curl_easy_setopt(handle, CURLOPT_XFERINFODATA, &prog_data);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

	curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 0L);

	ret = curl_easy_perform(handle);
	if (ret != 0) {
	} else {
		orangelog_log_info("CURL get file successful.\n");
	}

	return ret;
}

static double firmware_get_remote_file_length(const char* url)
{
	double downloadFileLenth = 0;
	CURL*  handle			 = curl_easy_init();
	if (NULL == handle) {
		return -1;
	}

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_HEADER, 1);
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
	// curl_easy_setopt(handle, CURLOPT_PROXY, getenv("http_proxy"));
	if (curl_easy_perform(handle) == CURLE_OK) {
		orangelog_log_info("[getRemoteFileLength OK]");
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
	} else {
		downloadFileLenth = -1;
		orangelog_log_info("[getRemoteFileLength FAILED]");
	}

	curl_easy_cleanup(handle);
	return downloadFileLenth;
}

int fimware_download_file_by_curl(char* url_path, char* update_dir, char* filename, char* sign_suffix, int* percentage)
{
	int  ret = 0;
	char url[URL_FILE_LEN_MAX];
	char local_path[512];
	chdir(update_dir);
	int	fd	 = -1;
	CURL*  handle = NULL;
	double remote_size;

	if (sign_suffix == NULL) {
		snprintf(url, URL_FILE_LEN_MAX, "%s/%s", url_path, filename);
		snprintf(local_path, 512, "%s/%s", update_dir, filename);
	} else {
		snprintf(url, URL_FILE_LEN_MAX, "%s/%s%s", url_path, filename, sign_suffix);
		snprintf(local_path, 512, "%s/%s%s", update_dir, filename, sign_suffix);
	}

	remote_size = firmware_get_remote_file_length(url);
	orangelog_log_info("%s size: %f\n", url, remote_size);
	if (remote_size <= 0) {
		ret = EIO;
		goto exit;
	}

	curl_global_init(CURL_GLOBAL_ALL);

	fd = open(local_path, O_CREAT | O_WRONLY | O_TRUNC);
	if (fd < 0) {
		ret = EIO;
		goto exit;
	}

	handle = curl_easy_init();
	if (NULL == handle) {
		ret = EIO;
		goto exit;
	}

	int count = 0;
	int times = 5;

	while (count++ < times) {
		ret = firmware_download_bycurl_process(handle, &fd, url, 30, percentage);
		if (ret == 0) {
			break;
		}
		usleep(500);
	}

exit:
	if (fd > 0) {
		close(fd);
	}
	if (handle) {
		curl_easy_cleanup(handle);
	}
	curl_global_cleanup();
	return ret;
}

int firmware_download_file(char* url_path, char* update_dir, char* filename, char* sign_suffix)
{
	int  status = -1;
	char url[URL_FILE_LEN_MAX];
	char outparameter[OUT_PARAMETER_LEN_MAX];
	char cmd[CMD_LEN_MAX];

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	chdir(update_dir);

	if (sign_suffix == NULL) {
		snprintf(url, URL_FILE_LEN_MAX, "%s/%s", url_path, filename);
		snprintf(outparameter, OUT_PARAMETER_LEN_MAX, "--output-document=%s/%s 2>&1 > /dev/null", update_dir, filename);
	} else {
		snprintf(url, URL_FILE_LEN_MAX, "%s/%s%s", url_path, filename, sign_suffix);
		snprintf(outparameter, OUT_PARAMETER_LEN_MAX, "--output-document=%s/%s%s 2>&1 > /dev/null", update_dir, filename, sign_suffix);
	}

	snprintf(cmd, CMD_LEN_MAX, "wget -qT 30 %s %s", url, outparameter);

	status = system(cmd);

	status = firmware_get_system_return_value(status);

	DEBUGP("%s:%d cmd: %s end ret: %d\n", __func__, __LINE__, cmd, status);

	return status;
}

int firmware_remove_file(char* update_dir, char* filename, char* sign_suffix)
{
	char cmd[CMD_LEN_MAX];

	if (update_dir == NULL || filename == NULL) {
		return -1;
	}

	snprintf(cmd, CMD_LEN_MAX, "rm %s/%s 2>&1 > /dev/null", update_dir, filename);
	system(cmd);

	if (sign_suffix != NULL) {
		snprintf(cmd, CMD_LEN_MAX, "rm %s/%s%s 2>&1 > /dev/null", update_dir, filename, sign_suffix);
		system(cmd);
	}

	return 0;
}

int firmware_available_get(struct firmware_info* current_fw, struct firmware_available* available_fw)
{
	int   ret			   = -1;
	char* signature_suffix = ".sign";
	char  xml_filename_path[128];

	if (current_fw == NULL || available_fw == NULL) {
		goto exit;
	}

	ret = firmware_download_file(config.update_url, config.update_dir, current_fw->xml_filename, NULL);
	if (ret != 0) {
		goto exit;
	}

	ret = firmware_download_file(config.update_url, config.update_dir, current_fw->xml_filename, signature_suffix);
	if (ret != 0) {
		goto exit;
	}

	ret = firmware_check_signature(config.update_dir, current_fw->xml_filename, signature_suffix);
	if (ret != 0) {
		goto exit;
	}

	snprintf(xml_filename_path, 128, "%s/%s", config.update_dir, current_fw->xml_filename);
	parse_xml(xml_filename_path, current_fw, available_fw);

#if 0
    firmware_remove_file(config.update_dir, current_fw->xml_filename,
						 signature_suffix);
#endif

exit:
	return ret;
}

void firmware_available_dump(struct firmware_available* fws)
{
	int i;
	struct firmware_info* __attribute__((unused)) fw = NULL;

	for (i = 0; i < fws->fw_count; i++) {
		fw = fws->fw[i];
		DEBUGP("serialMask: %s\nversion: %s\nminVersion: %s\nbuildNumber: "
			   "%d\nfirmwareFile: %s\nfirmwareSize: %d\nxmlFile: %s\n",
			   fw->serial_mask, fw->version, fw->min_version, fw->build_number, fw->firmware_filename, fw->firmware_size, fw->xml_filename);
#if 0
        if (fw->de) {
            printf("DE: %s\n", fw->de);
        }
        if (fw->en) {
            printf("EN: %s\n", fw->en);
        }
#endif
	}

	return;
}

void firmware_available_free(struct firmware_available* fws)
{
	int i;

	for (i = 0; i < fws->fw_count; i++) {
		if (fws->fw[i]) {
			if (fws->fw[i]->de != NULL) {
				free(fws->fw[i]->de);
				fws->fw[i]->de = NULL;
			}

			if (fws->fw[i]->en != NULL) {
				free(fws->fw[i]->en);
				fws->fw[i]->en = NULL;
			}
			free(fws->fw[i]);
			fws->fw[i] = NULL;
		}
	}

	return;
}

int firmware_local_current_get(char* filename, struct firmware_info* fw)
{
	int   ret  = -1;
	char* buff = NULL;

	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	ret = firmware_read_whole_file(filename, &buff);
	if (ret == 0 && buff != NULL) {
		ret = pase_json(JSON_LOAD_TYPE_STRING, buff, fw);
		free(buff);
	}

	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	return ret;
}

int firmware_check_upgradeable(U32 semid, struct firmware_available* upgradeable_firmwares)
{
	int					 ret = EINVAL;
	struct firmware_info current_fw;

	DEBUGP("%s:%d begin\n", __func__, __LINE__);

	if (check_status == CHECKING) {
		goto exit;
	}

	check_status = CHECKING;

	memset(&current_fw, 0, sizeof(struct firmware_info));
	ret = firmware_local_current_get(config.firmware_info, &current_fw);
	if (ret != 0) {
		DEBUGP("%s:%d get firmware config error. ret: %d\n", __func__, __LINE__, ret);
		goto exit;
	}

	firmware_available_free(upgradeable_firmwares);
	ret = firmware_available_get(&current_fw, upgradeable_firmwares);
	if (ret == 0) {
		firmware_send_upgradeable_firmwares_to_backend(semid, upgradeable_firmwares);
	}

exit:
	DEBUGP("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	check_status = 0;
	return ret;
}
