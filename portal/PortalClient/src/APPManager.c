#include "PortalClient.h"
#include "id_ip_conf.h"


#define MAX_CFG_SIZE  1024     //每行最大1024字符
char pFingerPrint[128] = {0};
	

INT GetWelcomeAPPRecordID(char * uuid)
{
	int i =0;
	char pAppSection[16] = {0};
	char puuidValue[64] = {0};
	int min = 0;

	/*从WelcomeAPP_1 开始遍历， 最多遍历4  条APP 记录*/
	for(i = 1;i <= MAX_PAIRING_USER; i++)
	{
		sprintf(pAppSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,i);  
		if(GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pAppSection,WELCOME_APP_PAIR_REQUEST_KEY_UUID,puuidValue,64) < 0 )
		{
			if(min == 0)
			{
				OSA_DBG_MSG("%sThe %s is requesting pairing ",DEBUG_HEADER_PORTALCLIENT,pAppSection);
				min = i;
			}
		}
		else if(strncmp(uuid,puuidValue,36) == 0)
		{
			/*该APP的配对请求已经存在*/
			OSA_DBG_MSG("%sThe APP(uuid=%s) has already request pairing which is recorded in Section[%s] in %s,discard pairing request",DEBUG_HEADER_PORTALCLIENT,uuid,pAppSection,PATH_WELCOME_APP_PAIR_REQUEST);
			return -1;
		}
	}
	if(min == 0) // 满了就不接收配对请求
	{
		OSA_DBG_MSG("%sThe max number %d reached ,discard pairing request\n",DEBUG_HEADER_PORTALCLIENT,MAX_PAIRING_USER);
		return -1;
	}
	return min;
}



/*处理手机配对请求, APP Pair Request -> Portal Server -> Portal Client*/
/*pAppPairPayLoad 中包含uuid (是PortalServer 分配给手机的), app 的证书，app的友好名字 */

VOID HandleWelcomeAppPairRequest(char * pAppPairPayLoad)
{
	OSA_DBG_MSG("%sHandle Welcome App Pair Request\n",DEBUG_HEADER_PORTALCLIENT);
	cJSON *pWelAPPUUIDObject = NULL;
	cJSON *pWelAPPCertObject =  NULL;
	cJSON *pWelAPPNameObject = NULL;
	
	int iWelcomeAPPRecordID = 0;
	char pWelAppSection[16] = {0};
	unsigned char * pDecCertificate = NULL;
	int iDecCertificateLen = 0;
	char snum[32] = {0};   //APP的序列号
	
	char tmp[2048] = {0};

	if(access(PATH_WELCOME_APP_ROOT,F_OK) != 0)
	{
		OSA_DBG_MSG("%sCreate Folder(%s) For APP\n",DEBUG_HEADER_PORTALCLIENT,PATH_WELCOME_APP_ROOT);
		sprintf(tmp,"mkdir -p %s",PATH_WELCOME_APP_ROOT);
		system(tmp);
	}
	cJSON * payloadInJson = cJSON_Parse(pAppPairPayLoad);
	if( payloadInJson != NULL)
	{
		pWelAPPUUIDObject = cJSON_GetObjectItem(payloadInJson,"uuid");
		pWelAPPCertObject = cJSON_GetObjectItem(payloadInJson,"certificate");
		pWelAPPNameObject = cJSON_GetObjectItem(payloadInJson,"name");
		iWelcomeAPPRecordID = GetWelcomeAPPRecordID(pWelAPPUUIDObject->valuestring);
		if(iWelcomeAPPRecordID == -1)
		{
			return;
		}
		sprintf(pWelAppSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,iWelcomeAPPRecordID);

		/*将APP的证书存放在user_x.crt 文件里*/
        pDecCertificate = (unsigned char *)malloc(strlen(pWelAPPCertObject->valuestring));   
        if(pDecCertificate == NULL)
        {
            return;
        }
		iDecCertificateLen = Base64Dec(pDecCertificate,pWelAPPCertObject->valuestring,strlen(pWelAPPCertObject->valuestring));
		sprintf(tmp,"%s/%s.crt",PATH_WELCOME_APP_ROOT,pWelAppSection);
		FILE *fcert =  fopen(tmp,"w+");
		if(fcert != NULL)
		{	
			fwrite(pDecCertificate,iDecCertificateLen,1,fcert);
		}
		fclose(fcert);

        free(pDecCertificate);

		/*从APP的证书中获取公钥，将APP的公钥存放在user_x.pubkey 文件里*/

		/*hash of publickey*/
		sprintf(tmp,"openssl x509 -pubkey -noout -in %s/%s.crt 1>  %s/%s.pubkey",PATH_WELCOME_APP_ROOT,pWelAppSection,PATH_WELCOME_APP_ROOT,pWelAppSection);
		system(tmp);

		
		/*从APP的证书中获取序列号，将APP的序列号存放在user_x.serial 文件里*/
		
		/*extract the serial number*/
		sprintf(tmp,"openssl x509 -serial -noout -in %s/%s.crt |grep 'serial=' 1>  %s/%s.serial",PATH_WELCOME_APP_ROOT,pWelAppSection,PATH_WELCOME_APP_ROOT,pWelAppSection);
		system(tmp);

		/*从WelcomAPP_x.serial 的serial=%s 中提取出序列号放在snum中*/
		sprintf(tmp,"%s/%s.serial",PATH_WELCOME_APP_ROOT,pWelAppSection);
		FILE *fserial =  fopen(tmp,"r");
		if(fserial != NULL)
		{
			fread(tmp,1,32,fserial);
			sscanf(tmp,"serial=%s",snum);
		}
		fclose(fserial);

		/*将UUID,  友好名字，配对状态，序列号存在配置文件中*/
		SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelAppSection,WELCOME_APP_PAIR_REQUEST_KEY_UUID,pWelAPPUUIDObject->valuestring);
		SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelAppSection,WELCOME_APP_PAIR_REQUEST_KEY_NAME,pWelAPPNameObject->valuestring);
		SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelAppSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE,WELCOME_APP_PAIR_STATE_UNPAIRED);
		SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelAppSection,WELCOME_APP_PAIR_REQUEST_KEY_SERIAL,snum);
		SaveIniData(PATH_WELCOME_APP_PAIR_REQUEST);

		/*通知UI更新APP列表*/
		NotifyUIWelcomeAppList(PORTAL_CLIENT_PUSH);
	}
}


