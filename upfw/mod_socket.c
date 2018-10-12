#include "events.h"
#include "message.h"
#include "module.h"
#include "msgdispatch.h"

#ifndef REMOTE_SOCKET_TEST
#define REMOTE_SOCKET_TEST
#endif

#ifdef REMOTE_SOCKET_TEST
#include "remotesocket.h"
#else
#include "localsocket.h"
#endif

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

void upfw_handlemsg(U8* msg, U32 length, U32 msgid)
{
	DEBUGP("%s:%d msg: %p, length: %u, msgid: %u\n", __func__, __LINE__, msg, length, msgid);
	msgdispatch_process((void*) msg, (U32) length, msgid);
}

static void socket_init(void)
{
	DEBUGP("init zzy localsocket communication 1\n");
	if (TRUE) {
#ifdef REMOTE_SOCKET_TEST
		CONFIG_PARAM param = {0};
		memcpy(param.pid, SOCK_UPFW_ID, MSG_ID_SIZE);
		param.enabletcpsrv = TRUE;
		param.enableudpsrv = TRUE;
		param.condpidnum   = 3;
		param.condpid	  = (PID_CONNECT*) malloc(param.condpidnum * sizeof(PID_CONNECT));

		memcpy(param.condpid[0].pid, SOCK_BACKEND_ID, MSG_ID_SIZE);
		param.condpid[0].contype = REMOTESOCKET_SOCKTYPE_UDP;

		memcpy(param.condpid[1].pid, SOCK_WATCHDOG_ID, MSG_ID_SIZE);
		param.condpid[1].contype = REMOTESOCKET_SOCKTYPE_UDP;

		memcpy(param.condpid[2].pid, SOCK_AC_ID, MSG_ID_SIZE);
		param.condpid[2].contype = REMOTESOCKET_SOCKTYPE_UDP;

		DEBUGP("init zzy localsocket communication 4, %d, %p\n", param.condpidnum, &param);
		remotesocket_init(&param, upfw_handlemsg);
		DEBUGP("init zzy localsocket communication 5\n");
		free(param.condpid);
#else
		LOCALSOCKET_CONFIG_PARAM param = {0};
		memcpy(param.pid, SOCK_UPFW_ID, MSG_ID_SIZE);
		param.enabletcpsrv = TRUE;
		param.enableudpsrv = TRUE;
		param.condpidnum   = 3;
		param.condpid	  = (LOCALSOCKET_PID_CONNECT*) malloc(param.condpidnum * sizeof(LOCALSOCKET_PID_CONNECT));

		memcpy(param.condpid[0].pid, SOCK_QXMPP_ID, MSG_ID_SIZE);
		param.condpid[0].contype = LOCALSOCKET_SOCKTYPE_LONGTCP;

		memcpy(param.condpid[1].pid, SOCK_WIFI_ID, MSG_ID_SIZE);
		param.condpid[1].contype = LOCALSOCKET_SOCKTYPE_LONGTCP;

		memcpy(param.condpid[2].pid, SOCK_AC_ID, MSG_ID_SIZE);
		param.condpid[2].contype = LOCALSOCKET_SOCKTYPE_LONGTCP;

		DEBUGP("init zzy localsocket communication 3, %d, %p\n", param.condpidnum, &param);
		localsocket_init(param, upfw_handlemsg);
		DEBUGP("init zzy localsocket communication 2\n");

		free(param.condpid);
#endif
	} else {
#ifdef REMOTE_SOCKET_TEST
		CONFIG_PARAM param = {0};
		memcpy(param.pid, SOCK_UPFW_ID, MSG_ID_SIZE);
		param.enabletcpsrv = 1;
		param.enableudpsrv = 1;
		param.condpidnum   = 1;
		/// param.condpid = (PID_CONNECT *)malloc(sizeof(PID_CONNECT));
		memcpy(param.condpid[0].pid, SOCK_UPFW_ID, MSG_ID_SIZE);

		param.condpid[0].contype = REMOTESOCKET_SOCKTYPE_LONGTCP;
		remotesocket_init(&param, upfw_handlemsg);
#else
		LOCALSOCKET_CONFIG_PARAM param = {0};
		memcpy(param.pid, SOCK_UPFW_ID, MSG_ID_SIZE);
		param.enabletcpsrv = 1;
		param.enableudpsrv = 1;
		param.condpidnum   = 1;
		/// param.condpid = (LOCALSOCKET_PID_CONNECT
		/// *)malloc(sizeof(LOCALSOCKET_PID_CONNECT));
		memcpy(param.condpid[0].pid, SOCK_UPFW_ID, MSG_ID_SIZE);

		param.condpid[0].contype = LOCALSOCKET_SOCKTYPE_LONGTCP;
		localsocket_init(param, upfw_handlemsg);
#endif
	}
}

static void socket_uninit(void)
{
}

static void socket_realtime(U32 semid, U32 moduleid, STREventElemType* event)
{
	DEBUGP("%s:%d buf: %p, semid: %u, moduleid: %u\n", __func__, __LINE__, event, semid, moduleid);

	if (NULL != event) {
		DEBUGP("socket event wake up\n");
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
MODULE_INFO(MD_SOCKET_NAME, MD_SOCKET_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0), socket_init, socket_realtime, socket_clear,
			socket_uninit, socket_getperiod, socket_setlocaltime);
