#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_thread_task);
ORANGE_DEFINE_MODULE_EXTENSION(orange_thread_task);

#define ORANGE_THREAD_TASK_NAME_LEN_MAX 16

typedef struct orange_thread_task_parameters {
    char        task_name[ORANGE_THREAD_TASK_NAME_LEN_MAX];
    uint8_t       priority;
    uint8_t       task_id;
    uint32_t      stack_size;
    void*       (*task_handler)(void*);
    int         (*task_init)(void*);
    uint16_t      queue_numbers;
    uint16_t      queue_size;
} orange_thread_task_parameters_t;

int orange_thread_task_create(struct orange_thread_task_parameters *task_params, void* init_params);