/*将字符串s中出现的字符":" 删除*/  
LOCAL VOID squeeze(char * s)  
{  
    int i,j;  
	char s_bak[256] = {0};
	strncpy(s_bak,s,256);
    for (i = 0, j = 0; s_bak[i] != '\0';i++)  
    {  
       if(s_bak[i] != ':')
       {
       		if( s_bak[i]>=65 && s_bak[i]<=90 )
             s_bak[i] = s_bak[i] + 0x20;
			s[j++] = s_bak[i];						
       }
    }  
    s[j] = '\0';    //这一条语句千万不能忘记，字符串的结束标记  
}  


/*从公钥中获取FingerPrint*/
LOCAL char * RetrieveFingerPrintFromCertificate(char * pPubKeyFile, BOOL bNeedColon)
{
	FILE *fcert =NULL;
	char strshell[256] = {0};
	char *start = NULL; 
	OSA_MemSet(pFingerPrint,0,sizeof(pFingerPrint));
	
	sprintf(strshell,"openssl x509 -in %s -fingerprint -noout 1> %s",pPubKeyFile,PATH_FINGERPRINT);
	system(strshell);
	fcert =  fopen(PATH_FINGERPRINT,"r");
	if(fcert != NULL)
	{
		fgets(pFingerPrint,sizeof(pFingerPrint),fcert);
	}
	fclose(fcert);
	start = strstr(pFingerPrint,"=");
    if(start!=NULL)
    {
		start++;
		if(start[strlen(start)-1] == '\n')
		{
			start[strlen(start)-1] = '\0';
		}
		
		if(bNeedColon == FALSE)
		{
			squeeze(start);   //去掉Fingerprint下的冒号
		}
    }
	return start;
}

LOCAL VOID AddUserToFlexisip(char* username,char* password)
{
	char account[128] = {0};
	FILE* auth_fd = fopen(PATH_FLEXISIP_AUTH_CFG,"a+");
	if(!auth_fd)
	{
		OSA_DBG_MSG("%sOpen %s Failed\n",DEBUG_HEADER_PORTALCLIENT,PATH_FLEXISIP_AUTH_CFG);
		return;
	}
	OSA_DBG_MSG("%sAdd User(userName=%s password=%s)To Flexisip\n",DEBUG_HEADER_PORTALCLIENT,username,password);
	sprintf(account,"%s@%s %s\n",username,GetDeviceDomain(),password);
	int len = fwrite(account,1,strlen(account),auth_fd);
	if(len <= 0)
	{
		OSA_ERROR("%sAdd User to Flexisip Failed",DEBUG_HEADER_PORTALCLIENT);
	}
	fclose(auth_fd);
}

