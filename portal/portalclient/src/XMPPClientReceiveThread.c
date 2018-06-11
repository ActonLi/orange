#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <strophe.h>
#include <unistd.h>

#include "XMPPClient.h"
#include "cJSON.h"
#include "lsrpc.h"
#include "message.h"

#define MAX_ROSTER_NUMBER 100
#define HEARTBEAT_INTERVAL 600 // 60s

/* hardcoded TCP keepalive timeout and interval */
#define KA_TIMEOUT 60
#define KA_INTERVAL 1

typedef struct tag_ROSTER_ITEM {
	char pName[512];
	char pJID[512];
	char pSubscription[50];
} ROSTER_ITEM;

LOCAL OSA_ThrHndl Thread_xmppconnection = {0};

static ROSTER_ITEM stRosterList[MAX_ROSTER_NUMBER];
static int		   gTotalFriendNumber = 0;

// static char pRosterJID[512];

static xmpp_ctx_t*  g_ctx  = NULL;
static xmpp_conn_t* g_conn = NULL;
char*				g_jid  = NULL;

LOCAL INT s_iheartTimerHandle = -1;
LOCAL UINT8 s_U8ConnectLoop   = FALSE;

int handle_presence_reply(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
	const char* jid;

	printf("handle_presence_reply\n");

	if ((jid = xmpp_stanza_get_attribute(stanza, "to"))) {
		g_jid = xmpp_jid_bare(g_ctx, jid);
		printf("jid: %s\n", g_jid);
	}

	xmpp_handler_delete(g_conn, handle_presence_reply); // todo

	return 1;
}

int handle_roster_reply(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
	xmpp_stanza_t *query, *item;
	const char *   type, *name;
	int			   i;
	int			   iFriendNumber = 0;

	for (i = 0; i < MAX_ROSTER_NUMBER; i++) {
		memset(stRosterList[i].pName, 0, sizeof(stRosterList[i].pName));
		memset(stRosterList[i].pJID, 0, sizeof(stRosterList[i].pJID));
		memset(stRosterList[i].pSubscription, 0, sizeof(stRosterList[i].pSubscription));
	}

	type = xmpp_stanza_get_type(stanza);
	if (strcmp(type, "error") == 0)
		fprintf(stderr, "[roster]ERROR: query failed\n");
	else {
		query = xmpp_stanza_get_child_by_name(stanza, "query");
		printf("Roster:\n");
		for (item = xmpp_stanza_get_children(query); item; item = xmpp_stanza_get_next(item))
			if ((name = xmpp_stanza_get_attribute(item, "name"))) {
				printf("\t %s (%s) sub=%s\n", name, xmpp_stanza_get_attribute(item, "jid"), xmpp_stanza_get_attribute(item, "subscription"));
				if (iFriendNumber == MAX_ROSTER_NUMBER) {
					printf("Your Friend is more than %d, disgard...\n", iFriendNumber);
					return 0;
				}
				sprintf(stRosterList[iFriendNumber].pName, "%s", name);
				sprintf(stRosterList[iFriendNumber].pJID, "%s", xmpp_stanza_get_attribute(item, "jid"));
				sprintf(stRosterList[iFriendNumber].pSubscription, "%s", xmpp_stanza_get_attribute(item, "subscription"));
				iFriendNumber++;
			} else {
				printf("\t %s sub=%s\n", xmpp_stanza_get_attribute(item, "jid"), xmpp_stanza_get_attribute(item, "subscription"));

				if (iFriendNumber == MAX_ROSTER_NUMBER) {
					printf("Your Friend is more than %d, disgard...\n", iFriendNumber);
					return 0;
				}
				sprintf(stRosterList[iFriendNumber].pName, "%s", "Unknown Name");
				sprintf(stRosterList[iFriendNumber].pJID, "%s", xmpp_stanza_get_attribute(item, "jid"));
				sprintf(stRosterList[iFriendNumber].pSubscription, "%s", xmpp_stanza_get_attribute(item, "subscription"));
				iFriendNumber++;
			}
		gTotalFriendNumber = iFriendNumber;
		printf("END OF LIST\n");
	}

	if (strcmp(stRosterList[0].pName, "") != 0) {
		printf("\n=================Your Friend List=============\n");
		for (iFriendNumber = 0; iFriendNumber < gTotalFriendNumber; iFriendNumber++) {
			printf("Name:%s JID:%s Subscription:%s\n", stRosterList[iFriendNumber].pName, stRosterList[iFriendNumber].pJID,
				   stRosterList[iFriendNumber].pSubscription);
		}
		printf("\n==============================================\n");
	}

	return 1;
}

