#include "xml.h"

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

static int __compare_version(const char* a, const char* b)
{
	while (*a != '\0' && *b != '\0' && *a == *b) {
		a++;
		b++;
	}

	if (*a == '\0' && *b != '\0') {
		return *b - '0';
	}

	if (*a != '\0' && *b == '\0') {
		return *a - '0';
	}

	return (*a - *b);
}

static int __parse_lang(xmlDocPtr doc, xmlNodePtr cur, struct firmware_info* fw, struct firmware_available* fws)
{
	int ret = EINVAL;
	int len;
	assert(doc || cur);
	xmlChar* key;
	xmlChar* attribute;

	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	fws->fw[fws->fw_count] = malloc(sizeof(struct firmware_info));
	if (fws->fw[fws->fw_count] == NULL) {
		DEBUGP("%s:%d malloc failed ret: %d\n", __func__, __LINE__, ret);
		goto exit;
	}

	memcpy(fws->fw[fws->fw_count], fw, sizeof(struct firmware_info));

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "releaseNotes"))) {
			attribute = xmlGetProp(cur, (const xmlChar*) "lang");
			key		  = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (attribute && key) {
				DEBUGP("attribute: %s\n", attribute);
				DEBUGP("key: %s\n", key);
				len = strlen((char*) key);
				if (strlen((char*) attribute) == 2 && memcmp((char*) attribute, "en", 2) == 0) {
					fws->fw[fws->fw_count]->en = malloc(len + 1);
					if (fws->fw[fws->fw_count]->en != NULL) {
						memcpy(fws->fw[fws->fw_count]->en, key, len);
					}
				} else if (strlen((char*) attribute) == 2 && memcmp((char*) attribute, "de", 2) == 0) {
					fws->fw[fws->fw_count]->de = malloc(len + 1);
					if (fws->fw[fws->fw_count]->de != NULL) {
						memcpy(fws->fw[fws->fw_count]->de, key, len);
					}
				}
				xmlFree(key);
			}
		}
		cur = cur->next;
	}

	DEBUGP_TEST("EN: %s\n", fws->fw[fws->fw_count]->en == NULL ? "null" : fws->fw[fws->fw_count]->en);
	DEBUGP_TEST("DE: %s\n", fws->fw[fws->fw_count]->de == NULL ? "null" : fws->fw[fws->fw_count]->de);

	fws->fw_count++;
	ret = 0;
exit:
	DEBUGP("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	return ret;
}

