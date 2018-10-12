#include "watchdog_client.h"
#include "lsrpc.h"
#include "message.h"
#include "remotesocket.h"
#include <stdarg.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

int watchdog_client_send_msg_by_method(char* method, char* src_id, char* dst_id, char* section, char* data)
{
	int	ret  = 255, len;
	char*  msg  = NULL;
	cJSON* root = NULL;

	if (NULL == src_id || NULL == dst_id || NULL == data || NULL == section) {
		goto exit;
	}

	root = cJSON_CreateObject();
	if (NULL == root) {
		goto exit;
	}

	cJSON_AddStringToObject(root, section, data);

	len = ls_generate_msg(method, root, &msg);
	if (len > 0 && NULL != msg) {
		remotesocket_sendmsg(dst_id, src_id, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, (U32) len);
		ret = 0;
	}

exit:
	if (msg) {
		free(msg);
	}
	if (root) {
		cJSON_Delete(root);
	}
	return ret;
}

int watchdog_client_register(char* src_id, char* dst_id, int type, int timeout, char* proc_name, char* poll_method, void* timeout_func)
{
	char*  msg  = NULL;
	cJSON* root = NULL;
	U32	len  = 0;
	int	ret  = 255;

	if ((type < 0) || (type > 2)) {
		goto exit;
	}

	if (proc_name == NULL) {
		goto exit;
	}

	if ((type == 1) && (poll_method == NULL)) {
		goto exit;
	}

	root = cJSON_CreateObject();
	if (NULL == root) {
		goto exit;
	}

	cJSON_AddNumberToObject(root, "type", type);
	cJSON_AddNumberToObject(root, "timeout", timeout);
	cJSON_AddStringToObject(root, "task_name", proc_name);
	cJSON_AddStringToObject(root, "task_poll_port", (char*) src_id);
	if (poll_method != NULL) {
		cJSON_AddStringToObject(root, "task_poll_method", poll_method);
	}
	if (NULL != timeout_func) {
		char pointer[32] = "";
		snprintf(pointer, sizeof(pointer), "%p", timeout_func);
		cJSON_AddStringToObject(root, "callback_after_timeout", pointer);
	}

	len = ls_generate_msg(WATCHDOG_TASK_REGISTER, root, &msg);
	if (len > 0 && NULL != msg) {
		remotesocket_sendmsg(dst_id, src_id, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, (U32) len);
		ret = 0;
	}

exit:
	if (NULL != root) {
		cJSON_Delete(root);
	}
	if (NULL != msg) {
		free(msg);
	}
	return ret;
}

int watchdog_client_unregister(char* src_id, char* dst_id, char* proc_name)
{
	return watchdog_client_send_msg_by_method(WATCHDOG_TASK_UNREGISTER, src_id, dst_id, "task_name", proc_name);
}

int watchdog_client_feed(char* src_id, char* dst_id, char* proc_name)
{
	return watchdog_client_send_msg_by_method(WATCHDOG_TASK_FEED, src_id, dst_id, "task_name", proc_name);
}

int watchdog_client_pause(char* src_id, char* dst_id, char* proc_name)
{
	return watchdog_client_send_msg_by_method(WATCHDOG_TASK_PAUSE, src_id, dst_id, "task_name", proc_name);
}

int watchdog_client_resume(char* src_id, char* dst_id, char* proc_name)
{
	return watchdog_client_send_msg_by_method(WATCHDOG_TASK_RESUME, src_id, dst_id, "task_name", proc_name);
}