VOID UpdateAccessControlList(VOID)
{
	char *pfEtcFile = PATH_WELCOME_CFG;
	UserPri stUserPri[4];	
	char pcSection[16] = {0};
	char pcAclLine[5][1024] = {{0}};
	int i = 0;
	
	
	//打开acl.list 文件
	FILE *pACLFileFd = fopen(PATH_FLEXISIP_ACL,"w");
	if(pACLFileFd == NULL)
	{
		OSA_ERROR("%sOpen %s Failed\n",DEBUG_HEADER_PORTALCLIENT,PATH_FLEXISIP_ACL);
		return;
	}

	for(i = 0;i < 4; i++)  //最多4台APP
	{ 
		OSA_MemSet(&stUserPri[i],0,sizeof(UserPri));
		OSA_MemSet(pcSection,0,sizeof(pcSection));
		sprintf(pcSection,"user_%d",(i+1));   //Index 1-4
		OSA_MemSet(stUserPri[i].id,0,128);
		if(0 == GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_USERNAME,stUserPri[i].id,128))   						//从config.ini中获取APP id
		{
			
			GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_PRIVILEGE_CONVERSATION_KEY,stUserPri[i].ring,4);  			// 通话权限 "yes" or "no"
			GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_PRIVILEGE_SURVEILLANCE_KEY,stUserPri[i].surveillance,4);
			GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_PRIVILEGE_OPENDOOR_KEY,stUserPri[i].opendoor,4);
			GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_PRIVILEGE_SWITCHLIGHT_KEY,stUserPri[i].switchlight,4);
			GetValueFromEtcFile(pfEtcFile,pcSection,WELCOME_APP_PRIVILEGE_SCREENSHOT_KEY,stUserPri[i].screenshot,4);
			sprintf(stUserPri[i].precamera,"%s","yes");
			sprintf(stUserPri[i].nextcamera,"%s","yes");
		}
	}

	for(i = 0; i < 5; i++)
	{
		OSA_MemSet(pcAclLine[i],0,1024);
	}
	
	// for static route , "root" must be added
	sprintf(pcAclLine[0],"%s","S ");  //允许监视的APP 列表
	sprintf(pcAclLine[1],"%s","R ");  //允许通话的APP 列表
	sprintf(pcAclLine[2],"%s","1 ");  //允许开锁的APP 列表
	sprintf(pcAclLine[3],"%s","2 ");  //允许开灯的APP 列表
	sprintf(pcAclLine[4],"%s","3 ");  //允许抓拍的APP 列表

	for(i = 0; i < 4; i++)
	{
		// S   surveillance  有监视权限的APP 的用户ID 列出来 用空格隔开
		if(strcmp(stUserPri[i].surveillance,WELCOME_APP_PRIVILEGE_ENABLE) == 0)   //该APP 允许监视
		{
			sprintf(pcAclLine[0],"%s %s",pcAclLine[0],stUserPri[i].id);
		}
		
		//R ring  有通话权限的APP 的用户ID 列出来 用空格隔开
		if(strcmp(stUserPri[i].ring,WELCOME_APP_PRIVILEGE_ENABLE) == 0)
		{
			sprintf(pcAclLine[1],"%s %s",pcAclLine[1],stUserPri[i].id);
		}
		//1opendoor  有开锁权限的APP的用户ID 列出来 用空格隔开
		if(strcmp(stUserPri[i].opendoor,WELCOME_APP_PRIVILEGE_ENABLE) == 0)
		{
			sprintf(pcAclLine[2],"%s %s",pcAclLine[2],stUserPri[i].id);
		}
		//2switchlight  有开灯权限的APP的用户ID 列出来 用空格隔开
		if(strcmp(stUserPri[i].switchlight,WELCOME_APP_PRIVILEGE_ENABLE) == 0)
		{
			sprintf(pcAclLine[3],"%s %s",pcAclLine[3],stUserPri[i].id);
		}
		// 3 ScreenShot  有抓拍权限的APP的用户ID 列出来 用空格隔开
		if(strcmp(stUserPri[i].screenshot,WELCOME_APP_PRIVILEGE_ENABLE) == 0)
		{
			sprintf(pcAclLine[4],"%s %s",pcAclLine[4],stUserPri[i].id);
		}

	}
	
	// write acl
	for(i = 0; i < 5; i++)
	{
		sprintf(pcAclLine[i],"%s\n",pcAclLine[i]);
		fwrite(pcAclLine[i],1,strlen(pcAclLine[i]),pACLFileFd);
	}
	
	fclose(pACLFileFd);
}


