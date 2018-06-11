#include "mod_lsapp.h"
#include "XMPPClient.h"
#include "events.h"
#include "mod_time.h"
#include "module.h"
#include <curl.h>
/*
00 00 00 00 00 00 B6 73 00 00 00 01 00 00 00 00 65 49 20 20 20 20 61 63 20 6C 6F 67 69 63 21 01 00 00 00 86 10 00 00 00 13 73 65 74 44 65 76 69 63 65 41 74 74
72 69 62 75 74 65 73 0F 00 00 00 55 5B 7B 0A 09 09 22 70 61 74 68 22 3A 09 22 31 31 30 31 30 30 30 30 30 30 30 30 45 46 2F 63 68 30 30 30 30 2F 6F 64 70 30 30
30 30 22 2C 0A 09 09 22 61 74 74 72 69 62 75 74 65 73 22 3A 09 7B 0A 09 09 09 22 76 61 6C 75 65 22 3A 09 32 0A 09 09 7D 0A 09 7D 5D

*/
#define SERVER_URL "120.24.226.63/test/pair.php?act=%s&wipapsn=%s&cloudaccount=%s&jid=%s&localaccount=%s"
void rpc_postevent(void* msg, U32 msgid, char* module)
{
}

void lsapp_rpc_remoteInterface(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
void lsapp_rpc_savePairingToPortal(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);

const STRLSRPCType lsappRPCFun[] = {
	{"remoteInterface", lsapp_rpc_remoteInterface}, {"savePairingToPortal", lsapp_rpc_savePairingToPortal},
};

static void lsapp_init(void)
{
	printf("call back func=%p\n", lsappRPCFun);
	ls_init(lsappRPCFun, 2);
}

static void lsapp_uninit(void)
{
}

static void lsapp_realtime(U32 semid, U32 moduleid, STREventElemType* buf)
{
}

static void lsapp_clear(void)
{
}
void lsapp_rpc_remoteInterface(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	/*
	cJSON *json;
	char *attributes = ls_getParamString(params, 0);
	printf("zzy receive lsapp method params=%s\n", attributes);
	json = cJSON_Parse(attributes);
	printf("zzy receive jsonattr attributes=%s\n", cJSON_Print(json));
	db_set_device_attr(json, NULL, NULL, "ac@device");
	*/
}

BOOL curl_do(int action, char* url)
{
	int		 retry_num = 0;
	CURL*	curl_handle;
	CURLcode res;
	BOOL	 ret = FALSE;

	/* multi-thread unsafe.
	 * will be called by curl_easy_init
	 */
	// curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	printf("begin to post data url=%s\n", url);
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all buff to this function  */
	// curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	/// curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent
		field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "wipap-agent/1.0");

	/* accept: application/json */
	curl_easy_setopt(curl_handle, CURLOPT_ENCODING, "application/json");

	/// curl_easy_setopt(curl_handle, CURLOPT_USERPWD, auth_buf);

	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 3L);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);
	/* for time_wait */
	curl_easy_setopt(curl_handle, CURLOPT_FORBID_REUSE, 1);

