/******************************************************************************
* Copyright 2010-2011 ABB Genway Co.,Ltd.
* FileName: 	 main.c
* Desc:    
* 
* 
* Author: 	Shelly-Yunqing.Ye@cn.abb.com
* Date: 	 	2018/08/23
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2018/08/23, Shelly-Yunqing.Ye@cn.abb.com create this file
* 
******************************************************************************/ 


/*-------------------------------- Includes ----------------------------------*/

#include "PortalClient.h"

/*-------------------- Global Definitions and Declarations -------------------*/


/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Type Declarations ---------------------------------*/


/*------------------------ Variable Declarations -----------------------------*/


/*------------------------ Function Prototype --------------------------------*/

extern VOID fnCreatePortalClientSendThread(VOID);
extern VOID fnCreatePortalClientReceiveThread(VOID);
extern VOID GeneratePortalClientDirectory(VOID);
extern VOID GeneratePrivateKey(VOID);
extern VOID LoadDeviceDomainFromCfgFile(VOID);
extern VOID InitB2BSip(VOID);


/******************************************************************************
* Name: 	 main 
*
* Desc: 	 
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author:  Shelly-Yunqing.Ye@cn.abb.com
* -------------------------------------
* Log: 	 2017/08/23, Shelly-Yunqing.Ye@cn.abb.com Create this function
 ******************************************************************************/

#if 0
int main(int argc, char *argv[])
{
	printf("\n###############################################################\n\n");
	printf("[Portal Client] Compile %s %s ",__DATE__,__TIME__);
	printf("\n###############################################################\n");

	DeviceManager_Init();  				//��ȡDevice Manager ��share memory��ָ��
	GeneratePortalClientDirectory();

	IniGlobalInit(5);
	LoadIniData(PATH_OPENSSL_CFG);
	LoadIniData(PATH_WELCOME_CFG);
	LoadIniData(PATH_WELCOME_APP_PAIR_REQUEST);
	LoadIniData(PATH_PORTAL_SERVER_CFG);
	LoadIniData(PATH_B2B_CFG);

	/*Generate Private Key*/
	GeneratePrivateKey();

	SystemDeviceInfo_Init();  //Ϊ�˳ɹ����� SystemDeviceInfo_GetCommunityIPAddress ������������Init�� �����ڴ�Ļ���
	OSA_InitTimer(); 		  //����Timer�̣߳�����OSA_SetTimer���ܳɹ�

	SetPortalServerUrl();
	SetPortalDomain();
	InitB2BSip();
	
	//�豸����ID��PATH_WELCOME_CFG
	SetValueToEtcFile(PATH_WELCOME_CFG,WELCOME_SECTION_NETWORK,WELCOME_KEY_LOCAL_ID,SystemDeviceInfo_GetLocalStrID());
	SaveIniData(PATH_WELCOME_CFG);
	
	LoadDeviceDomainFromCfgFile();
	
	LoadPortClientLoginStatus();
	/*Create Portal Client Communicate with UI Thread and Other Thread, then send message to Portal Server*/
	fnCreatePortalClientSendThread();
	
	/*Connect with Portal Server and Receive message from Portal Server*/
	fnCreatePortalClientReceiveThread();

	/*TCP Server Thread, wait event from other device*/
	fnCreatePortalClientTCPServerThread();

	while(1)
	{
		OSA_Sleep(3000);
	}
    return 1; 

}
#endif

#if 1
int PortalClientModuleInit(void)
{
	printf("\n###############################################################\n\n");
	printf("[Portal Client] Compile %s %s ",__DATE__,__TIME__);
	printf("\n###############################################################\n");

	DeviceManager_Init();  				//��ȡDevice Manager ��share memory��ָ��
	GeneratePortalClientDirectory();

	IniGlobalInit(6);
	LoadIniData(PATH_OPENSSL_CFG);
	LoadIniData(PATH_WELCOME_CFG);
	LoadIniData(PATH_WELCOME_APP_PAIR_REQUEST);
	LoadIniData(PATH_PORTAL_SERVER_CFG);
	LoadIniData(PATH_B2B_CFG);
	LoadIniData(PATH_FIRMWARE_VERSION_CFG);

	/*Generate Private Key*/
	GeneratePrivateKey();

	SystemDeviceInfo_Init();  //Ϊ�˳ɹ����� SystemDeviceInfo_GetCommunityIPAddress ������������Init�� �����ڴ�Ļ���
	OSA_InitTimer(); 		  //����Timer�̣߳�����OSA_SetTimer���ܳɹ�

	SetPortalServerUrl();
	SetPortalDomain();
	InitB2BSip();
	
	//�豸����ID��PATH_WELCOME_CFG
	SetValueToEtcFile(PATH_WELCOME_CFG,WELCOME_SECTION_NETWORK,WELCOME_KEY_LOCAL_ID,SystemDeviceInfo_GetLocalStrID());
	SaveIniData(PATH_WELCOME_CFG);
	
	LoadDeviceDomainFromCfgFile();
	
	LoadPortClientLoginStatus();
	/*Create Portal Client Communicate with UI Thread and Other Thread, then send message to Portal Server*/
	fnCreatePortalClientSendThread();
	
	/*Connect with Portal Server and Receive message from Portal Server*/
	fnCreatePortalClientReceiveThread();

	return 0;
}

#endif

