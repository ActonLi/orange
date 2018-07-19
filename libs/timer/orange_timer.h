#pragma once

#include "../orange/orange.h"

#define MAX_TIMER_NUM 20
#define TIMER_START 1
#define TIMER_TICK 1
#define INVALID_TIMER_ID (-1)

typedef enum orange_timer_type {
	ORANGE_TIMER_NONE = 0,
	ORANGE_TIMER_ONCE,
	ORANGE_TIMER_CONTINUED,
	ORANGE_TIMER_TYPE_MAX,
} orange_timer_type_t;

typedef int orange_timer_func_t(int id, void* data, int data_len);

int orange_timer_kill(int timer_id);
int orange_timer_set(uint32_t timeout, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data);
int orange_timer_init(int ms_seconds);
void orange_timer_fini(void);