VOID sendACLuploadEvent(char * section,int pwflag)
{
	InternalEventInfo stinfo;
	memset(&stinfo,0,sizeof(stinfo));
	
	stinfo.type = eEventACLupload;
	stinfo.pwflag = pwflag;
	sprintf(stinfo.user_section,"%s",section);
	
	fnSendMsg2PortClientSendThread((unsigned char*)&stinfo,sizeof(InternalEventInfo));
}

VOID NotifyPortalServerACLForAllPairedApp(VOID)
{
	INT  i = 0;
	char pAppSection[16] = {0};

	char pState[16] = {0};
	
		
	/*从User_1 开始遍历， 最多遍历4  条APP 记录*/
	for(i = 0;i < MAX_PAIRING_USER; i++)
	{
		OSA_MemSet(pAppSection,0,sizeof(pAppSection));
		sprintf(pAppSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,i+1);	

		OSA_MemSet(pState,0,sizeof(pState));
		GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pAppSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE,pState,16);

		if(strcmp(pState,WELCOME_APP_PAIR_STATE_PAIRED) == 0)
		{
			sendACLuploadEvent(pAppSection,1);
		}
	}
}

/*激活请求配对的手机，实现配对*/
INT ActivatePairingAPP(char * pWelcomeAPPSection, BYTE * pInterityCode)
{	
	OSA_DBG_MSG("%sInterityCode is %s",DEBUG_HEADER_PORTALCLIENT,pInterityCode);
	
	char pSipName[128] = {0};
	char pPubKeyFile[512] = {0};
	char *pFingerPrint = NULL;

	unsigned char pComputeData[64] = {0};
	char * pMdMix = NULL;
	char pHashResult[16] = {0};

	char pcPassword[128] = {0};
	
	GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelcomeAPPSection,WELCOME_APP_PAIR_REQUEST_KEY_SIP_NAME,pSipName,128);
	sprintf(pPubKeyFile,"%s/%s.crt",PATH_WELCOME_APP_ROOT,pWelcomeAPPSection);
	pFingerPrint = RetrieveFingerPrintFromCertificate(pPubKeyFile,TRUE);

	if(pFingerPrint == NULL)
	{
		return -1;
	}

	OSA_DBG_MSG("%sFingerPrint = %s\n",DEBUG_HEADER_PORTALCLIENT,pFingerPrint);
	
	memcpy(pComputeData,pInterityCode,4);//salt
	pComputeData[4] = ':';
	strncpy(pComputeData+5,pFingerPrint,strlen(pFingerPrint));//salt
	pMdMix = MD5(pComputeData,strlen(pFingerPrint)+5,NULL);
	sprintf(pHashResult,"%02x%02x%02x%02x",pMdMix[0],pMdMix[1],pMdMix[2],pMdMix[3]);

	OSA_DBG_MSG("%sHashResult : %s\n",DEBUG_HEADER_PORTALCLIENT,pHashResult);
	OSA_DBG_MSG("%sEntered InterityCode Last Four: %s\n",DEBUG_HEADER_PORTALCLIENT,pInterityCode + 4);

	if(strncasecmp(pHashResult,pInterityCode+4,4) == 0)
	{
		GetUnformattedUUID(pcPassword,16);
		SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelcomeAPPSection,WELCOME_APP_PAIR_REQUEST_KEY_PASSWORD,pcPassword);
		SaveIniData(PATH_WELCOME_APP_PAIR_REQUEST);
		AddUserToFlexisip(pSipName,pcPassword);
		UpdateAccessControlList();
        SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelcomeAPPSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE,WELCOME_APP_PAIR_STATE_PAIRED);
		SaveIniData(PATH_WELCOME_APP_PAIR_REQUEST);
		sendACLuploadEvent(pWelcomeAPPSection,1);
		UpdateRecordToRouteFile();  
		
		/*Notify UI Paring Successful*/
		OSA_DBG_MSG("%sParing Successful",DEBUG_HEADER_PORTALCLIENT);
		return 0;
	}

	/*Notify UI Paring Fail*/
	OSA_DBG_MSG("%sParing Fail",DEBUG_HEADER_PORTALCLIENT);
	return -1;
}