int handle_message_reply(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata)
{
	// xmpp_ctx_t *ctx = (xmpp_ctx_t*)userdata;
	xmpp_stanza_t* body;
	const char*	type;
	char*		   intext;

	body = xmpp_stanza_get_child_by_name(stanza, "body");
	if (body == NULL)
		return 0;
	type = xmpp_stanza_get_type(stanza);
	if (type != NULL && strcmp(type, "error") == 0)
		return 0;

	intext = xmpp_stanza_get_text(body);

	printf("Incoming message from %s: %s\n", xmpp_stanza_get_from(stanza), intext);

	xmpp_free(g_ctx, intext);

	return 1;
}
void xmpp_stanza_rpc_get_string(xmpp_stanza_t* const param, char** value, int* len)
{
	xmpp_stanza_t* item;
	*value = NULL;
	*len   = 0;
	item   = xmpp_stanza_get_child_by_name(param, "value");
	if (NULL != item) {
		item   = xmpp_stanza_get_child_by_name(item, "string");
		*value = xmpp_stanza_get_text(item);
		// xmpp_stanza_to_text(item, value, &len);
	}
	return;
}

int handle_rpc(xmpp_conn_t* const conn, xmpp_stanza_t* const stanza, void* const userdata) // lxy
{
	char *		   jid = NULL, *methodstr = NULL, *paramstr = NULL, *accessstr = NULL;
	cJSON *		   json = NULL, *jsondata = NULL;
	xmpp_stanza_t *query = NULL, *methodCall = NULL;
	xmpp_stanza_t *params = NULL, *param = NULL;
	int			   len;

	query = xmpp_stanza_get_child_by_name(stanza, "query");
	if (NULL != query) {
		methodCall = xmpp_stanza_get_child_by_name(query, "methodCall");
	}
	if (NULL != methodCall) {
		params = xmpp_stanza_get_child_by_name(methodCall, "params");
	}

	if ((NULL != params)) {
		param = xmpp_stanza_get_children(params);
		if (NULL != param) {
			xmpp_stanza_rpc_get_string(param, &methodstr, &len);
			param = xmpp_stanza_get_next(param);
			if (NULL != param) {
				xmpp_stanza_rpc_get_string(param, &paramstr, &len);
				if (NULL != param) {
					param = xmpp_stanza_get_next(param);
					if (NULL != param) {
						xmpp_stanza_rpc_get_string(param, &accessstr, &len);
					}
				}
			}
		}
	}
	jid = (char*) xmpp_stanza_get_from(stanza);

	printf("zzy incoming rpc message%s, %s, from=%s\n", methodstr, paramstr, jid);

	json = NULL;

	if ((NULL != jid) && (NULL != methodstr) && (NULL != paramstr)) {
		json = cJSON_CreateObject();
		cJSON_AddStringToObject(json, "method", methodstr);
		cJSON_AddStringToObject(json, "jid", jid);
		jsondata = cJSON_Parse(paramstr);
		cJSON_AddItemToObject(json, "data", jsondata);
		if (accessstr != NULL) {
			cJSON_AddStringToObject(json, "accesstoken", accessstr);
		}
		cJSON_AddNumberToObject(json, "queryid", 0);
		cJSON_AddNumberToObject(json, "remote", 1);

		ls_send_jsonmsg(json, SOCK_BACKEND_ID, SOCK_REMOTEXMPP_ID);
	}
	printf("send command to ls %s\n", cJSON_Print(json));
	if (json != NULL) {
		cJSON_Delete(json);
	}
	if (NULL != methodstr) {
		free(methodstr);
		methodstr = NULL;
	}
	if (NULL != paramstr) {
		free(methodstr);
		methodstr = NULL;
	}
	/*
	<iq id="2A7876DB-3595-401B-9244-DFBAB42A2DB6" type="set" to="87c83439-a316-4841-be04-7726791cbb55@xmpp.my-staging.busch-jaeger.de/device" lang="en"
from="28ede591-0102-46e7-b931-6633aedf63c8@xmpp.my-staging.busch-jaeger.de/ios"><query
xmlns="jabber:iq:rpc"><methodCall><methodName>CommonInterface.sendStringRequest</methodName><params><param><value><string>getAll</string></value></param><param><value><string>{
  &quot;mode&quot; : 2,
  &quot;language&quot; : &quot;en&quot;
}</string></value></param></params></methodCall></query></iq>
	*/
	//{"method":"getAll", "jid":"fdafafa","data":{"mode":2, "language":"en"}}
	return 1;
}

