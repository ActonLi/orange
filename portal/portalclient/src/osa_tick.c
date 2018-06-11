#include "osa_tick.h"
#include "events.h"
#include "osa_thr.h"
#include "osa_time.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

LOCAL OSA_ThrHndl	  s_thrTickSignfn;
LOCAL struct tick_list s_stTickList;
LOCAL BOOLEAN bTickFuncRunning = TRUE;

void OSA_InitTick(UINT32 periodms);

int OSA_SetTick(UINT32 periodms, UINT32 times, UINT32 threadid);
BOOL OSA_ChangePeriod(UINT32 semid, UINT32 periodms);
static void OSA_TickSigFun(INT32 signo);

int OSA_SetTick(UINT32 periodms, UINT32 times, UINT32 threadid)
{
	UINT32 i;
	OSA_MutexLock(&s_stTickList.Mutex);
	for (i = 0; i < MAX_TIMER_NUM; i++) {
		if (FALSE == s_stTickList.list[i].used) {
			s_stTickList.list[i].periodms = periodms;
			s_stTickList.list[i].threadid = threadid;
			s_stTickList.list[i].times	= times;
			s_stTickList.list[i].curtime  = OSA_TimeGetTimeValEx();
			s_stTickList.list[i].used	 = TRUE;
			DEBUGP("set tick period=%d,index=%d\n", periodms, i);
			OSA_SemSignal(&s_stTickList.sem, 1);
			break;
		}
	}
	OSA_MutexUnlock(&s_stTickList.Mutex);
	return 0;
}

static UINT32 OSA_GetTickPeriod(void)
{
	int i;
	U32 res;
	res = s_stTickList.periodms;
	for (i = 0; i < MAX_TIMER_NUM; i++) {
		if (TRUE == s_stTickList.list[i].used) {
			if (res > s_stTickList.list[i].periodms) {
				res = s_stTickList.list[i].periodms;
			}
		}
	}
	return (res);
}
BOOL OSA_ChangePeriod(UINT32 semid, UINT32 periodms)
{
	// BOOL res = FALSE;
	int i;
	OSA_MutexLock(&s_stTickList.Mutex);
	for (i = 0; i < MAX_TIMER_NUM; i++) {
		if (s_stTickList.list[i].used) {
			if (semid == s_stTickList.list[i].threadid) {
				if (s_stTickList.list[i].periodms != periodms) {
					s_stTickList.list[i].periodms = periodms;
					OSA_SemSignal(&s_stTickList.sem, 1);
				}
				break;
			}
		}
	}
	OSA_MutexUnlock(&s_stTickList.Mutex);

	return TRUE;
}

void OSA_InitTick(UINT32 periodms)
{
	// BOOL iRet = FALSE;
	memset(&s_stTickList, 0, sizeof(struct tick_list));
	s_stTickList.periodms = periodms;
	OSA_MutexCreate(&s_stTickList.Mutex);
	event_register(SEM_GET_ID(SEM_ID_PROCESS_BACKEND, SEM_ID_THREAD_TICK, 0));
	if (TRUE == OSA_SemCreate(&s_stTickList.sem, 1, 0)) {
		bTickFuncRunning = TRUE;
		if (OSA_SOK == OSA_ThreadCreate(&s_thrTickSignfn, (VOID*) OSA_TickSigFun, NULL)) {
			// iRet = TRUE;
		}
	}
	return;
}
static void OSA_TickSigFun(INT32 signo)
{
	INT32		 i;
	struct tick* pNode = NULL;
	UINT32		 periodms;
	UINT64		 curtime;
	UINT64		 elapsed;
	DEBUGP("OSA_TICK thread\n");
	while (bTickFuncRunning) {
		OSA_MutexLock(&s_stTickList.Mutex);
		periodms = OSA_GetTickPeriod();
		OSA_MutexUnlock(&s_stTickList.Mutex);

		// DEBUGP("OSA_TICK thread, period=%d\n", periodms);
		OSA_SemTimedWait(&s_stTickList.sem, periodms);
		////printf("OSA_TICK thread, timerout,period=%d\n", periodms);
		OSA_MutexLock(&s_stTickList.Mutex);
		for (i = 0; i < MAX_TIMER_NUM; i++) {
			pNode = &s_stTickList.list[i];
			if (TRUE == pNode->used) {
				curtime = OSA_TimeGetTimeValEx();
				elapsed = curtime - pNode->curtime;
				if (elapsed >= (U64) pNode->periodms) {
					pNode->curtime = curtime;
					event_signal(pNode->threadid);
					if (pNode->times == 0) {
					} else {
						pNode->times--;
						if (0 == pNode->times) {
							pNode->elapse = 0;
							pNode->used   = FALSE;
						}
					}
				}
			}
		}
		OSA_MutexUnlock(&s_stTickList.Mutex);
	}
}