VOID SaveAPPAccessProperty(BYTE * pu8AccessControl)
{ 
	char sipname[128] = {0};
	char pPubKeyFile[256] = {0};
	BYTE u8Index = pu8AccessControl[0];
	BYTE u8Conversation = pu8AccessControl[1];
	BYTE u8Surveillance = pu8AccessControl[2];
	BYTE u8SwitchLight = pu8AccessControl[3];
	BYTE u8OpenDoor = pu8AccessControl[4];
	BYTE u8AccessHistory = pu8AccessControl[5];
	BYTE u8DeleteHistory = pu8AccessControl[6];
	char cTmp[4];
	char pcPairState[20];
	char pWelcomeAPPSection[16];
	sprintf(pWelcomeAPPSection,"%s%d",WELCOME_APP_RECORD_SECTION_PREFIX,u8Index);
	
	/*generate_sip_username*/
	sprintf(pPubKeyFile,"%s/%s.crt",PATH_WELCOME_APP_ROOT,pWelcomeAPPSection);
	char *fingerprint = RetrieveFingerPrintFromCertificate(pPubKeyFile,FALSE);
	
	//GetValueFromEtcFile_new(PATH_IPGW_CFG,IPGW_SECTION_PORTAL,IPGW_KEY_PORTALNAME,portalname,64);
	sprintf(sipname,"%s_%.5s",GetPortalServerUsrName(),fingerprint);

	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_USERNAME,sipname);

	//设置通话权限
	if(u8Conversation == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_CONVERSATION_KEY,cTmp);

	//设置监视权限
	if(u8Surveillance == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_SURVEILLANCE_KEY,cTmp);

	//设置开门权限
	if(u8OpenDoor == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_OPENDOOR_KEY,cTmp);

	//设置开灯权限
	if(u8SwitchLight == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_SWITCHLIGHT_KEY,cTmp);

	//设置访问通话记录权限
	if(u8AccessHistory == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_ACCESSHISTORY_KEY,cTmp);

	//设置删除通话记录权限
	if(u8DeleteHistory == 1)
	{
		sprintf(cTmp,"%s","yes");
	}
	else
	{
		sprintf(cTmp,"%s","no");
	}
	
	SetValueToEtcFile(PATH_WELCOME_CFG,pWelcomeAPPSection,WELCOME_APP_PRIVILEGE_DELETEHISTORY_KEY,cTmp);
	SetValueToEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelcomeAPPSection,WELCOME_APP_PAIR_REQUEST_KEY_SIP_NAME,sipname);
	SaveIniData(PATH_WELCOME_CFG);
	SaveIniData(PATH_WELCOME_APP_PAIR_REQUEST);
	GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pWelcomeAPPSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE, pcPairState,sizeof(pcPairState));
		
	if(strcmp(pcPairState,WELCOME_APP_PAIR_STATE_PAIRED)  == 0)
	{
		UpdateAccessControlList();
        UpdateRecordToRouteFile();
		sendACLuploadEvent(pWelcomeAPPSection,1);
	}
}


LOCAL INT DeleleSection(const char* pEtcFile, const char* pSection)
{
    FILE *fileOld, *fileNew;
    char buf[MAX_CFG_SIZE],lineAsRead[MAX_CFG_SIZE];
	char *eol,*index;
	int findSection=0;
    char tempfile[128] = {0};
    int pid = 0;
	
    if(!(fileOld = fopen(pEtcFile, "r")))
    {    //文件不存在
 
        fclose(fileOld);
        return -1;
    }
    pid = getpid();
    sprintf(tempfile,"./tmp%dXXXXXX",pid);
	int outf = mkstemp(tempfile);
    if(outf == -1)
    {
        sprintf(tempfile,"%s.tmp",pEtcFile);
    }
    
    if(!(fileNew = fopen(tempfile, "w+")))
    {
        fclose(fileNew);
        return -1;
    }  
    while (fgets(buf, MAX_CFG_SIZE, fileOld) != NULL)
    {
        for (index = buf; *index == ' ' || *index == '\t'; index++);
        if (*index == '#' )
        {
    	    continue;
        }
    	eol = strrchr(index, '\n');
    	if (eol != NULL)
    	{
    		*eol = '\0';
    	}

        
    	strcpy(lineAsRead, buf);	
    	if(strchr(lineAsRead, '[') == lineAsRead)
    	{
    		char *tmp = NULL;
    		char *p = strchr(lineAsRead,']');
			findSection = 0; 
			if(p)
			{
				tmp = malloc(p-lineAsRead);
				if(tmp)
				{
					memset(tmp,0,p-lineAsRead);
					memcpy(tmp,lineAsRead+1,p-lineAsRead-1);
					if(strcmp(tmp, pSection) == 0)
		            {
		                findSection = 1; 
		            }
				}
			}
			if(tmp)
			{
				free(tmp);
				tmp=NULL;
			}
			
        }    
        if(findSection == 0)
        {
            fputs(lineAsRead, fileNew);
            fputc('\n', fileNew);
        }
    }
    fclose(fileOld);
    fclose(fileNew);

    char pShellCmd[128] = {0};
    sprintf(pShellCmd,"cp %s %s ",tempfile,pEtcFile);
    system(pShellCmd);
    sprintf(pShellCmd,"rm %s -f",tempfile);
    system(pShellCmd);
    close(outf);
    return 0;
 }


