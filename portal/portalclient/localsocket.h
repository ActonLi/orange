#ifndef _H_LOCALSOCKET_
#define _H_LOCALSOCKET_
#include "define.h"
#include "message.h"

#define LOCALSOCKET_MAX_CLIENTS 12
#define LOCALSOCKET_MAX_SERVERS 12

#define LOCALSOCKET_SOCKTYPE_UNKNOW 0
#define LOCALSOCKET_SOCKTYPE_INSTANCE 1
#define LOCALSOCKET_SOCKTYPE_SHORTTCP 2
#define LOCALSOCKET_SOCKTYPE_LONGTCP 3
#define LOCALSOCKET_SOCKTYPE_UDP 4
#define LOCALSOCKET_SOCKTYPE_TCPSERVER 5
#define LOCALSOCKET_SOCKTYPE_NUM 6

#define LOCALSOCKET_MAX_PAYLOAD_SIZE 1024
#define LOCALSOCKET_MAX_MSG_SIZE (SOCK_DATA_HEADER_LENTH + LOCALSOCKET_MAX_PAYLOAD_SIZE)
//#define MAKEFOURCC_BE4(a,b,c,d) ( ((U32)(U8)(a) << 24) | ((U32)(U8)(b) << 16)
//| ((U32)(U8)(c) << 8) | ((U32)(U8)(d) << 0) )

enum LOCALSOCKET_CLIENT_STATE {
	LOCALSOCKET_CLI_STATE_DISCONNECT = 0,
	LOCALSOCKET_CLI_STATE_CONNECT,
};

typedef enum tag_NodeFlag {
	NODE_IDLE,
	NODE_WAIT,
	NODE_TIMEOUT,
	NODE_RECVED,
} LocalsocketNodeFlag;

typedef void (*LOCALSOCKET_CALLBACK_FUN)(U8* msg, U32 length, U32 msgid);
typedef struct {
	char pid[MSG_ID_SIZE];
	U32  contype;
} LOCALSOCKET_PID_CONNECT;

typedef struct {
	BOOL					 enabletcpsrv;
	BOOL					 enableudpsrv;
	LOCALSOCKET_PID_CONNECT* condpid;
	U32						 condpidnum;
	char					 pid[MSG_ID_SIZE];
} LOCALSOCKET_CONFIG_PARAM;

extern int localsocket_findsocktypebypid(char* pid);
extern void localsocket_addsyncheader(char* dst_id, U8* msg, U32 size, U32 msgid, int socktype);
extern void localsocket_clients_init(LOCALSOCKET_CALLBACK_FUN callback);
extern BOOL localsocket_init(LOCALSOCKET_CONFIG_PARAM config, LOCALSOCKET_CALLBACK_FUN callback);
extern BOOL localsocket_uninit();
extern void localsocket_set_header(int funcode, int opercode, U8* msg, U32 size);
extern BOOL localsocket_send(char* pid, U8* msg, U32 len);
extern BOOL localsocket_sendack(char* pid, U8* msg, U32 len, U32 timeout, U32* msgid);
extern LocalsocketNodeFlag localsocket_node_flag(U32 msgid);

#endif
