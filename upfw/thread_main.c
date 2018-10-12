#include "thread_main.h"
#include "events.h"
#include "mod_socket.h"
#include "module.h"
#include "osa_tick.h"
//#include "mod_wifi.h"
#include "remotesocket.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

pthread_t thread_main_thread;
BOOL	  thread_main_run_flag;

void thread_main_init(void)
{
	thread_main_run_flag = TRUE;
	event_register(SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0));
	OSA_SetTick(1000, 0, SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0));
	if (pthread_create(&thread_main_thread, NULL, thread_main_fun, NULL)) {
		DEBUGP("main pthread_create failed\n");
	} else {
		DEBUGP("main pthread_create succeed\n");
	}
}
void* thread_main_fun(void* thread)
{
	U32 semid = SEM_GET_ID(SEM_ID_PROCESS_UPFW, SEM_ID_THREAD_MAIN, 0);
	modules_init(semid);
	while (thread_main_run_flag) {
		modules_realtime(semid);
	}

	return NULL;
}
