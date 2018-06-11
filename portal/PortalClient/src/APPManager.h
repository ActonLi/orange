#ifndef APP_MANAGER_H_H__
#define APP_MANAGER_H_H__

#include "CSR.h"

#define MAX_PAIRING_USER 4   //�����Ե��ֻ�����


#define PATH_WELCOME_APP_ROOT  PATH_CERT_ROOT"WelcomeAPP/"
#define PATH_WELCOME_APP_PAIR_REQUEST PATH_WELCOME_APP_ROOT"WelAppPairRequest.ini"
#define PATH_FINGERPRINT PATH_WELCOME_APP_ROOT"FingerPrint"


#define WELCOME_APP_PAIR_REQUEST_KEY_CERTIFICATE "certificate"
#define WELCOME_APP_PAIR_REQUEST_KEY_UUID "uuid"
#define WELCOME_APP_PAIR_REQUEST_KEY_NAME "name"
#define WELCOME_APP_PAIR_REQUEST_KEY_STATE "state"
#define WELCOME_APP_PAIR_REQUEST_KEY_SERIAL "serial"

#define WELCOME_APP_PAIR_REQUEST_KEY_PASSWORD "password"
#define WELCOME_APP_PAIR_REQUEST_KEY_SIP_NAME "sipname"

#define WELCOME_APP_PAIR_STATE_PAIRED "paired"
#define WELCOME_APP_PAIR_STATE_UNPAIRED "unpaired"

#define WELCOME_APP_RECORD_SECTION_PREFIX "user_"


#define	KEY_DEVICE_NAME "name"
#define	KEY_DEVICE_TYPE "type"
#define	KEY_DEVICE_ADDRESS "address"
#define	KEY_DEVICE_calltype "calltype"

#define WELCOME_APP_USERNAME 						"user"
#define WELCOME_APP_PRIVILEGE_CONVERSATION_KEY 		"conversation"   		//ͨ��
#define WELCOME_APP_PRIVILEGE_SURVEILLANCE_KEY 		"surveillance"   		//����
#define WELCOME_APP_PRIVILEGE_OPENDOOR_KEY 			"opendoor"           	//����
#define WELCOME_APP_PRIVILEGE_SWITCHLIGHT_KEY 		"switchlight"     		//����
#define WELCOME_APP_PRIVILEGE_ACCESSHISTORY_KEY 	"accesshistory"   		//�鿴ͨ����¼
#define WELCOME_APP_PRIVILEGE_DELETEHISTORY_KEY 	"deletehistory"	 		//ɾ��ͨ����¼
#define WELCOME_APP_PRIVILEGE_SCREENSHOT_KEY 		"screenshot"  			//ץ��

#define WELCOME_APP_PRIVILEGE_ENABLE 				"yes"
#define WELCOME_APP_PRIVILEGE_DISABLE 				"no"


typedef struct tagUserPri
{
	char id[128];
	char surveillance[4]; 	//����Ȩ��  yes/no
	char ring[4];        	//ͨ��Ȩ��  yes/no
	char screenshot[4]; 	//ץ��Ȩ��  yes/no  �� Ĭ�϶���yes
	char opendoor[4];		//����Ȩ��  yes/no
	char switchlight[4];	//����Ȩ��  yes/no
	char precamera[4];		
	char nextcamera[4];
}UserPri;


extern VOID HandleWelcomeAppPairRequest(char * pAppPairPayLoad);
extern INT ActivatePairingAPP(char * pWelcomeAPPSection, BYTE * pInterityCode);
extern VOID SaveAPPAccessProperty(BYTE * pu8AccessControl);

#endif

