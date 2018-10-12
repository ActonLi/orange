#include "json.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

#if 0
#define DEBUGP_TEST printf
#else
#define DEBUGP_TEST(format, args...)
#endif

static int __structure_to_json(struct firmware_info* fw, cJSON* root)
{
	cJSON* value;

	value = cJSON_CreateString(fw->version);
	if (value) {
		cJSON_AddItemToObject(root, "version", value);
	}
	value = cJSON_CreateString(fw->min_version);
	if (value) {
		cJSON_AddItemToObject(root, "minVersion", value);
	}
	value = cJSON_CreateString(fw->serial_mask);
	if (value) {
		cJSON_AddItemToObject(root, "serialMask", value);
	}
	value = cJSON_CreateString(fw->firmware_filename);
	if (value) {
		cJSON_AddItemToObject(root, "fileName", value);
	}
	value = cJSON_CreateString(fw->xml_filename);
	if (value) {
		cJSON_AddItemToObject(root, "xmlFile", value);
	}
	value = cJSON_CreateNumber(fw->build_number);
	if (value) {
		cJSON_AddItemToObject(root, "build", value);
	}
	value = cJSON_CreateBool(fw->is_local);
	if (value) {
		cJSON_AddItemToObject(root, "local", value);
	}

	return 0;
}

int structure_to_json_object(struct firmware_info* fw, cJSON* root)
{
	return __structure_to_json(fw, root);
}

int structure_to_json(struct firmware_info* fw, char** json)
{
	int	ret  = 0;
	cJSON* root = NULL;
	// cJSON* value = NULL;

	if (fw == NULL) {
		ret = -1;
		goto exit;
	}

	root = cJSON_CreateObject();
	if (root == NULL) {
		ret = -1;
		goto exit;
	}

	__structure_to_json(fw, root);

	*json = cJSON_Print(root);

exit:
	if (root) {
		cJSON_Delete(root);
	}
	return ret;
}

static int __pase_json(cJSON* root, struct firmware_info* fw)
{
	int	ret = -1;
	int	len;
	cJSON* item;

	item = cJSON_GetObjectItem(root, "version");
	if (item && item->type == cJSON_String) {
		DEBUGP("value is %p item: %s\n", root, item->valuestring);
		len = strlen(item->valuestring);
		len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
		memcpy(fw->version, item->valuestring, len);
	}

	item = cJSON_GetObjectItem(root, "deviceType");
	if (item && item->type == cJSON_Number) {
		fw->deviceid = item->valueint;
	}

	item = cJSON_GetObjectItem(root, "build");
	if (item && item->type == cJSON_Number) {
		fw->build_number = item->valueint;
	}

	item = cJSON_GetObjectItem(root, "minVersion");
	if (item && item->type == cJSON_String) {
		DEBUGP("value is %p item: %s\n", root, item->valuestring);
		len = strlen(item->valuestring);
		len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
		memcpy(fw->min_version, item->valuestring, len);
	}

	item = cJSON_GetObjectItem(root, "serialMask");
	if (item && item->type == cJSON_String) {
		DEBUGP("value is %p item: %s\n", root, item->valuestring);
		len = strlen(item->valuestring);
		len = len > SERIAL_MASK_LEN_MAX ? SERIAL_MASK_LEN_MAX - 1 : len;
		memcpy(fw->serial_mask, item->valuestring, len);
		snprintf(fw->xml_filename, XML_FILENAME_LEN_MAX, "%s.xml", fw->serial_mask);
	}

	item = cJSON_GetObjectItem(root, "fileName");
	if (item && item->type == cJSON_String) {
		DEBUGP("value is %p item: %s\n", root, item->valuestring);
		len = strlen(item->valuestring);
		len = len > FIRMWARE_NAME_LEN_MAX ? FIRMWARE_NAME_LEN_MAX - 1 : len;
		memcpy(fw->firmware_filename, item->valuestring, len);
	}

	item = cJSON_GetObjectItem(root, "xmlFile");
	if (item && item->type == cJSON_String) {
		DEBUGP("value is %p item: %s\n", root, item->valuestring);
		len = strlen(item->valuestring);
		len = len > XML_FILENAME_LEN_MAX ? XML_FILENAME_LEN_MAX - 1 : len;
		memcpy(fw->xml_filename, item->valuestring, len);
	}

	item = cJSON_GetObjectItem(root, "local");
	if (item) {
		if (item->type == cJSON_True) {
			fw->is_local = 1;
		} else {
			fw->is_local = 0;
		}
	}

	DEBUGP_TEST("%s:%d pase json config end\n\n", __func__, __LINE__);

	ret = 0;
	return ret;
}

int pase_json_object(cJSON* root, struct firmware_info* fw)
{
	return __pase_json(root, fw);
}

int pase_json(unsigned char json_load_type, char* json_data, struct firmware_info* fw)
{
	int	ret  = -1;
	cJSON* root = NULL;

	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	if (json_load_type == JSON_LOAD_TYPE_FILE) {
		/* TODO: json file */
		goto exit;
	} else if (json_load_type == JSON_LOAD_TYPE_STRING) {
		root = cJSON_Parse(json_data);
		if (root == NULL) {
			goto exit;
		}
	} else {
		goto exit;
	}

	ret = __pase_json(root, fw);

exit:
	DEBUGP("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	if (root) {
		cJSON_Delete(root);
	}
	return ret;
}