void fnXmppSendMessage(const char* const jid, const char* const text)
{
#if 0
	xmpp_stanza_t *pmsg;
	pmsg = xmpp_message_new(g_ctx, "chat", jid, "message_1");
	xmpp_message_set_body(pmsg, "abcdef");
	xmpp_send(g_conn, pmsg);
	xmpp_stanza_release(pmsg);
#endif
#if 0
	xmpp_stanza_t *iq, *query;

    /* create iq stanza for request */
    iq = xmpp_iq_new(g_ctx, "get", "e2e1");
	xmpp_stanza_set_to(iq, jid);
	xmpp_stanza_set_from(iq, "qxmpp@abb.com/device");
	
    query = xmpp_stanza_new(g_ctx);
    xmpp_stanza_set_name(query, "ping");
    xmpp_stanza_set_ns(query, XMPP_NS_PING);
	
    xmpp_stanza_add_child(iq, query);

    /* we can release the stanza since it belongs to iq now */
    xmpp_stanza_release(query);

    /* set up reply handler */
    //xmpp_id_handler_add(conn, handle_roster_reply, "roster_1", g_ctx);

    /* send out the stanza */
    xmpp_send(g_conn, iq);
	///printf("send ping message to %s,result=%d\n", jid, res);

    /* release the stanza */
    xmpp_stanza_release(iq);

#endif
#if 1
	/*
	<iq type='result'
		from='responder@company-a.com/jrpc-server'
		to='requester@company-b.com/jrpc-client'
		id='rpc1'>
	  <query xmlns='jabber:iq:rpc'>
		<methodResponse>
		  <params>
			<param>
			  <value><string>Colorado</string></value>
			</param>
		  </params>
		</methodResponse>
	  </query>
	</iq>

	*/
	// xmpp_stanza_t* pmsg = NULL;
	xmpp_stanza_t *piq, *ptext, *pstring, *pvalue, *pparam, *pparams, *pmethod, *pquery;

	printf("lxy into send xmpp message, jid=%s, msg=%s\n", jid, text);

	piq = xmpp_iq_new(g_ctx, "result", "message_1");

	ptext   = xmpp_stanza_new(g_ctx);
	pstring = xmpp_stanza_new(g_ctx);
	pvalue  = xmpp_stanza_new(g_ctx);
	pparam  = xmpp_stanza_new(g_ctx);
	pparams = xmpp_stanza_new(g_ctx);
	pmethod = xmpp_stanza_new(g_ctx);
	pquery  = xmpp_stanza_new(g_ctx);

	xmpp_stanza_set_to(piq, jid);
	// xmpp_stanza_set_from(piq, "qxmpp@abb.com/device");
	xmpp_stanza_set_ns(piq, "jabber:client");

	// printf(text);

	// xmpp_stanza_set_text(pstring, text);
	// xmpp_stanza_set_name(pstring, "string");
	// xmpp_stanza_add_child(pvalue, pstring);
	// xmpp_stanza_release(pstring);

	xmpp_stanza_set_text(ptext, text);
	// xmpp_stanza_set_name(ptext, "text");
	xmpp_stanza_add_child(pstring, ptext);
	xmpp_stanza_release(ptext);

	xmpp_stanza_set_name(pstring, "string");
	xmpp_stanza_add_child(pvalue, pstring);
	xmpp_stanza_release(pstring);

	xmpp_stanza_set_name(pvalue, "value");
	xmpp_stanza_add_child(pparam, pvalue);
	xmpp_stanza_release(pvalue);

	xmpp_stanza_set_name(pparam, "param");
	xmpp_stanza_add_child(pparams, pparam);
	xmpp_stanza_release(pparam);

	xmpp_stanza_set_name(pparams, "params");
	xmpp_stanza_add_child(pmethod, pparams);
	xmpp_stanza_release(pparams);

	xmpp_stanza_set_name(pmethod, "methodResponse");
	xmpp_stanza_add_child(pquery, pmethod);
	xmpp_stanza_release(pmethod);

	xmpp_stanza_set_name(pquery, "query");
	xmpp_stanza_set_ns(pquery, "jabber:iq:rpc");
	xmpp_stanza_add_child(piq, pquery);
	xmpp_stanza_release(pquery);

	xmpp_send(g_conn, piq);
	xmpp_stanza_release(piq);
#endif
}

