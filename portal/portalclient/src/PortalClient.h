#ifndef _PORTAL_CLIENT_H_H__
#define _PORTAL_CLIENT_H_H__

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
#include "curl.h"
#include "nopoll.h"

#include "openssl/err.h"
#include "openssl/pem.h"
#include "openssl/rsa.h"
#include <openssl/md5.h>

#include "APPManager.h"
#include "CSR.h"
#include "NotifyUI.h"
#include "PushEventToServer.h"

#include "devicemanager_interface.h"
#include "process_interface.h"

#define PORTAL_CLIENT_RELEASE
#define DEBUG_HEADER_PORTALCLIENT_RECEIVE_UI_MSG "[UI->PortalClient] "
#define DEBUG_HEADER_PORTALCLIENT_SENDTO_UI_MSG "[PortalClient->UI] "

#define DEBUG_HEADER_PORTALCLIENT "[PortalClient] "

#define DEBUG_HEADER_PORTALCLIENTRECEIVE "[PortalClientReceive] "

#define DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND "[Others->PortalClientSendThread] "
#define DEBUG_HEADER_PORTALCLIENTSEND "[PortalClientSend] "

#define DEBUG_HEADER_PORTALCLIENT_SENDTO_IPGW_MSG "[PortalClient->IPGW] "

//??????
#define OPER_LOGIN_PORTAL_SERVER 0x63
#define OPER_LOGOUT_PORTAL_SERVER 0x64
#define OPER_CHECK_PORTAL_SERVER 0x65

#define OPER_PUSH_EVENT_TO_PORTAL_SERVER 0x66
#define OPER_NOTIFY_UI_UPDATE_APP_LIST 0x69
#define OPER_UI_REQUEST_APP_LIST 0x6a
#define OPER_UI_EDIT_APP_ACCESS_CONTROL 0x6b
#define OPER_UI_DELETE_APP 0x6c
#define OPER_UI_SEND_APP_INTIGRITY_CODE 0x6d
#define OPER_LOGIC_PUSH_STATUS 0x6e

/*Portal domain/port url for each application*/
#define PORTAL_WSS_PORT "443"
#define WSS_API "/socket"

#define WEB_SOCKET_PAYLOAD_MAXLEN 10 * 1024

//#define DEVICE_TYPE JSON_DEVICE_TYPE_DESAP
//#define DEVICE_TYPE JSON_DEVICE_TYPE_IPGATEWAY
#define DEVICE_TYPE JSON_DEVICE_TYPE_FREEATHOME

extern INT fnSendMsg2PortClientSendThread(BYTE* pSrcDataBuf, UINT32 u32DataLen);
extern int Base64Dec(unsigned char* buf, char* text, int size);
extern VOID LockWebsocketMutex(VOID);
extern VOID UnLockWebsocketMutex(VOID);
extern VOID LockEventMutex(VOID);
extern VOID UnLockEventMutex(VOID);
extern VOID UnBlockEventCond(VOID);
extern INT WaitRespForPushEventToPortalServer(struct timespec* pstWaittime);
extern noPollConn* GetNoPollConn(VOID);
extern INT		   GetConnectState(VOID);
extern VOID SetHeartBeatCount(INT iValue);
extern INT   NoPollSafeLoopStop(VOID);
extern VOID  DeleleAllCertAndConfig(VOID);
extern char* GetPortalServerUsrName(VOID);
extern VOID  DeleteAllPairedApp(VOID);
extern VOID UpdateAuthFileDomain(char* domain);
// extern VOID DeleteAppBySenderUUID(void* param);
extern VOID LoadPortClientLoginStatus(VOID);
extern VOID SetPortalClientLoginStatus(ePORTAL_CLIENT_LOGIN_STATUS eStatus);
extern VOID SetPortalClientConnectStatus(ePORTAL_CLIENT_CONNECT_STATUS eStatus);
extern VOID ReplyPortalClientStatusToUI(VOID);
// extern VOID DeletePairedAppBySection(char* section);
// extern DEVICEMANAGER_LIST * GetOSList(VOID);
// extern VOID NotifyPortalServerACLForAllPairedApp(VOID);
extern VOID UpdateFlexisipServeIPAddress(VOID);
// extern VOID UpdateRecordToRouteFile(VOID);
extern char* GetExternalIP(VOID);
extern VOID  LogOutDeal(VOID);
// extern INT fnPortalClientSendMsg2IPGateway(BYTE u8OperCode, BYTE * pSrcDataBuf, UINT32 u32DataLen);
// extern VOID SaveB2BInternalUri(char* domain,int port,char* protocal);
// extern VOID SaveB2BExternalUri(char* domain,int port,char* protocal);
extern VOID fnPortalClientHandleMessageFromUI(BYTE* msg_buf, UINT32 len);
extern VOID fnPortalClientSendThreadDealMsgTask(void* argv);

#endif