VOID DeleteAllPairedApp(VOID)
{
	int i = 0;
	char section[16] = {0};
	char pShellCmd[128] ={0};
	sprintf(pShellCmd,"rm %s -f",PATH_WELCOME_APP_PAIR_REQUEST);
	system(pShellCmd);
	LoadIniData(PATH_WELCOME_APP_PAIR_REQUEST);

	if(OSA_FileIsExist(PATH_WELCOME_CFG) == TRUE)
	{
		/*删除config.ini中app*/
		for(i = 1; i < 1+ MAX_PAIRING_USER; i++)
		{
			sprintf(section,"user_%d",i);
			DeleleSection(PATH_WELCOME_CFG,section);
			LoadIniData(PATH_WELCOME_CFG);
		}
	}
	/*删除acl.list*/
	UpdateAccessControlList();
	UpdateRecordToRouteFile();
	/*删除auth_db*/
	UpdateAuthFileDomain(GetDeviceDomain());
}

LOCAL VOID DeletePairedAppInfo(char * sid)
{
	OSA_DBG_MSG("%sDelete Paired App Section=%s\n",DEBUG_HEADER_PORTALCLIENT,sid);
	if(sid != NULL)
	{
		/*删除公钥、证书*/
		char pShellCmd[128] ={0};
		sprintf(pShellCmd,"rm %s%s.crt -f",PATH_WELCOME_APP_ROOT,sid);
		system(pShellCmd);
		sprintf(pShellCmd,"rm %s%s.pubkey -f",PATH_WELCOME_APP_ROOT,sid);
		system(pShellCmd);
		sprintf(pShellCmd,"rm %s%s.serial -f",PATH_WELCOME_APP_ROOT,sid);
		system(pShellCmd);
		/*删除WelAppPairRequest.ini 中的段*/
		DeleleSection(PATH_WELCOME_APP_PAIR_REQUEST,sid);
		system("sync");
		LoadIniData(PATH_WELCOME_APP_PAIR_REQUEST);
		/*删除config.ini 中的段*/
		DeleleSection(PATH_WELCOME_CFG,sid);
		system("sync");
		LoadIniData(PATH_WELCOME_CFG);
		/*删除acl.list*/
		UpdateAccessControlList();
        UpdateRecordToRouteFile();
		/*删除auth_db*/
		UpdateAuthFileDomain(GetDeviceDomain());
	}
}


VOID DeletePairedAppBySection(char* section)
{
	if(section == NULL)
	{
		return;
	}
	
	char pState[16] = {0};
	GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,section,WELCOME_APP_PAIR_REQUEST_KEY_STATE,pState,16);

	if(strcmp(pState,WELCOME_APP_PAIR_STATE_PAIRED) == 0)
	{
		sendACLuploadEvent(section,0);
		DeletePairedAppInfo(section);

	}
	else
	{
		DeletePairedAppInfo(section);
	}

	//通知UI 更新APP 列表
	NotifyUIWelcomeAppList(PORTAL_CLIENT_PUSH);
}



VOID DeleteAppBySenderUUID(void* param)
{
	char *pSenderuuid = (char *)param;
	int i = 0;
	char section[16] = {0};
	char uuid[64] = {0};
	
	for(i = 1;i<1+MAX_PAIRING_USER;i++)
	{
		memset(uuid,0,64);
		sprintf(section,"user_%d",i);
		GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,section,WELCOME_APP_PAIR_REQUEST_KEY_UUID,uuid,64);
		if(strcmp(uuid,pSenderuuid) == 0)
		{
			DeletePairedAppBySection(section);
		}
	}
	free(pSenderuuid);
	pSenderuuid = NULL;
}

