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

extern int orange_thread_delete(orange_thread_handle_t* hndl);

extern int orange_thread_join(orange_thread_handle_t* hndl);

extern int orange_thread_join_ex(orange_thread_handle_t* hndl, char* p_func, int line);

extern int orange_thread_change_pri(orange_thread_handle_t* hndl, uint32_t pri);

extern int orange_thread_exit(void* return_value);

extern int orange_thread_detach(orange_thread_handle_t* hndl);

extern int orange_thread_get_stack_size(void);

extern int orange_thread_set_stack_size(uint32_t size);

extern int orange_thread_get_self_handle(orange_thread_handle_t* hndl);

extern int orange_thread_set_pri(orange_thread_handle_t* hndl, int new_pri);

extern int orange_thread_get_pri(orange_thread_handle_t* hndl);

extern int orange_thread_compare(orange_thread_handle_t* thread_1, orange_thread_handle_t* thread_2);

extern int orange_thread_create(orange_thread_handle_t* hndl, orange_thread_entry_func thread_func, void* paramter);

extern int orange_thread_create_ex(orange_thread_handle_t* hndl, orange_thread_entry_func thread_func, void* paramter, int pri, uint8_t system_scope);

extern int orange_thread_cond_event_init(orange_cond_event_handle_t* handle);

extern int orange_thread_cond_event_de_init(orange_cond_event_handle_t* handle);

extern int orange_thread_cond_event_set(orange_cond_event_handle_t* handle);

extern int orange_thread_cond_event_wait(orange_cond_event_handle_t* handle, int ms);

extern int orange_thread_is_cond_event_trig(orange_cond_event_handle_t* handle);

extern void orange_thread_sleep(uint32_t ms);
