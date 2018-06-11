#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/sockios.h>
#endif
#include "common.h"
#include "crc.h"
#include "events.h"
#include "localsocket.h"
#include "msgdispatch.h"
#include <fcntl.h>
#include <malloc.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

extern char app_name[128];

// server
typedef struct tag_SockRecMsgState {
	U8*			  recv_buff;
	U32			  headerpointer;
	U8			  headerring[SOCKET_HEADER_LEN];
	SOCKET_HEADER sock_header;
	U32			  datalen;
	U32			  msgid;
	U32			  headercount;
	U32			  state;
	U32			  recvcount;
} SOCK_RECMSG_STATE;
typedef struct tag_SockNode {
	int				   fd;
	struct sockaddr_un addr_un;
	pthread_mutex_t	lock;
	char			   from[MSG_ID_SIZE];
	char			   to[MSG_ID_SIZE];
	SOCK_RECMSG_STATE  recstate;
	int				   socketype;
	int				   connectstate;
} SOCK_CLIENT_NODE;

typedef struct tag_SockServerParam {
	int						 sockfd;
	struct sockaddr_un		 addr_un;
	LOCALSOCKET_CALLBACK_FUN callback;
	void*					 arg;
	pthread_t				 thread;
	pthread_mutex_t			 lock;
	SOCK_CLIENT_NODE		 client[LOCALSOCKET_MAX_CLIENTS];
	fd_set					 readfds;
} SOCK_SERVER_PARAM;

typedef struct tag_SockClientParam {
	int						 sockfd;
	struct sockaddr_un		 addr_un;
	LOCALSOCKET_CALLBACK_FUN callback;
	void*					 arg;
	pthread_t				 thread;
	pthread_mutex_t			 lock;
	SOCK_CLIENT_NODE		 client[LOCALSOCKET_MAX_SERVERS];
	fd_set					 readfds;
} SOCK_CLIENT_PARAM;

typedef struct tag_SockUDPParam {
	int						 sockfd;
	struct sockaddr_un		 addr_un;
	LOCALSOCKET_CALLBACK_FUN callback;
	void*					 arg;
	pthread_t				 thread;
	pthread_mutex_t			 lock;
	SOCK_CLIENT_NODE		 client;
	fd_set					 readfds;
} SOCK_UDP_PARAM;

#define LOCALSOCKET_MAX_DST_PID 30
typedef struct tag_SockesParam {
	pthread_t				localsocket_receive_thread;
	pthread_t				localsocket_timer_thread;
	BOOL					enabletcpsrv;
	BOOL					enableudpsrv;
	LOCALSOCKET_PID_CONNECT conpid[LOCALSOCKET_MAX_DST_PID];
	U32						conpidnum;
	char					pid[MSG_ID_SIZE];
	SOCK_SERVER_PARAM		tcp_server;
	SOCK_UDP_PARAM			udp_server;
	SOCK_CLIENT_PARAM		sock_clients;

} SOCKS_PARAM;

static SOCKS_PARAM localsocket_regs = {0};

#define LOCALSOCKET_SEND_QUEUE_SIZE 30

typedef struct {
	U32					msgid;
	LocalsocketNodeFlag state;
} LOCALSOCKETSendQueueNode;
typedef struct tag_LocalSocketSendRegs {
	U32						 count;
	U32						 counter;
	LOCALSOCKETSendQueueNode queue[LOCALSOCKET_SEND_QUEUE_SIZE];
	pthread_mutex_t			 lock;
} LOCALSOCKETSendRegsType;
LOCALSOCKETSendRegsType localsocket_sendRegs = {0};

const int socktypeorder[] = {LOCALSOCKET_SOCKTYPE_TCPSERVER, LOCALSOCKET_SOCKTYPE_SHORTTCP, LOCALSOCKET_SOCKTYPE_UDP, LOCALSOCKET_SOCKTYPE_LONGTCP};

static int   debug_count = 0;
static int   write_fd	= -1;
static void* localsocket_listen(void* thread);
static void* localsocket_time(void* thread);
static BOOL localsocket_udpserver_init(LOCALSOCKET_CALLBACK_FUN callback, const char* sun_path);

#if 0
static void handle_time_event(void* param)
{
	pthread_mutex_lock(&localsocket_sendRegs.lock);
	LOCALSOCKETSendQueueNode* node = (LOCALSOCKETSendQueueNode*) param;
	if (node->state == NODE_WAIT) {
		// memset(node, 0, sizeof(LOCALSOCKETSendQueueNode));
		node->state = NODE_TIMEOUT;
		localsocket_sendRegs.count--;
	}
	pthread_mutex_unlock(&localsocket_sendRegs.lock);
}
#endif

static int change_node_flag(U32 msgid, LocalsocketNodeFlag flag)
{
	int res = -1;
	int i   = 0;
	pthread_mutex_lock(&localsocket_sendRegs.lock);
	for (i = 0; i < LOCALSOCKET_SEND_QUEUE_SIZE; i++) {
		LOCALSOCKETSendQueueNode* node = (LOCALSOCKETSendQueueNode*) &localsocket_sendRegs.queue[i];
		if (node->msgid == msgid && node->state == NODE_WAIT) {
			// memset(node, 0, sizeof(LOCALSOCKETSendQueueNode));
			node->state = flag;
			localsocket_sendRegs.count--;
			res = 0;
		}
	}
	pthread_mutex_unlock(&localsocket_sendRegs.lock);
	return res;
}

