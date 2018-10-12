#ifndef _H_MSGDISPATCH_
#define _H_MSGDISPATCH_
#include "define.h"
#include "linkedlists.h"
#include "message.h"
#define MSGDISPATCH_MATCHTYPE_JSON 0
#define MSGDISPATCH_MATCHTYPE_BINARY 1
typedef struct {
	U32   id;
	void* method;
	BYTE  matchtype;
	void (*function)(void* msg, U32 msgid);
} MSGDISPATCH_REQ_TYPE;

extern int msgdispatch_register(MSGDISPATCH_REQ_TYPE* req, int count);
extern int msgdispatch_unregister(MSGDISPATCH_REQ_TYPE* req, int count);
extern int msgdispatch_process(void* msg, int length, U32 msgid);
#endif
