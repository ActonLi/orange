#include "PortalClient.h"


char pu8MasterUUID[64] = {0};

LOCAL char s_szAdapterVerString[1024] = {0};


extern int LWSB64EncodeString(const char *in, int in_len, char *out, int out_size);
extern char * GetPortalServerUrl(void);
extern INT fnSendMsg2PortClientSendThread(BYTE *pSrcDataBuf, UINT32 u32DataLen);
		


/*获取版本号信息只是临时函数*/
LOCAL const char *ad__GetAdapterVer(void)
{
	char pcDevice[64];
	char pcPrefix[64];
	char pcMajor[64];
	char pcMinor[64];
	char pcDate[64];
	char pcSuffix[64];
	char pcTmp[128];
	
	OSA_MemSet(s_szAdapterVerString,0,sizeof(s_szAdapterVerString));
	OSA_MemSet(pcTmp,0,sizeof(pcTmp));
	
	
	OSA_MemSet(pcDevice,0,sizeof(pcDevice));
	OSA_MemSet(pcPrefix,0,sizeof(pcPrefix));
	OSA_MemSet(pcMajor,0,sizeof(pcMajor));
	OSA_MemSet(pcMinor,0,sizeof(pcMinor));
	OSA_MemSet(pcDate,0,sizeof(pcDate));
	OSA_MemSet(pcSuffix,0,sizeof(pcSuffix));

	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Device",pcDevice,sizeof(pcDevice));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Prefix",pcPrefix,sizeof(pcPrefix));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Major",pcMajor,sizeof(pcMajor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Minor",pcMinor,sizeof(pcMinor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Date",pcDate,sizeof(pcDate));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Main","Suffix",pcSuffix,sizeof(pcSuffix));
	
	
	sprintf(s_szAdapterVerString, "\n%s_%s_V%s.%s_%s_%s\n",
                        pcDevice,
                        pcPrefix,
                        pcMajor,
                        pcMinor,
                        pcDate,
                        pcSuffix
                      );


	OSA_MemSet(pcDevice,0,sizeof(pcDevice));
	OSA_MemSet(pcPrefix,0,sizeof(pcPrefix));
	OSA_MemSet(pcMajor,0,sizeof(pcMajor));
	OSA_MemSet(pcMinor,0,sizeof(pcMinor));
	OSA_MemSet(pcDate,0,sizeof(pcDate));
	OSA_MemSet(pcSuffix,0,sizeof(pcSuffix));

	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Device",pcDevice,sizeof(pcDevice));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Prefix",pcPrefix,sizeof(pcPrefix));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Major",pcMajor,sizeof(pcMajor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Minor",pcMinor,sizeof(pcMinor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Date",pcDate,sizeof(pcDate));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Ext","Suffix",pcSuffix,sizeof(pcSuffix));
	
	OSA_MemSet(pcTmp,0,sizeof(pcTmp));
	sprintf(pcTmp, "%s_%s_V%s.%s_%s_%s\n",
                        pcDevice,
                        pcPrefix,
                        pcMajor,
                        pcMinor,
                        pcDate,
                        pcSuffix
                      );

	strncat(s_szAdapterVerString,pcTmp,strlen(pcTmp));

	
	OSA_MemSet(pcDevice,0,sizeof(pcDevice));
	OSA_MemSet(pcPrefix,0,sizeof(pcPrefix));
	OSA_MemSet(pcMajor,0,sizeof(pcMajor));
	OSA_MemSet(pcMinor,0,sizeof(pcMinor));
	OSA_MemSet(pcDate,0,sizeof(pcDate));
	OSA_MemSet(pcSuffix,0,sizeof(pcSuffix));

	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Device",pcDevice,sizeof(pcDevice));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Prefix",pcPrefix,sizeof(pcPrefix));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Major",pcMajor,sizeof(pcMajor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Minor",pcMinor,sizeof(pcMinor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Date",pcDate,sizeof(pcDate));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Alarm","Suffix",pcSuffix,sizeof(pcSuffix));
	
	OSA_MemSet(pcTmp,0,sizeof(pcTmp));
	sprintf(pcTmp, "%s_%s_V%s.%s_%s_%s\n",
                        pcDevice,
                        pcPrefix,
                        pcMajor,
                        pcMinor,
                        pcDate,
                        pcSuffix
                      );

	strncat(s_szAdapterVerString,pcTmp,strlen(pcTmp));

	OSA_MemSet(pcDevice,0,sizeof(pcDevice));
	OSA_MemSet(pcPrefix,0,sizeof(pcPrefix));
	OSA_MemSet(pcMajor,0,sizeof(pcMajor));
	OSA_MemSet(pcMinor,0,sizeof(pcMinor));
	OSA_MemSet(pcDate,0,sizeof(pcDate));
	OSA_MemSet(pcSuffix,0,sizeof(pcSuffix));

	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Device",pcDevice,sizeof(pcDevice));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Prefix",pcPrefix,sizeof(pcPrefix));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Major",pcMajor,sizeof(pcMajor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Minor",pcMinor,sizeof(pcMinor));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Date",pcDate,sizeof(pcDate));
	GetValueFromEtcFile(PATH_FIRMWARE_VERSION_CFG,"Hardware","Suffix",pcSuffix,sizeof(pcSuffix));
	
	OSA_MemSet(pcTmp,0,sizeof(pcTmp));
	sprintf(pcTmp, "%s_%s_V%s.%s_%s_%s\n",
                        pcDevice,
                        pcPrefix,
                        pcMajor,
                        pcMinor,
                        pcDate,
                        pcSuffix
                      );

	strncat(s_szAdapterVerString,pcTmp,strlen(pcTmp));
	printf("[PortalClient] Firmware verion to update is %s\n",s_szAdapterVerString);
    return (s_szAdapterVerString);
}


