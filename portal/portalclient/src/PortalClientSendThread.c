#include "PortalClient.h"

extern int SendCertSignedRequest(char* username, char* password, char* fname);

// LOCAL INT32 s_h2PortalClientSendFd			= -1;
LOCAL OSA_ThrHndl s_hPortalClientSendThread = {0};
LOCAL char		  s_pu8PortalUsrName[64];
LOCAL pthread_mutex_t s_Mutex_PortalClientLoginStatus;
LOCAL BYTE u8PortalClientLoginStatus = PORTAL_CLIENT_STATUS_LOGOUT;
LOCAL pthread_mutex_t s_Mutex_PortalClientConnectStatus;
LOCAL BYTE u8PortalClientConnectStatus = PORTAL_CLIENT_STATUS_CONNECT_FAIL;

VOID SetPortalClientLoginStatus(ePORTAL_CLIENT_LOGIN_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_PortalClientLoginStatus);
	u8PortalClientLoginStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_PortalClientLoginStatus);
	// NotifyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID SetPortalClientConnectStatus(ePORTAL_CLIENT_CONNECT_STATUS eStatus)
{
	pthread_mutex_lock(&s_Mutex_PortalClientConnectStatus);
	u8PortalClientConnectStatus = eStatus;
	pthread_mutex_unlock(&s_Mutex_PortalClientConnectStatus);
	// NotifyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID ReplyPortalClientStatusToUI(VOID)
{
	// ReplyUIPortalClientStatus(u8PortalClientLoginStatus,u8PortalClientConnectStatus);
}

VOID LoadPortClientLoginStatus(VOID)
{
	char pUUID[128];

	OSA_MemSet(pUUID, 0, sizeof(pUUID));

	GetValueFromEtcFile(PATH_WELCOME_CFG, WELCOME_SECTION_NETWORK, WELCOME_KEY_UUID, pUUID, sizeof(pUUID) - 1);
	if (strcmp(pUUID, "") == 0) {
		SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGOUT);
	} else {
		SetPortalClientLoginStatus(PORTAL_CLIENT_STATUS_LOGIN);
	}
}

VOID SetPortalServerUsrName(char* pu8UsrName)
{
	sprintf(s_pu8PortalUsrName, "%s", pu8UsrName);
	SetValueToEtcFile(PATH_PORTAL_SERVER_CFG, PORTAL_SERVER_SECTION, PORTAL_SERVER_USER_KEY, s_pu8PortalUsrName);
	SaveIniData(PATH_PORTAL_SERVER_CFG);
}

char* GetPortalServerUsrName(VOID)
{
	GetValueFromEtcFile(PATH_PORTAL_SERVER_CFG, PORTAL_SERVER_SECTION, PORTAL_SERVER_USER_KEY, s_pu8PortalUsrName, 64);
	return s_pu8PortalUsrName;
}

VOID fnPortalClientHandleMessageFromUI(BYTE* msg_buf, UINT32 len)
{
}

VOID fnPortalClientSendThreadDealMsgTask(void* argv)
{
}

VOID fnDeletePortalClientSendThread(VOID)
{
	OSA_ThrDelete(&s_hPortalClientSendThread);
}

VOID fnCreatePortalClientSendThread(VOID)
{
}

INT fnSendMsg2PortClientSendThread(BYTE* pSrcDataBuf, UINT32 u32DataLen)
{
	return 0;
}
