#include "module.h"
#include "events.h"
#include "orangelog.h"
#include "osa_tick.h"
#include "osa_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

static int				  global_md_cnt = 0;
static struct module_info global_modules[MODULE_MAX_SIZE];

static struct module_info* find_module(const char* name)
{
	int i = 0;
	for (; i < MODULE_MAX_SIZE; i++) {
		if (global_modules[i].name && strcasecmp(global_modules[i].name, name) == 0)
			return &global_modules[i];
	}
	return NULL;
}

static void insert_module_info(struct module_info modules[MODULE_MAX_SIZE], struct module_info* info)
{
	int idx = 0;
	for (; idx < global_md_cnt; idx++) {
		if (info->priority > modules[idx].priority)
			break;
	}
	int move = global_md_cnt;
	for (; move > idx; move--) {
		memcpy(modules + move, modules + move - 1, sizeof(struct module_info));
	}
	memcpy(modules + idx, info, sizeof(struct module_info));

	orangelog_log_info("%s register successful.\n", info->name);
	global_md_cnt++;
#if 0
	int i = 0;
	for ( ; i < global_md_cnt; i++)
	{
		printf("module-%d name:%s", i+1, modules[i].name);
	}
#endif
}

void module_register(const struct module_info* info)
{
	if (!info || !info->name)
		return;

	DEBUGP("%s:%d info->name: %s, priority: %c, thread: %d\n", __func__, __LINE__, info->name, info->priority, info->thread);
	if (find_module(info->name))
		return;
	if (global_md_cnt >= MODULE_MAX_SIZE)
		return;
	int i = 0;
	for (; i < MODULE_MAX_SIZE; i++) {
		if (global_modules[i].name)
			continue;
		insert_module_info(global_modules, (struct module_info*) (info));
		break;
	}
}
void module_unregister(const struct module_info* info)
{
	if (!info || !info->name)
		return;
	struct module_info* mod = find_module(info->name);
	if (!mod)
		return;
	memset(mod, 0, sizeof(struct module_info));
	global_md_cnt--;
}

int modules_get(struct module_info** modules, int* capability)
{
	if (!modules || !capability)
		return -1;
	*modules	= global_modules;
	*capability = global_md_cnt;
	return 0;
}
U32  counter = 0;
void modules_realtime(U32 semid)
{
	int i = 0, j = 0;
	U32 size = 0;
	U64 curtime, ttime;
	int module_cnt						   = 0;
	U32 minperiodms						   = 0xffffffff, threadperiod;
	int thread_module_cnt				   = 0;
	BOOL __attribute__((unused)) runmodule = FALSE;
	BOOL			  changeperoid		   = FALSE;
	STREventElemType* elems				   = NULL;
	event_wait(semid);
	struct module_info* modules = 0;
	modules_get(&modules, &module_cnt);
	event_get(semid, &elems, &size);
	DEBUGP("%s:%d size: %d, module_cnt: %d\n", __func__, __LINE__, size, module_cnt);
	////printf("module cnt=%d, counter=%d, size=%d\n", module_cnt, counter,
	/// size);
	counter++;
	curtime = OSA_TimeGetTimeValEx();
	counter++;
	if (size == 0) {
		runmodule		  = FALSE;
		thread_module_cnt = 0;
		for (j = 0; j < module_cnt; j++) {
			DEBUGP("%s:%d thread: %d, semid: %d\n", __func__, __LINE__, modules[j].thread, semid);
			if (modules[j].thread == semid) {
				if (modules[j].getperiod == NULL) {
					runmodule = TRUE;
				} else {
					ttime		 = curtime - modules[j].curtime;
					threadperiod = modules[j].getperiod();
					if (ttime >= threadperiod) {
						modules[j].curtime = ttime;
						runmodule		   = TRUE;
					}
				}
				if (NULL != modules[j].setlocaltime) {
					modules[j].setlocaltime(curtime);
				}
				DEBUGP("%s:%d modules name: %s\n", __func__, __LINE__, modules[j].name);
				modules[j].realtime(semid, thread_module_cnt, NULL);
				thread_module_cnt++;
			}
		}
		event_clear(semid, thread_module_cnt);
	} else {
		for (i = 0; i < size; i++) {
            thread_module_cnt = 0;
			for (j = 0; j < module_cnt; j++) {
				if (modules[j].thread == semid) {
					if (elems[i].flags & (1 << thread_module_cnt)) {
						if (NULL != modules[j].setlocaltime) {
							modules[j].setlocaltime(curtime);
						}
						DEBUGP("%s:%d modules name: %s\n", __func__, __LINE__, modules[j].name);
						modules[j].realtime(semid, thread_module_cnt, &elems[i]);
					}
					thread_module_cnt++;
				}
			}
		}
		event_clear(semid, thread_module_cnt);
		if (elems != NULL) {
			free(elems);
			elems = NULL;
		}
	}
	changeperoid = FALSE;
	minperiodms  = 0xffffffff;
	/// DEBUGP("module=%d\n", module_cnt);
	for (j = 0; j < module_cnt; j++) {
		if (modules[j].thread == semid) {
			if (0 != modules[j].getperiod()) {
				threadperiod = modules[j].getperiod();
				////printf("module=%d,period=%d\n", j, threadperiod);
				if ((minperiodms >= threadperiod) && (threadperiod > 0)) {
					changeperoid = TRUE;
					minperiodms  = threadperiod;
				}
			}
		}
	}
	if (TRUE == changeperoid) {
		OSA_ChangePeriod(semid, minperiodms);
	}

	return;
}
void modules_init(U32 semid)
{
	int					module_cnt = 0;
	int					i;
	struct module_info* modules = 0;

	modules_get(&modules, &module_cnt);
	DEBUGP("init module zzy\n");
	for (i = 0; i < module_cnt; i++) {
		if (modules[i].thread == semid) {
			modules[i].init();
			DEBUGP("=========init module name: %s, semid=%d =========\n", modules[i].name, modules[i].thread);
		}
	}
}