static BOOL localsocket_checkheader(SOCKET_HEADER* sock_header, U8* header, U32 pointer)
{
	int err_code				= 0;
	U32 i						= 0;
	U32 p						= 0;
	U8  buff[SOCKET_HEADER_LEN] = {0};
	U8  crc[2]					= {0};

	p = pointer;
	for (i = 0; i < SOCKET_HEADER_LEN; i++) {
		if (p >= SOCKET_HEADER_LEN) {
			p = 0;
		}
		buff[i] = header[p];
		p++;
	}

	SOCKET_HEADER* h = (SOCKET_HEADER*) buff;
	if (h->syncHeader.frameHeader.word != SOCKET_SYNC_HEADER)
		goto ERROR_HEADER;
	err_code = 1;
	crc_calc((U8*) (&h->msgHeader), MSG_HEADER_LENTH, crc);
	DEBUGP("receive message crc= %02x,%02x, cal=%02x,%02x\n", crc[0], crc[1], h->syncHeader.headCRC.bytes[2], h->syncHeader.headCRC.bytes[3]);
	if ((h->syncHeader.headCRC.bytes[2] != crc[0]) || (h->syncHeader.headCRC.bytes[3] != crc[1])) {
		goto ERROR_HEADER;
	}
	err_code = 2;
	if (0 == h->msgHeader.srcID[0] && 0 == h->msgHeader.srcID[1] && 0 == h->msgHeader.srcID[2] && 0 == h->msgHeader.srcID[3] && 0 == h->msgHeader.srcID[4] &&
		0 == h->msgHeader.srcID[5]) {
		goto ERROR_HEADER;
	}
	err_code = 3;
	if (0 == h->msgHeader.dstID[0] && 0 == h->msgHeader.dstID[1] && 0 == h->msgHeader.dstID[2] && 0 == h->msgHeader.dstID[3] && 0 == h->msgHeader.dstID[4] &&
		0 == h->msgHeader.dstID[5]) {
		goto ERROR_HEADER;
	}
	/*
	   err_code = 4;
	   if(0 == h->msgHeader.funcCode)
	   {
	   goto ERROR_HEADER;

	   }
	   err_code = 5;
	   if(0 == h->msgHeader.operCode)
	   {
	   goto ERROR_HEADER;

	   }
	   */
	err_code	 = 6;
	U32 iDataLen = MAKEFOURCC_BE4(h->msgHeader.dataLen[0], h->msgHeader.dataLen[1], h->msgHeader.dataLen[2], h->msgHeader.dataLen[3]);
	if (iDataLen < MSG_HEADER_LENTH) {
		goto ERROR_HEADER;
	}
	err_code = 7;
	memcpy((U8*) sock_header, buff, SOCKET_HEADER_LEN);
	return TRUE;

ERROR_HEADER:
	printf("=== Error head check. error code:%d, %d ====\n", err_code, h->syncHeader.frameHeader.word);
	return FALSE;
}
BOOL localsocket_tcp_close(int sockfd)
{
	int  i			= 0;
	int  leftunsend = 0;
	BOOL res		= FALSE;

	for (i = 0; i < 50; i++) {
		leftunsend = 0;
		// if(ioctl(sockfd, SIOCOUTQ, (char*)&leftunsend) >= 0)
		{
			if (leftunsend > 0) {
				struct timeval stTimeout = {0};
				stTimeout.tv_sec		 = 0;
				stTimeout.tv_usec		 = 10 * 1000;
				select(0, 0, 0, 0, &stTimeout);
			} else {
				// DEBUGP("=====ccccccc====\n");
				close(sockfd);
				// DEBUGP("=====dddddddd====\n");
				res = TRUE;
				break;
			}
		}
	}
	return (res);
}
static BOOL localsocket_tcpclient_del(SOCK_CLIENT_PARAM* clients, int pos)
{
	BOOL res = FALSE;
	if (pos < LOCALSOCKET_MAX_SERVERS) {
		pthread_mutex_lock(&clients->client[pos].lock);
		clients->client[pos].connectstate = LOCALSOCKET_CLI_STATE_DISCONNECT;
		memset(clients->client[pos].to, 0, MSG_ID_SIZE);

		localsocket_tcp_close(clients->client[pos].fd);
		clients->client[pos].fd = -1;
		pthread_mutex_unlock(&clients->client[pos].lock);
		res = TRUE;
	}
	return res;
}
BOOL localsocket_socks_send(char* pid, U8* msg, U32 length, U32 contype, BOOL waitack)
{
	if (msg == NULL || length < MSG_SYNC_HEADER_LEN) {
		return FALSE;
	}

	U16				   i					   = 0;
	U32				   llen					   = 0;
	int				   pos					   = -1;
	int				   sockfd				   = -1;
	BOOL			   send					   = FALSE;
	SOCK_CLIENT_NODE*  client				   = NULL;
	struct sockaddr_un addr					   = {0};
	char			   sun_path[SOCK_PATH_LEN] = {0};
	if (FALSE == msg_findpathbyid(pid, sun_path)) {
		return FALSE;
	}
	if ((contype == LOCALSOCKET_SOCKTYPE_UNKNOW) || (contype == LOCALSOCKET_SOCKTYPE_LONGTCP)
		//|| (contype == LOCALSOCKET_SOCKTYPE_TCPSERVER)
		|| (contype == LOCALSOCKET_SOCKTYPE_SHORTTCP)) {
		for (i = 0; i < LOCALSOCKET_MAX_SERVERS; i++) {
			pthread_mutex_lock(&localsocket_regs.sock_clients.client[i].lock);
			client = &localsocket_regs.sock_clients.client[i];

			if (client->connectstate == LOCALSOCKET_CLI_STATE_CONNECT) {
				if ((client->socketype == LOCALSOCKET_SOCKTYPE_LONGTCP) && (0 == memcmp(client->to, pid, MSG_ID_SIZE))) {
					debug_count = 3;
					write_fd	= client->fd;
					llen		= write(client->fd, msg, length);
					debug_count++;
					//
					if (llen == length) {
						send = TRUE;
					}
				}
			} else {
				if (pos < 0)
					pos = i;
			}
			pthread_mutex_unlock(&localsocket_regs.sock_clients.client[i].lock);
		}
		if (FALSE == send) {
			if (pos >= 0) {
				pthread_mutex_lock(&localsocket_regs.sock_clients.client[pos].lock);
				client	 = &localsocket_regs.sock_clients.client[pos];
				client->fd = socket(AF_UNIX, SOCK_STREAM, 0);
				if (client->fd > 0) {
					memset(&client->addr_un, 0, sizeof(client->addr_un));
					client->addr_un.sun_family = AF_UNIX;
					strcpy(client->addr_un.sun_path, sun_path);
					fcntl(client->fd, F_SETFD, FD_CLOEXEC);
					if (0 == connect(client->fd, (struct sockaddr*) &(client->addr_un), sizeof(client->addr_un))) {
						client->connectstate = LOCALSOCKET_CLI_STATE_CONNECT;
						if (contype == LOCALSOCKET_SOCKTYPE_UNKNOW)
							client->socketype = LOCALSOCKET_SOCKTYPE_SHORTTCP;
						else
							client->socketype = contype;
						memcpy(client->to, pid, MSG_ID_SIZE);

						debug_count = 5;
						llen		= write(client->fd, msg, length);
						debug_count++;
						//

						if (llen == length) {
							send = TRUE;
							if (contype == LOCALSOCKET_SOCKTYPE_UNKNOW)
								contype = LOCALSOCKET_SOCKTYPE_SHORTTCP;
						}
					} else {
						close(client->fd);
						client->fd = -1;
					}
				}
				pthread_mutex_unlock(&localsocket_regs.sock_clients.client[pos].lock);

				if (send == TRUE && waitack == FALSE && contype == LOCALSOCKET_SOCKTYPE_SHORTTCP) {
					localsocket_tcpclient_del(&localsocket_regs.sock_clients, pos);
				}
			}
		}
	}
	if ((send == FALSE && contype == LOCALSOCKET_SOCKTYPE_UNKNOW) || contype == LOCALSOCKET_SOCKTYPE_UDP) {
		sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
		fcntl(sockfd, F_SETFD, FD_CLOEXEC);
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
		addr.sun_family = AF_UNIX;
		snprintf(addr.sun_path, sizeof(addr.sun_path), "%s_udp", sun_path);
		debug_count = 7;
		llen		= sendto(sockfd, msg, length, 0, (struct sockaddr*) &addr, sizeof(struct sockaddr_un));
		debug_count++;
		if (llen == length) {
			send	= TRUE;
			contype = LOCALSOCKET_SOCKTYPE_UDP;
		}
		close(sockfd);
	}
	return send;
}
static BOOL localsocket_tcpserver_send(char* pid, U8* msg, int length)
{
	if ((msg == NULL) || (length < MSG_SYNC_HEADER_LEN)) {
		return FALSE;
	}
	int  i   = 0;
	BOOL res = FALSE;
	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		pthread_mutex_lock(&localsocket_regs.tcp_server.client[i].lock);
		// DEBUGP("send by tcp server connectstate=%d, send pid=%c, memory
		// pid=%d\n", remotesocket_regs.tcp_server.client[i].connectstate,
		// pid,remotesocket_regs.tcp_server.client[i].from);
		if (localsocket_regs.tcp_server.client[i].connectstate == LOCALSOCKET_CLI_STATE_CONNECT) {
			if ((0 == (memcmp(pid, localsocket_regs.tcp_server.client[i].from, MSG_ID_SIZE))) && (localsocket_regs.tcp_server.client[i].fd > 0) &&
				(localsocket_regs.tcp_server.client[i].socketype == LOCALSOCKET_SOCKTYPE_LONGTCP ||
				 localsocket_regs.tcp_server.client[i].socketype == LOCALSOCKET_SOCKTYPE_TCPSERVER)) {
				debug_count = 1;
				int len		= write(localsocket_regs.tcp_server.client[i].fd, msg, length);
				debug_count++;
				if (len == length) {
					res = TRUE;
				}
				//
				pthread_mutex_unlock(&localsocket_regs.tcp_server.client[i].lock);
				break;
			}
		}
		pthread_mutex_unlock(&localsocket_regs.tcp_server.client[i].lock);
	}
	return (res);
}
BOOL localsocket_send_bysocktype(char* pid, U8* msg, U32 len, int socktype, BOOL waitack)
{
	BOOL res = FALSE;
	if (socktype == LOCALSOCKET_SOCKTYPE_UNKNOW || socktype == LOCALSOCKET_SOCKTYPE_TCPSERVER || socktype == LOCALSOCKET_SOCKTYPE_LONGTCP) {
		res = localsocket_tcpserver_send(pid, msg, len);
	}

	if (res == FALSE) {
		res = localsocket_socks_send(pid, msg, len, socktype, waitack);
	}
	return (res);
}