VOID GetUUID(char * uuidstring)
{
	uuid_t uuid;
	
	uuid_generate(uuid);
    uuid_unparse(uuid, uuidstring); // The  uuid_unparse function converts the supplied UUID uu from the binary representation into a 36-byte string (plus tailing '\0') of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427 and stores this value in the character string pointed to by out.
	return;
}

static char eventid[64] = {0};

LOCAL void SetEventUUID(char *uuid)
{
	sprintf(eventid,"%s",uuid);
}

char* GetEventUUID(VOID)
{
	return eventid;
}


LOCAL CHAR* GetISO8601Time(VOID)
{
    static char szTimeBuf[32] = {0};
    struct timeval tv;
    struct timezone tz;
    struct tm *tm;
    memset(szTimeBuf,0,32);
    gettimeofday(&tv,&tz);
    tm = localtime(&tv.tv_sec);
    strftime(szTimeBuf,sizeof(szTimeBuf),"%FT%T%z",tm);
	return szTimeBuf;
}


VOID GetLocalNameFromId(char* pLocalID, char* pLocalName)
{
	//pLocalID example "740001010101"
	
	BYTE pu8LocalID[6];
	
	if(pLocalID == NULL)
	{
		return;
	}

	generate_bcdid_by_text(pLocalID,pu8LocalID);

	OSA_DBG_MSG("%s pLocalIDRecv is %s[0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x]",DEBUG_HEADER_PORTALCLIENT,pLocalID,pu8LocalID[0],pu8LocalID[1],pu8LocalID[2],pu8LocalID[3],pu8LocalID[4],pu8LocalID[5]);

	BYTE u8Type = pu8LocalID[0];
	INT32 u32Unit = pu8LocalID[1]*256 + pu8LocalID[2];
	INT32 u32Room = pu8LocalID[3]*256 + pu8LocalID[4];
	BYTE u8ID = pu8LocalID[5];

	switch(u8Type)
	{
		case DT_PC_GUARD_UNIT:               //0x70	  /*PC管理机 */	
		{
			sprintf(pLocalName,"%s","PC Guard ");
		}
		break;
		
		case DT_PHONE_GUARD_UNIT:			//0x71   /*电话管理机 */
		{
			sprintf(pLocalName,"Phone Guard Unit%d",u32Unit);
		}
		break;             
		
		case DT_GUARD_UNIT:					 //0x72   /*管理机, 用于SIP Server群呼管理机*/
		{
			sprintf(pLocalName,"Guard Unit%d",u32Unit);
		}
		break;      
		
		case DT_GATE_STATION:				//0x73   /*围墙机*/
		{
			sprintf(pLocalName,"GateStation Unit%d",u32Unit);
		}
		break; 
		
		case DT_DOOR_STATION:				//0x74   /*门口机*/
		{
			sprintf(pLocalName,"OutdoorStation Unit%d ID %d",u32Unit,u8ID);
		}
		break;
		
		case DT_INDOOR_STATION:				//0x75   /*室内数字分机*/
		{
			sprintf(pLocalName,"IndoorStation Unit%d Room%d ID %d",u32Unit,u32Room,u8ID);
		}
		break;
		
		case DT_DIGITAL_SECOND_DOOR_STATION:	//0x76   /*数字二次门口机*/
		{
			sprintf(pLocalName,"SecondDoorStation Unit%d Room%d ID %d",u32Unit,u32Room,u8ID);
		}
		break;
		
	}
		
}

VOID GetAPPInfoFromSipname(char* sipname,char* friendlyname,char* uuid)
{
	int i = 0;
	char tmp[32] = {0};
	char name[64]  = {0}; 
	
	for(i = 1; i < (1 + MAX_PAIRING_USER); i++)
	{
		sprintf(tmp,"user_%d",i);
		GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,tmp,WELCOME_APP_PAIR_REQUEST_KEY_SIP_NAME,name,64);

		if(strcmp(name,sipname) == 0)
		{
			GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,tmp,WELCOME_APP_PAIR_REQUEST_KEY_NAME,friendlyname,64);
			GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,tmp,WELCOME_APP_PAIR_REQUEST_KEY_UUID,uuid,64);
			break;
		}
	}

}

