#ifndef _H_REMOTESOCKET_
#define _H_REMOTESOCKET_
#include "define.h"
#include "message.h"

typedef struct {
	char* pid;
	char* ip;
	U16   tcp_port;
	U16   udp_port;
} STRREMOTE_IPMAP;
#define REMOTE_TCP TRUE
#define REMOTE_TCP_IP "127.0.0.1"
#define REMOTE_TCP_PORT (unsigned short) 7433
#define REMOTE_UDP_PORT (unsigned short) 7434

#define REMOTESOCKET_MAX_CLIENTS 12
#define REMOTESOCKET_MAX_SERVERS 12

#define REMOTESOCKET_SOCKTYPE_UNKNOW 0
#define REMOTESOCKET_SOCKTYPE_SHORTTCP 1
#define REMOTESOCKET_SOCKTYPE_LONGTCP 2
#define REMOTESOCKET_SOCKTYPE_UDP 3
#define REMOTESOCKET_SOCKTYPE_TCPSERVER 4
#define REMOTESOCKET_SOCKTYPE_NUM 5

#define REMOTESOCKET_MAX_PAYLOAD_SIZE 1024
#define REMOTESOCKET_MAX_MSG_SIZE (SOCK_DATA_HEADER_LENTH + REMOTESOCKET_MAX_PAYLOAD_SIZE)
//#define MAKEFOURCC_BE4(a,b,c,d) ( ((U32)(U8)(a) << 24) | ((U32)(U8)(b) << 16)
//| ((U32)(U8)(c) << 8) | ((U32)(U8)(d) << 0) )

enum CLIENT_STATE {
	CLI_STATE_DISCONNECT = 0,
	CLI_STATE_CONNECT,
};

typedef void (*REMOTESOCKET_CALLBACK_FUN)(U8* msg, U32 length, U32 msgid);
typedef struct {
	char pid[MSG_ID_SIZE];
	U32  contype;
} PID_CONNECT;

typedef struct {
	BOOL		 enabletcpsrv;
	BOOL		 enableudpsrv;
	PID_CONNECT* condpid;
	U32			 condpidnum;
	char		 pid[MSG_ID_SIZE];
} CONFIG_PARAM;
extern U32  msgcounter;
extern BOOL remotesocket_init(CONFIG_PARAM* config, REMOTESOCKET_CALLBACK_FUN callback);
extern BOOL remotesocket_send(char* pid, U8* msg, U32 len, BOOL waitack);
extern BOOL remotesocket_foward(char* pid, U8* msg, U32 len, U32 msgid);
extern void remotesocket_clients_init(REMOTESOCKET_CALLBACK_FUN callback);
void remotesocket_send_addheader(U8* msg, U32 msgid);

extern BOOL remotesocket_sendack(char* pid, U8* msg, U32 len, U32 msgid);
extern BOOL remotesocket_sendbysocktype(U32 pid, U8* msg, U32 len, int socktyp);
extern BOOL remotesocket_sendackbysocktype(char* pid, U8* msg, U32 len, U32 msgid, int socktype);
extern void remotesocket_generatemsg(char* sid, char* did, U8 funcode, U8 operatecode, const char* msg, U32 len, U32 msgid);
extern BOOL remotesocket_sendmsg(char* did, char* sid, U8 funcode, U8 operatecode, U8* msg, U32 len);
extern BOOL remotesocket_sendmsgAck(U32 msgid, char* did, char* sid, U8 funcode, U8 operatecode, U8* msg, U32 len);

#endif