static BOOL localsocket_send_process(char* pid, U8* msg, U32 len, BOOL waitack, int socktype, BOOL fixedsocketype, BOOL isack, U32* msgid, U32 timeout)
{
	U8*  buff = NULL;
	U32  llen = 0;
	BOOL res  = FALSE;
	BOOL req  = TRUE;
	// int socktypes[LOCALSOCKET_SOCKTYPE_NUM] = {0};
	// int type = -1;
	U32 i = 0;
	//	U32						  socknum = 0;
	U32						  lmsgid = 0;
	LOCALSOCKETSendQueueNode* node   = NULL;
	/*
		socknum = 0;
		if((socktype >= 0)
		  && (socktype < LOCALSOCKET_SOCKTYPE_NUM))
		  {
		  socktypes[socknum] = socktype;
		  socknum ++;
		  }
		  if(fixedsocketype == FALSE){
		  for(i = 0; i< sizeof(socktypeorder)/sizeof(int); i++)
		  {
		  if(socknum < LOCALSOCKET_SOCKTYPE_NUM)
		  {
		  if(socktype != socktypeorder[i])
		  {
		  socktypes[socknum] = socktypeorder[i];
		  socknum ++;
		  }
		  }
		  }
		  }
		  if(socknum == 0)
		  {
		  return(FALSE);
		  }
		  */
	if (TRUE == waitack) {
		if (localsocket_sendRegs.count >= LOCALSOCKET_SEND_QUEUE_SIZE) {
			req = FALSE;
		}
	}
	pthread_mutex_lock(&localsocket_sendRegs.lock);
	if (req == TRUE) {
		if (msgid != NULL) {
			if (*msgid <= 0)
				*msgid = localsocket_sendRegs.counter;
			if (isack) {
				lmsgid = 0x80000000 | localsocket_sendRegs.counter;
			} else {
				lmsgid = localsocket_sendRegs.counter & 0x7FFFFFFF;
			}
		} else
			lmsgid = 0;
		if (msg != NULL && len > 0) {
			llen = len + MSG_SYNC_HEADER_LEN;
			buff = malloc(llen);
			memset(buff, 0, llen);
			memcpy(buff + MSG_SYNC_HEADER_LEN, msg, len);
		} else {
			pthread_mutex_unlock(&localsocket_sendRegs.lock);
			return (FALSE);
		}

		localsocket_addsyncheader(pid, buff, llen, lmsgid, socktype);
		// for(i = 0; i< socknum; i++)
		{
			res = localsocket_send_bysocktype(pid, buff, llen, socktype, waitack);
			/*if(TRUE == res)
			  {
			  type = socktypes[i];
			  break;
			  }*/
		}
	}
	if (TRUE == res) {
		if (waitack) {
			for (i = 0; i < LOCALSOCKET_SEND_QUEUE_SIZE; i++) {
				node = &localsocket_sendRegs.queue[i];
				if (node->state == NODE_IDLE || node->state == NODE_TIMEOUT || node->state == NODE_RECVED) {
					node->state = NODE_WAIT;
					node->msgid = lmsgid;
					localsocket_sendRegs.count++;
					localsocket_sendRegs.counter++;
					// time_regist_timer(timeout, handle_time_event, node);
				}
			}
		}
	}
	if (buff != NULL) {
		free(buff);
		buff = NULL;
	}
	pthread_mutex_unlock(&localsocket_sendRegs.lock);
	return (res);
}
BOOL localsocket_tcpserver_init(LOCALSOCKET_CALLBACK_FUN callback, const char* sun_path)
{
	if (sun_path == NULL || callback == NULL) {
		return FALSE;
	}

	BOOL res   = -1;
	int  i	 = 0;
	int  reuse = 1;
	unlink(sun_path);
	memset(&localsocket_regs.tcp_server, 0, sizeof(SOCK_SERVER_PARAM));
	localsocket_regs.tcp_server.sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (localsocket_regs.tcp_server.sockfd <= 0) {
		return FALSE;
	}

	localsocket_regs.tcp_server.addr_un.sun_family = AF_UNIX;
	strcpy(localsocket_regs.tcp_server.addr_un.sun_path, sun_path);
	if (setsockopt(localsocket_regs.tcp_server.sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
		DEBUGP("%s:%d set socket option failed\n", __FUNCTION__, __LINE__);
	}
	res = bind(localsocket_regs.tcp_server.sockfd, (struct sockaddr*) &localsocket_regs.tcp_server.addr_un, sizeof(localsocket_regs.tcp_server.addr_un));
	if (res) {
		DEBUGP("Bind error\n server is already running\n");
		close(localsocket_regs.tcp_server.sockfd);
		return FALSE;
	}
	pthread_mutex_init(&localsocket_regs.tcp_server.lock, NULL);
	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		pthread_mutex_init(&localsocket_regs.tcp_server.client[i].lock, NULL);
	}
	localsocket_regs.tcp_server.callback = callback;

	usleep(100);
	return TRUE;
}