LOCAL char* GenerateEventInJsonFmt(EventNode *event)
{
	char *csrjson;
	char *base64payload;
	int b64payloadlen = 0;
	
	if(event == NULL)
	{
		return NULL;
	}
	if(event->szUuid[0] == '\0' || event->szType[0] == '\0' || event->szTimeStamp[0] =='\0')
	{
		OSA_DBG_MSG("%sMiss Required Info(UUID,Type,TimeStamp)for Event Generation\n",DEBUG_HEADER_PORTALCLIENT);
		return NULL;
	}
	cJSON *root = cJSON_CreateObject();
	
	/*required information*/
	cJSON_AddStringToObject(root,"id",event->szUuid);
	cJSON_AddStringToObject(root,"timestamp",event->szTimeStamp);
	cJSON_AddStringToObject(root,"type",event->szType);
	if(strcmp(event->szType,EVENT_TYPE_ACL_UPLOAD) == 0)
	{
		OSA_DBG_MSG("%sDestination = %s\n",DEBUG_HEADER_PORTALCLIENT,event->szDestination);
	}
	
	if(event->szBelongto[0] != '\0')
	{
		cJSON_AddStringToObject(root,"belongsto",event->szBelongto);	
	}
	if(event->szDestination[0] != '\0')
	{
		char *strings[1];
		strings[0] = (char*)malloc(64);
		sprintf(strings[0],"%s",event->szDestination);
		cJSON* destination = cJSON_CreateStringArray(strings,1);

		cJSON_AddItemToObject(root,"destination",destination);	
		free(strings[0]);
		strings[0] = NULL;
	}
	
	/*optional information */
	if(event->szPayload  != NULL  )
	{
		if( event->szPayload[0] != '\0')
		{
			/*JSON doesn't support multiline strings for values, payload must be encoded by base64*/
			b64payloadlen = event->iPayloadLen*(4.0/3)+4;
			base64payload = (char *)OSA_MemMalloc(b64payloadlen);
			memset(base64payload,0,b64payloadlen);
			LWSB64EncodeString(event->szPayload,event->iPayloadLen,base64payload,b64payloadlen);
			cJSON_AddStringToObject(root,"payload",base64payload);
			OSA_MemFree(base64payload);
		}
	}
	csrjson = cJSON_Print(root);
	/*release mem*/
	cJSON_Delete(root);	
	return csrjson;
}

LOCAL EventNode *CreateHistoryEvent(char* EventType,InternalEventInfo *pEventInfo)
{
	char friendlyname [64] = {0};
	char pLocalName [100] = {0};
	char localid [128] = {0};
	char remoteid [128] = {0};
	char remote_uuid[64] = {0};
	
	if(EventType == NULL )
	{
		OSA_DBG_MSG("create histroy EventType is NULL\n");
		return NULL;
	}

	EventNode *pstEventNode = (EventNode *)OSA_MemMalloc(sizeof(EventNode));
	memset(pstEventNode,0,sizeof(EventNode));

	GetUUID(pstEventNode->szUuid);      //任何事件都要有自己的UUID  

	sprintf(pstEventNode->szTimeStamp,"%s",pEventInfo->iostime);  
	sprintf(pstEventNode->szType,"%s",EventType); 

	/*save master uuid ring suevy */
	if(strcmp(EventType,EVENT_TYPE_RING) == 0 
		|| strcmp(EventType,EVENT_TYPE_SURVEILLANCE) == 0)
	{
		//新增SessionID-MaterID 映射结点
		fnAddNewSessionMasterIDMapNode(pEventInfo->sessionID, pstEventNode->szUuid);
	}

	/*save	slave uuid*/
	if(strcmp(EventType,EVENT_TYPE_DOOR_OPEN) == 0    //开门
		|| strcmp(EventType,EVENT_TYPE_SWITCH_LIGHT) == 0  //开灯
		|| strcmp(EventType,EVENT_TYPE_MISSED_CALL) == 0	//未接来电
		|| strcmp(EventType,EVENT_TYPE_ANSWERED) == 0		//接听
		|| strcmp(EventType,EVENT_TYPE_SCREENSHOT) == 0		//抓拍
		|| strcmp(EventType,EVENT_TYPE_TERMINATED) == 0)	//挂断
	{
		//根据sessionID 查到该事件belongto 的ID
		EVENT_SESSION_MASTERID_MAP_LIST * pSessionMasterIDNode = fnFindSessionMasterIDMapNode(pEventInfo->sessionID);
		OSA_MemCopy(pstEventNode->szBelongto,pSessionMasterIDNode->node.pu8MasterID,sizeof(pstEventNode->szBelongto)-1);
	}

	if(strcmp(EventType,EVENT_TYPE_ANSWERED) == 0)
	{
		//更新SessionID-MaterID 映射结点
		fnUpdateNewSessionMasterIDMapNode(pEventInfo->sessionID, pstEventNode->szUuid);
	}

	if( strcmp(EventType,EVENT_TYPE_MISSED_CALL) == 0	//未接来电
		|| strcmp(EventType,EVENT_TYPE_TERMINATED) == 0 )	//挂断
	{
		//删除SessionID-MaterID 映射结点
		fnDeleteSessionMasterIDMapNode(pEventInfo->sessionID);
	}

	cJSON *payload = cJSON_CreateObject();
	if(pEventInfo->localid[0] != '\0')
	{
		GetLocalNameFromId(pEventInfo->localid, pLocalName);  //localid 是"750001010100" 的格式
		sprintf(localid,"%s@%s",pEventInfo->localid,GetDeviceDomain());
		cJSON_AddStringToObject(payload,HISTORY_LOCAL_ID,localid);
		cJSON_AddStringToObject(payload,HISTORY_LOCAL_NAME,pLocalName);
	}
	if(pEventInfo->remoteid[0] != '\0')
	{
		sprintf(remoteid,"%s@%s",pEventInfo->remoteid,GetDeviceDomain());
		cJSON_AddStringToObject(payload,HISTORY_REMOTE_ID,remoteid);
		GetAPPInfoFromSipname(pEventInfo->remoteid,friendlyname,remote_uuid);
		cJSON_AddStringToObject(payload,HISTORY_REMOTE_NAME,friendlyname);
		cJSON_AddStringToObject(payload,HISTORY_REMOTE_UUID,remote_uuid);
	}
	if(pEventInfo->localid[0] !='\0' || pEventInfo->remoteid[0] != '\0')
	{
		char *payloadstr = cJSON_PrintUnformatted(payload);
		pstEventNode->iPayloadLen = strlen(payloadstr);
		pstEventNode->szPayload = payloadstr;
		OSA_printf("===history event payload : %s",pstEventNode->szPayload);
	}

	cJSON_Delete(payload);
	return pstEventNode;
}

