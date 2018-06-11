#ifndef _PORTAL_CLIENT_H_H__
#define _PORTAL_CLIENT_H_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "pthread.h"
#include <uuid/uuid.h>


#include "osa/osa.h"
#include "osa/osa_thr.h"
#include "osa/osa_mutex.h"
#include "osa/osa_timer.h"
#include "osa/osa_debug.h"
#include "osa/osa_mem.h"
#include "osa/osa_file.h"

#include "deviceinfo.h"
#include "network.h"
#include "cfg_ini.h"
#include "localsocket.h"
#include "ui_interface.h"


#include "curl.h"
#include "json/cJSON.h"
#include "nopoll.h"

#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include <openssl/md5.h>


#include "CSR.h"
#include "PushEventToServer.h"
#include "APPManager.h"
#include "NotifyUI.h"

#include "devicemanager_interface.h" 
#include "process_interface.h"
#include "setting_interface.h"

#include "EventMaintain.h"


//#define PORTAL_CLIENT_RELEASE
#define DEBUG_HEADER_PORTALCLIENT_RECEIVE_UI_MSG  "[UI->PortalClient] "
#define DEBUG_HEADER_PORTALCLIENT_SENDTO_UI_MSG  "[PortalClient->UI] "

#define DEBUG_HEADER_PORTALCLIENT "[PortalClient] "

#define DEBUG_HEADER_PORTALCLIENTRECEIVE  "[PortalClientReceive] "

#define DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND "[Others->PortalClientSendThread] "
#define DEBUG_HEADER_PORTALCLIENTSEND "[PortalClientSend] "

#define DEBUG_HEADER_PORTALCLIENT_SENDTO_IPGW_MSG  "[PortalClient->IPGW] "

#define DEBUG_HEADER_PORTALCLIENT_RECEIVE_NETWORK_MSG  "[Network->PortalClient] "



//操作码
#define OPER_LOGIN_PORTAL_SERVER  0x63
#define OPER_LOGOUT_PORTAL_SERVER 0x64
#define OPER_CHECK_PORTAL_SERVER  0x65

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

#define WEB_SOCKET_PAYLOAD_MAXLEN 10*1024  

//#define DEVICE_TYPE JSON_DEVICE_TYPE_DESAP
#define DEVICE_TYPE JSON_DEVICE_TYPE_IPGATEWAY
#define LOCAL_SIP_SERVER 1   //定义后会使用sipserver BJE_SIP_SERVER_LOCAL "abb.chinacloudapp.cn" ，注释掉则使用  ABB_SIP_SERVER "sip.my.busch-jaeger.de"


extern INT fnSendMsg2PortClientSendThread(BYTE *pSrcDataBuf, UINT32 u32DataLen);
extern int Base64Dec(unsigned char * buf, char * text, int size);
extern VOID LockWebsocketMutex(VOID);
extern VOID UnLockWebsocketMutex(VOID);
extern VOID LockEventMutex(VOID);
extern VOID UnLockEventMutex(VOID);
extern VOID UnBlockEventCond(VOID);
extern INT WaitRespForPushEventToPortalServer(struct timespec * pstWaittime);
extern noPollConn * GetNoPollConn(VOID);
extern INT GetConnectState(VOID);
extern VOID SetHeartBeatCount(INT iValue);
extern INT NoPollSafeLoopStop(VOID);
extern VOID DeleleAllCertAndConfig(VOID);
extern char * GetPortalServerUsrName(VOID);
extern VOID DeleteAllPairedApp(VOID);
extern VOID UpdateAuthFileDomain(char* domain);
extern VOID DeleteAppBySenderUUID(void* param);
extern VOID LoadPortClientLoginStatus(VOID);
extern VOID SetPortalClientLoginStatus(ePORTAL_CLIENT_LOGIN_STATUS eStatus);
extern VOID SetPortalClientConnectStatus(ePORTAL_CLIENT_CONNECT_STATUS eStatus);
extern VOID ReplyPortalClientStatusToUI(VOID);
extern VOID DeletePairedAppBySection(char* section);
extern DEVICEMANAGER_LIST * GetOSList(VOID);
extern DEVICEMANAGER_LIST * GetSecondDoorStationList(VOID);
extern DEVICEMANAGER_LIST * GetGateStationList(VOID);
extern VOID NotifyPortalServerACLForAllPairedApp(VOID);
extern VOID UpdateFlexisipServeIPAddress(VOID);
extern VOID UpdateRecordToRouteFile(VOID);
extern char * GetExternalIP(VOID);
extern VOID LogOutDeal(VOID);
extern INT fnPortalClientSendMsg2IPGateway(BYTE u8OperCode, BYTE * pSrcDataBuf, UINT32 u32DataLen);
extern VOID SaveB2BInternalUri(char* domain,int port,char* protocal);
extern VOID SaveB2BExternalUri(char* domain,int port,char* protocal);
extern VOID UpdateProgramButton(VOID);
extern VOID fnCreatePortalClientTCPServerThread(VOID);

#endif

