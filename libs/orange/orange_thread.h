#pragma once
#include "orange.h"

#define ORANGE_THR_PRI_NONE 0

#define ORANGE_THR_PRI_FIFO_MAX sched_get_priority_max(SCHED_FIFO)
#define ORANGE_THR_PRI_FIFO_MIN sched_get_priority_min(SCHED_FIFO)
#define ORANGE_THR_PRI_RR_MAX sched_get_priority_max(SCHED_RR)
#define ORANGE_THR_PRI_RR_MIN sched_get_priority_min(SCHED_RR)

#define ORANGE_THR_STACK_SIZE_DEFAULT 0

#define ORANGE_THR_RETURN_SUCCESS NULL
#define ORANGE_THR_RETURN_FAIL (void*) (-1)

#define ORANGE_THREAD_HANDLE_INVLALID ((pthread_t) 0L)

typedef void* (*orange_thread_entry_func)(void*);

typedef pthread_t orange_thread_handle_t;

typedef struct orange_cond_event_handle {
	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	uint8_t			initialized;
	uint8_t			spurious_wakeup;
} orange_cond_event_handle_t;

extern void orange_thread_sleep(uint32_t sleep_ms);

extern int orange_thread_create(orange_thread_handle_t* hndl, orange_thread_entry_func entryFunc, int pri, void* prm);

extern int orange_thread_create_ex(orange_thread_handle_t* hndl, orange_thread_entry_func entryFunc, int pri, void* prm, char* pFunc, int line);

extern int orange_thread_delete(orange_thread_handle_t* hndl);

extern int orange_thread_join(orange_thread_handle_t* hndl);

extern int orange_thread_join_ex(orange_thread_handle_t* hndl, char* pFunc, int line);

extern int orange_thread_change_pri(orange_thread_handle_t* hndl, uint32_t pri);

extern int orange_thread_exit(void* return_value);

extern uint8_t orange_ThrIsValid(orange_thread_handle_t* hndl);

extern int orange_ThrDetach(orange_thread_handle_t* hndl);

extern int orange_GetStackSize(void);

extern uint8_t orange_SetThrStackSize(uint32_t uStackSize);

extern uint8_t orange_GetSelfThrHandle(orange_thread_handle_t* pSelfThrHndl);

extern uint8_t orange_SetThrPri(orange_thread_handle_t* pThr, int iNewPri);

extern int orange_GetThrPri(orange_thread_handle_t* pThr);

extern uint8_t orange_ThrCompare(orange_thread_handle_t* pThr1, orange_thread_handle_t* pThr2);

extern int orange_ThreadCreate(orange_thread_handle_t* hndl, orange_thread_entry_func ThreadFunc, void* pParamter);

extern int orange_ThreadCreateEx(orange_thread_handle_t* hndl, orange_thread_entry_func ThreadFunc, void* pParamter, int i32Pri, uint8_t bSystemScope);

extern int orange_CondEventInit(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE);

extern int orange_CondEventDeInit(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE);

extern int orange_SetCondEvent(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE);

extern int orange_WaitCondEvent(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE, int iTimeOutMilliSec);

extern uint8_t orange_IsCondEventTrig(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE);

extern void orange_Sleep(uint32_t u32SleepMs);
