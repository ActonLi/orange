#ifndef _H_THREAD_
#define _H_THREAD_
#include "define.h"
#include "events.h"
#include "osa_mutex.h"
#include "osa_sem.h" //?????Í±?À£
#define THREADPOOL_THREAD_NUM 10
#define MODULE_NAME_SIZE 256
typedef struct {
	EnumEventType eventtype;
	void*		  params;
	U32			  size;
} STRThreadParamType;
typedef struct {
	pthread_t		thread;
	pthread_mutex_t lock;
	void (*run)(STRThreadParamType*, U32, U32);
	U8				   used;
	U32				   timeout;
	char			   modulename[MODULE_NAME_SIZE];
	U32				   semid;
	STRThreadParamType args;
} STRThreadPoolNodeType;
typedef struct {
	pthread_mutex_t		  muxlock;
	STRThreadPoolNodeType pools[THREADPOOL_THREAD_NUM];
	U32					  flags;
} STRThreadPoolRegsTypes;
extern STRThreadPoolRegsTypes threadpool_regs;

extern int thread_request(EnumEventType eventtype, void* params, U32 size, U32 timeout, void* func);

#endif
