#ifndef __WATCHDOG_CLIENT_H_

#define CLIENT_TYPE_REPORT 0
#define CLIENT_TYPE_QUERY 1
#define CLIENT_TYPE_THIRDPARTY 2

#define WATCHDOG_TASK_REGISTER "watchDogRegister"
#define WATCHDOG_TASK_UNREGISTER "watchDogUnRegister"
#define WATCHDOG_TASK_FEED "watchDogFeed"
#define WATCHDOG_TASK_PAUSE "watchDogPause"
#define WATCHDOG_TASK_RESUME "watchDogResume"
#define WATCHDOG_TASK_LOG "watchDogLog"

#define WATCHDOG_LOG_LINE_MAX 2048

int watchdog_client_register(char* src_id, char* dst_id, int type, int timeout, char* task_name, char* poll_method, void* timeout_callback);
int watchdog_client_unregister(char* src_id, char* dst_id, char* task_name);
int watchdog_client_feed(char* src_id, char* dst_id, char* task_name);
int watchdog_client_pause(char* src_id, char* dst_id, char* task_name);
int watchdog_client_resume(char* src_id, char* dst_id, char* task_name);
int watchdog_client_send_msg_by_method(char* method, char* src_id, char* dst_id, char* section, char* data);

#endif
