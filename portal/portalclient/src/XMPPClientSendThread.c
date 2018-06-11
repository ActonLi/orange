#include "XMPPClient.h"
#include <strophe.h>

#define kPORT_MSG2XMPPCLIENT "/tmp/UDN_Msg2XmppClient"

LOCAL INT32 s_h2XmppClientSendFd		  = -1;
LOCAL OSA_ThrHndl s_hXmppClientSendThread = {0};
LOCAL char		  s_pu8XmppUsrName[64];
LOCAL pthread_mutex_t s_Mutex_XmppClientLoginStatus;
LOCAL BYTE u8XmppClientLoginStatus = XMPP_CLIENT_STATUS_LOGOUT;
LOCAL pthread_mutex_t s_Mutex_XmppClientConnectStatus;
LOCAL BYTE u8XmppClientConnectStatus = XMPP_CLIENT_STATUS_CONNECT_FAIL;

// extern xmpp_ctx_t *g_ctx;
// extern xmpp_conn_t * g_conn;
extern char* g_jid;

VOID SetXmppClientLoginStatus(eXMPP_CLIENT_LOGIN_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_XmppClientLoginStatus);
	u8XmppClientLoginStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_XmppClientLoginStatus);
	// NotifyUIXmppClientStatus(u8XmppClientLoginStatus,u8XmppClientConnectStatus);
}

VOID SetXmppClientConnectStatus(eXMPP_CLIENT_CONNECT_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_XmppClientConnectStatus);
	u8XmppClientConnectStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_XmppClientConnectStatus);
	// NotifyUIXmppClientStatus(u8XmppClientLoginStatus,u8XmppClientConnectStatus);
}

VOID LoadXmppClientLoginStatus(VOID)
{
	char pUUID[128];

	OSA_MemSet(pUUID, 0, sizeof(pUUID));

	// GetValueFromEtcFile(PATH_WELCOME_CFG,WELCOME_SECTION_NETWORK,WELCOME_KEY_UUID,pUUID,sizeof(pUUID)-1);
	if (strcmp(pUUID, "") == 0) {
		SetXmppClientLoginStatus(XMPP_CLIENT_STATUS_LOGOUT);
	} else {
		SetXmppClientLoginStatus(XMPP_CLIENT_STATUS_LOGIN);
	}
}

VOID SetXmppServerUsrName(char* pu8UsrName)
{
	sprintf(s_pu8XmppUsrName, "%s", pu8UsrName);
	// SetValueToEtcFile(PATH_XMPP_SERVER_CFG,XMPP_SERVER_SECTION,XMPP_SERVER_USER_KEY,s_pu8XmppUsrName);
	// SaveIniData(PATH_XMPP_SERVER_CFG);
}

char* GetXmppServerUsrName(VOID)
{
	// GetValueFromEtcFile(PATH_XMPP_SERVER_CFG,XMPP_SERVER_SECTION,XMPP_SERVER_USER_KEY,s_pu8XmppUsrName,64);
	return s_pu8XmppUsrName;
}

LOCAL VOID fnXmppClientSendThreadDealMsgTask(void* argv)
{
	INT32 u32DataLen = 0;
	BYTE  pDataBuf[2048];
	INT32 iRet = 0;

	s_PacketMsg stHeaderMsg = {0};

	while (1) {
		OSA_MemSet(pDataBuf, 0, 2048);
		iRet = LocalSocket_UDPRecvTinyMsg(s_h2XmppClientSendFd, (unsigned char*) &stHeaderMsg, INTER_PACKET_SIZE, pDataBuf, &u32DataLen, 500);
		if (0 == iRet) {
			OSA_DBG_MSG("stHeaderMsg.order_type = 0x%x \n", stHeaderMsg.order_type);
			switch (stHeaderMsg.order_type) {
				case 0x01: // Login
				{
					OSA_DBG_MSG("Login xmpp server \n");
				} break;

				case 0x02: // Logout
				{
					OSA_DBG_MSG("Logout xmpp server \n");
					// PushEventToXmppServer(pDataBuf,u32DataLen);
				} break;

				case 0x03: // Send msg to roster jid
				{
					OSA_DBG_MSG("Send msg to roster jid \n");
					fnXmppSendMessage("725f834a-2c41-4d22-a188-6ba3085f9d86@xmpp.my-staging.busch-jaeger.de", "hello");
				} break;

				case 0x04: // Send myjid to roster jid
				{
					OSA_DBG_MSG("Send myjid to roster jid \n");
					fnXmppSendMessage("725f834a-2c41-4d22-a188-6ba3085f9d86@xmpp.my-staging.busch-jaeger.de", g_jid);
				} break;

				default:
					break;
			}
		}
	}
}