BOOL localsocket_init(LOCALSOCKET_CONFIG_PARAM config, LOCALSOCKET_CALLBACK_FUN callback)
{
	U32  i						 = 0;
	BOOL res					 = TRUE;
	char sun_path[SOCK_PATH_LEN] = {0};
	BOOL enabletcpsrv			 = config.enabletcpsrv;
	BOOL enableudpsrv			 = config.enableudpsrv;
	memcpy(localsocket_regs.pid, config.pid, MSG_ID_SIZE);
	localsocket_regs.conpidnum = config.condpidnum;
	if (localsocket_regs.conpidnum > LOCALSOCKET_MAX_DST_PID) {
		localsocket_regs.conpidnum = LOCALSOCKET_MAX_DST_PID;
	}
	for (i = 0; i < localsocket_regs.conpidnum; i++) {
		localsocket_regs.conpid[i].contype = config.condpid[i].contype;
		memcpy(localsocket_regs.conpid[i].pid, config.condpid[i].pid, MSG_ID_SIZE);
	}
	localsocket_regs.enabletcpsrv = FALSE;
	localsocket_regs.enableudpsrv = FALSE;
	if ((TRUE == enabletcpsrv) || (TRUE == enableudpsrv)) {
		if (msg_findpathbyid(config.pid, sun_path)) {
			if (TRUE == enableudpsrv) {
				char udp_sun_path[SOCK_PATH_LEN] = {0};
				snprintf(udp_sun_path, sizeof(udp_sun_path), "%s_udp", sun_path);
				if (TRUE == localsocket_udpserver_init(callback, udp_sun_path)) {
					localsocket_regs.enableudpsrv = TRUE;
				}
			}
			if (TRUE == enabletcpsrv) {
				if (TRUE == localsocket_tcpserver_init(callback, sun_path)) {
					localsocket_regs.enabletcpsrv = TRUE;
				}
			}
		}
	}

	localsocket_clients_init(callback);
	if (localsocket_regs.enableudpsrv != enableudpsrv) {
		res = FALSE;
	}
	if (localsocket_regs.enabletcpsrv != enabletcpsrv) {
		res = FALSE;
	}
	if (res == TRUE) {
		if (pthread_create(&localsocket_regs.localsocket_receive_thread, NULL, localsocket_listen, NULL)) {
			// DEBUGP("localsocket pthread_create failed\n");
			res = TRUE;
		}
	}

	if (pthread_create(&localsocket_regs.localsocket_timer_thread, NULL, localsocket_time, NULL)) {
		// DEBUGP("localsocket pthread_create failed\n");
		res = TRUE;
	}
	return res;
}

