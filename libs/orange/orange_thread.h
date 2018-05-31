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

#define ORANGE_SLEEP_CAL_VAL 1000

typedef void* (*orange_thread_entry_func)(void*);

typedef struct orange_thread_handle {
	pthread_t hndl;
} orange_thread_handle_t;

typedef struct orange_cond_event_handle_tag {
	pthread_cond_t  cond;
	pthread_mutex_t mutex;
	uint8_t			initialized;
	uint8_t			spuriouswakeup;
} orange_cond_event_handle_tag_t;

int orange_thread_create(struct orange_thread_handle* hndl, orange_thread_entry_func entry_func, int pri, void* prm);
int orange_thread_delete(struct orange_thread_handle* hndl);
int orange_thread_join(struct orange_thread_handle* hndl);
int orange_thread_join_ex(struct orange_thread_handle* hndl);
int orange_thread_change_pri(struct orange_thread_handle* hndl, uint32_t pri);
int orange_thread_exit(void* return_val);
int orange_thread_is_valid(struct orange_thread_handle* hndl);
int orange_thread_detach(struct orange_thread_handle* hndl);
int orange_thread_get_stack_size(void);
int orange_thread_set_thread_stack_size(uint32_t stack_size);
int orange_thread_get_self_thread_handle(struct orange_thread_handle* selft_thread_handle);
int orange_thread_set_thread_pri(struct orange_thread_handle* pthread, int new_pri);
int orange_thread_get_thread_pri(struct orange_thread_handle* pthread);
int orange_thread_compare(struct orange_thread_handle* pthread_a, struct orange_thread_handle* pthread_b);
int orange_thread_create_ex(struct orange_thread_handle* hndl, orange_thread_entry_func thread_func, void* paramter);
int orange_thread_cond_event_init(struct orange_cond_event_handle_tag* cond_event);
int orange_thread_cond_event_de_init(struct orange_cond_event_handle_tag* cond_event);
int orange_thread_set_cond_event(struct orange_cond_event_handle_tag* cond_event);
int orange_thread_wait_cond_event(struct orange_cond_event_handle_tag* cond_event, uint32_t timeout_ms);
int orange_thread_is_cond_event_trig(struct orange_cond_event_handle_tag* cond_event);
void orange_thread_sleep(uint32_t sleep_ms);