retry:

	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK) {
		if (retry_num < 3) {
			retry_num++;
			goto retry;
		}

	} else {
		long http_code = 0;
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
		if ((http_code == 200) && (res != CURLE_ABORTED_BY_CALLBACK)) {
			ret = TRUE;
		} else {
		}
	}

	curl_easy_cleanup(curl_handle);

	return ret;
}
void lsapp_rpc_sendMessageToRemote(cJSON* attr, char* jid, char* method)
{
	cJSON *jsontojid, *out;
	char*  tojid;
	char*  msg;
	tojid	 = NULL;
	jsontojid = cJSON_GetObjectItem(attr, "tojid");
	if ((jsontojid != NULL) && (jsontojid->valuestring != NULL)) {
		tojid = jsontojid->valuestring;
	}
	if (NULL != tojid) {
		out = cJSON_CreateObject();
		cJSON_AddStringToObject(out, "jid", jid);
		cJSON_AddStringToObject(out, "method", method); //,name,s)
		cJSON_AddItemReferenceToObject(out, "data", attr);
		msg = cJSON_Print(out);
		fnXmppSendMessage(tojid, msg);
		if (NULL != msg) {
			free(msg);
			msg = NULL;
		}
		if (NULL != out) {
			cJSON_Delete(out);
		}
	}
}
void lsapp_rpc_savePairingToPortal(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source)
{
	char   path[512];
	char * wipap, *cloudaccount, *jid, *localaccount, *act;
	cJSON *json, *item, *out;
	wipap			 = NULL;
	cloudaccount	 = NULL;
	jid				 = NULL;
	localaccount	 = NULL;
	act				 = NULL;
	BOOL  ret		 = FALSE;
	char* attributes = ls_getParamString(params, 0);
	json			 = cJSON_Parse(attributes);
	if (NULL != json) {
		item = cJSON_GetObjectItem(json, "wipap");
		if ((NULL != item) && (NULL != item->valuestring)) {
			wipap = item->valuestring;
		}
		item = cJSON_GetObjectItem(json, "cloudaccount");
		if ((NULL != item) && (NULL != item->valuestring)) {
			cloudaccount = item->valuestring;
		}
		item = cJSON_GetObjectItem(json, "jid");
		if ((NULL != item) && (NULL != item->valuestring)) {
			jid = item->valuestring;
		}

		item = cJSON_GetObjectItem(json, "localaccount");
		if ((NULL != item) && (NULL != item->valuestring)) {
			localaccount = item->valuestring;
		}

		item = cJSON_GetObjectItem(json, "act");
		if ((NULL != item) && (NULL != item->valuestring)) {
			act = item->valuestring;
		}
	}

	path[0] = 0;

	out = cJSON_CreateObject();
	printf("save to wipap=%s, cloudaccount=%s, localaccount=%s, jid=%s, act=%s\n", wipap, cloudaccount, localaccount, jid, act);
	if ((NULL != wipap) && (NULL != cloudaccount) && (NULL != localaccount) && (NULL != jid) && (NULL != act)) {
		if ((0 == strcmp(act, "connect")) || (0 == strcmp(act, "disconnect"))) {
			snprintf(path, sizeof(path), SERVER_URL, act, wipap, cloudaccount, jid, localaccount);
			ret = curl_do(0, path);
			cJSON_AddStringToObject(out, "cloudaccount", cloudaccount);
			cJSON_AddStringToObject(out, "jid", jid);
			cJSON_AddStringToObject(out, "wipap", wipap);
		}
		if (0 == strcmp(act, "ackcode")) {
			snprintf(path, sizeof(path), SERVER_URL, act, wipap, cloudaccount, jid, localaccount);
			ret = curl_do(0, path);
			lsapp_rpc_sendMessageToRemote(json, jid, "requestRemotePairing");
		}
	}
	cJSON_AddNumberToObject(out, "result", ret);

	ls_sendmsg("savePairingToPortalACK", out, SOCK_BACKEND_ID, SOCK_REMOTEXMPP_ID); //, char *did, char *sid)
	printf("zzy receive lsapp method savePairingToPortal params=%s, %p, %p\n", attributes, attributes, json);
	if (NULL != out) {
		cJSON_Delete(out);
	}
	if (NULL != json) {
		cJSON_Delete(json);
	}
}

void lsapp_rpc_requestData(struct tag_CommParams* params)
{
	// char *module = NULL;
	// char *datatype = NULL;
}
static void lsapp_setlocaltime(U64 time)
{
}
static U32 lsapp_getperiod(void)
{
	return (0);
}

MODULE_INFO(MD_LSAPP_NAME, MD_LSAPP_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_BACKEND, SEM_ID_THREAD_MAIN, 0), lsapp_init, lsapp_realtime, lsapp_clear, lsapp_uninit,
			lsapp_getperiod, lsapp_setlocaltime);
