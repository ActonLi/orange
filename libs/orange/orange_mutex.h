#pragma once
#include "orange.h"

typedef struct orange_mutex {
    pthread_mutex_t lock;
} orange_mutex_t;

typedef enum orange_mutex_type {
    ORANGE_MUTEX_NORMAL = 0,
    ORANGE_ERRORCHECK,
    ORANGE_RECURSIVE,
	ORANGE_ADAPTIVE,
	ORANGE_MAX     
} orange_mutex_type_t;

int orange_mutex_create(orange_mutex_t *hndl);

int orange_mutex_create_ex(orange_mutex_t *hndl, orange_mutex_type_t mutex_type);

int orange_mutex_delete(orange_mutex_t *hndl);

int orange_mutex_lock(orange_mutex_t *hndl);

int orange_mutex_unlock(orange_mutex_t *hndl);

int orange_mutex_lock_ex(orange_mutex_t *hndl);

