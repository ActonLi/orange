#pragma once

#include "orange.h"

typedef struct orange_sem_hanle {
	uint32_t		count;
	uint32_t		max_count;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
} orange_sem_hanle_t;

extern int orange_sem_create(struct orange_sem_hanle* hndl, uint32_t max_count, uint32_t init_val);
extern int orange_sem_wait(struct orange_sem_hanle* hndl, uint32_t timeout);
extern int orange_sem_timed_wait(struct orange_sem_hanle* hndl, uint32_t timeoutms);
extern int orange_sem_try_wait(struct orange_sem_hanle* hndl);
extern int orange_sem_signal(struct orange_sem_hanle* hndl, uint32_t count);
extern int orange_sem_set_count(struct orange_sem_hanle* hndl, uint32_t count);
extern int orange_sem_delete(struct orange_sem_hanle* hndl);
