#include "mod_time.h"
#include "events.h"
#include "osa_timer.h"

typedef enum tag_TIME_EVENT_TYPE {
	EVT_NONE  = 0,
	EVT_TIMER = 1,
	EVT_CLOCK = 2,

} TIME_EVENT_TYPE;

typedef struct tag_time_info {
	TIME_EVENT_TYPE type;
	timer_cb*		func;
	U8*				para;
	U32				timeout;
	U32				count; //??À´??Ê±????
} time_info;

#define MAX_TIMEEVENT_COUNT 40
static OSA_MutexHndl g_szMutex;
static int			 g_timer_id = INVALID_TIMER_ID;
time_info			 g_time_info[MAX_TIMEEVENT_COUNT];

time_flag  g_time_flag;
time_flag  g_time_flag_tmp;
static U64 g_aTimeUnit[TM_TIME_FLAG_NUM] = {1, 2, 5, 10, 100};
U64		   g_aTimeCnt[TM_TIME_FLAG_NUM];

void time_mod_timer(void)
{
	int i = -1;
	OSA_MutexLock(&g_szMutex);
	for (i = 0; i < TM_TIME_FLAG_NUM; i++) {
		g_aTimeCnt[i]++;
		if (g_aTimeCnt[i] >= g_aTimeUnit[i]) {
			g_aTimeCnt[i]			= 0;
			g_time_flag_tmp.byte[i] = TRUE;
		}
	}

	for (i = 0; i < MAX_TIMEEVENT_COUNT; i++) {
		if (g_time_info[i].type == EVT_TIMER) {
			g_time_info[i].count++;
			if (g_time_info[i].count >= g_time_info[i].timeout) {
				if (g_time_info[i].func != NULL)
					g_time_info[i].func(g_time_info[i].para);
				g_time_info[i].count = 0;
			}
		} else if (g_time_info[i].type == EVT_CLOCK) {
			g_time_info[i].count++;
			if (g_time_info[i].count >= g_time_info[i].timeout) {
				if (g_time_info[i].func != NULL)
					g_time_info[i].func(g_time_info[i].para);
				g_time_info[i].type	= EVT_NONE;
				g_time_info[i].timeout = 0;
				g_time_info[i].func	= NULL;
				g_time_info[i].para	= NULL;
				g_time_info[i].count   = 0;
			}
		}
	}
	OSA_MutexUnlock(&g_szMutex);
}

void time_init(void)
{
	if (g_timer_id == INVALID_TIMER_ID) {
		printf("timer init 1\n");
		memset(g_aTimeCnt, 0, sizeof(g_aTimeCnt));
		memset(&g_time_flag, 0, sizeof(g_time_flag));
		memset(&g_time_flag_tmp, 0, sizeof(g_time_flag_tmp));

		OSA_MutexCreate(&g_szMutex);
		OSA_InitTimer();
		// g_timer_id = OSA_SetTimer(1, eTimerContinued, (timer_expiry * )time_mod_timer, NULL);
	}
}

void time_unit(void)
{
	if (g_timer_id != INVALID_TIMER_ID) {
		SAFE_RELEASE_TIMER(g_timer_id);
		OSA_DeinitTimer();
		OSA_MutexDelete(&g_szMutex);
		g_timer_id = INVALID_TIMER_ID;
	}
}

void time_realtime(STREventElemType* buf, int size)
{
	OSA_MutexLock(&g_szMutex);
	memcpy(&g_time_flag.byte, &g_time_flag_tmp.byte, sizeof(g_time_flag_tmp.byte));
	memset(&g_time_flag_tmp.byte, 0, sizeof(g_time_flag_tmp));
	OSA_MutexUnlock(&g_szMutex);
}

void time_clear(void)
{
	memset(&g_time_flag.byte, 0, sizeof(g_time_flag.byte));
}

int time_regist_timer(U32 timeout, timer_cb* func, void* para)
{
	if (timeout <= 0)
		return -1;
	time_init();

	int i = -1;
	OSA_MutexLock(&g_szMutex);
	for (i = 0; i < MAX_TIMEEVENT_COUNT; i++) {
		if (g_time_info[i].type == EVT_NONE) {
			g_time_info[i].type	= EVT_TIMER;
			g_time_info[i].timeout = timeout;
			g_time_info[i].func	= func;
			g_time_info[i].para	= para;
			g_time_info[i].count   = 0;
			break;
		}
	}
	OSA_MutexUnlock(&g_szMutex);
	if (i >= MAX_TIMEEVENT_COUNT)
		i = -1;
	return i;
}

void time_kill_timer(int id)
{
	OSA_MutexLock(&g_szMutex);
	if (id >= 0 && id < MAX_TIMEEVENT_COUNT) {
		g_time_info[id].type	= EVT_NONE;
		g_time_info[id].timeout = 0;
		g_time_info[id].func	= NULL;
		g_time_info[id].para	= NULL;
		g_time_info[id].count   = 0;
	}
	OSA_MutexUnlock(&g_szMutex);
}

int time_regist_clock(U32 timeout, timer_cb* func, void* para)
{

	if (timeout <= 0)
		return -1;
	time_init();

	int i = -1;
	OSA_MutexLock(&g_szMutex);
	for (i = 0; i < MAX_TIMEEVENT_COUNT; i++) {
		if (g_time_info[i].type == EVT_NONE) {
			g_time_info[i].type	= EVT_CLOCK;
			g_time_info[i].timeout = timeout;
			g_time_info[i].func	= func;
			g_time_info[i].para	= para;
			g_time_info[i].count   = 0;
			break;
		}
	}
	OSA_MutexUnlock(&g_szMutex);
	if (i >= MAX_TIMEEVENT_COUNT)
		i = -1;
	return i;
}

void time_kill_clock(int id)
{
	OSA_MutexLock(&g_szMutex);
	if (id >= 0 && id < MAX_TIMEEVENT_COUNT) {
		g_time_info[id].type	= EVT_NONE;
		g_time_info[id].timeout = 0;
		g_time_info[id].func	= NULL;
		g_time_info[id].para	= NULL;
		g_time_info[id].count   = 0;
	}
	OSA_MutexUnlock(&g_szMutex);
}