char* fnXmppGetJID(void)
{
	return g_jid;
}

void fnXmppPresence(void)
{
	xmpp_stanza_t* pres;

	pres = xmpp_presence_new(g_ctx);
	xmpp_send(g_conn, pres);
	xmpp_stanza_release(pres);
}

void fnXmppGetRoster(void)
{
	xmpp_stanza_t *iq, *query;

	/* create iq stanza for request */
	iq	= xmpp_iq_new(g_ctx, "get", "roster_1");
	query = xmpp_stanza_new(g_ctx);
	xmpp_stanza_set_name(query, "query");
	xmpp_stanza_set_ns(query, XMPP_NS_ROSTER);

	xmpp_stanza_add_child(iq, query);

	/* we can release the stanza since it belongs to iq now */
	xmpp_stanza_release(query);

	/* set up reply handler */
	// xmpp_id_handler_add(conn, handle_roster_reply, "roster_1", g_ctx);

	/* send out the stanza */

	{
		// xmpp_send_queue_t *sq = gconn->send_queue_head;
		// int towrite = sq->len - sq->written;
		// printf("%d, %s",towrite, &sq->data[sq->written]);
	}

	xmpp_send(g_conn, iq);

	/* release the stanza */
	xmpp_stanza_release(iq);
}

void fnXmppPing(void)
{
	xmpp_stanza_t *iq, *query;

	/* create iq stanza for request */
	iq	= xmpp_iq_new(g_ctx, "get", "c2s1");
	query = xmpp_stanza_new(g_ctx);
	xmpp_stanza_set_name(query, "ping");
	xmpp_stanza_set_ns(query, XMPP_NS_PING);

	xmpp_stanza_add_child(iq, query);

	/* we can release the stanza since it belongs to iq now */
	xmpp_stanza_release(query);

	/* set up reply handler */
	// xmpp_id_handler_add(conn, handle_roster_reply, "roster_1", g_ctx);

	/* send out the stanza */
	xmpp_send(g_conn, iq);

	/* release the stanza */
	xmpp_stanza_release(iq);
}

LOCAL void HandleXmppHeartbeat(timer_id id, void* user_data, int len)
{
	printf("\n=================HandleXmppHeartbeat=============\n");
	// printf("conn DEBUG SENT: <iq id=\"s2c1\" type=\"get\"><query xmlns=\"urn:ietf:params:xml:ns:xmpp-ping\"/></iq>\n");
	fnXmppPing();
}

