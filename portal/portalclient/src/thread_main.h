#ifndef _H_THREAD_MAIN_
#define _H_THREAD_MAIN_
#include "define.h"
// gdb test.exe test.exe.core, bt, where
enum THREAD_MAIN_MODULE_ID {
	THREAD_MAIN_MODULE_ID_SOCKET = 0,
	THREAD_MAIN_MODULE_ID_WIFI,
	THREAD_MAIN_MODULE_ID_NUM,
};
extern void* thread_main_fun(void*);
extern void  thread_main_init(void);
#endif
