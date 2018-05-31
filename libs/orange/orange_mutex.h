#pragma once

#include "orange.h"

typedef struct orange_mutex_tag {
    pthread_mutex_t lock;
} orange_mutex_tag_t;

typedef enum orange_mutex_type_tag
{
    MUTEX_NORMAL = 0,
    MUTEX_ERROR_CHECK,
    MUTEX_RECURSIVE, 
	MUTEX_ADAPTIVE, 
	NUTEX_MAX      
}orange_mutex_type_tag_t;


int orange_mutex_create(struct orange_mutex_tag *handle);
int orange_mutex_create_ex(struct orange_mutex_tag *handle,  uint8_t mutex_type);
int orange_mutex_delete(struct orange_mutex_tag *handle);
int orange_mutex_lock(struct orange_mutex_tag *handle);
int orange_mutex_unlock(struct orange_mutex_tag *handle);
int orange_mutex_lock_ex(struct orange_mutex_tag *handle);


