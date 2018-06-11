#ifndef _H_OSATICK_
#define _H_OSATICK_
#include "osa_mutex.h"
#include "osa_sem.h"
#include "osa_timer.h"

//#define MAX_TIMER_NUM 30
#define OSA_TICK_TYPE_TIMER 0
#define OSA_TICK_TYPE_CLOCK 1

struct tick {
	int	id;		 /**< timer id		*/
	UINT32 periodms; // ???ٸ?10ms
	UINT32 elapse;
	UINT32 times; // continue or once
	UINT8  used;
	UINT64 curtime;
	UINT32 threadid;
};

struct tick_list {
	struct tick   list[MAX_TIMER_NUM];
	OSA_SemHndl   sem;
	OSA_MutexHndl Mutex;
	UINT32		  periodms;
};

extern void OSA_InitTick(UINT32 periodms);
extern int OSA_SetTick(UINT32 periodms, UINT32 times, UINT32 threadid);
extern BOOL OSA_ChangePeriod(UINT32 semid, UINT32 periodms);

#endif
