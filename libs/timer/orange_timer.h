#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_timer);
ORANGE_DEFINE_MODULE_EXTENSION(orange_timer);

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

extern int orange_timer_init(void);
extern int orange_timer_init_ex(uint32_t ms_seconds);
extern int orange_timer_kill(int timer_id);
extern int orange_timer_set(uint32_t timeout, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data, int data_size);
