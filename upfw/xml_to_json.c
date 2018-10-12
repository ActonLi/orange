#include "xml_to_json.h"
#include "common.h"
#include <cJSON.h>
#include <osa_file.h>

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

static int __parse_lang(xmlDocPtr doc, xmlNodePtr cur, char* language, cJSON* root)
{
	int ret = EINVAL;
	assert(doc || cur);
	xmlChar* key;
	xmlChar* attribute;

	if (root == NULL || language == NULL) {
		goto exit;
	}
	DEBUGP("%s:%d begin ret: %d\n", __func__, __LINE__, ret);

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "releaseNotes"))) {
			attribute = xmlGetProp(cur, (const xmlChar*) "lang");
			key		  = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (attribute && key) {
				if (strlen((char*) attribute) == 2 && memcmp(attribute, language, 2) == 0) {
					cJSON_AddStringToObject(root, "description", (char*) key);
					break;
				}
				xmlFree(key);
			}
		}
		cur = cur->next;
	}

	ret = 0;
exit:
	DEBUGP("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	return ret;
}

int xml_to_json(char* language, struct device_key_list* keys, int software_id, char* serial_mask, char* xml_file, char* json_file)
{
	int		   ret = -1;
	xmlDocPtr  doc = NULL;
	xmlNodePtr cur;
	xmlChar*   attribute;
	cJSON*	 root = NULL;

	cJSON* item = NULL;
	int	i;

	if (-1 == access(xml_file, F_OK) || serial_mask == NULL) {
		goto exit;
	}

	doc = xmlParseFile(xml_file);
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

	root = cJSON_CreateArray();
	if (root == NULL) {
		goto exit;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar*) "update"))) {
			item = cJSON_CreateObject();
			if (NULL == item) {
				break;
			}

			for (i = 0; i < keys->key_count; i++) {
				attribute = xmlGetProp(cur, (const xmlChar*) keys->key_info[i].key);
				if (attribute) {
					if (keys->key_info[i].key_type == cJSON_Number) {
						cJSON_AddNumberToObject(item, keys->key_info[i].key, atoi((char*) attribute));
					} else if (keys->key_info[i].key_type == cJSON_String) {
						cJSON_AddStringToObject(item, keys->key_info[i].key, (char*) attribute);
					}
				}
			}

			__parse_lang(doc, cur, language, item);
			cJSON_AddNumberToObject(item, "softwareId", software_id);

			cJSON_AddItemToArray(root, item);
		}
		cur = cur->next;
	}

	if (TRUE == write_jsonfile(json_file, root, MODE_CREATE_ALWAYS)) {
		ret = 0;
	}

exit:
	DEBUGP_TEST("%s:%d end ret: %d\n", __func__, __LINE__, ret);
	if (doc) {
		xmlFreeDoc(doc);
	}
	if (root) {
		cJSON_Delete(root);
	}
	return ret;
}
