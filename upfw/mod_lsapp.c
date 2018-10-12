#include "mod_lsapp.h"
#include "events.h"
#include "module.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

static void lsapp_init(void)
{
	return;
}

static void lsapp_uninit(void)
{
	return;
}

static void lsapp_realtime(U32 semid, U32 moduleid, STREventElemType* event)
{
	if (NULL != event) {
		event_clearflags(semid, event, moduleid);
	}
	DEBUGP("%s:%d buf: %p, semid: %u, moduleid: %u\n", __func__, __LINE__, buf, semid, moduleid);
	return;
}

static void lsapp_clear(void)
{
	return;
}

static void lsapp_setlocaltime(U64 time)
{
	return;
}
static U32 lsapp_getperiod(void)
{
	return (0);
}

MODULE_INFO(MD_LSAPP_NAME, MD_LSAPP_PRIORITY, SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0), lsapp_init, lsapp_realtime, lsapp_clear, lsapp_uninit,
			lsapp_getperiod, lsapp_setlocaltime);