int pase_xml_callback(char* file_name, char* serial_mask, int (*callback)(void* data, void* arg), void* arg)
{
	int ret = -1;
	assert(file_name);

	xmlDocPtr		   doc;
	xmlNodePtr		   cur;
	xmlChar*		   attribute;
	int				   len;
	struct device_info info;

	DEBUGP_TEST("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	doc = xmlParseFile(file_name);
	if (doc == NULL) {
		DEBUGP("%s:%d xmlParseFile failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		DEBUGP("%s:%d xmlDocGetRootElement failed\n", __func__, __LINE__);
		goto exit;
	}

	if ((xmlStrcmp(cur->name, (const xmlChar*) "updates"))) {
		DEBUGP("%s:%d xmlStrcmp failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "update"))) {
			attribute = xmlGetProp(cur, (const xmlChar*) "serialMask");
			if (NULL == attribute) {
				DEBUGP("attribute is NULL\n");
				continue;
			}
			len = strlen((char*) attribute);
			len = len > SERIAL_MASK_LEN_MAX ? SERIAL_MASK_LEN_MAX - 1 : len;
			if (len != strlen(serial_mask) || 0 != memcmp(attribute, serial_mask, len)) {
				DEBUGP("attribute is %s\n", attribute);
				continue;
			}

			memset(&info, 0, sizeof(info));

			attribute = xmlGetProp(cur, (const xmlChar*) "version");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
				memcpy(info.version, attribute, len);
				DEBUGP("version is %s\n", attribute);
			}
			attribute = xmlGetProp(cur, (const xmlChar*) "build");
			if (attribute) {
				info.build_number = atoi((char*) attribute);
				DEBUGP("build is %s\n", attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "minVersion");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
				memcpy(info.min_version, attribute, len);
				DEBUGP("min_version is %s\n", attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "serialMask");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > SERIAL_MASK_LEN_MAX ? SERIAL_MASK_LEN_MAX - 1 : len;
				memcpy(info.serial_mask, attribute, len);
				DEBUGP("serial_mask is %s\n", attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "fileName");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > FIRMWARE_NAME_LEN_MAX ? FIRMWARE_NAME_LEN_MAX - 1 : len;
				memcpy(info.firmware_filename, attribute, len);
				DEBUGP("filename is %s\n", attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "fileSize");
			if (attribute) {
				info.firmware_size = atoi((char*) attribute);
				DEBUGP("filesize is %s\n", attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "i");
			if (attribute) {
				info.index = atoi((char*) attribute);
				DEBUGP("index is %s\n", attribute);
			}

			callback(&info, arg);
		}
		cur = cur->next;
	}

	ret = 0;
exit:
	DEBUGP_TEST("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	if (doc) {
		xmlFreeDoc(doc);
	}
	return ret;
}

int parse_xml(const char* file_name, struct firmware_info* myfw, struct firmware_available* fws)
{
	int ret = -1;
	assert(file_name);

	xmlDocPtr			 doc;
	xmlNodePtr			 cur;
	xmlChar*			 attribute;
	int					 len;
	struct firmware_info newfw;

	DEBUGP_TEST("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	doc = xmlParseFile(file_name);
	if (doc == NULL) {
		DEBUGP("%s:%d xmlParseFile failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		DEBUGP("%s:%d xmlDocGetRootElement failed\n", __func__, __LINE__);
		goto exit;
	}

	if ((xmlStrcmp(cur->name, (const xmlChar*) "updates"))) {
		DEBUGP("%s:%d xmlStrcmp failed\n", __func__, __LINE__);
		goto exit;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "update"))) {
			memset(&newfw, 0, sizeof(struct firmware_info));
			attribute = xmlGetProp(cur, (const xmlChar*) "serialMask");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > SERIAL_MASK_LEN_MAX ? SERIAL_MASK_LEN_MAX - 1 : len;
				memcpy(newfw.serial_mask, attribute, len);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "version");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
				memcpy(newfw.version, attribute, len);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "build");
			if (attribute) {
				newfw.build_number = atoi((char*) attribute);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "minVersion");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > VERSION_LEN_MAX ? VERSION_LEN_MAX - 1 : len;
				memcpy(newfw.min_version, attribute, len);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "fileName");
			if (attribute) {
				len = strlen((char*) attribute);
				len = len > FIRMWARE_NAME_LEN_MAX ? FIRMWARE_NAME_LEN_MAX - 1 : len;
				memcpy(newfw.firmware_filename, attribute, len);
			}

			attribute = xmlGetProp(cur, (const xmlChar*) "fileSize");
			if (attribute) {
				newfw.firmware_size = atoi((char*) attribute);
			}

			DEBUGP("3 xml: %s, json: %s\n", newfw.version, myfw->version);
			DEBUGP("3 min xml: %s, min json: %s\n", newfw.min_version, myfw->min_version);
			DEBUGP("3 build xml: %d, build json: %d\n", newfw.build_number, myfw->build_number);

			if (memcmp(newfw.serial_mask, myfw->serial_mask, SERIAL_MASK_LEN_MAX) != 0 || __compare_version(newfw.version, myfw->version) < 0) {
				cur = cur->next;
				continue;
			}

			if (__compare_version(newfw.version, myfw->version) == 0 && __compare_version(newfw.min_version, myfw->min_version) < 0) {
				cur = cur->next;
				continue;
			}

			if (__compare_version(newfw.version, myfw->version) == 0 && __compare_version(newfw.min_version, myfw->min_version) == 0 &&
				newfw.build_number <= myfw->build_number) {
				cur = cur->next;
				continue;
			}

			/* new firmware can update */
			__parse_lang(doc, cur, &newfw, fws);
		}
		cur = cur->next;
	}

	ret = 0;
exit:
	DEBUGP_TEST("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	if (doc) {
		xmlFreeDoc(doc);
	}
	return ret;
}