BOOL localsocket_uninit()
{
	return TRUE;
}

BOOL localsocket_send(char* pid, U8* msg, U32 len)
{
	BOOL res	  = FALSE;
	int  socktype = 0;
	socktype	  = localsocket_findsocktypebypid(pid);
	DEBUGP("start to send %s, %d\n", pid, socktype);
	res = localsocket_send_process(pid, msg, len, FALSE, socktype, FALSE, FALSE, NULL, 0);
	return (res);
}

BOOL localsocket_sendack(char* pid, U8* msg, U32 len, U32 timeout, U32* msgid)
{
	if (msgid == NULL)
		return FALSE;

	BOOL res	  = FALSE;
	int  socktype = 0;
	socktype	  = localsocket_findsocktypebypid(pid);
	DEBUGP("start to send ack %s, %d\n", pid, socktype);
	res = localsocket_send_process(pid, msg, len, FALSE, socktype, FALSE, TRUE, msgid, timeout);
	return (res);
}

LocalsocketNodeFlag localsocket_node_flag(U32 msgid)
{
	int					i	= 0;
	LocalsocketNodeFlag flag = NODE_IDLE;
	pthread_mutex_lock(&localsocket_sendRegs.lock);
	for (i = 0; i < LOCALSOCKET_SEND_QUEUE_SIZE; i++) {
		LOCALSOCKETSendQueueNode* node = (LOCALSOCKETSendQueueNode*) &localsocket_sendRegs.queue[i];
		if (node->msgid == msgid) {
			flag = node->state;
		}
	}
	pthread_mutex_unlock(&localsocket_sendRegs.lock);
	return (flag);
}

static int localsocket_tcpserver_add(SOCK_SERVER_PARAM* server, int fd, struct sockaddr_un* addr)
{
	int i	  = 0;
	int res	= -1;
	int emptyi = -1;
	int oldi   = -1;
	int pos	= -1;

	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		if (server->client[i].connectstate == LOCALSOCKET_CLI_STATE_DISCONNECT) {
			if (emptyi < 0) {
				emptyi = i;
			}
		} else {
			if (fd == server->client[i].fd) {
				oldi = i;
			}
		}
	}
	if (oldi >= 0) {
		pos = oldi;
	} else {
		if (emptyi >= 0) {
			pos = emptyi;
		}
	}
	// DEBUGP("%s add to client %d\n", app_name, pos);
	if (pos >= 0) {
		FD_SET(fd, &server->readfds);
		pthread_mutex_lock(&server->client[pos].lock);
		server->client[pos].fd = fd;
		memcpy(server->client[pos].from, localsocket_regs.pid, MSG_ID_SIZE);
		memcpy(&server->client[pos].addr_un, addr, sizeof(struct sockaddr_un));
		server->client[pos].connectstate = LOCALSOCKET_CLI_STATE_CONNECT;
		server->client[pos].socketype	= LOCALSOCKET_SOCKTYPE_SHORTTCP;
		pthread_mutex_unlock(&server->client[pos].lock);
		res = pos;
	}
	return res;
}

static BOOL localsocket_tcpserver_del(SOCK_SERVER_PARAM* server, int fd)
{
	int  i   = 0;
	BOOL res = FALSE;
	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		if (server->client[i].fd == fd) {
			FD_CLR(fd, &server->readfds);
			pthread_mutex_lock(&server->client[i].lock);
			server->client[i].connectstate = LOCALSOCKET_CLI_STATE_DISCONNECT;
			// memset(server->client[i].from, 0, MSG_ID_SIZE);
			memset(server->client[i].to, 0, MSG_ID_SIZE);
			close(server->client[i].fd);
			server->client[i].fd		= -1;
			server->client[i].socketype = -1;
			pthread_mutex_unlock(&server->client[i].lock);
			res = TRUE;
		}
	}
	return res;
}

static int localsocket_tcpserver_set(SOCK_SERVER_PARAM* server, int fd, char* pid, int conntype)
{
	int i   = 0;
	int res = -1;
	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		if (server->client[i].connectstate == LOCALSOCKET_CLI_STATE_CONNECT) {
			if (fd == server->client[i].fd) {
				pthread_mutex_lock(&server->client[i].lock);
				memcpy(server->client[i].to, pid, MSG_ID_SIZE);
				server->client[i].socketype = conntype;
				pthread_mutex_unlock(&server->client[i].lock);
				res = i;
				break;
			}
		}
	}
	return (res);
}

