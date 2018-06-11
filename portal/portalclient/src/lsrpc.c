#include "lsrpc.h"
#include "cJSON.h"
#include "message.h"
#include "msgdispatch.h"
#include "queue.h"
#include "remotesocket.h"
#include "tree.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

STRLSRPCType const* ls_rpcfuns   = NULL;
U32					ls_rpcfunnum = 0;
#define WATCHDOG_OPERATE_CODE 0x01
void ls_handleMsg(void* msg, U32 msgid);

typedef struct ls_rpc_entry {
	int  len;
	char method[LS_METHOD_LEN_MAX];
	void (*function)(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
	RB_ENTRY(ls_rpc_entry) entry;
} ls_rpc_entry_t;

static inline int __ls_rpc_entry_cmp(struct ls_rpc_entry* a, struct ls_rpc_entry* b);
RB_HEAD(ls_rpc_tree, ls_rpc_entry);
RB_PROTOTYPE(ls_rpc_tree, ls_rpc_entry, entry, __ls_rpc_entry_cmp);
RB_GENERATE(ls_rpc_tree, ls_rpc_entry, entry, __ls_rpc_entry_cmp);

static struct ls_rpc_tree rpc_tree;
static int				  init = 0;

typedef struct ls_rpc_msg_node {
	U8  type;
	U32 len;
	U8* value;
	TAILQ_ENTRY(ls_rpc_msg_node) list;
} ls_rpc_msg_node_t;

typedef struct ls_rpc_msg {
	U8  value_count;
	U32 value_size;
	TAILQ_HEAD(ls_rpc_msg_head, ls_rpc_msg_node) list_head;
} ls_rpc_msg_t;

MSGDISPATCH_REQ_TYPE ls_recMsg_callback_reqs[] = {
	{WATCHDOG_OPERATE_CODE, "", MSGDISPATCH_MATCHTYPE_BINARY, ls_handleMsg},

};

static inline int __ls_rpc_entry_cmp(struct ls_rpc_entry* a, struct ls_rpc_entry* b)
{
	if (a->len > b->len) {
		return 1;
	}

	if (a->len < b->len) {
		return -1;
	}

	return memcmp(a->method, b->method, a->len);
}

static struct ls_rpc_entry* __ls_rpc_entry_zalloc(void)
{
	struct ls_rpc_entry* entry = NULL;

	entry = malloc(sizeof(struct ls_rpc_entry));
	if (entry) {
		memset(entry, 0, sizeof(struct ls_rpc_entry));
	}

	return entry;
}

static struct ls_rpc_entry* __ls_rpc_entry_find(char* method, int len)
{
	struct ls_rpc_entry cmp;

	memset(&cmp, 0, sizeof(struct ls_rpc_entry));
	cmp.len = len;
	memcpy(cmp.method, method, len);

	return RB_FIND(ls_rpc_tree, &rpc_tree, &cmp);
}

static void __ls_rpc_entry_free(struct ls_rpc_entry* entry)
{
	if (entry) {
		free(entry);
	}

	return;
}

static int __ls_rpc_entry_insert(const STRLSRPCType* rpcfunc)
{
	int					 ret   = -1;
	struct ls_rpc_entry* entry = NULL;
	int					 len   = strlen(rpcfunc->method);

	entry = __ls_rpc_entry_find(rpcfunc->method, len);
	if (entry) {
		ret   = 0;
		entry = NULL;
		goto exit;
	}

	entry = __ls_rpc_entry_zalloc();
	if (NULL == entry) {
		goto exit;
	}

	entry->len = len;
	memcpy(entry->method, rpcfunc->method, len);

	if (NULL != RB_INSERT(ls_rpc_tree, &rpc_tree, entry)) {
		goto exit;
	}
	entry->function = rpcfunc->function;

	ret = 0;
exit:
	if (ret != 0 && entry != NULL) {
		__ls_rpc_entry_free(entry);
	}
	return ret;
}

static int __ls_rpc_init(void)
{
	RB_INIT(&rpc_tree);
	msgdispatch_register(ls_recMsg_callback_reqs, sizeof(ls_recMsg_callback_reqs) / sizeof(MSGDISPATCH_REQ_TYPE));
	return 0;
}

void ls_rpc_register(const STRLSRPCType* rpcfuns, U32 funnum)
{
	int					i;
	const STRLSRPCType* rpc;

	if (!init) {
		__ls_rpc_init();
		init = 1;
	}

	for (i = 0; i < funnum; i++) {
		rpc = rpcfuns + i;
		__ls_rpc_entry_insert(rpc);
	}
}

void ls_init(const STRLSRPCType* rpcfuns, U32 funnum)
{
	if (NULL != rpcfuns && funnum > 0) {
		ls_rpc_register(rpcfuns, funnum);
	}

	return;
}

void ls_delete(struct tag_CommParams* params)
{
	struct tag_CommParams *item, *node;
	node = params;
	while (NULL != node) {
		if (node->valuestring != NULL) {
			free(node->valuestring);
			node->valuestring = NULL;
		}
		item = node;
		node = node->next;
		free(item);
		item = NULL;
	}
}
void ls_parseMsg(U8* payload, U32 msglen, struct tag_CommParams** params)
{
	struct tag_CommParams *paramnode = NULL, *paramlast = NULL;
	U32					   pointer, len, type;
	int					   value, i;
	*params = NULL;

	pointer = 0;
	while ((5 + pointer) < msglen) {
		type = payload[pointer];

		len = MAKEFOURCC_BE4(payload[pointer + 1], payload[pointer + 2], payload[pointer + 3], payload[pointer + 4]);

		if ((5 + pointer + len) > msglen) {
			break;
		} else {
			paramnode			   = (struct tag_CommParams*) malloc(sizeof(struct tag_CommParams));
			paramnode->next		   = NULL;
			paramnode->datatype	= type;
			paramnode->datalen	 = len;
			paramnode->valuestring = NULL;
			if ((type == COMM_PARAM_TYPE_JSON) || (type == COMM_PARAM_TYPE_METHOD) || (type == COMM_PARAM_TYPE_STRING16) ||
				(type == COMM_PARAM_TYPE_STRUCTURE)) {
				paramnode->valuestring = (char*) malloc((len + 1) * sizeof(char));
				if (NULL != paramnode->valuestring) {
					memcpy(paramnode->valuestring, &payload[5 + pointer], len);
					paramnode->valuestring[len] = 0;
					paramnode->valueint			= 0;
				}
			} else {
				value = 0;
				for (i = 0; i < len; i++) {
					value = value << 8;
					value |= payload[pointer + 5 + i];
				}
				paramnode->valueint	= value;
				paramnode->valuestring = NULL;
			}
			if (NULL == *params) {
				*params   = paramnode;
				paramlast = *params;
			} else {
				paramlast->next = paramnode;
				paramlast		= paramnode;
			}
		}
		pointer = (pointer + 5 + len);
	}
}

char* ls_getMethod(struct tag_CommParams* params)
{
	if ((NULL != params) && (NULL != params->valuestring)) {
		return (params->valuestring);
	} else {
		return (NULL);
	}
}
void ls_dispatchMsg(U32 msgid, MSG_DATA_HEADER* source, U8* payload, U32 msglen)
{
	struct tag_CommParams* params	 = NULL;
	char*				   methodname = NULL;
	struct ls_rpc_entry*   entry	  = NULL;
	int					   len		  = 0;

	ls_parseMsg(payload, msglen, &params);
	methodname = ls_getMethod(params);
	if (NULL != methodname) {
		len   = strlen(methodname);
		entry = __ls_rpc_entry_find(methodname, len);
		if (entry) {
			entry->function(params, msgid, source);
		}
	}
	ls_delete(params);
}

void ls_handleMsg(void* msg, U32 msgid)
{
	U8*				 payload;
	U32				 msglen;
	MSG_DATA_HEADER* header;

	printf("%s:%d msg: %p\n", __func__, __LINE__, msg);

	payload = (U8*) msg + MSG_HEADER_LENTH;
	header  = (MSG_DATA_HEADER*) msg;
	msglen  = MAKEFOURCC_BE4(header->dataLen[0], header->dataLen[1], header->dataLen[2], header->dataLen[3]);
	ls_dispatchMsg(msgid, header, payload, msglen - MSG_HEADER_LENTH);
	return;
}

U32 ls_getParamNumber(struct tag_CommParams* params, U32 paramind)
{
	U32					   pointer = 0;
	U32					   res	 = 0;
	struct tag_CommParams* item;
	item = params;
	while (item != NULL) {
		if (pointer > paramind) {
			res = item->valueint;
			break;
		}
		pointer++;
		item = item->next;
	}
	return (res);
}

char* ls_getParamString(struct tag_CommParams* params, U32 paramid)
{
	U32					   pointer = 0;
	char*				   res	 = NULL;
	struct tag_CommParams* item;
	item = params;
	while (item != NULL) {
		if (pointer > paramid) {
			res = item->valuestring;
			break;
		}
		pointer++;
		item = item->next;
	}
	return res;
}

void* ls_getParamBinary(struct tag_CommParams* params, U32 paramid)
{
	return (void*) ls_getParamString(params, paramid);
}

U32 ls_generate_binary_msg(char* method, int data_size, void* data, char** msg)
{
	U32 len = 0;
	int methodlen, paramlen;
	*msg = NULL;
	if (NULL != data) {
		methodlen   = strlen(method);
		paramlen	= data_size;
		len			= methodlen + paramlen + 2 + 8;
		*msg		= (char*) malloc(len * sizeof(char));
		*(*msg + 0) = COMM_PARAM_TYPE_METHOD;
		*(*msg + 1) = (U8)(methodlen >> 24);
		*(*msg + 2) = (U8)(methodlen >> 16);
		*(*msg + 3) = (U8)(methodlen >> 8);
		*(*msg + 4) = (U8) methodlen;
		memcpy((*msg + 5), method, methodlen);
		*(*msg + 5 + methodlen) = COMM_PARAM_TYPE_STRUCTURE;
		*(*msg + 6 + methodlen) = (U8)(paramlen >> 24);
		*(*msg + 7 + methodlen) = (U8)(paramlen >> 16);
		*(*msg + 8 + methodlen) = (U8)(paramlen >> 8);
		*(*msg + 9 + methodlen) = (U8) paramlen;
		memcpy((*msg + 10 + methodlen), data, paramlen);
	}
	return (len);
}

U32 ls_generate_msg(char* method, cJSON* attrs, char** msg)
{
	U32   len = 0;
	int   methodlen, paramlen;
	char* jsonstr = cJSON_Print(attrs);
	*msg		  = NULL;
	if (NULL != jsonstr) {
		methodlen   = strlen(method);
		paramlen	= strlen(jsonstr);
		len			= methodlen + paramlen + 2 + 8;
		*msg		= (char*) malloc((methodlen + paramlen + 2 + 8) * sizeof(char));
		*(*msg + 0) = COMM_PARAM_TYPE_METHOD;
		*(*msg + 1) = (U8)(methodlen >> 24);
		*(*msg + 2) = (U8)(methodlen >> 16);
		*(*msg + 3) = (U8)(methodlen >> 8);
		*(*msg + 4) = (U8) methodlen;
		memcpy((*msg + 5), method, methodlen);
		*(*msg + 5 + methodlen) = COMM_PARAM_TYPE_JSON;
		*(*msg + 6 + methodlen) = (U8)(paramlen >> 24);
		*(*msg + 7 + methodlen) = (U8)(paramlen >> 16);
		*(*msg + 8 + methodlen) = (U8)(paramlen >> 8);
		*(*msg + 9 + methodlen) = (U8) paramlen;
		memcpy((*msg + 10 + methodlen), jsonstr, paramlen);
		free(jsonstr);
		jsonstr = NULL;
	}
	return (len);
}

BOOL ls_send_binary_msg(char* method, int data_size, void* data, char* did, char* sid)
{
	BOOL  ret = FALSE;
	U32   datalen;
	char* msg;
	datalen = ls_generate_binary_msg(method, data_size, data, &msg);
	remotesocket_sendmsg(did, sid, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, datalen);
	if (NULL != msg) {
		free(msg);
		msg = NULL;
		ret = TRUE;
	}

	return ret;
}

struct ls_rpc_msg* ls_rpc_msg_create(void)
{
	struct ls_rpc_msg* msg = NULL;
	msg					   = malloc(sizeof(struct ls_rpc_msg));
	if (msg) {
		msg->value_count = 0;
		msg->value_size  = 0;
		TAILQ_INIT(&(msg->list_head));
	}

	return msg;
}

static int __ls_rpc_msg_append(U8 type, U32 data_size, U8* data, struct ls_rpc_msg* msg)
{
	int						ret  = -1;
	struct ls_rpc_msg_node* node = NULL;

	node = malloc(sizeof(struct ls_rpc_msg_node));
	if (node == NULL) {
		goto exit;
	}

	node->type  = type;
	node->len   = data_size;
	node->value = NULL;

	node->value = malloc(node->len);
	if (node->value == NULL) {
		free(node);
		goto exit;
	}

	memcpy(node->value, data, node->len);
	TAILQ_INSERT_TAIL(&(msg->list_head), node, list);

	msg->value_count++;
	msg->value_size += node->len;

	ret = msg->value_count;

exit:
	return ret;
}

static int __ls_rpc_msg_append_json(U8 type, U32 data_size, void* data, struct ls_rpc_msg* msg)
{
	int	ret	 = -1;
	cJSON* attrs   = NULL;
	char*  jsonstr = NULL;

	if (data == NULL || msg == NULL) {
		goto exit;
	}

	attrs = (cJSON*) data;

	jsonstr = cJSON_Print(attrs);
	if (jsonstr == NULL) {
		goto exit;
	}

	int paramlen = strlen(jsonstr);

	ret = __ls_rpc_msg_append(type, paramlen, (U8*) jsonstr, msg);
exit:
	if (jsonstr) {
		free(jsonstr);
	}
	return ret;
}

static int __ls_rpc_msg_append_method(U8 type, U32 data_size, void* data, struct ls_rpc_msg* msg)
{
	int   ret	= -1;
	char* method = (char*) data;

	if (data == NULL || msg == NULL) {
		goto exit;
	}

	if (msg->value_count > 0) {
		goto exit;
	}

	ret = __ls_rpc_msg_append(type, data_size, (U8*) method, msg);

exit:
	return ret;
}

static int __ls_rpc_msg_append_binary(U8 type, U32 data_size, void* data, struct ls_rpc_msg* msg)
{
	int ret = -1;
	U8* bin = (U8*) data;

	if (data == NULL || msg == NULL) {
		goto exit;
	}

	ret = __ls_rpc_msg_append(type, data_size, bin, msg);

exit:
	return ret;
}

int ls_rpc_msg_append(U8 type, U32 data_size, void* data, struct ls_rpc_msg* msg)
{
	int ret = -1;

	switch (type) {
		case COMM_PARAM_TYPE_JSON:
			ret = __ls_rpc_msg_append_json(type, data_size, data, msg);
			break;
		case COMM_PARAM_TYPE_METHOD:
			ret = __ls_rpc_msg_append_method(type, data_size, data, msg);
			break;
		case COMM_PARAM_TYPE_STRUCTURE:
		case COMM_PARAM_TYPE_INT32:
			ret = __ls_rpc_msg_append_binary(type, data_size, data, msg);
			break;
		default:
			break;
	}

	return ret;
}

int ls_rpc_msg_update(U8 type, U32 data_size, void* data, U8 index, struct ls_rpc_msg* msg)
{
	struct ls_rpc_msg_node* node   = NULL;
	struct ls_rpc_msg_node* update = NULL;
	int						count  = 0;

	TAILQ_FOREACH(node, &(msg->list_head), list)
	{
		if ((++count) == index) {
			update = node;
			break;
		}
	}

	if (update) {
		if (update->value) {
			free(update->value);
			update->value = NULL;
		}

		update->type = type;

		if (update->type == COMM_PARAM_TYPE_JSON) {
			cJSON* attrs   = (cJSON*) data;
			char*  jsonstr = cJSON_Print(attrs);
			if (jsonstr == NULL) {
				return -1;
			}

			update->len   = strlen(jsonstr);
			update->value = malloc(update->len);
			if (update->value) {
				memcpy(update->value, (U8*) jsonstr, update->len);
				free(jsonstr);
			}
		} else {
			update->len   = data_size;
			update->value = malloc(update->len);
			if (update->value) {
				memcpy(update->value, (U8*) data, update->len);
			}
		}
	}

	return 0;
}

void ls_rpc_msg_destroy(struct ls_rpc_msg* msg)
{
	struct ls_rpc_msg_node* node = NULL;
	struct ls_rpc_msg_node* tmp  = NULL;

	if (msg) {
		TAILQ_FOREACH_SAFE(node, &(msg->list_head), list, tmp)
		{
			if (node) {
				TAILQ_REMOVE(&(msg->list_head), node, list);
				if (node->value) {
					free(node->value);
				}
				free(node);
			}
		}
		free(msg);
	}

	return;
}

static int __ls_rpc_append_node_to_send_msg(U8* msg, struct ls_rpc_msg_node* node)
{
	if (msg == NULL || node == NULL) {
		return -1;
	}

	*(msg + 0) = node->type;
	*(msg + 1) = (U8)(node->len >> 24);
	*(msg + 2) = (U8)(node->len >> 16);
	*(msg + 3) = (U8)(node->len >> 8);
	*(msg + 4) = (U8) node->len;

	if (node->type == COMM_PARAM_TYPE_INT32) {
		U32 valueint = *(U32*) (node->value);
		*(msg + 5)   = (U8)(valueint >> 24);
		*(msg + 6)   = (U8)(valueint >> 16);
		*(msg + 7)   = (U8)(valueint >> 8);
		*(msg + 8)   = (U8) valueint;
	} else {
		memcpy(msg + 5, node->value, node->len);
	}

	return 0;
}

int ls_rpc_send_msg(struct ls_rpc_msg* msg, char* did, char* sid)
{
	U32						send_msg_size = 0;
	U8*						send_msg	  = NULL;
	U8*						p_send_msg	= NULL;
	struct ls_rpc_msg_node* node		  = NULL;

	if (msg == NULL) {
		return -1;
	}

	send_msg_size = msg->value_size + msg->value_count * (1 + 4);
	send_msg	  = malloc(send_msg_size);
	if (send_msg == NULL) {
		return -1;
	}

	p_send_msg = send_msg;

	TAILQ_FOREACH(node, &(msg->list_head), list)
	{
		if (node) {
			if (0 != __ls_rpc_append_node_to_send_msg(p_send_msg, node)) {
				break;
			}
			p_send_msg += (node->len + 5);
		}
	}

	remotesocket_sendmsg(did, sid, MSG_FUNCODE_LOGIC_BINARY, 1, send_msg, send_msg_size);
	free(send_msg);
	return 0;
}

int ls_rpc_msg_dump(struct ls_rpc_msg* msg)
{
	U32						send_msg_size = 0;
	U8*						send_msg	  = NULL;
	U8*						p_send_msg	= NULL;
	struct ls_rpc_msg_node* node		  = NULL;

	if (msg == NULL) {
		return -1;
	}

	send_msg_size = msg->value_size + msg->value_count * (1 + 4);
	send_msg	  = malloc(send_msg_size);
	if (send_msg == NULL) {
		return -1;
	}

	p_send_msg = send_msg;

	TAILQ_FOREACH(node, &(msg->list_head), list)
	{
		if (node) {
			if (0 != __ls_rpc_append_node_to_send_msg(p_send_msg, node)) {
				break;
			}
			p_send_msg += (node->len + 5);
		}
	}

	int i;
	printf("%s:%d msg_size: %d\nmsg:", __func__, __LINE__, send_msg_size);
	for (i = 0; i < send_msg_size; i++) {
		printf(" %02x", *(send_msg + i));
	}
	printf("\n");

	free(send_msg);
	return 0;
}

BOOL ls_sendmsgAck(U32 msgid, char* method, cJSON* attrs, char* did, char* sid)
{
	U32   datalen;
	char* msg;
	datalen = ls_generate_msg(method, attrs, &msg);
	remotesocket_sendmsgAck(msgid, did, sid, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, datalen);
	if (NULL != msg) {
		free(msg);
		msg = NULL;
	}

	return TRUE;
}

BOOL ls_sendmsg(char* method, cJSON* attrs, char* did, char* sid)
{
	U32   datalen;
	char* msg;
	datalen = ls_generate_msg(method, attrs, &msg);
	remotesocket_sendmsg(did, sid, MSG_FUNCODE_LOGIC_BINARY, 1, (U8*) msg, datalen);
	if (NULL != msg) {
		free(msg);
		msg = NULL;
	}

	return TRUE;
}

BOOL ls_send_jsonmsg(cJSON* json, char* did, char* sid)
{
	char* jsonstr = cJSON_Print(json);
	if (NULL != jsonstr) {
		remotesocket_sendmsg(did, sid, MSG_FUNCODE_LOGIC_JSON, 1, (U8*) jsonstr, strlen(jsonstr));
		free(jsonstr);
		jsonstr = NULL;
	}
	return (TRUE);
}