LOCAL void KillXmppHeartBeatTimer(void)
{
	printf("\n=================KillXmppHeartBeatTimer=============\n");

	OSA_KillTimer(s_iheartTimerHandle);
	// s_iHeartBeatSendCount = 1;
}

void conn_handler(xmpp_conn_t* const conn, const xmpp_conn_event_t status, const int error, xmpp_stream_error_t* const stream_error, void* const userdata)
{
	char* jid = NULL;
	g_ctx	 = (xmpp_ctx_t*) userdata;
	g_conn	= (xmpp_conn_t*) conn;

	OSA_DBG_MSG("***** conn_handler *****");

	if (status == XMPP_CONN_CONNECT) { // Ö»????Á¬?Ó³É¹?
		fprintf(stderr, "[roster]DEBUG: connected successfully\n");

		jid = (char*) xmpp_conn_get_bound_jid(g_conn);
		if (jid) {
			g_jid = xmpp_jid_bare(g_ctx, jid);
		}

		fnXmppPresence();

		fnXmppGetRoster();

		/* set up presence reply handler */
		// xmpp_handler_add(g_conn, handle_presence_reply, NULL, "presence", NULL, g_ctx);

		/* set up roster reply handler */
		xmpp_id_handler_add(g_conn, handle_roster_reply, "roster_1", g_ctx);

		/*Add Incoming message Deal Function*/
		xmpp_handler_add(g_conn, handle_message_reply, NULL, "message", NULL, g_ctx);

		xmpp_handler_add(g_conn, handle_rpc, "jabber:iq:rpc", NULL, NULL, g_ctx);

		/*????10s ??Ê±??????À´??????Ì¬??, ????Á¬??6??????????Ã»???Õµ?Ó¦??????Ã´nopoll_loop_stop ?á±»Ö´??*/
		OSA_DBG_MSG("OSA_SetTimer(HEARTBEAT_INTERVAL)");
		s_iheartTimerHandle = OSA_SetTimer(HEARTBEAT_INTERVAL, eTimerContinued, (timer_expiry*) HandleXmppHeartbeat, NULL);

	} else {

		fprintf(stderr, "DEBUG: disconnected\n");
		xmpp_stop(g_ctx);
	}
}