#if 0
static int localsocket_tcpserver_search(SOCK_SERVER_PARAM* server, char* pid)
{
	int i   = 0;
	int res = -1;
	for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
		if (server->client[i].connectstate == LOCALSOCKET_CLI_STATE_CONNECT) {
			if (0 == memcmp(pid, server->client[i].to, MSG_ID_SIZE)) {
				res = i;
				break;
			}
		}
	}
	return (res);
}
#endif

#define LOCALSOCKET_TCP_BUFF_SIZE 10240
#define LOCALSOCKET_TCP_STATE_HEADER 0
#define LOCALSOCKET_TCP_STATE_MALLOC 1
#define LOCALSOCKET_TCP_STATE_PAYLOAD 2
#define LOCALSOCKET_TCP_STATE_FINISH 3

static BOOL localsocket_receive(int sockfd, SOCK_RECMSG_STATE* recstate, LOCALSOCKET_CALLBACK_FUN callback)
{
	if (recstate == NULL) {
		return FALSE;
	}

	U32  i									= 0;
	U32  recvlength							= 0;
	U32  msglength							= 0;
	U32  len								= 0;
	U8   recvbuf[LOCALSOCKET_TCP_BUFF_SIZE] = {0};
	U32  bytes								= LOCALSOCKET_TCP_BUFF_SIZE;
	BOOL res								= FALSE;
	while (bytes > i) {
		bytes = read(sockfd, recvbuf, LOCALSOCKET_TCP_BUFF_SIZE);
		i	 = 0;
		while (i < bytes) {
			if (recstate->state == LOCALSOCKET_TCP_STATE_HEADER) {
				if (recstate->headerpointer >= SOCKET_HEADER_LEN) {
					recstate->headerpointer = 0;
				}
				recstate->headerring[recstate->headerpointer] = recvbuf[i];
				recstate->headerpointer++;
				recstate->headercount++;
				i++;
				if (recstate->headercount >= SOCKET_HEADER_LEN) {
					if (TRUE == localsocket_checkheader(&recstate->sock_header, recstate->headerring, recstate->headerpointer)) {
						recstate->state = LOCALSOCKET_TCP_STATE_MALLOC;
						// DEBUGP("goto fadfaa\n");
					}
				}
			}
			if (recstate->state == LOCALSOCKET_TCP_STATE_MALLOC) {
				if (recstate->recv_buff != NULL) {
					free((void*) recstate->recv_buff);
					recstate->recv_buff = NULL;
				}
				recstate->datalen = MAKEFOURCC_BE4(recstate->sock_header.msgHeader.dataLen[0], recstate->sock_header.msgHeader.dataLen[1],
												   recstate->sock_header.msgHeader.dataLen[2], recstate->sock_header.msgHeader.dataLen[3]);

				recstate->msgid = MAKEFOURCC_BE4(recstate->sock_header.syncHeader.msgid.bytes[0], recstate->sock_header.syncHeader.msgid.bytes[1],
												 recstate->sock_header.syncHeader.msgid.bytes[2], recstate->sock_header.syncHeader.msgid.bytes[3]);
				DEBUGP("=========test by zzyrecv buf:%d,%d,%d,%d=======\n", recstate->sock_header.msgHeader.dataLen[0],
					   recstate->sock_header.msgHeader.dataLen[1], recstate->sock_header.msgHeader.dataLen[2], recstate->sock_header.msgHeader.dataLen[3]);
				if (recstate->datalen > 0) // add by mjx. whether to take a limit of the max size
					recstate->recv_buff = (U8*) malloc(recstate->datalen + 1);
				if (recstate->recv_buff != NULL)
					memcpy(recstate->recv_buff, (U8*) &recstate->sock_header.msgHeader, MSG_HEADER_LENTH);
				recstate->recvcount = MSG_HEADER_LENTH;
				recstate->state		= LOCALSOCKET_TCP_STATE_PAYLOAD;
			}
			if (recstate->state == LOCALSOCKET_TCP_STATE_PAYLOAD) {
				if (recstate->recvcount >= recstate->datalen) {
					recstate->state = LOCALSOCKET_TCP_STATE_FINISH;
				} else {
					msglength = 0;
					if (bytes > i) {
						msglength = bytes - i;
					}
					if (msglength > 0) {
						recvlength = recstate->datalen - recstate->recvcount;
						if (msglength >= recvlength) {
							len = recvlength;
						} else {
							len = msglength;
						}
						if (recstate->recv_buff != NULL)
							memcpy(&recstate->recv_buff[recstate->recvcount], &recvbuf[i], len);
						i += len;
						recstate->recvcount += len;
						if (recstate->recvcount >= recstate->datalen) {
							recstate->state = LOCALSOCKET_TCP_STATE_FINISH;
						}
					}
				}
			}
			if (recstate->state == LOCALSOCKET_TCP_STATE_FINISH) {
				recstate->headerpointer = 0;
				recstate->headercount   = 0;

				// callback//
				if (recstate->recv_buff != NULL) {
					int ret = 0;
					if (recstate->msgid > 0) {
						ret = change_node_flag(recstate->msgid, NODE_RECVED);
					}
					if (ret == 0) {
						localsocket_tcpserver_set(&localsocket_regs.tcp_server, sockfd, (char*) &recstate->sock_header.msgHeader.srcID[0],
												  recstate->sock_header.msgHeader.reserved[0]);
						callback(recstate->recv_buff, recstate->recvcount, recstate->msgid);
					}
					if (recstate->recv_buff != NULL) {
						free(recstate->recv_buff);
						recstate->recv_buff = NULL;
					}
					recstate->recvcount = 0;
					res					= TRUE;
				}
				recstate->state = LOCALSOCKET_TCP_STATE_HEADER;
			}
		}
	}
	return (res);
}

