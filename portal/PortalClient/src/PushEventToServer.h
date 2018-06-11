#ifndef _PUSH_EVENT_TO_PORTAL_SERVER_H_H__
#define _PUSH_EVENT_TO_PORTAL_SERVER_H_H__

#include <stdio.h>
#include <stdlib.h>
#include "json/cJSON.h"


typedef struct tagHistoryEvent
{
	struct tagHistoryEvent *prev;
	struct tagHistoryEvent *next;

	int  order;
	char szUuid[64];
	char szTimeStamp[32]; 
	char szType[64];
	char szBelongto[64];
	char szPayload[256];
}HistoryEvent;


typedef struct tagEventNode
{
	struct tagEventNode *prev;
	struct tagEventNode *next;

	int  order;
	char szUuid[64]; //128 uuid +4 identify a event  //The portal client must generate an id for every event. The id must be an UUID 
	char szTimeStamp[32];  //ISO8601 format e.g 2008-02-01T09:00:22+05:00"   //The portal client must supply a local timestamp for the event. The timestamp must be compatible to the ISO 8601 format
	char szType[64];	//The portal client must supply the type of the event. See document "004-Events-Types.pdf" for a list of available events.	
	char szBelongto[64];  //The portal client can supply an optional reference to another event within Belongs-to
	char szDestination[64];
	char *szPayload;
	int iPayloadLen;
	int expire;
}EventNode;

typedef struct tagEventNode1
{
	struct tagEventNode *prev;
	struct tagEventNode *next;

	cJSON *event;
	int expire;
}EventNode1;

enum tagEventType
{
	eEventRing = 0,
	eEventAnswered,
	eEventTerminated,
	eEventMissCall,
	eEventSurvellance,
	eEventOpenDoor,
	eEventSwitchLight,
	eEventSnapshot,
	eEventDeviceInfo,
	eEventPing,
	eEventACLupload,
	eEventDefault,
};

enum
{
	eACL_SNDSUCC,
	eACL_SNDFAIL,
};

typedef struct tagSnapshot
{
	char *imgbuf;
	int imglength;
}Snapshot;

typedef struct tagInternalEventInfo
{
	int  type;
	char sessionID[32];
	char localid[16];
	char remoteid[128];
	char iostime[32];
	Snapshot snapshot;
	char user_section[16];   //acl 使用
	int  pwflag;  			 //acl 使用
}InternalEventInfo;

#define EVENT_TYPE_ECHO "com.abb.ispf.event.echo"
#define EVENT_TYPE_SUCCESS "com.abb.ispf.event.success"
#define EVENT_TYPE_ERROR "com.abb.ispf.event.error"
#define EVENT_TYPE_DEVICE_INFO "com.abb.ispf.event.deviceinfo"
#define EVENT_TYPE_DISCOVERY "com.abb.ispf.event.discovery"

#define EVENT_TYPE_DELETE "com.abb.ispf.event.delete"
#define EVENT_TYPE_CLIENT_REMOVE "com.abb.ispf.event.client-remove"
#define EVENT_TYPE_WELCOME_CONNECT	"com.abb.ispf.event.welcome.connect"

#define EVENT_TYPE_ACL_REQUEST "com.abb.ispf.event.welcome.acl-request"
#define EVENT_TYPE_ACL_UPLOAD  "com.abb.ispf.event.welcome.acl-upload"
#define EVENT_TYPE_ACL_FETCH  "com.abb.ispf.event.welcome.acl-fetch"
#define EVENT_TYPE_CRL		  "com.abb.ispf.event.crl"

#define EVENT_TYPE_SURVEILLANCE "com.abb.ispf.event.welcome.call-surveillance"
#define EVENT_TYPE_RING   "com.abb.ispf.event.welcome.ring"
#define EVENT_TYPE_MISSED_CALL  "com.abb.ispf.event.welcome.call-missed"
#define EVENT_TYPE_ANSWERED   "com.abb.ispf.event.welcome.call-answered"
#define EVENT_TYPE_TERMINATED   "com.abb.ispf.event.welcome.call-terminated"
#define EVENT_TYPE_SCREENSHOT "com.abb.ispf.event.welcome.screenshot"
#define EVENT_TYPE_SWITCH_LIGHT	"com.abb.ispf.event.welcome.light"
#define EVENT_TYPE_DOOR_OPEN   	"com.abb.ispf.event.welcome.door-open"
#define EVENT_TYPE_ACL_REVOKE "com.abb.ispf.event.welcome.acl-revoke"

#define EVENT_PORTAL_DELETE_DEVICE "client deleted/blocked or revoked its certificate"

#define MAX_CACHE_EVENT_NUMBER 110
#define WAIT_EVENT_TIMEOUT 50


extern VOID PushEventToPortalServer(BYTE * msg_buf,UINT32 len);
extern char* GetEventUUID(VOID);
extern VOID SendEventToPortalServer(VOID *param);

#endif
