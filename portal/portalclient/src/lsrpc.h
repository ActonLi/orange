#ifndef _H_LSDEF_
#define _H_LSDEF_
#include "cJSON.h"
#include "define.h"
#include "message.h"

#define LS_METHOD_LEN_MAX 64

struct tag_CommParams {
	int					   datatype;
	int					   datalen;
	int					   valueint;
	char*				   valuestring;
	struct tag_CommParams* next;
	struct tag_CommParams* pre;
};
typedef struct {
	char* method;
	void (*function)(struct tag_CommParams* params, U32 msgid, MSG_DATA_HEADER* source);
} STRLSRPCType;

struct ls_rpc_msg;

extern struct ls_rpc_msg* ls_rpc_msg_create(void);
extern int ls_rpc_msg_append(U8 type, U32 data_size, void* data, struct ls_rpc_msg* msg);
extern int ls_rpc_msg_update(U8 type, U32 data_size, void* data, U8 index, struct ls_rpc_msg* msg);
extern void ls_rpc_msg_destroy(struct ls_rpc_msg* msg);
extern int ls_rpc_send_msg(struct ls_rpc_msg* msg, char* did, char* sid);
extern int ls_rpc_msg_dump(struct ls_rpc_msg* msg);

extern void ls_rpc_register(const STRLSRPCType* rpcfuns, U32 funnum);
extern void ls_init(const STRLSRPCType* rpcfuns, U32 funnum);
extern U32 ls_getParamNumber(struct tag_CommParams* params, U32 paramind);
extern char* ls_getParamString(struct tag_CommParams* params, U32 paramind);
extern void* ls_getParamBinary(struct tag_CommParams* params, U32 paramid);
extern void ls_handleMsg(void* msg, U32 msgid);
extern U32 ls_generate_msg(char* method, cJSON* attrs, char** msg);
extern BOOL ls_send_binary_msg(char* method, int data_size, void* data, char* did, char* sid);
extern U32 ls_generate_binary_msg(char* method, int data_size, void* data, char** msg);
extern BOOL ls_sendmsg(char* method, cJSON* attrs, char* did, char* sid);
extern BOOL ls_sendmsgAck(U32 msgid, char* method, cJSON* attrs, char* did, char* sid);
extern BOOL ls_send_jsonmsg(cJSON* json, char* did, char* sid);

#endif