// static U32 err_count = 0;

static void* localsocket_time(void* thread)
{
	// int i = 0;
	// int bytes = 0;
	int wait = 0;
	while (1) {
		if (wait > 10) {
			// char recvbuf[LOCALSOCKET_TCP_BUFF_SIZE] = {0};
			if (write_fd >= 0) {
				// DEBUGP("=====read begin ====\n");
				// read(write_fd, recvbuf, LOCALSOCKET_TCP_BUFF_SIZE);

				/*
				   localsocket_tcpserver_del(&localsocket_regs.tcp_server,
				   write_fd);
				   for(i = 0; i< LOCALSOCKET_MAX_SERVERS; i++)
				   {
				   if(localsocket_regs.sock_clients.client[i].fd ==
				   write_fd)
				   {
				   localsocket_tcpclient_del(&localsocket_regs.sock_clients,
				   i);
				   break;
				   }
				   }
				   */
				// DEBUGP("=====read end ====\n");
				wait = 0;
			}
		}
		if (debug_count == 3) {
			wait++;
			// DEBUGP("=====%s test by mjx debug count = %d ===========\n",
			// app_name,debug_count);
		} else
			wait = 0;
		// DEBUGP("=====%s test by mjx debug count = %d ===========\n",
		// app_name,debug_count);
		debug_count++;
		sleep(2);
	}
	return NULL;
}

