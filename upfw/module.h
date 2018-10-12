/*
 * Filename:      	module
 * Created at:    	2017.08.03
 * Description:   	module controller.
 * Author:        	mjx
 * Copyright (C)  	ABB(Genway)
 * ****************************************************************/
#ifndef _MODULE_H_
#define _MODULE_H_
#include "define.h"
#include "events.h"

#define MD_SOCKET_NAME "socket"
#define MD_THREAD_NAME "thread"
#define MD_UPFM_NAME "upfw"
#define MD_LSAPP_NAME "lsapp"
enum MD_PRIORITY { MD_SOCKET_PRIORITY, MD_UPFM_PRIORITY, MD_THREAD_PRIORITY, MD_LSAPP_PRIORITY };

#define MODULE_MAX_SIZE 32
#define MODULE_NAME_MAX_SIZE 64

typedef struct module_info {
	const char*   name;
	unsigned char priority;
	U32			  thread; //???????
	U64			  curtime;
	void (*init)(void);
	void (*realtime)(U32, U32, STREventElemType*);
	void (*clear)(void);
	void (*uninit)(void);
	U32 (*getperiod)(void);
	void (*setlocaltime)(U64);
} module_info_t;

extern void module_register(const struct module_info* info);
extern void module_unregister(const struct module_info* info);

#define MODULE_INFO(name, load_pri, thread, init_func, realtime_func, clear_func, uninit_func, getperiod_func, setlocaltime_func)                              \
	static struct module_info __mod_info = {                                                                                                                   \
		name, load_pri, thread, 0, init_func, realtime_func, clear_func, uninit_func, getperiod_func, setlocaltime_func,                                       \
	};                                                                                                                                                         \
	static void __attribute__((constructor)) __reg_module(void)                                                                                                \
	{                                                                                                                                                          \
		module_register(&__mod_info);                                                                                                                          \
	}                                                                                                                                                          \
	static void __attribute__((destructor)) __unreg_module(void)                                                                                               \
	{                                                                                                                                                          \
		module_unregister(&__mod_info);                                                                                                                        \
	}

extern int modules_get(struct module_info** modules, int* capability);
extern void modules_realtime(U32 semid);
extern void modules_init(U32 semid);
#endif