LOCAL EventNode * CreateDeviceInfoEvent(VOID)
{
	OSA_DBG_MSG("%sCreate Device Info Event\n",DEBUG_HEADER_PORTALCLIENTSEND);
	char * pEventPayloadStr;
	
	cJSON * pcJsonObject = cJSON_CreateObject();
	const char *version = ad__GetAdapterVer();
	cJSON_AddStringToObject(pcJsonObject,DEVICE_INFO_SW_VERSION,version);
	pEventPayloadStr = cJSON_Print(pcJsonObject);
	
	EventNode *eventNode = (EventNode *)OSA_MemMalloc(sizeof(EventNode));
	memset(eventNode,0,sizeof(EventNode));
	
	GetUUID(eventNode->szUuid);
	sprintf(eventNode->szTimeStamp,"%s",GetISO8601Time());
	sprintf(eventNode->szType,"%s",EVENT_TYPE_DEVICE_INFO);

	eventNode->iPayloadLen = strlen(pEventPayloadStr);
	eventNode->szPayload = pEventPayloadStr;
	
	OSA_DBG_MSG("%sDevice Info Payload : %s",DEBUG_HEADER_PORTALCLIENTSEND,eventNode->szPayload);
	cJSON_Delete(pcJsonObject);	
	return eventNode;
}

LOCAL INT NoPollSafeSendMsg(char * pData,INT iLen)
{
	int iSendlen = 0;

	LockWebsocketMutex();
	if(nopoll_conn_is_ok(GetNoPollConn()))
	{
		if ((iSendlen = nopoll_conn_send_text(GetNoPollConn(), pData, iLen)) < 0) 
		{
			OSA_ERROR("%snopoll_conn_send_text Error\n",DEBUG_HEADER_PORTALCLIENT);
		}
	}
	else
	{
		UnLockWebsocketMutex();
		return -1;
	}
	UnLockWebsocketMutex();
	return iSendlen;

}



LOCAL INT NoPollSafeSendPing(VOID)
{
	LockWebsocketMutex();
	if(nopoll_conn_is_ok(GetNoPollConn()))
	{
		if (nopoll_conn_send_ping(GetNoPollConn()) < 0) 
		{
			UnLockWebsocketMutex();
			OSA_ERROR("%snopoll_conn_send_ping Error\n",DEBUG_HEADER_PORTALCLIENTSEND);
			return nopoll_false;
		} 	
		OSA_DBG_MSG("%sSend msg:ping To Portal Server\n",DEBUG_HEADER_PORTALCLIENTSEND);
	}
	UnLockWebsocketMutex();
	return nopoll_true;
}

LOCAL INT RsaEncryptViaCert(char *p_en,char *str,char *path_cert,int *outlen)
{
  int flen,rsa_len;

  EVP_PKEY *pkey = NULL;
  BIO              *certbio = NULL;
  BIO               *outbio = NULL;
  X509                *cert = NULL;
 
  /* ---------------------------------------------------------- *
   * These function calls initialize openssl for correct work.  *
   * ---------------------------------------------------------- */
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();

  /* ---------------------------------------------------------- *
   * Create the Input/Output BIO's.                             *
   * ---------------------------------------------------------- */
  certbio = BIO_new(BIO_s_file());
  outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

  /* ---------------------------------------------------------- *
   * Load the certificate from file (PEM).                      *
   * ---------------------------------------------------------- */
  OSA_DBG_MSG("path_cert = %s \n",path_cert);
  BIO_read_filename(certbio, path_cert);
  if (! (cert = PEM_read_bio_X509(certbio, NULL, 0, NULL))) {
    BIO_printf(outbio, "Error loading cert into memory\n");
	BIO_free_all(certbio);
	BIO_free_all(outbio);
   	return -1;
  }

  /* ---------------------------------------------------------- *
   * Extract the certificate's public key data.                 *
   * ---------------------------------------------------------- */
  if ((pkey = X509_get_pubkey(cert)) == NULL)
    BIO_printf(outbio, "Error getting public key from certificate");


    flen=strlen(str);
    rsa_len=RSA_size(pkey->pkey.rsa);
    //p_en=(unsigned char *)malloc(rsa_len+1);
    memset(p_en,0,rsa_len+1);
	/*
		use RSA_PKCS1_PADDING to match the RSA mode of peer side
		use string length as flen 
	*/
	*outlen = RSA_public_encrypt(flen,(unsigned char *)str,(unsigned char*)p_en,pkey->pkey.rsa,RSA_PKCS1_PADDING);

	if(*outlen < 0)
	{
		EVP_PKEY_free(pkey);
		X509_free(cert);
		BIO_free_all(certbio);
		BIO_free_all(outbio);
        return -1;
    }
	
	EVP_PKEY_free(pkey);
	X509_free(cert);
	BIO_free_all(certbio);
	BIO_free_all(outbio);
    return 0;
}