static void* localsocket_listen(void* thread)
{
	U32  i   = 0;
	int  fd  = -1;
	BOOL res = -1;
	int  len = 0;
	// U16  counter = 0;
	int nread = 0;
	// int				   pid			= -1;
	BOOL			   enabletcpsrv = 0, enableudpsrv = 0;
	SOCK_SERVER_PARAM* tcpserver = NULL;
	SOCK_UDP_PARAM*	udpserver = NULL;

	fd_set			   set;
	struct timeval	 tm;
	struct sockaddr_un address;

	memset(&set, 0, sizeof(set));
	memset(&tm, 0, sizeof(tm));
	memset(&address, 0, sizeof(address));

	tcpserver	= &localsocket_regs.tcp_server;
	udpserver	= &localsocket_regs.udp_server;
	enabletcpsrv = localsocket_regs.enabletcpsrv;
	enableudpsrv = localsocket_regs.enableudpsrv;
	if (TRUE == enabletcpsrv) {
		listen(tcpserver->sockfd, 5);
		FD_ZERO(&tcpserver->readfds);
		FD_SET(tcpserver->sockfd, &tcpserver->readfds);
	}
	tm.tv_sec  = 0;
	tm.tv_usec = 500000;

	// DEBUGP("server socket fd=%d\n", localsocket_regs.tcp_server.sockfd);
	while (1) {
		FD_ZERO(&set);
		if (TRUE == enabletcpsrv) {
			set = tcpserver->readfds;
		}
		if (TRUE == enableudpsrv) {
			FD_SET(udpserver->sockfd, &set);
		}
		for (i = 0; i < LOCALSOCKET_MAX_SERVERS; i++) {
			if (localsocket_regs.sock_clients.client[i].connectstate == LOCALSOCKET_CLI_STATE_CONNECT) {
				FD_SET(localsocket_regs.sock_clients.client[i].fd, &set);
			}
		}

		res = select(FD_SETSIZE, &set, NULL, NULL, &tm);
		if (res > 0) {
			if (TRUE == enabletcpsrv) {
				if (FD_ISSET(tcpserver->sockfd, &set)) {
					FD_CLR(tcpserver->sockfd, &set);
					len = sizeof(address);
					fd  = accept(tcpserver->sockfd, (struct sockaddr*) &address, (socklen_t*) &len);
					localsocket_tcpserver_add(tcpserver, fd, &address);
					// DEBUGP("%s tcp communication connect fd=%d\n", app_name,
					// fd);
				}

				for (i = 0; i < LOCALSOCKET_MAX_CLIENTS; i++) {
					if (FD_ISSET(tcpserver->client[i].fd, &set)) {
						ioctl(tcpserver->client[i].fd, FIONREAD, &nread);
						if (nread == 0) {
							// DEBUGP("%s tcp fd:%d communication
							// disconnection\n", app_name,
							// tcpserver->client[i].fd);
							localsocket_tcpserver_del(tcpserver, tcpserver->client[i].fd);
						} else {
							(void) localsocket_receive(tcpserver->client[i].fd, &tcpserver->client[i].recstate, tcpserver->callback);
						}
					}
				}
			}
			if (TRUE == enableudpsrv) {
				if (FD_ISSET(udpserver->sockfd, &set)) {
					(void) localsocket_receive(udpserver->sockfd, &udpserver->client.recstate, udpserver->callback);
				}
			}

			for (i = 0; i < LOCALSOCKET_MAX_SERVERS; i++) {
				if (FD_ISSET(localsocket_regs.sock_clients.client[i].fd, &set)) {
					DEBUGP("%s tcp client recv \n", app_name);
					ioctl(localsocket_regs.sock_clients.client[i].fd, FIONREAD, &nread);
					if (nread == 0) {
						localsocket_tcpclient_del(&localsocket_regs.sock_clients, i);
					} else {
						res = localsocket_receive(localsocket_regs.sock_clients.client[i].fd, &localsocket_regs.sock_clients.client[i].recstate,
												  localsocket_regs.sock_clients.callback);
						if (TRUE == res) {
							if (localsocket_regs.sock_clients.client[i].socketype != LOCALSOCKET_SOCKTYPE_LONGTCP) {
								localsocket_tcpclient_del(&localsocket_regs.sock_clients, i);
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

BOOL localsocket_udpserver_init(LOCALSOCKET_CALLBACK_FUN callback, const char* sun_path)
{
	if (sun_path == NULL || callback == NULL) {
		return FALSE;
	}

	// BOOL res = FALSE;
	// int				   iLen = 0;
	struct sockaddr_un address;

	memset(&localsocket_regs.udp_server, 0, sizeof(SOCK_UDP_PARAM));
	unlink(sun_path);

	localsocket_regs.udp_server.sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (localsocket_regs.udp_server.sockfd <= 0) {
		DEBUGP("%s:%d open socket failed\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	fcntl(localsocket_regs.udp_server.sockfd, F_SETFD, FD_CLOEXEC);

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, sun_path);
	if (bind(localsocket_regs.udp_server.sockfd, (struct sockaddr*) &address, sizeof(address)) == -1) {
		DEBUGP("%s:%d bind failed\n", __FUNCTION__, __LINE__);
		close(localsocket_regs.udp_server.sockfd);
		return (FALSE);
	}
	pthread_mutex_init(&localsocket_regs.udp_server.lock, NULL);
	localsocket_regs.udp_server.callback = callback;
	return TRUE;
}
void localsocket_clients_init(LOCALSOCKET_CALLBACK_FUN callback)
{
	U32 i = 0;
	memset(&localsocket_regs.sock_clients, 0, sizeof(SOCK_CLIENT_PARAM));
	pthread_mutex_init(&localsocket_regs.sock_clients.lock, NULL);
	for (i = 0; i < LOCALSOCKET_MAX_SERVERS; i++) {
		pthread_mutex_init(&localsocket_regs.sock_clients.client[i].lock, NULL);
	}
	localsocket_regs.sock_clients.callback = callback;
}

void localsocket_addsyncheader(char* dst_id, U8* msg, U32 size, U32 msgid, int socktype)
{
	U8			   crc[2] = {0};
	SOCKET_HEADER* header = NULL;
	// BOOL		   res	= FALSE;
	// U32			   llen   = ((size - MSG_SYNC_HEADER_LEN));
	header = (SOCKET_HEADER*) msg;
	memcpy(header->msgHeader.dstID, dst_id, MSG_ID_SIZE);
	if (socktype < LOCALSOCKET_SOCKTYPE_NUM && socktype > LOCALSOCKET_SOCKTYPE_UNKNOW)
		header->msgHeader.reserved[0] = socktype;
	else
		header->msgHeader.reserved[0] = LOCALSOCKET_SOCKTYPE_SHORTTCP;
	crc_calc((U8*) &header->msgHeader, MSG_HEADER_LENTH, crc);
	header->syncHeader.headCRC.bytes[0] = 0; //;
	header->syncHeader.headCRC.bytes[1] = 0; //;
	header->syncHeader.headCRC.bytes[2] = crc[0];
	header->syncHeader.headCRC.bytes[3] = crc[1];
	header->syncHeader.control			= 0;
	header->syncHeader.msgid.bytes[0]   = (msgid >> 24) & 0xff;
	header->syncHeader.msgid.bytes[1]   = (msgid >> 16) & 0xff;
	header->syncHeader.msgid.bytes[2]   = (msgid >> 8) & 0xff;
	header->syncHeader.msgid.bytes[3]   = (msgid) &0xff;
	header->syncHeader.frameHeader.word = SOCKET_SYNC_HEADER;
}

void localsocket_set_header(int funcode, int opercode, U8* msg, U32 size)
{
	MSG_DATA* header = NULL;
	// BOOL	  res			   = FALSE;
	U32 llen				   = ((size));
	header					   = (MSG_DATA*) msg;
	header->header.reserved[0] = 0;
	header->header.reserved[1] = 0;
	header->header.dataLen[3]  = (U8)(llen & 0xff);
	header->header.dataLen[2]  = (U8)((llen >> 8) & 0xff);
	header->header.dataLen[1]  = (U8)((llen >> 16) & 0xff);
	header->header.dataLen[0]  = (U8)((llen >> 24) & 0xff);
	memcpy(header->header.srcID, localsocket_regs.pid, MSG_ID_SIZE);
	header->header.funcCode = funcode;
	header->header.operCode = opercode;
}

int localsocket_findsocktypebypid(char* pid)
{
	U32 i   = 0;
	int res = LOCALSOCKET_SOCKTYPE_UNKNOW;
	for (i = 0; i < LOCALSOCKET_MAX_DST_PID; i++) {
		if (0 == memcmp(localsocket_regs.conpid[i].pid, pid, MSG_ID_SIZE)) {
			res = localsocket_regs.conpid[i].contype;
		}
	}
	return (res);
}

BOOL localsocket_tcp_send(char* pid, U8* msg, U32 len, BOOL waitack, U32 timeout)
{
	BOOL res = FALSE;
	res		 = localsocket_send_process(pid, msg, len, waitack, LOCALSOCKET_SOCKTYPE_SHORTTCP, FALSE, FALSE, NULL, timeout);
	return (res);
}

BOOL localsocket_udp_send(char* pid, U8* msg, U32 len, BOOL waitack, U32 timeout)
{
	BOOL res = FALSE;
	res		 = localsocket_send_process(pid, msg, len, waitack, LOCALSOCKET_SOCKTYPE_UDP, FALSE, FALSE, NULL, timeout);
	return (res);
}

BOOL localsocket_sendackbysocktype(char* pid, U8* msg, U32 len, U32 msgid, int socktype)
{
	return TRUE;
}
