#ifndef __XML_TO_JSON_H_
#define __XML_TO_JSON_H_

#include "define.h"
#include "var.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define FOR_FRONTEND 1
#define FOR_UPFW 2
#define FOR_BOTH 3

#define DEVICE_KEY_COUNT_MAX 16
#define DEVICE_KEY_LEN_MAX 32

typedef struct device_key_info {
	char key[DEVICE_KEY_LEN_MAX];
	U8   key_type;
} device_key_info_t;

typedef struct device_key_list {
	int					   key_count;
	struct device_key_info key_info[DEVICE_KEY_COUNT_MAX];
} device_key_list_t;

int xml_to_json(char* language, struct device_key_list* keys, int device_type_id, char* serial_mask, char* xml_file, char* json_file);

#endif
