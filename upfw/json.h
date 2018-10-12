#ifndef __JSON_H_
#define __JSON_H_

#include "define.h"
#include "var.h"
#include <cJSON.h>

extern int pase_json_object(cJSON* root, struct firmware_info* fw);
extern int pase_json(unsigned char json_load_type, char* json_data, struct firmware_info* fw);
extern int structure_to_json(struct firmware_info* fw, char** json);
extern int structure_to_json_object(struct firmware_info* fw, cJSON* root);

#endif
