#include "events.h"
#include "message.h"
#include "module.h"
#include "msgdispatch.h"
#include "remotesocket.h"

#include "XMPPClient.h"
#include "curl.h"
#include "strophe.h"

void backend_handlemsg(U8* msg, U32 length, U32 msgid)
{
	//	xmpp_stanza_t*   pmsg   = NULL;
	MSG_DATA_HEADER* header = NULL;
	cJSON *			 json, *jsonjid;

	void* payload = NULL;
	payload		  = &msg[MSG_HEADER_LENTH];
	header		  = (MSG_DATA_HEADER*) msg;
	printf(" ======backend_handlemsg======= for portal client\n");

	json	= NULL;
	jsonjid = NULL;
	if (header->funcCode == MSG_FUNCODE_LOGIC_JSON) {
		json = cJSON_Parse((char*) payload);
		if (NULL != json) {
			jsonjid = cJSON_GetObjectItem(json, "jid");
		}
		if ((jsonjid != NULL) && (NULL != jsonjid->valuestring)) {
			fnXmppSendMessage(jsonjid->valuestring, payload);
		}
		if (NULL != json) {
			cJSON_Delete(json);
		}
	} else {
		msgdispatch_process(msg, length, msgid);
	}
}

static void socket_init(void)
{

	if (TRUE) {
		CONFIG_PARAM param = {0};
		memcpy(param.pid, SOCK_REMOTEXMPP_ID, MSG_ID_SIZE);
		param.enabletcpsrv = FALSE;
		param.enableudpsrv = TRUE;
		param.condpidnum   = 1;
		param.condpid	  = (PID_CONNECT*) malloc(param.condpidnum * sizeof(PID_CONNECT));

		memcpy(param.condpid[0].pid, SOCK_BACKEND_ID, MSG_ID_SIZE);
		param.condpid[0].contype = REMOTESOCKET_SOCKTYPE_UDP;

		printf("init zzy localsocket communication 3, %d, %p\n", param.condpidnum, &param);
		remotesocket_init(&param, backend_handlemsg);
		printf("init zzy localsocket communication 2\n");

		free(param.condpid);
	}
}

static void socket_uninit(void)
{
}

static void socket_realtime(U32 semid, U32 moduleid, STREventElemType* event)
{
	if (NULL != event) {
		printf("socket event wake up\n");
		event_clearflags(semid, event, moduleid);
	} else {
		////printf("socket period wake up\n");
	}
}

static void socket_clear(void)
{
}
static void socket_setlocaltime(U64 time)
{
}
static U32 socket_getperiod(void)
{
	return (0);
}
MODULE_INFO(MD_SOCKET_NAME, MD_SOCKET_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_BACKEND, SEM_ID_THREAD_MAIN, 0), socket_init, socket_realtime, socket_clear,
			socket_uninit, socket_getperiod, socket_setlocaltime);
