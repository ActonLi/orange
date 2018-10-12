#include "mod_thread.h"
#include "db.h"
#include "message.h"
#include "module.h"
#include "osa_thr.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

STRThreadPoolRegsTypes threadpool_regs;

static void* thread_run(void* arg)
{
	int			pos = (int) arg;
	OSA_ThrHndl pSelfThrHndl;

	if (TRUE == OSA_GetSelfThrHandle(&pSelfThrHndl)) {
		OSA_ThrDetach(&pSelfThrHndl);
	}

	if (pos >= 0 && pos < THREADPOOL_THREAD_NUM) {
		threadpool_regs.pools[pos].run(&threadpool_regs.pools[pos].args, pos, threadpool_regs.pools[pos].semid);
		threadpool_regs.pools[pos].used = FALSE;
		if (threadpool_regs.pools[pos].args.params != NULL) {
			free(threadpool_regs.pools[pos].args.params);
			threadpool_regs.pools[pos].args.params = NULL;
		}
	}

	OSA_ThrExit(NULL);
	return NULL;
}

int thread_request(EnumEventType eventtype, void* params, U32 size, U32 timeout, void* func)
{
	int pos = -1;
	int res = -1;
	int i;
	DEBUGP("%s:%d\n", __func__, __LINE__);
	for (i = 0; i < THREADPOOL_THREAD_NUM; i++) {
		pthread_mutex_lock(&threadpool_regs.pools[i].lock);
		DEBUGP("%s:%d, i=%d\n", __func__, __LINE__, i);
		if (threadpool_regs.pools[i].used == FALSE) {
			threadpool_regs.pools[i].args.eventtype = eventtype;
			if (threadpool_regs.pools[i].args.params != NULL) {
				free(threadpool_regs.pools[i].args.params);
				threadpool_regs.pools[i].args.params = NULL;
			}
			threadpool_regs.pools[i].args.params = (void*) malloc(size);
			memcpy(threadpool_regs.pools[i].args.params, params, size);
			threadpool_regs.pools[i].args.size = size;

			threadpool_regs.pools[i].semid = SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_THREAD, i);
			threadpool_regs.pools[i].run   = func;
			event_register(threadpool_regs.pools[i].semid);
			pos = i;
			if (OSA_ThrCreate((OSA_ThrHndl*) &(threadpool_regs.pools[i].thread), thread_run, OSA_THR_PRI_RR_MIN, (void*) pos)) {
			} else {
			}
			threadpool_regs.pools[i].used = TRUE;
		}
		pthread_mutex_unlock(&threadpool_regs.pools[i].lock);
		if (pos >= 0) {
			break;
		}
	}
	DEBUGP("%s:%d, i=%d\n", __func__, __LINE__, i);
	res = pos;
	return (res);
}

static void thread_init(void)
{
	int i;
	printf("%s:%d begin\n", __func__, __LINE__);

	for (i = 0; i < THREADPOOL_THREAD_NUM; i++) {
		threadpool_regs.pools[i].used		 = FALSE;
		threadpool_regs.pools[i].args.params = NULL;
	}
	OSA_MutexCreate((OSA_MutexHndl*) &threadpool_regs.muxlock);
}

static void thread_uninit(void)
{
}

static void thread_realtime(U32 semid, U32 moduleid, STREventElemType* event)
{
	int i;
	// BOOL res;
	if (NULL != event) {
		//	res = TRUE;
		for (i = 0; i < THREADPOOL_THREAD_NUM; i++) {
			pthread_mutex_lock(&threadpool_regs.pools[i].lock);
			if (TRUE == threadpool_regs.pools[i].used) {
				event_post(threadpool_regs.pools[i].semid, "localsocket", event->type, event->params, event->size, (50));
			}
			pthread_mutex_unlock(&threadpool_regs.pools[i].lock);
		}
		event_clearflags(semid, event, moduleid);
	} else {
	}

	return;
}

static void thread_clear(void)
{
	return;
}

static void thread_setlocaltime(U64 time)
{
	return;
}

static U32 thread_getperiod(void)
{
	return (0);
}

MODULE_INFO(MD_THREAD_NAME, MD_THREAD_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0), thread_init, thread_realtime, thread_clear,
			thread_uninit, thread_getperiod, thread_setlocaltime);
