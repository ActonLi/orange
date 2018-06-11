#include "PortalClient.h"


extern int  SendCertSignedRequest(char* username,char* password, char* fname);


LOCAL INT32 s_h2PortalClientSendFd = -1;
LOCAL OSA_ThrHndl s_hPortalClientSendThread = {0};
LOCAL char s_pu8PortalUsrName[64];
LOCAL pthread_mutex_t  s_Mutex_PortalClientLoginStatus;
LOCAL BYTE u8PortalClientLoginStatus = PORTAL_CLIENT_STATUS_LOGOUT;
LOCAL pthread_mutex_t  s_Mutex_PortalClientConnectStatus;
LOCAL BYTE u8PortalClientConnectStatus = PORTAL_CLIENT_STATUS_CONNECT_FAIL;


VOID SetPortalClientLoginStatus(ePORTAL_CLIENT_LOGIN_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_PortalClientLoginStatus);
	u8PortalClientLoginStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_PortalClientLoginStatus);
	NotifyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID SetPortalClientConnectStatus(ePORTAL_CLIENT_CONNECT_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_PortalClientConnectStatus);
	u8PortalClientConnectStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_PortalClientConnectStatus);
	NotifyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID ReplyPortalClientStatusToUI(VOID)
{
	ReplyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID LoadPortClientLoginStatus(VOID)
{
	char pUUID[128];
	
	OSA_MemSet(pUUID,0,sizeof(pUUID));
	
	GetValueFromEtcFile(PATH_WELCOME_CFG,WELCOME_SECTION_NETWORK,WELCOME_KEY_UUID,pUUID,sizeof(pUUID)-1);
	if( strcmp(pUUID,"") == 0 )
	{
		SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGOUT);
	}
	else
	{
		SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGIN);
	}
}

VOID SetPortalServerUsrName(char * pu8UsrName)
{
	sprintf(s_pu8PortalUsrName,"%s",pu8UsrName);
	SetValueToEtcFile(PATH_PORTAL_SERVER_CFG,PORTAL_SERVER_SECTION,PORTAL_SERVER_USER_KEY,s_pu8PortalUsrName);
	SaveIniData(PATH_PORTAL_SERVER_CFG);
}

char * GetPortalServerUsrName(VOID)
{
	GetValueFromEtcFile(PATH_PORTAL_SERVER_CFG,PORTAL_SERVER_SECTION,PORTAL_SERVER_USER_KEY,s_pu8PortalUsrName,64);
	return s_pu8PortalUsrName;
}
			