DEVICEMANAGER_LIST stSecondDoorStationList;
DEVICEMANAGER_LIST stOSDeviceManagerList;
DEVICEMANAGER_LIST stGateStationList;

DEVICEMANAGER_LIST * GetOSList(VOID)
{
#if 1
	DeviceManager_GetDeviceListByDeviceType(DT_DOOR_STATION, &stOSDeviceManagerList);

	OSA_DBG_MSG("couns = %d---",stOSDeviceManagerList.u16Counts);

	int i = 0;
	for(i = 0; i < stOSDeviceManagerList.u16Counts; i++)
	{	
		OSA_DBG_MSG("device id(%s), ip(%s)",stOSDeviceManagerList.stItem[i].szDeviceStrId, stOSDeviceManagerList.stItem[i].szDeviceIp);
			
	}

	return &stOSDeviceManagerList;

	/*Example*/

	
	/*device Type(74)(0)
	
	device (740015000100)(10.3.192.0)(1)
	
	couns = 1---
	
	device id(740015000100), ip(10.3.192.0)*/
#endif


#if 0

	stDeviceManagerList.u16Counts = 2;
	sprintf(stDeviceManagerList.stItem[0].szDeviceStrId,"%s","740007000100");
	sprintf(stDeviceManagerList.stItem[0].szDeviceIp,"%s","10.1.192.0");
	sprintf(stDeviceManagerList.stItem[1].szDeviceStrId,"%s","740007000101");
	sprintf(stDeviceManagerList.stItem[1].szDeviceIp,"%s","10.1.192.1");
	return &stDeviceManagerList;
#endif
}


DEVICEMANAGER_LIST * GetSecondDoorStationList(VOID)
{
	DeviceManager_GetDeviceListByDeviceType(DT_DIGITAL_SECOND_DOOR_STATION, &stSecondDoorStationList);

	OSA_DBG_MSG("couns = %d---",stSecondDoorStationList.u16Counts);

	int i = 0;
	for(i = 0; i < stSecondDoorStationList.u16Counts; i++)
	{	
		OSA_DBG_MSG("device id(%s), ip(%s)",stSecondDoorStationList.stItem[i].szDeviceStrId, stSecondDoorStationList.stItem[i].szDeviceIp);
			
	}

	return &stSecondDoorStationList;
}


DEVICEMANAGER_LIST * GetGateStationList(VOID)
{
	DeviceManager_GetDeviceListByDeviceType(DT_GATE_STATION, &stGateStationList);

	OSA_DBG_MSG("couns = %d---",stGateStationList.u16Counts);

	int i = 0;
	for(i = 0; i < stGateStationList.u16Counts; i++)
	{	
		OSA_DBG_MSG("device id(%s), ip(%s)",stGateStationList.stItem[i].szDeviceStrId, stGateStationList.stItem[i].szDeviceIp);
			
	}

	return &stGateStationList;
}