LOCAL EventNode * CreateAclUploadEvent(char *section,int pwflag)
{
	char certfile[128] = {0};
	char sip_password[64] = {0};
	char sip_encry_password[256 +4] = {0};
	char base64_password[512] = {0};
	int fileLen = 0;
	int en_len;
	int base64_en_len=0;
	char uuid[64] = {0};
	int iFilepos = 0;
	int iSecondDoorStationConfPos = 0;
	int iOSConfPos = 0;
	int iGateStationConfPos = 0;

   	int iRet = 0;
	UINT32 u32Offset = 0;
	int i = 0;
	UINT32 u32ItemLen = 0;
	
	iRet = GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,section,WELCOME_APP_PAIR_REQUEST_KEY_UUID,uuid,64);
	if(iRet != 0)  //event 时间的目的uuid没有获取成功
	{
		OSA_DBG_MSG("%sWhen Upload ACL,no APP uuid For %s Found, Discard...",DEBUG_HEADER_PORTALCLIENT,section);
		return NULL;
	}
	GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,section,WELCOME_APP_PAIR_REQUEST_KEY_PASSWORD,sip_password,32);		
	
	EventNode *eventnode = (EventNode *)OSA_MemMalloc(sizeof(EventNode));
	memset(eventnode,0,sizeof(EventNode));
	GetUUID(eventnode->szUuid);
	strncpy(eventnode->szTimeStamp,GetISO8601Time(),sizeof(eventnode->szTimeStamp));
	strncpy(eventnode->szType,EVENT_TYPE_ACL_UPLOAD,sizeof(eventnode->szType));
	strncpy(eventnode->szDestination,uuid,sizeof(eventnode->szDestination));
	eventnode->iPayloadLen = 0;
	eventnode->szPayload = NULL;

	
	if(pwflag == 1)
	{
		FILE_HANDLE CfgIniFd = INVALID_FILE_HANDLE ;
		if(uuid == NULL)
		{
			OSA_ERROR("%suuid is NULL",__func__);
			return NULL;
		}
	
		/*specify certificate path*/
		sprintf(certfile,"%s/%s.crt",PATH_WELCOME_APP_ROOT,section);

		/*encrypt password*/
		if(strcmp(sip_password,"") == 0)
		{
			GetUnformattedUUID(sip_password,16);
			SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,section,WELCOME_APP_PAIR_REQUEST_KEY_PASSWORD,sip_password);
			SaveIniData(PATH_WELCOME_APP_PAIR_REQUEST);
		}
		int ret = RsaEncryptViaCert(sip_encry_password,sip_password,certfile,&en_len);
		if(ret == -1)
		{
			OSA_MemFree(eventnode);
			eventnode = NULL;
			return NULL;
		}
		
		if(sip_encry_password == NULL)
		{
			OSA_ERROR("encrypt password is NULL");
			OSA_MemFree(eventnode);
			return NULL;
		}
		
		base64_en_len = en_len*(4.0/3)+4;
		base64_en_len = LWSB64EncodeString(sip_encry_password,en_len,base64_password,base64_en_len);
		eventnode->iPayloadLen = base64_en_len + 1;  //为了加换行\n
		
		/*read content of config.ini*/
		CfgIniFd = OSA_FileOpen(PATH_WELCOME_CFG, MODE_OPEN_EXISTING, ACCESS_BOTH);
		fileLen = OSA_FileGetSize(CfgIniFd);
		eventnode->iPayloadLen += fileLen;		//为了加换行\n

		/*get content of Second Door Station list*/
		DEVICEMANAGER_LIST * pstSecondDoorStationList = GetSecondDoorStationList();
		char pSecondDoorStationConfigItem[64][1024];  //最多就32台二次门口机
		char pSecondDoorStationLineTemp[512];

		if(pstSecondDoorStationList != NULL)
		{
			int i = 0;
			for(i = 0; i < pstSecondDoorStationList->u16Counts; i++)
			{	
				OSA_DBG_MSG("%sSecondDoorStationList:device id(%s), ip(%s)",DEBUG_HEADER_PORTALCLIENT,pstSecondDoorStationList->stItem[i].szDeviceStrId,pstSecondDoorStationList->stItem[i].szDeviceIp);
				OSA_MemSet(pSecondDoorStationConfigItem[i],0,sizeof(pSecondDoorStationConfigItem[i]));
				
				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"[outdoorstation_%d]\n",(i+64+32));  //二次门口机的编号偏移是64 + 32
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"name=2ndOS%d\n",i+1);
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"%s\n","type=1");
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"address=sip:%s@%s\n",pstSecondDoorStationList->stItem[i].szDeviceStrId,GetDeviceDomain());
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"%s\n","screenshot=yes");
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_MemSet(pSecondDoorStationLineTemp,0,sizeof(pSecondDoorStationLineTemp));
				sprintf(pSecondDoorStationLineTemp,"%s\n","surveillance=yes");
				strcat(pSecondDoorStationConfigItem[i], pSecondDoorStationLineTemp);

				OSA_DBG_MSG("%sSecondDoorStation(%d)ConfigItem is %s",DEBUG_HEADER_PORTALCLIENT,i,pSecondDoorStationConfigItem[i]);
				eventnode->iPayloadLen += strlen(pSecondDoorStationConfigItem[i]);

			} 
		}


		/*get content of os list*/
		DEVICEMANAGER_LIST * pstOSList = GetOSList();
		char pOSConfigItem[64][1024];  //最多就64台门口机
		char pLineTemp[512];

		if(pstOSList != NULL)
		{
			int i = 0;
			for(i = 0; i < pstOSList->u16Counts; i++)
			{	
				OSA_DBG_MSG("%sOSList:device id(%s), ip(%s)",DEBUG_HEADER_PORTALCLIENT,pstOSList->stItem[i].szDeviceStrId,pstOSList->stItem[i].szDeviceIp);
				OSA_MemSet(pOSConfigItem[i],0,sizeof(pOSConfigItem[i]));
				
				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"[outdoorstation_%d]\n",i);  //门口机的编号偏移是0
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"name=OS%d\n",i+1);
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"%s\n","type=1");
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"address=sip:%s@%s\n",pstOSList->stItem[i].szDeviceStrId,GetDeviceDomain());
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"%s\n","screenshot=yes");
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
				sprintf(pLineTemp,"%s\n","surveillance=yes");
				strcat(pOSConfigItem[i], pLineTemp);

				OSA_DBG_MSG("%sOS(%d)ConfigItem is %s",DEBUG_HEADER_PORTALCLIENT,i,pOSConfigItem[i]);
				eventnode->iPayloadLen += strlen(pOSConfigItem[i]);

			} 
		}


		/*get content of Gate Station list*/
		DEVICEMANAGER_LIST * pstGateStationList = GetGateStationList();
		char pGateStationConfigItem[64][1024];  //最多就32 台管理机
		char pGateStationLineTemp[512];

		if(pstGateStationList != NULL)
		{
			int i = 0;
			for(i = 0; i < pstGateStationList->u16Counts; i++)
			{	
				OSA_DBG_MSG("%sGateStationList:device id(%s), ip(%s)",DEBUG_HEADER_PORTALCLIENT,pstGateStationList->stItem[i].szDeviceStrId,pstGateStationList->stItem[i].szDeviceIp);
				OSA_MemSet(pGateStationConfigItem[i],0,sizeof(pGateStationConfigItem[i]));
				
				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"[outdoorstation_%d]\n",(i+64)); //管理机的编号偏移是64
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"name=GS%d\n",i+1);
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"%s\n","type=1");
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"address=sip:%s@%s\n",pstGateStationList->stItem[i].szDeviceStrId,GetDeviceDomain());
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"%s\n","screenshot=yes");
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_MemSet(pGateStationLineTemp,0,sizeof(pGateStationLineTemp));
				sprintf(pGateStationLineTemp,"%s\n","surveillance=yes");
				strcat(pGateStationConfigItem[i], pGateStationLineTemp);

				OSA_DBG_MSG("%sGateStation(%d)ConfigItem is %s",DEBUG_HEADER_PORTALCLIENT,i,pGateStationConfigItem[i]);
				eventnode->iPayloadLen += strlen(pGateStationConfigItem[i]);

			} 
		}

		//分配payload 内存区域
		eventnode->iPayloadLen += 1;
		eventnode->szPayload = OSA_MemMalloc(eventnode->iPayloadLen);
		OSA_MemSet(eventnode->szPayload,0,eventnode->iPayloadLen);

		//payload-sip_password
		memcpy(eventnode->szPayload,base64_password,base64_en_len);
		eventnode->szPayload[base64_en_len] = '\n';   
		iFilepos = base64_en_len + 1;
		
		//printf("\n\n\n\n=====filepos is %d, iOSConfPos is %d\n\n",filepos,iOSConfPos);
		
		//payload+config.ini
		if(INVALID_FILE_HANDLE == CfgIniFd)
	    {
	        OSA_ERROR("open file :%s fail...", PATH_WELCOME_CFG);
	        return NULL;
	    }
	    else
	    {
	    	OSA_FileRead(CfgIniFd,eventnode->szPayload + iFilepos,fileLen);
	    }
		OSA_FileClose(CfgIniFd);

		//payload+SecondDoorStationlist
		iSecondDoorStationConfPos = iFilepos + fileLen;

		if(pstSecondDoorStationList != NULL)
		{
			u32Offset = 0;
			
			for(i = 0; i < pstSecondDoorStationList->u16Counts; i++)
			{
				u32ItemLen = strlen(pSecondDoorStationConfigItem[i]);
				memcpy(eventnode->szPayload+iSecondDoorStationConfPos+u32Offset,pSecondDoorStationConfigItem[i],u32ItemLen);
				u32Offset += u32ItemLen;
			}
		}
		
		iOSConfPos = iSecondDoorStationConfPos + u32Offset;

		//payload+oslist
		if(pstOSList != NULL)
		{
			u32Offset = 0;
			
			for(i = 0; i < pstOSList->u16Counts; i++)
			{
				u32ItemLen = strlen(pOSConfigItem[i]);
				memcpy(eventnode->szPayload+iOSConfPos+u32Offset,pOSConfigItem[i],u32ItemLen);
				u32Offset += u32ItemLen;
			}
		}

		iGateStationConfPos = iOSConfPos + u32Offset;

		//payload+GateStationlist
		if(pstGateStationList != NULL)
		{
			u32Offset = 0;
			
			for(i = 0; i < pstGateStationList->u16Counts; i++)
			{
				u32ItemLen = strlen(pGateStationConfigItem[i]);
				memcpy(eventnode->szPayload+iGateStationConfPos+u32Offset,pGateStationConfigItem[i],u32ItemLen);
				u32Offset += u32ItemLen;
			}
		}
	}

	OSA_DBG_MSG("%sACL Upload payload pushed is %s",DEBUG_HEADER_PORTALCLIENT,eventnode->szPayload);
	return eventnode;
}

