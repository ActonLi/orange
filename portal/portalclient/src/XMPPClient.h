#ifndef _XMPP_CLIENT_H_H__
#define _XMPP_CLIENT_H_H__

#include "pthread.h"
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <uuid/uuid.h>

#include "osa.h"
#include "osa_debug.h"
#include "osa_file.h"
#include "osa_mem.h"
#include "osa_mutex.h"
#include "osa_thr.h"
#include "osa_timer.h"

#include "cfg_ini.h"
#include "deviceinfo.h"
#include "localsocket.h"
#include "network.h"
#include "ui_interface.h"

#include "cJSON.h"

#include "openssl/err.h"
#include "openssl/pem.h"
#include "openssl/rsa.h"
#include <openssl/md5.h>

#include "devicemanager_interface.h"
#include "process_interface.h"

//#define XMPP_CLIENT_RELEASE
#define DEBUG_HEADER_XMPPCLIENT_RECEIVE_UI_MSG "[UI->XmppClient] "
#define DEBUG_HEADER_XMPPCLIENT_SENDTO_UI_MSG "[XmppClient->UI] "

#define DEBUG_HEADER_XMPPCLIENT "[XmppClient] "

#define DEBUG_HEADER_XMPPCLIENTRECEIVE "[XmppClientReceive] "

#define DEBUG_HEADER_OTHERS_TO_XMPPCLIENTSEND "[Others->XmppClientSendThread] "
#define DEBUG_HEADER_XMPPCLIENTSEND "[XmppClientSend] "

#define DEBUG_HEADER_XMPPCLIENT_SENDTO_IPGW_MSG "[XmppClient->IPGW] "

#define EVENT_XMPP_DELETE_DEVICE "client deleted/blocked or revoked its certificate"
#define EVENT_TYPE_SUCCESS "com.abb.ispf.event.success"

//??????
#define OPER_LOGIN_XMPP_SERVER 0x63
#define OPER_LOGOUT_XMPP_SERVER 0x64
#define OPER_CHECK_XMPP_SERVER 0x65

#define OPER_PUSH_EVENT_TO_XMPP_SERVER 0x66
#define OPER_NOTIFY_UI_UPDATE_APP_LIST 0x69
#define OPER_UI_REQUEST_APP_LIST 0x6a
#define OPER_UI_EDIT_APP_ACCESS_CONTROL 0x6b
#define OPER_UI_DELETE_APP 0x6c
#define OPER_UI_SEND_APP_INTIGRITY_CODE 0x6d
#define OPER_LOGIC_PUSH_STATUS 0x6e

/*XMPP domain/port url for each application*/
#define XMPP_WSS_PORT "443"
#define WSS_API "/socket"

//#define WEB_SOCKET_PAYLOAD_MAXLEN 10*1024

typedef enum {
	XMPP_CLIENT_STATUS_LOGIN  = 0x01,
	XMPP_CLIENT_STATUS_LOGOUT = 0x02,
} eXMPP_CLIENT_LOGIN_STATUS;

typedef enum {
	XMPP_CLIENT_STATUS_CONNECT_FAIL	= 0x01,
	XMPP_CLIENT_STATUS_CONNECT_SUCCESS = 0x02,
} eXMPP_CLIENT_CONNECT_STATUS;

void fnXmppSendMessage(const char* const jid, const char* const text);
char* fnXmppGetJID(void);

#endif
