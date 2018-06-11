#include "PortalClient.h"

INT fnPortalClientSendMsg2UI(BYTE u8OperCode, BYTE * pSrcDataBuf, UINT32 u32DataLen)
{
	BYTE aNetCmdDat[1024] = { 0 };   //最多有4 台APP, 必须留足够的Buffer，否则会溢出，出现段错误
	SOCK_DATA_PACKET_T *pNetCmdHeader = (SOCK_DATA_PACKET_T *)aNetCmdDat;
	BYTE *pDestDataBuf = &aNetCmdDat[sizeof(SOCK_DATA_PACKET_T)]; 			/*数据段的起始地址*/

	if(u32DataLen != 0)
	{
		OSA_MemCopy(pDestDataBuf,pSrcDataBuf,u32DataLen);
	}
	
	pNetCmdHeader->dstId[0] = 0x00;
	pNetCmdHeader->srcId[0] = 0x00;
	pNetCmdHeader->funcCode = UI_FUNC_SETTING;
	pNetCmdHeader->operCode = u8OperCode;
	MAKE_DATALEN(pNetCmdHeader->dataLen, u32DataLen);
	
	#ifndef PORTAL_CLIENT_RELEASE
	printf("%s\n",DEBUG_HEADER_PORTALCLIENT_SENDTO_UI_MSG);
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

	Send2UI(pDestDataBuf, u32DataLen, UI_FUNC_SETTING, u8OperCode);

	return 0;
}



VOID NotifyUIWelcomeAppList(ePUSH_REPLY_FLAG eFlag)
{
	BYTE u8APPCount = 0;
	INT  i =0;
	char pAppSection[16] = {0};
	char pNameValue[64] = {0};
	char pPairStatus[64] = {0};
	char u8PairStatus = 0;
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 1;
	char pAccess[4];
	UINT32 u32Offset = 1;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	APP_ITEM stAppItem[MAX_PAIRING_USER];

	/*从WelcomeAPP_1 开始遍历， 最多遍历4  条APP 记录*/
	for(i = 0;i < MAX_PAIRING_USER; i++)
	{
		APP_ITEM * pAPPItem = &stAppItem[i];
		OSA_MemSet(pAPPItem,0,sizeof(APP_ITEM));
		sprintf(pAppSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,i+1);  
		if(GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pAppSection,WELCOME_APP_PAIR_REQUEST_KEY_NAME,pNameValue,64) == 0 )
		{
			u8APPCount++;
			pAPPItem->u8Index = i+1;
			sprintf(pAPPItem->pcName,"%s",pNameValue);
			if(GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pAppSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE,pPairStatus,64) == 0 )
			{
				if(strcmp(pPairStatus,WELCOME_APP_PAIR_STATE_PAIRED) == 0)
				{
					u8PairStatus = 1;
				}
				else
				{
					u8PairStatus = 0;
				}
				
				pAPPItem->u8PairStatus = u8PairStatus;

				
				//通话权限
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_CONVERSATION_KEY,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8ConversationAccess = 1;
				}

				//监视权限
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_SURVEILLANCE_KEY,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8SurveillanceAccess = 1;
				}

				//开锁权限
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_OPENDOOR_KEY,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8OpenDoorAccess = 1;
				}

				//开灯权限
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_SWITCHLIGHT_KEY,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8SwitchLightAccess = 1;
				}

				//查看通话记录
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_ACCESSHISTORY_KEY ,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8ReviewHistoryAccess = 1;
				}

				//删除通话记录
				OSA_MemSet(pAccess,0,sizeof(pAccess));
				GetValueFromEtcFile(PATH_WELCOME_CFG,pAppSection,WELCOME_APP_PRIVILEGE_DELETEHISTORY_KEY,pAccess,3);
				if(strcmp(pAccess,"yes") == 0)
				{
					pAPPItem->u8DeleteHistoryAccess = 1;
				}
			}

			OSA_MemCopy(&pu8Send2UIData[u32Offset],&stAppItem[i],sizeof(APP_ITEM));
			u32SendLen += sizeof(APP_ITEM);
			u32Offset += sizeof(APP_ITEM);
		}
	}

	pu8Send2UIData[0] = u8APPCount;

	if(eFlag == PORTAL_CLIENT_PUSH)
	{
		fnPortalClientSendMsg2UI(OPER_NOTIFY_UI_UPDATE_APP_LIST,pu8Send2UIData,u32SendLen);
	}
	else
	{
		fnPortalClientSendMsg2UI(OPER_UI_REQUEST_APP_LIST | 0x80,pu8Send2UIData,u32SendLen);
	}

}



VOID NotifyUILoginResult(BYTE u8Result)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = u8Result;
	u32SendLen = 1;
	
	fnPortalClientSendMsg2UI(OPER_LOGIN_PORTAL_SERVER | 0x80,pu8Send2UIData,u32SendLen);
}


VOID NotifyUILogoutResult(BYTE u8Result)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = u8Result;
	u32SendLen = 1;
	
	fnPortalClientSendMsg2UI(OPER_LOGOUT_PORTAL_SERVER | 0x80,pu8Send2UIData,u32SendLen);
}

VOID NotifyUIPortalClientStatus(ePORTAL_CLIENT_LOGIN_STATUS eLoginStatus,ePORTAL_CLIENT_LOGIN_STATUS eLogoutStatus)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = eLoginStatus;
	pu8Send2UIData[1] = eLogoutStatus;
	u32SendLen = 2;
	
	fnPortalClientSendMsg2UI(OPER_LOGIC_PUSH_STATUS,pu8Send2UIData,u32SendLen);
}


VOID ReplyUIPortalClientStatus(ePORTAL_CLIENT_LOGIN_STATUS eLoginStatus,ePORTAL_CLIENT_LOGIN_STATUS eLogoutStatus)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = eLoginStatus;
	pu8Send2UIData[1] = eLogoutStatus;
	u32SendLen = 2;
	
	fnPortalClientSendMsg2UI(OPER_CHECK_PORTAL_SERVER | 0x80,pu8Send2UIData,u32SendLen);
}


VOID ReplyUIPortalClientAccessControl(BYTE * pDataBuf)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	OSA_MemCopy(pu8Send2UIData,pDataBuf,7);
	
	u32SendLen = 7;
	
	fnPortalClientSendMsg2UI(OPER_UI_EDIT_APP_ACCESS_CONTROL | 0x80,pu8Send2UIData,u32SendLen);
}


VOID ReplyUIPortalClientDeleteResult(BYTE u8Index, BYTE u8Ret)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = u8Index;
	pu8Send2UIData[1] = u8Ret;
	u32SendLen = 2;
	
	fnPortalClientSendMsg2UI(OPER_UI_DELETE_APP | 0x80,pu8Send2UIData,u32SendLen);
}


VOID ReplyUIPortalClientPairResult(BYTE u8Index, BYTE u8Ret, BYTE * pAccessData)
{
	BYTE pu8Send2UIData[1024];
	UINT32 u32SendLen = 0;

	OSA_MemSet(pu8Send2UIData,0,sizeof(pu8Send2UIData));
	
	pu8Send2UIData[0] = u8Index;
	pu8Send2UIData[1] = u8Ret;
	OSA_MemCopy(&pu8Send2UIData[2],pAccessData,6);
	u32SendLen = 8;
	
	fnPortalClientSendMsg2UI(OPER_UI_SEND_APP_INTIGRITY_CODE | 0x80,pu8Send2UIData,u32SendLen);
}


