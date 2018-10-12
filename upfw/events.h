#ifndef _EVENTS_H_
#define _EVENTS_H_
#include "cJSON.h"
#include "define.h"
#include "message.h"
#define SEM_ID_PROCESS_UPFW 0x01

#define SEM_ID_THREAD_MAIN 0x01
#define SEM_ID_THREAD_LOCALSOCKET 0x02
#define SEM_ID_THREAD_THREAD 0x03
#define SEM_ID_THREAD_TICK 0x04

#define SEM_MAX_COUNT 20
#define SEM_INIT_COUNT 0

#define SEM_GET_ID(processid, threadid, moduleid) ((processid << 24) | (threadid << 16) | (moduleid))

//#define SEM_GET_ID(processid,threadid,moduleid) 123

typedef enum {
	EVENT_TYPE_TIMEOUT		= 0,
	EVENT_TYPE_TICK			= 1,
	EVENT_TYPE_LOCALSOCKET  = 2,
	EVENT_TYPE_REMOTESOCKET = 3,
	EVENT_TYPE_INNERCMD		= 4,
	EVENT_TYPE_UART			= 5,
} EnumEventType;
typedef struct {
	U32   msgid;
	void* msg;
} STREventTypeMsgType;

typedef struct tag_STREventElemType {
	U32			  semid;
	EnumEventType type;
	char		  module[64];
	void*		  params;
	U32			  size;
	U32			  flags;
	U64			  occurtime;
	U64			  expiredtime;
	struct tag_STREventElemType* this;
	struct tag_STREventElemType* pre;
	struct tag_STREventElemType* next;

} STREventElemType;

extern void event_init(void);
extern void event_uninit(void);
extern void event_wait(U32 semid);

extern void event_register(U32 semid);
extern void event_post(U32 semid, char* module, EnumEventType eventtype, void* param, U32 size, U32 timeout);
extern void event_realtime(U32 semid);
extern void event_clear(U32 semid, U32 modulecnt);
extern void event_clearflags(U32 semid, STREventElemType* event, U32 moduleid);

extern void event_get(U32 semid, STREventElemType** elem, U32* size);
extern void event_timedwait(U32 semid, U32 timeoutms);
extern void event_clearall(U32 semid);

extern void event_signal(U32 semid);
extern BOOL event_get_jsonmsg(void* params, MSG_DATA_HEADER* header, cJSON** json, U32* msgid);

#endif