LOCAL VOID DestoryEvent(EventNode* eventnode)
{
	if(eventnode->szPayload != NULL && eventnode->iPayloadLen > 0)
	{
		if(strcmp(eventnode->szType,EVENT_TYPE_SCREENSHOT) == 0)
			OSA_printf("delete image buffer addr = %p \n",eventnode->szPayload);
		free(eventnode->szPayload);
	}
	if(eventnode !=NULL)
	{
		OSA_MemFree(eventnode);
	}
}

EventNode *CreateSnapshotEvent(char* EventType, InternalEventInfo *pEventInfo)
{
	if(pEventInfo->snapshot.imgbuf == NULL)
	{
		return;
	}
	EventNode *pstEventNode = (EventNode *)OSA_MemMalloc(sizeof(EventNode));
	memset(pstEventNode,0,sizeof(EventNode));
	
	GetUUID(pstEventNode->szUuid);		
	
	sprintf(pstEventNode->szTimeStamp,"%s",pEventInfo->iostime);  
	sprintf(pstEventNode->szType,"%s",EventType); 
	EVENT_SESSION_MASTERID_MAP_LIST * pSessionMasterIDMapNode = fnFindSessionMasterIDMapNode(pEventInfo->sessionID);
	if(pSessionMasterIDMapNode != NULL)
	{
		sprintf(pstEventNode->szBelongto,"%s",pSessionMasterIDMapNode->node.pu8MasterID);
	}

	pstEventNode->iPayloadLen = pEventInfo->snapshot.imglength;
	pstEventNode->szPayload = pEventInfo->snapshot.imgbuf;
	
	return pstEventNode;
}

