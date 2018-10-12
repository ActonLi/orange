#ifndef __XML_H_
#define __XML_H_

#include "define.h"
#include "var.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

typedef struct device_info {
	int  index;
	char version[VERSION_LEN_MAX];
	char min_version[VERSION_LEN_MAX];
	char max_version[VERSION_LEN_MAX];
	char serial_mask[SERIAL_MASK_LEN_MAX];
	int  build_number;
	int  firmware_size;
	char firmware_filename[FIRMWARE_NAME_LEN_MAX];
} device_info_t;

extern int parse_xml(const char* file_name, struct firmware_info* myfw, struct firmware_available* fws);

extern int pase_xml_callback(char* file_name, char* serial_mask, int (*callback)(void* data, void* arg), void* arg);
#endif
