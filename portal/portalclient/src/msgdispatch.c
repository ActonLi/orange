#include "msgdispatch.h"
#include "cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

typedef struct tag_MsgDispatch_Node {
	U32   id;
	void* method;
	BYTE  matchtype;
	void (*function)(void* msg, U32 msgid);
	AST_LIST_ENTRY(tag_MsgDispatch_Node) list;
} MSGDISPATCH_NODE;
static AST_LIST_HEAD_NOLOCK_STATIC(reqlist, tag_MsgDispatch_Node);
int msgdispatch_register(MSGDISPATCH_REQ_TYPE* req, int count)
{
	int				  res = -1;
	int				  i;
	BOOL			  found = FALSE;
	MSGDISPATCH_NODE* node  = NULL;
	DEBUGP("msgdispatch register count=%d\n", count);
	for (i = 0; i < count; i++) {
		found = FALSE;
		if (AST_LIST_EMPTY(&reqlist)) {
		}

		else {
			AST_LIST_TRAVERSE(&reqlist, node, list)
			{
				if ((node != NULL) && (node->id == req[i].id) && (node->method == req[i].method) && (node->matchtype == req[i].matchtype) &&
					(node->function == req[i].function)) {
					found = TRUE;
				}
			}
		}

		if (found == FALSE) {
			node = malloc(sizeof(MSGDISPATCH_NODE));

			if (node != NULL) {
				node->id		= req[i].id;
				node->method	= req[i].method;
				node->matchtype = req[i].matchtype;
				node->function  = req[i].function;
				node->list.next = NULL;
				AST_LIST_INSERT_TAIL(&reqlist, node, list);
				res = 0;
			}
		}
	}
	return (res);
}

int msgdispatch_unregister(MSGDISPATCH_REQ_TYPE* req, int count)
{
	int				  res = -1;
	int				  i;
	BOOL			  found = FALSE;
	MSGDISPATCH_NODE* node  = NULL;
	for (i = 0; i < count; i++) {
		if (AST_LIST_EMPTY(&reqlist)) {
		} else {
			AST_LIST_TRAVERSE(&reqlist, node, list)
			{
				if ((node != NULL) && (node->id == req[i].id) && (node->method == req[i].method) && (node->matchtype == req[i].matchtype) &&
					(node->function == req[i].function)) {
					found = TRUE;
					break;
				}
			}
		}
		if (TRUE == found) {
			AST_LIST_REMOVE(&reqlist, node, list);
			free(node);
			res = 0;
		}
	}
	return (res);
}

#define RPC_FUN_NAME_MAXLEN 256

void GetJsonString(cJSON* json, char* outstring, int max_len)
{
	char* string;
	int   templen = 0;
	if (outstring == NULL || json == NULL) {
		return;
	}
	string  = cJSON_Print(json);
	templen = (strlen(string) - 2) > max_len ? max_len : (strlen(string) - 2);
	memcpy(outstring, string + 1, templen);
	if (string != NULL)
		free(string);
}

int msgdispatch_process(void* msg, int length, U32 msgid)
{
	MSG_DATA_HEADER*  header					   = NULL;
	MSGDISPATCH_NODE* node						   = NULL;
	cJSON*			  json						   = NULL;
	cJSON*			  rpcnameobj				   = NULL;
	char			  rpcname[RPC_FUN_NAME_MAXLEN] = {0};
	int				  res						   = -1;
	char*			  payload					   = NULL;
	payload										   = ((char*) msg) + MSG_HEADER_LENTH;
	header										   = (MSG_DATA_HEADER*) msg;
	memset(rpcname, 0, RPC_FUN_NAME_MAXLEN);
	DEBUGP("msgdispatch_process %d, %d\n", header->funcCode, payload);
	if (header->funcCode == MSG_FUNCODE_LOGIC_JSON) {
		printf("msgdispatch_process 1\n");
		json = cJSON_Parse(payload);
		if (json != NULL) {
			printf("json valied\n");
			rpcnameobj = cJSON_GetObjectItem(json, "method");
			if ((rpcnameobj != NULL) && (rpcnameobj->valuestring != NULL)) {
				snprintf(rpcname, RPC_FUN_NAME_MAXLEN, "%s", rpcnameobj->valuestring);
				DEBUGP("receive rpcname=%s\n", rpcname);
			}
		}
		if (json != NULL) {
			cJSON_Delete(json);
		}
	}

	if (AST_LIST_EMPTY(&reqlist)) {
	} else {
		AST_LIST_TRAVERSE(&reqlist, node, list)
		{
			if (node != NULL) {
				if (node->matchtype == MSGDISPATCH_MATCHTYPE_JSON) {
					if (0 == strcmp(rpcname, node->method)) {

						*((char*) msg + length) = 0;
						(node->function)(msg, msgid);
						res = 0;
					}
				}
				if (node->matchtype == MSGDISPATCH_MATCHTYPE_BINARY) {
					if ((header->funcCode == MSG_FUNCODE_LOGIC_BINARY) && (node->id == (U32) header->operCode)) {
						printf("zzy get ls rpc=%d\n", node->id);
						(node->function)(msg, msgid);
						res = 0;
					}
				}
			}
		}
	}
	return (res);
}