VOID SendEventToPortalServer(VOID *param)
{	
	EventNode * node = NULL;
	char *EventInJsonFmt = NULL;
	int iSendlen= 0;
	int iRet = 0;
	struct timeval now;
	struct timespec waittime;
	
	InternalEventInfo *pEventInfo = (InternalEventInfo *)param;
	OSA_DBG_MSG("%sSendEventToPortalServer EventType = %d\n",DEBUG_HEADER_PORTALCLIENTSEND,pEventInfo->type);
	
	switch(pEventInfo->type)
	{
		case eEventRing:
		{
			node = CreateHistoryEvent(EVENT_TYPE_RING,pEventInfo);
		}
		break;
		
		case eEventAnswered:
		{
			node = CreateHistoryEvent(EVENT_TYPE_ANSWERED,pEventInfo);
		}
		break;
		
		case eEventTerminated:
		{
			node = CreateHistoryEvent(EVENT_TYPE_TERMINATED,pEventInfo);
		}
		break;
		
		case eEventMissCall:
		{
			node = CreateHistoryEvent(EVENT_TYPE_MISSED_CALL,pEventInfo);
		}
		break;
		
		case eEventSurvellance:
		{
			node = CreateHistoryEvent(EVENT_TYPE_SURVEILLANCE,pEventInfo);
		}
		break;
		
		case eEventOpenDoor:
		{
			node = CreateHistoryEvent(EVENT_TYPE_DOOR_OPEN,pEventInfo);
		}
		break;
		
		case eEventSwitchLight:
		{
			node = CreateHistoryEvent(EVENT_TYPE_SWITCH_LIGHT,pEventInfo);
		}
		break;
		
		case eEventSnapshot:
		{
			node = CreateSnapshotEvent(EVENT_TYPE_SCREENSHOT,pEventInfo);
		}
		break;
		
		case eEventDeviceInfo:
		{
			node = CreateDeviceInfoEvent();
		}
		break;
			
		case eEventPing:
		{
			if(GetConnectState() == eStateEstablish)
			{
				NoPollSafeSendPing();
			}
			return ;
		}
		break;
		
		case eEventACLupload:
		{
			node = CreateAclUploadEvent(pEventInfo->user_section,pEventInfo->pwflag);
		}
		break;
		
		case eEventDefault:
		{
			//node = get_first_pending_event();
		}
		break;
	}
	
    if(NULL == node)
    {
        return;
    }
	
	if( GetConnectState() == eStateEstablish)
	{
		EventInJsonFmt = GenerateEventInJsonFmt(node);
		if(EventInJsonFmt == NULL)
		{
			DestoryEvent(node); 
			return;
		}
		//OSA_DBG_MSG("%sEvent In Json data to be sent: %200.200s\n",DEBUG_HEADER_PORTALCLIENTSEND,EventInJsonFmt);
		OSA_DBG_MSG("%sEvent In Json data to be sent: %s\n",DEBUG_HEADER_PORTALCLIENTSEND,EventInJsonFmt);
		SetEventUUID(node->szUuid);
		iSendlen = NoPollSafeSendMsg(EventInJsonFmt,strlen(EventInJsonFmt));
		free(EventInJsonFmt);
		
		if(iSendlen <= 0)
		{
		}
		else
		{
			gettimeofday(&now, NULL);    
			waittime.tv_nsec = 0;
			waittime.tv_sec = time(NULL) + WAIT_EVENT_TIMEOUT;  //50s 超时
			LockEventMutex();
			iRet = WaitRespForPushEventToPortalServer(&waittime);
			UnLockEventMutex();
			if(iRet == ETIMEDOUT)
			{
				if(pEventInfo->type != eEventDeviceInfo)
				{
					OSA_DBG_MSG("%sSend Event Timeout Type = %d \n",DEBUG_HEADER_PORTALCLIENTSEND,pEventInfo->type);
				}
				NoPollSafeLoopStop();
				OSA_DBG_MSG("%sWait Webosket Response Timeout,Close the Bad Connection\n",DEBUG_HEADER_PORTALCLIENTSEND);
			}
			else
			{
				SetHeartBeatCount(1);
			}
		}
	}
	
	DestoryEvent(node); //删除event

	return;

}


