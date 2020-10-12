#pragma once

#include "../orange/orange.h"
#include "../orange/orange_module.h"

ORANGE_VERSION_TYPE(orange_epoll);
ORANGE_DEFINE_MODULE_EXTENSION(orange_epoll);

#define ORANGE_EPOLL_EVENTS_MAX    128
#define ORANGE_EPOLL_TIMEOUT_MAX   (500)

struct orange_epoll;

typedef void (*orange_epoll_onrecv_func)(int fd, void *var, int data);
typedef void (*orange_epoll_onsend_func)(int fd, void *var, int data);
typedef void (*orange_epoll_onerror_func)(int fd, void *var, int flags);
typedef void (*orange_epoll_ontimeout_func)(int fd, void *var);

extern struct orange_epoll *orange_epoll_create(void);
extern void orange_epoll_destroy(struct orange_epoll *epoll);
extern int orange_epoll_register(struct orange_epoll *epoll,
        int fd,
        orange_epoll_onrecv_func  onrecv,
        orange_epoll_onsend_func  onsend,
        orange_epoll_onerror_func onerror,
        orange_epoll_ontimeout_func ontimeout,
        void *data);
extern int orange_epoll_unregister(struct orange_epoll *epoll, int fd);
extern int orange_epoll_process(struct orange_epoll *epoll, unsigned long long timeout_mseconds);



