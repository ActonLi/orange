#include "common.h"
#include "cJSON.h"
#include "message.h"
#include "string.h"
#include <dirent.h>
#include <sys/time.h>
const U32 TM_TICK_DIV[] = {100, 200, 400, 800, 1000};

typedef struct {
	char  pid[MSG_ID_SIZE];
	char* path;
} STRMsgAddrType;

const STRMsgAddrType msg_addr[] = {
	{SOCK_QXMPP_ID, SOCK_QXMPP_PATH}, {SOCK_WIFI_ID, SOCK_WIFI_PATH}, {SOCK_BACKEND_ID, SOCK_BACKEND_PATH}, {SOCK_AC_ID, SOCK_AC_PATH},
};

BOOL msg_findpathbyid(char* pid, char* path)
{
	if (NULL == pid || NULL == path)
		return FALSE;

	U32  i;
	BOOL res = FALSE;
	for (i = 0; i < sizeof(msg_addr) / sizeof(STRMsgAddrType); i++) {
		if (0 == memcmp(pid, msg_addr[i].pid, MSG_ID_SIZE)) {
			memset(path, 0, strlen(msg_addr[i].path) + 1);
			strcpy(path, msg_addr[i].path);
			res = TRUE;
			break;
		}
	}
	return (res);
}
