#ifndef __FIRMWARE_H_
#define __FIRMWARE_H_

#include "define.h"
#include "var.h"

extern int firmware_get_system_return_value(int status);

extern int firmware_remove_file(char* update_dir, char* filename, char* sign_suffix);
extern int firmware_download_file(char* url_path, char* update_dir, char* filename, char* sign_suffix);

extern int firmware_check_signature_ex(char* update_dir, char* filename, char* sign_filename);
extern int firmware_check_signature(char* update_dir, char* filename, char* sign_suffix);
extern void firmware_available_dump(struct firmware_available* fws);
extern void firmware_available_free(struct firmware_available* fws);
extern int firmware_available_get(struct firmware_info* current_fw, struct firmware_available* available_fw);
extern int firmware_local_current_get(char* filename, struct firmware_info* fw);

extern int firmware_check_upgradeable(U32 semid, struct firmware_available* upgradeable_firmwares);

extern double firmware_get_remote_file_size(const char* url);
extern int fimware_download_file_by_curl(char* url_path, char* update_dir, char* filename, char* sign_suffix, int* percentage);

#endif