LOCAL VOID StartConnectWithXmppService(void)
{
	// xmpp_ctx_t *ctx;
	// xmpp_conn_t *conn;
	long		flags = 0;
	xmpp_log_t* log;

	printf("\nlibstrophe compile %s %s\n", __DATE__, __TIME__);

	while (s_U8ConnectLoop) {
		printf("xmpp init\n");
		/* initialize lib */
		xmpp_initialize();

#if 1
		/*Set Certificate add by shelly*/
		SetXMPPServerCertificatePath("/home/wipap/portal/certificate/ca.crt");
		SetXMPPClientCertificatePath("/home/wipap/portal/certificate/client.pem");
		SetXMPPClientPrivateKeyPath("/home/wipap/portal/certificate/private.key.pem");

		log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);

		/* create a context */
		g_ctx = xmpp_ctx_new(NULL, log);

		/* create a connection */
		g_conn = xmpp_conn_new(g_ctx);

		/*
		 * also you can disable TLS support or force legacy SSL
		 * connection without STARTTLS
		 *
		 * see xmpp_conn_set_flags() or examples/basic.c
		 */

		flags |= XMPP_CONN_FLAG_TRUST_TLS;
		// flags |= XMPP_CONN_FLAG_LEGACY_SSL;
		xmpp_conn_set_flags(g_conn, flags);
		/* configure TCP keepalive (optional) */
		xmpp_conn_set_keepalive(g_conn, KA_TIMEOUT, KA_INTERVAL);

		/* setup authentication information */
		char cJID[1024];
		OSA_MemSet(cJID, 0, sizeof(cJID));
		sprintf(cJID, "%s@%s/device", "user", "xmpp.my-staging.busch-jaeger.de");

		////  xmpp_conn_set_jid(g_conn, cJID);
		/// xmpp_conn_set_pass(g_conn, "");
		/// xmpp_connect_client(g_conn, NULL, 0, conn_handler, g_ctx);

		// sprintf(cJID, "%s@%s/device", "remoterpc", "abb.com");
		xmpp_conn_set_jid(g_conn, cJID);
		xmpp_conn_set_pass(g_conn, "123456");
		// xmpp_connect_client(g_conn, "127.0.0.1", 5222, conn_handler, g_ctx);
		xmpp_connect_client(g_conn, NULL, 0, conn_handler, g_ctx);

/* initiate connection */

#else
		/*Set Certificate add by shelly*/

		SetXMPPServerCertificatePath("/usr/portal/clientCerts/ca.crt");
		SetXMPPClientCertificatePath("/usr/portal/clientCerts/client.crt");
		SetXMPPClientPrivateKeyPath("/usr/portal/clientCerts/client.key");

		log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);

		/* create a context */
		g_ctx = xmpp_ctx_new(NULL, log);

		/* create a connection */
		g_conn = xmpp_conn_new(g_ctx);

		flags |= XMPP_CONN_FLAG_TRUST_TLS;
		flags |= XMPP_CONN_FLAG_LEGACY_SSL;
		xmpp_conn_set_flags(g_conn, flags);
		/* configure TCP keepalive (optional) */
		xmpp_conn_set_keepalive(g_conn, KA_TIMEOUT, KA_INTERVAL);

		/* setup authentication information */
		char cJID[1024];
		OSA_MemSet(cJID, 0, sizeof(cJID));
		sprintf(cJID, "%s@%s/device", "qxmpp", "abb.com"); // lxy
		// sprintf(cJID,"%s@%s","test","127.0.0.1");

		xmpp_conn_set_jid(g_conn, cJID);
		xmpp_conn_set_pass(g_conn, "123456");

		/* initiate connection */
		xmpp_connect_client(g_conn, "127.0.0.1", 5223, conn_handler, g_ctx);
//// xmpp_connect_client(g_conn, 192.168.0.28, 5223, conn_handler, g_ctx);
// xmpp_connect_client(g_conn, "192.168.0.28", 5223, conn_handler, g_ctx);
#endif

		/* start the event loop */
		xmpp_run(g_ctx);

		KillXmppHeartBeatTimer();

		/* disconnect */
		xmpp_disconnect(g_conn);

		/* release our connection and context */
		xmpp_conn_release(g_conn);
		xmpp_ctx_free(g_ctx);

		/* shutdown lib */
		xmpp_shutdown();
		printf("xmpp_shutdown\n");

		// SetXmppClientConnectStatus(XMPP_CLIENT_STATUS_CONNECT_FAIL);
	}
}

VOID fnDeleteXmppClientReceiveThread(VOID)
{
	s_U8ConnectLoop = FALSE;

	KillXmppHeartBeatTimer();

	xmpp_stop(g_ctx);

	/* disconnect */
	xmpp_disconnect(g_conn);

	/* release our connection and context */
	xmpp_conn_release(g_conn);
	xmpp_ctx_free(g_ctx);

	g_conn = NULL;
	g_ctx  = NULL;

	/* shutdown lib */
	xmpp_shutdown();
	printf("xmpp_shutdown\n");

	OSA_ThrDelete(&Thread_xmppconnection);
}

VOID fnCreateXmppClientReceiveThread(VOID)
{
	s_U8ConnectLoop = TRUE;

	int iRet = OSA_ThrCreate(&Thread_xmppconnection, (void*) StartConnectWithXmppService, OSA_THR_PRI_FIFO_MIN, NULL);

	if (iRet != OSA_SOK) {
		OSA_ERROR("%sCreate Xmpp Client Receive Thread Error!\n", DEBUG_HEADER_XMPPCLIENT);
		exit(1);
	}
}