VOID PushEventToPortalServer(BYTE * msg_buf,UINT32 len)
{
	SOCK_DATA_PACKET_T *pNetCmdHeader = (SOCK_DATA_PACKET_T *)msg_buf;
	BYTE * pDataBuf = &msg_buf[sizeof(SOCK_DATA_PACKET_T)];	
	
	#ifndef PORTAL_CLIENT_RELEASE
	UINT32 u32DataLenTmp = len - sizeof(SOCK_DATA_PACKET_T);
	BYTE * pDataBufTmp = &msg_buf[sizeof(SOCK_DATA_PACKET_T)];	
	printf("%s\n",DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND);
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

	
	//IP地址变化
	if( (pNetCmdHeader->funcCode == PROCESS_FUNC_LOGIC) && (pNetCmdHeader->operCode == PROCESS_OPER_LOGIC_IP_CHANGED))
	{
		OSA_DBG_MSG("%sIP Address Change",DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND);
		
		UpdateFlexisipServeIPAddress();
		 /*update route.ini*/
		UpdateRecordToRouteFile();

		 /*update /etc/b2bsip/b2bsip.conf*/
		 /*inertal_uri*/
		SaveB2BInternalUri(SystemDeviceInfo_GetHomeIPAddress(),B2B_INTERNAL_PORT,B2B_INTERNAL_PROTOCAL);

		 /*重新推送给APP */
		 NotifyPortalServerACLForAllPairedApp();
		 
	}

	//门口机列表变化
	if( (pNetCmdHeader->funcCode == PROCESS_FUNC_LOGIC) && (pNetCmdHeader->operCode == PROCESS_OPER_LOGIC_DEVICE_LIST_CHANGED))
	{
		OSA_DBG_MSG("%sOut Door Station Change",DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND);
		 /*update route.ini*/
		UpdateRecordToRouteFile();

		 /*重新推送给APP */
		 NotifyPortalServerACLForAllPairedApp();

	}

	//Program Button 变化
	if( (pNetCmdHeader->funcCode ==  PROCESS_FUNC_IPGW) && (pNetCmdHeader->operCode == PROCESS_OPER_IPGW_PROGRAM_BTN_CHANGED))
	{
		OSA_DBG_MSG("%sProgram Button Change",DEBUG_HEADER_OTHERS_TO_PORTALCLIENTSEND);
		/*update program button*/
		UpdateProgramButton();
		NotifyPortalServerACLForAllPairedApp();
	}

	switch(pNetCmdHeader->operCode)
	{
		case OPER_PUSH_EVENT_TO_PORTAL_SERVER:
		{
			InternalEventInfo *inter_msg = (InternalEventInfo *)pDataBuf;
			SendEventToPortalServer(inter_msg);
		}
		break;
		
		default:
		{
			OSA_ERRORX("Unknown OperCode From Net(0x%02x)",pNetCmdHeader->operCode);
		}
		break;
	}

	
}