VOID fnCreateXmppClientSendThread(VOID)
{
	int iRet = OSA_EFAIL;

	s_h2XmppClientSendFd = LocalSocket_UDPServer(kPORT_MSG2XMPPCLIENT);
	if (s_h2XmppClientSendFd < 0) {
		OSA_ERROR("XMPP Client Send Thread bind Local Socket failed.");
		exit(1);
	}

	iRet = OSA_ThrCreate(&s_hXmppClientSendThread, (void*) fnXmppClientSendThreadDealMsgTask, OSA_THR_PRI_FIFO_MIN, NULL); // 64KBytes stack pri:50
	if (iRet) {
		OSA_ERROR("Create XmppClient Send Thread error!\n");
		exit(1);
	}
}

// fnSendMsg2XmppClientSendThread, ????:  ??Recv ?߳??յ???Ϣ????Ҫת????Send ?߳?
INT fnSendMsg2XmppClientSendThread(BYTE* pSrcDataBuf, UINT32 u32DataLen)
{
	BYTE				aNetCmdDat[1024] = {0};
	SOCK_DATA_PACKET_T* pNetCmdHeader	= (SOCK_DATA_PACKET_T*) aNetCmdDat;
	BYTE*				pDestDataBuf	 = &aNetCmdDat[sizeof(SOCK_DATA_PACKET_T)]; /*???ݶε???ʼ??ַ*/

	OSA_MemCopy(pDestDataBuf, pSrcDataBuf, u32DataLen);

	pNetCmdHeader->dstId[0] = 0x00;
	pNetCmdHeader->srcId[0] = 0x00;
	pNetCmdHeader->funcCode = 0x00;
	pNetCmdHeader->operCode = OPER_PUSH_EVENT_TO_XMPP_SERVER;
	MAKE_DATALEN(pNetCmdHeader->dataLen, u32DataLen);

#if 0
#ifndef FH_RELEASE
	printf("%s\n",DEBUG_HEADER_XMPPCLIENT);
	printf("\t\tDst Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dstId[5],pNetCmdHeader->dstId[4],pNetCmdHeader->dstId[3],pNetCmdHeader->dstId[2],pNetCmdHeader->dstId[1],pNetCmdHeader->dstId[0]);
	printf("\t\tSrc Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->srcId[5],pNetCmdHeader->srcId[4],pNetCmdHeader->srcId[3],pNetCmdHeader->srcId[2],pNetCmdHeader->srcId[1],pNetCmdHeader->srcId[0]);
	printf("\t\tFuncCode: [0x%02x]\n",pNetCmdHeader->funcCode);
	printf("\t\tOperCode: [0x%02x]\n",pNetCmdHeader->operCode);
	printf("\t\tDataLen:  [0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dataLen[3],pNetCmdHeader->dataLen[2],pNetCmdHeader->dataLen[1],pNetCmdHeader->dataLen[0]);
	printf("\t\tData[");
	int i;
	for(i = 0; i < u32DataLen; i++)
	{
		printf("0x%02x ",pDestDataBuf[i]);
	}
	printf("]\n\n");
#endif
#endif

	s_PacketMsg stHeaderMsg = {0};

	stHeaderMsg.order_type = PHONE_ORDER_ENTER_LOGIC;
	stHeaderMsg.sockfd	 = -1;
	stHeaderMsg.datalen	= sizeof(SOCK_DATA_PACKET_T) + u32DataLen;

	return LocalSocket_UDPSendMsg(kPORT_MSG2XMPPCLIENT, (BYTE*) &stHeaderMsg, INTER_PACKET_SIZE, aNetCmdDat, sizeof(SOCK_DATA_PACKET_T) + u32DataLen);
}
