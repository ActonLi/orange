#ifndef _NOTIFY_UI_H_H_
#define _NOTIFY_UI_H_H_

typedef struct tagAPPItem
{
	BYTE u8Index;
	char pcName[64];
	BYTE u8PairStatus;
	BYTE u8ConversationAccess;
	BYTE u8SurveillanceAccess;
	BYTE u8SwitchLightAccess;
	BYTE u8OpenDoorAccess;
	BYTE u8ReviewHistoryAccess;
	BYTE u8DeleteHistoryAccess;
}APP_ITEM;

#define PORTAL_CLIENT_LOGIN_SUCCESS 0x01
#define PORTAL_CLIENT_LOGIN_FAIL 0x02

#define PORTAL_CLIENT_LOGOUT_SUCCESS 0x01
#define PORTAL_CLIENT_LOGOUT_FAIL 0x02

#define PORTAL_CLIENT_PAIR_SUCCESS 0x01
#define PORTAL_CLIENT_PAIR_FAIL 0x02


typedef enum
{
	PORTAL_CLIENT_STATUS_LOGIN = 0x01,
	PORTAL_CLIENT_STATUS_LOGOUT = 0x02,	
}ePORTAL_CLIENT_LOGIN_STATUS;

typedef enum
{
	PORTAL_CLIENT_STATUS_CONNECT_FAIL = 0x01,
	PORTAL_CLIENT_STATUS_CONNECT_SUCCESS = 0x02,
}ePORTAL_CLIENT_CONNECT_STATUS;


typedef enum
{
	PORTAL_CLIENT_PUSH = 0x01,   //主动上报
	PORTAL_CLIENT_REPLY = 0x02,  //应答UI
}ePUSH_REPLY_FLAG;


extern VOID NotifyUIWelcomeAppList(ePUSH_REPLY_FLAG eFlag);
extern VOID NotifyUILoginResult(BYTE u8Result);
extern VOID NotifyUILogoutResult(BYTE u8Result);
extern VOID NotifyUIPortalClientStatus(ePORTAL_CLIENT_LOGIN_STATUS eLoginStatus,ePORTAL_CLIENT_LOGIN_STATUS eLogoutStatus);
extern VOID ReplyUIPortalClientStatus(ePORTAL_CLIENT_LOGIN_STATUS eLoginStatus,ePORTAL_CLIENT_LOGIN_STATUS eLogoutStatus);
extern VOID ReplyUIPortalClientAccessControl(BYTE * pDataBuf);
extern VOID ReplyUIPortalClientDeleteResult(BYTE u8Index, BYTE u8Ret);
extern VOID ReplyUIPortalClientPairResult(BYTE u8Index, BYTE u8Ret, BYTE * pAccessData);

#endif