LOCAL VOID fnPortalClientHandleMessageFromUI(BYTE * msg_buf,UINT32 len)
{
	SOCK_DATA_PACKET_T *pNetCmdHeader = (SOCK_DATA_PACKET_T *)msg_buf;
	
	BYTE * pDataBuf = &msg_buf[sizeof(SOCK_DATA_PACKET_T)];	
	
	#ifndef PORTAL_CLIENT_RELEASE
	UINT32 u32DataLenTmp = len - sizeof(SOCK_DATA_PACKET_T);
	BYTE * pDataBufTmp = &msg_buf[sizeof(SOCK_DATA_PACKET_T)];	
	printf("%s\n",DEBUG_HEADER_PORTALCLIENT_RECEIVE_UI_MSG);
	printf("\t\tDst Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dstId[0],pNetCmdHeader->dstId[1],pNetCmdHeader->dstId[2],pNetCmdHeader->dstId[3],pNetCmdHeader->dstId[4],pNetCmdHeader->dstId[5]);
	printf("\t\tSrc Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->srcId[0],pNetCmdHeader->srcId[1],pNetCmdHeader->srcId[2],pNetCmdHeader->srcId[3],pNetCmdHeader->srcId[4],pNetCmdHeader->srcId[5]);
	printf("\t\tFuncCode: [0x%02x]\n",pNetCmdHeader->funcCode);
	printf("\t\tOperCode: [0x%02x]\n",pNetCmdHeader->operCode);
	printf("\t\tDataLen:  [0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dataLen[0],pNetCmdHeader->dataLen[1],pNetCmdHeader->dataLen[2],pNetCmdHeader->dataLen[3]);  //大端，高位存高字节
	printf("\t\tData[");
	int i;
	for(i = 0; i < u32DataLenTmp; i++)
	{
		printf("0x%02x ",pDataBufTmp[i]);
	}
	printf("]\n\n"); 
	#endif

	switch (pNetCmdHeader->operCode)
	{
		case OPER_LOGIN_PORTAL_SERVER:
		{
			char pu8UsrName[64];
			char pu8UsrPasswd[128];
			char pu8FriendName[64];

			OSA_MemSet(pu8UsrName,0,sizeof(pu8UsrName));
			OSA_MemSet(pu8UsrPasswd,0,sizeof(pu8UsrPasswd));
			OSA_MemSet(pu8FriendName,0,sizeof(pu8FriendName));

			OSA_MemCopy(pu8UsrName,&pDataBuf[0],sizeof(pu8UsrName));
			OSA_MemCopy(pu8UsrPasswd,&pDataBuf[64],sizeof(pu8UsrPasswd));
			OSA_MemCopy(pu8FriendName,&pDataBuf[64+128],sizeof(pu8FriendName));

			OSA_DBG_MSG("%sUsrName([%s]) UsrPasswd([%s]) FriendName([%s])",DEBUG_HEADER_PORTALCLIENT,pu8UsrName,pu8UsrPasswd,pu8FriendName);
			
			/* ca-certificates.crt / openssl.cfg Needed to perform SendCertSignedRequest*/
			if( 0 == SendCertSignedRequest(pu8UsrName,pu8UsrPasswd,pu8FriendName))
			{
				/*通知UI 登录成功*/
				/*Portal Server 的UserName 需要用来生成app的sipname*/
				SetPortalServerUsrName(pu8UsrName);
				NotifyUILoginResult(PORTAL_CLIENT_LOGIN_SUCCESS);  //弹出Login portal successful!  对话框
				SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGIN);
			}
			else /*失败*/
			{
				NotifyUILoginResult(PORTAL_CLIENT_LOGIN_FAIL);
			}
		}
		break;
		
		case OPER_LOGOUT_PORTAL_SERVER:
		{
			if( 0 == RemoveDeviceFromPortalServer())
			{
				/*通知UI Logout 成功*/
				NotifyUILogoutResult(PORTAL_CLIENT_LOGOUT_SUCCESS);  
				SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGOUT);
				/*通知UI更新APP列表*/
				NotifyUIWelcomeAppList(PORTAL_CLIENT_PUSH);
			}
			else  
			{
				/*通知失败*/
				NotifyUILogoutResult(PORTAL_CLIENT_LOGOUT_FAIL);
			}
		}
		break;
		
		case OPER_CHECK_PORTAL_SERVER:
		{
			ReplyPortalClientStatusToUI();
		}
		break;

		case OPER_UI_REQUEST_APP_LIST:
		{
			NotifyUIWelcomeAppList(PORTAL_CLIENT_REPLY);
		}
		break;
		
		case OPER_UI_EDIT_APP_ACCESS_CONTROL:
		{
			SaveAPPAccessProperty(pDataBuf);
			ReplyUIPortalClientAccessControl(pDataBuf);
		}
		break;
		
		case OPER_UI_DELETE_APP :
		{
			BYTE u8AppIndex = pDataBuf[0];

			char pWelcomeAPPSection[16];
			sprintf(pWelcomeAPPSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,u8AppIndex);

			DeletePairedAppBySection(pWelcomeAPPSection);
			ReplyUIPortalClientDeleteResult(u8AppIndex,1);
		}
		break;
		
		case OPER_UI_SEND_APP_INTIGRITY_CODE:
		{
			SaveAPPAccessProperty(pDataBuf);
			
			BYTE u8Index = pDataBuf[0];
			char pWelcomeAPPSection[16];
			BYTE pInterityCode[9];
			int i = 0;
			char pcTmp[2];
			OSA_MemSet(pInterityCode,0,sizeof(pInterityCode));

			sprintf(pWelcomeAPPSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,u8Index);

			for(i = 0; i < 8; i++)
			{
				sprintf(pcTmp,"%x",pDataBuf[i+7]);
				strncat(pInterityCode,pcTmp,1);
			}
				
			INT iRet = ActivatePairingAPP(pWelcomeAPPSection,pInterityCode);
			if(iRet == 0) //通知UI 配对成功
			{
				ReplyUIPortalClientPairResult(u8Index, PORTAL_CLIENT_PAIR_SUCCESS, &pDataBuf[1]);
			}
			else	//通知UI 配对失败
			{
				ReplyUIPortalClientPairResult(u8Index, PORTAL_CLIENT_PAIR_FAIL, &pDataBuf[1]);
			}
		}
		break;
		
		default:
		{
			OSA_ERRORX("Unknown UI Command");
		}
		break;
	}
	
}


LOCAL VOID fnPortalClientSendThreadDealMsgTask(void *argv)
{
	INT32 u32DataLen = 0;
	BYTE pDataBuf[2048];
	INT32 iRet = 0;

	s_PacketMsg stHeaderMsg = { 0 };
	
	while (1)
    {
    	OSA_MemSet(pDataBuf,0,2048);
		iRet = LocalSocket_UDPRecvTinyMsg(s_h2PortalClientSendFd, (unsigned char *)&stHeaderMsg, INTER_PACKET_SIZE, pDataBuf, &u32DataLen, 500);
		if (0 == iRet)
		{
			switch (stHeaderMsg.order_type)
			{
				case PHONE_ORDER_ENTER_QT:
				{
					fnPortalClientHandleMessageFromUI(pDataBuf,u32DataLen);
				}
				break;

				case PHONE_ORDER_ENTER_LOGIC:  //Events 推送
				{
					PushEventToPortalServer(pDataBuf,u32DataLen);
				}
				break;

				default:
				break;
			}
		}
	}

}


VOID fnCreatePortalClientSendThread(VOID)
{
    int iRet = OSA_EFAIL;

	s_h2PortalClientSendFd = LocalSocket_UDPServer(kPORT_MSG2PORTALCLIENT);
	if (s_h2PortalClientSendFd < 0)
	{
		OSA_ERROR("\n\nPortal Client Send Thread bind Local Socket failed.");
		exit(1);
	}

	/*接收本进程其他子线程的消息*/
   	iRet = OSA_ThrCreate(&s_hPortalClientSendThread, (void *)fnPortalClientSendThreadDealMsgTask, OSA_THR_PRI_FIFO_MIN, NULL);// 64KBytes stack pri:50
	if (iRet)
	{
		OSA_ERROR ("Create PortalClient Send Thread error!\n");
		exit(1);
	}
}

INT fnSendMsg2PortClientSendThread(BYTE *pSrcDataBuf, UINT32 u32DataLen)
{
	BYTE aNetCmdDat[1024] = { 0 };
	SOCK_DATA_PACKET_T *pNetCmdHeader = (SOCK_DATA_PACKET_T *)aNetCmdDat;
	BYTE *pDestDataBuf = &aNetCmdDat[sizeof(SOCK_DATA_PACKET_T)]; 			/*数据段的起始地址*/

	OSA_MemCopy(pDestDataBuf,pSrcDataBuf,u32DataLen);

	pNetCmdHeader->dstId[0] = 0x00;
	pNetCmdHeader->srcId[0] = 0x00;
	pNetCmdHeader->funcCode = 0x00;
	pNetCmdHeader->operCode = OPER_PUSH_EVENT_TO_PORTAL_SERVER;
	MAKE_DATALEN(pNetCmdHeader->dataLen, u32DataLen);

#if 0
	#ifndef FH_RELEASE
	printf("%s\n",DEBUG_HEADER_PORTALCLIENT);
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

	s_PacketMsg stHeaderMsg = { 0 };

	stHeaderMsg.order_type = PHONE_ORDER_ENTER_LOGIC;
	stHeaderMsg.sockfd = -1;
	stHeaderMsg.datalen = sizeof(SOCK_DATA_PACKET_T) + u32DataLen;

	return LocalSocket_UDPSendMsg(kPORT_MSG2PORTALCLIENT, (BYTE*)&stHeaderMsg, INTER_PACKET_SIZE, aNetCmdDat,sizeof(SOCK_DATA_PACKET_T) + u32DataLen); 

}



