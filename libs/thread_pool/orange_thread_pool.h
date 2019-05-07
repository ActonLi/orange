#ifndef ORANGE_THREAD_POOL_H_ 
#define ORANGE_THREAD_POOL_H_ 

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_thread_pool);
ORANGE_DEFINE_MODULE_EXTENSION(orange_thread_pool);

#define THREAD_POOL_MAX_NUM   64
#define THREAD_POOL_MIN_NUM   4
#define THREAD_POOL_DEFAULT_NUM  32 

#define THREAD_POOL_MANAGE_INTERVAL 5
#define THREAD_WORKER_HIGH_RATIO  3
#define THREAD_WORKER_LOW_RATIO  1

extern int orange_thread_pool_add_task(void*(*task_func)(void *arg), void *arg, size_t size);

#endif
