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
#include "XMPPClient.h"
#include "module.h"
#include "remotesocket.h"
#include "thread_main.h"

/*-------------------- Global Definitions and Declarations -------------------*/

#define FILE_CERT_CA_CRT_INIT "/usr/app/ca.crt"
#define FILE_CERT_CA_CRT "/usr/mnt/userdata/PortalClient/ca.crt"

/*----------------------- Constant / Macro Definitions -----------------------*/

/*------------------------ Type Declarations ---------------------------------*/

/*------------------------ Variable Declarations -----------------------------*/

/*------------------------ Function Prototype --------------------------------*/

extern VOID fnCreatePortalClientSendThread(VOID);
extern VOID fnDeletePortalClientSendThread(VOID);
extern VOID fnCreatePortalClientReceiveThread(VOID);
extern VOID fnDeletePortalClientReceiveThread(VOID);
extern VOID GeneratePortalClientDirectory(VOID);
extern VOID GeneratePrivateKey(VOID);
extern VOID LoadDeviceDomainFromCfgFile(VOID);
// extern VOID InitB2BSip(VOID);

extern VOID fnCreateXmppClientSendThread(VOID);
extern VOID fnCreateXmppClientReceiveThread(VOID);

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
int main(int argc, char* argv[])
{
	printf("\n###############################################################\n\n");
	printf("[Portal Client] Compile %s %s ", __DATE__, __TIME__);
	printf("\n###############################################################\n");

	INT iXmppConnectWaitCount = 0;
	INT iXmppConnectResult	= FALSE;

	OSA_InitTimer();

#if 1
	/// DeviceManager_Init();
	GeneratePortalClientDirectory();

	IniGlobalInit(5);
	LoadIniData(PATH_OPENSSL_CFG);
	LoadIniData(PATH_WELCOME_CFG);
	// LoadIniData(PATH_WELCOME_APP_PAIR_REQUEST);
	// LoadIniData(PATH_PORTAL_SERVER_CFG);
	// LoadIniData(PATH_B2B_CFG);

	/*Generate Private Key*/
	GeneratePrivateKey();

	// SystemDeviceInfo_Init();

	SetPortalServerUrl();
	SetPortalDomain();
	// InitB2BSip();

	/// SetValueToEtcFile(PATH_WELCOME_CFG,WELCOME_SECTION_NETWORK,WELCOME_KEY_LOCAL_ID,SystemDeviceInfo_GetLocalStrID());
	/// SaveIniData(PATH_WELCOME_CFG);

	LoadDeviceDomainFromCfgFile();

	LoadPortClientLoginStatus();

	/*Create Portal Client Communicate with UI Thread and Other Thread, then send message to Portal Server*/
	fnCreatePortalClientSendThread();

	// need change lxy
	fnCreateXmppClientReceiveThread();

	/*Connect with Portal Server and Receive message from Portal Server*/
	// fnCreatePortalClientReceiveThread();

	event_init();
	thread_main_init();

	while (1) {
		if (eStateEstablish != GetConnectState()) {
			if (iXmppConnectWaitCount < 1024) {
				iXmppConnectWaitCount++;
				sleep(1);
				continue;
			} else {
				OSA_DBG_MSG("Connect to XMPP Failed Due to Handshake Timeout !\n");
				iXmppConnectResult = FALSE;
				break;
			}
		} else {
			OSA_DBG_MSG("Connect to Portal successfully!\n");
			iXmppConnectResult = TRUE;
			break;
		}
	}
#endif

	if (iXmppConnectResult == TRUE) {
		fnDeletePortalClientSendThread();
		fnDeletePortalClientReceiveThread();

		if (OSA_FileIsExist(FILE_CERT_CA_CRT) == FALSE) {
			OSA_FileCopy(FILE_CERT_CA_CRT_INIT, FILE_CERT_CA_CRT);
		} else {
			OSA_DBG_MSG("The ca.crt is existing");
		}

		/*Create XMPP Client Communicate with UI Thread and Other Thread, then send message to XMPP Server*/
		// fnCreateXmppClientSendThread();

		/*Connect with XMPP Server and Receive message from XMPP Server*/
		fnCreateXmppClientReceiveThread();
	}

	while (1) {
		OSA_Sleep(500);
	}
	event_uninit();
	return 1;
}