VOID UpdateRecordToRouteFile(VOID)
{
	OSA_DBG_MSG("%s%s Enter",DEBUG_HEADER_PORTALCLIENT,__func__);
			
	FILE* pFileNewFd;
    char pfileContentTmp[1024] = {0};
	char pSection[12];
	char pState[12];
	char pUserName[MAX_PAIRING_USER][128];
	int i = 0;
	char pTmp[1024] = {0};
	BOOL bFirstPairedItem = FALSE;
	BOOL bFoundFirstPairedItem = FALSE;

    if(!(pFileNewFd = fopen(PATH_FLEXISIP_ROUTE_CFG, "w+")))
    {
        fclose(pFileNewFd);
        return;
    }  

	OSA_MemSet(pfileContentTmp,0,sizeof(pfileContentTmp));
	//APP
   	for(i = 0; i < MAX_PAIRING_USER; i++)
	{
		if(OSA_FileIsExist(PATH_WELCOME_APP_PAIR_REQUEST) == FALSE)
		{
			OSA_DBG_MSG("%s%s But %s is missing",DEBUG_HEADER_PORTALCLIENT,__func__,PATH_WELCOME_CFG);
			break;
		}
		
		OSA_MemSet(pSection,0,sizeof(pSection));
		sprintf(pSection,"user_%d",i+1);
		OSA_MemSet(pState,0,sizeof(pState));
		if(0 == GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pSection,WELCOME_APP_PAIR_REQUEST_KEY_SIP_NAME,pUserName[i],128))
		{
			GetValueFromEtcFile(PATH_WELCOME_APP_PAIR_REQUEST,pSection,WELCOME_APP_PAIR_REQUEST_KEY_STATE,pState,12);
			if(strcmp(pState,WELCOME_APP_PAIR_STATE_PAIRED) == 0)
			{
				OSA_DBG_MSG("%sFounding Paired APP %s",DEBUG_HEADER_PORTALCLIENT,pSection);
				if(bFoundFirstPairedItem == FALSE)
				{
					bFoundFirstPairedItem = TRUE;  //已经找到第一条记录
					bFirstPairedItem = TRUE;
				}
				else
				{
					bFirstPairedItem = FALSE;
				}
				
				if(bFirstPairedItem == TRUE)
				{
					bFirstPairedItem = FALSE;
					sprintf(pfileContentTmp,"<sip:%s@%s:5070> <sip:%s@%s>",SystemDeviceInfo_GetLocalStrID(),GetExternalIP(),pUserName[i],GetDeviceDomain()); 
					//OSA_DBG_MSG("%sroute.ini content is %s",DEBUG_HEADER_PORTALCLIENT,pfileContentTmp);
				}
				else
				{
					OSA_MemSet(pTmp,0,sizeof(pTmp));
					sprintf(pTmp,",<sip:%s@%s>",pUserName[i],GetDeviceDomain());
					strcat(pfileContentTmp,pTmp);
				}
			}
			
		}
	}

	strcat(pfileContentTmp,"\n");

	OSA_DBG_MSG("%sroute.ini content is %s",DEBUG_HEADER_PORTALCLIENT,pfileContentTmp);
	fwrite(pfileContentTmp, strlen(pfileContentTmp), 1, pFileNewFd);

	char pLineTemp[512];
	
	//OS
	DEVICEMANAGER_LIST * pstOSList = GetOSList();
	if(pstOSList != NULL)
	{
		int i = 0;
		for(i = 0; i < pstOSList->u16Counts; i++)
		{	
			OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
			sprintf(pLineTemp,"<sip:%s@%s> <sip:%s@%s:5070>\n",pstOSList->stItem[i].szDeviceStrId,GetDeviceDomain(),pstOSList->stItem[i].szDeviceStrId,pstOSList->stItem[i].szDeviceIp);
			fwrite(pLineTemp, strlen(pLineTemp), 1, pFileNewFd);
		}
	}

	//2nd Door Station
	DEVICEMANAGER_LIST * pstSecondDoorStationList = GetSecondDoorStationList();
	if(pstSecondDoorStationList != NULL)
	{
		int i = 0;
		for(i = 0; i < pstSecondDoorStationList->u16Counts; i++)
		{	
			OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
			sprintf(pLineTemp,"<sip:%s@%s> <sip:%s@%s:5070>\n",pstSecondDoorStationList->stItem[i].szDeviceStrId,GetDeviceDomain(),pstSecondDoorStationList->stItem[i].szDeviceStrId,pstSecondDoorStationList->stItem[i].szDeviceIp);
			fwrite(pLineTemp, strlen(pLineTemp), 1, pFileNewFd);
		}
	}

	//Gate Station
	DEVICEMANAGER_LIST * pstGateStationList = GetGateStationList();
	if(pstGateStationList != NULL)
	{
		int i = 0;
		for(i = 0; i < pstGateStationList->u16Counts; i++)
		{	
			OSA_MemSet(pLineTemp,0,sizeof(pLineTemp));
			sprintf(pLineTemp,"<sip:%s@%s> <sip:%s@%s:5070>\n",pstGateStationList->stItem[i].szDeviceStrId,GetDeviceDomain(),pstGateStationList->stItem[i].szDeviceStrId,pstGateStationList->stItem[i].szDeviceIp);
			fwrite(pLineTemp, strlen(pLineTemp), 1, pFileNewFd);
		}
	}
	
	fclose(pFileNewFd);

	//通知IPGateway route.ini更新了
	fnPortalClientSendMsg2IPGateway(PROCESS_OPER_IPGW_FLEXISIP_ROUTE_CHANGED, NULL, 0);

	//system("killall -SIGUSR1 flexisip"); //joerg说用来让flexisip同步配置为文件
}


