#include "orange_epoll.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_epoll_version.h"

ORANGE_VERSION_GENERATE(orange_epoll, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

#define ORANGE_EPOLL_NEVENTS          64
#define ORANGE_EPOLL_DELETE_TIMEOUT   5

typedef struct orange_epoll_entry {
    int                                         fd;
    int                                         deleted;
    unsigned long long                          unregister_timestamp;
    orange_epoll_onrecv_func                    onrecv;
    orange_epoll_onsend_func                    onsend;
    orange_epoll_onerror_func                   onerror;
    orange_epoll_ontimeout_func                 ontimeout;
    void *                                      data;
    RB_ENTRY(orange_epoll_entry)                entry;
    TAILQ_ENTRY(orange_epoll_entry)             list;
} orange_epoll_entry_t;

typedef struct orange_epoll_entry_cmp {
    int fd;
} orange_epoll_entry_cmp_t;

static int __orange_epoll_entry_compare(struct orange_epoll_entry *, struct orange_epoll_entry *);
RB_HEAD(orange_epoll_tree, orange_epoll_entry);
RB_PROTOTYPE(orange_epoll_tree, orange_epoll_entry, entry, __orange_epoll_entry_compare);
RB_GENERATE(orange_epoll_tree, orange_epoll_entry, entry, __orange_epoll_entry_compare);

typedef struct orange_epoll {
    pthread_mutex_t		            lock;
    int                             epoll_fd;
    int                             events_max;
    struct epoll_event              *events;
    int                             nevents;

    TAILQ_HEAD(kevent_header, orange_epoll_entry) head;
    struct orange_epoll_tree   tree;
} orange_epoll_t;


static int __orange_epoll_entry_compare(struct orange_epoll_entry *a, struct orange_epoll_entry *b)
{
    if(a->fd > b->fd) {
        return (1);
    }
    if(a->fd < b->fd) {
        return (-1);
    }
    return 0;
}

static int __orange_epoll_register(struct orange_epoll *epoll,
        int fd,
        orange_epoll_onrecv_func  onrecv,
        orange_epoll_onsend_func  onsend,
        orange_epoll_onerror_func onerror,
        orange_epoll_ontimeout_func ontimeout,
        void *data)
{
    int ret = EEXIST;
    struct epoll_event ev;
    struct orange_epoll_entry *entry;
    struct orange_epoll_entry_cmp cmp;
    struct orange_epoll_entry *tmp = NULL;

    memset(&ev, 0, sizeof(struct epoll_event));
    cmp.fd = fd;
    entry = RB_FIND(orange_epoll_tree, &epoll->tree, (struct orange_epoll_entry *)&cmp);
    if(entry != NULL) {
        ret = 0;
        goto exit;
    }

    entry = malloc(sizeof(struct orange_epoll_entry));
    if(entry == NULL) {
        ret = ENOMEM;
        goto exit;
    }

    memset(entry, 0, sizeof(struct orange_epoll_entry));

    entry->unregister_timestamp = 0;
    entry->fd = fd;

    entry->onrecv = onrecv;
    entry->onsend = onsend;
    entry->onerror = onerror;
    entry->ontimeout = ontimeout;

    entry->data = data;

    ev.data.ptr = entry;

    if (entry->onrecv != NULL && entry->onsend != NULL) {
        ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    } else if(entry->onrecv != NULL) {
        ev.events = EPOLLIN | EPOLLRDHUP;
    } else if(entry->onsend != NULL) {
        ev.events = EPOLLOUT | EPOLLRDHUP;
    }

    ret = epoll_ctl(epoll->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if(ret != 0) {
        goto exit;
    }

    tmp = RB_INSERT(orange_epoll_tree, &epoll->tree, entry);
    if (tmp != NULL) {
        ret = EINVAL;
        free(entry);
        entry = NULL;
        goto exit;
    }

    ret = 0;

exit:
    if (ret != 0 && entry != NULL) {
        free(entry);
    }

    return ret;
}

static void __orange_epoll_entry_delete(struct orange_epoll *epoll, int all_flag)
{
    struct orange_epoll_entry *entry = NULL;
    struct orange_epoll_entry *tmp = NULL;
    unsigned long long now;

    now = time(NULL);

    pthread_mutex_lock(&epoll->lock);

    TAILQ_FOREACH_SAFE(entry, &epoll->head, list, tmp) {
        if (all_flag == 0 && 
                (entry->unregister_timestamp == 0 || entry->unregister_timestamp > now - ORANGE_EPOLL_DELETE_TIMEOUT)) {
            break;
        }

        TAILQ_REMOVE(&epoll->head, entry, list);
        free(entry);
    }

    pthread_mutex_unlock(&epoll->lock);
}

static int __orange_epoll_unregister(struct orange_epoll *epoll, int fd)
{
    int ret = ENOENT;
    struct epoll_event ev = {0};
    struct orange_epoll_entry *entry;
    struct orange_epoll_entry_cmp cmp;

    cmp.fd = fd;
    entry = RB_FIND(orange_epoll_tree, &epoll->tree, (struct orange_epoll_entry *)&cmp);
    if(entry == NULL) {
        goto exit;
    }
    entry->deleted = 1;

 	if (entry->onrecv != NULL) {
        ev.data.ptr = entry;
        ev.events = EPOLLIN;
        ret = epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, fd, &ev);
        if(ret != 0) {
            goto exit;
        }
	}

	if (entry->onsend != NULL) {
        ev.data.ptr = entry;
        ev.events = EPOLLOUT;
        ret = epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, fd, &ev);
        if(ret != 0) {
            goto exit;
        }
    }

    RB_REMOVE(orange_epoll_tree, &epoll->tree, entry);

    entry->unregister_timestamp = time(NULL);

    TAILQ_INSERT_TAIL(&epoll->head, entry, list);

    ret = 0;
exit:
    return ret;
}

struct orange_epoll *orange_epoll_create(void)
{
    struct orange_epoll *epoll = NULL;

    epoll = malloc(sizeof(struct orange_epoll));
    if(epoll == NULL) {
        goto exit;
    }

    memset(epoll, 0, sizeof(struct orange_epoll));

    pthread_mutex_init(&epoll->lock, NULL);

    RB_INIT(&epoll->tree);
    TAILQ_INIT(&epoll->head);

    epoll->epoll_fd = epoll_create(32000);

    epoll->events = malloc(sizeof(struct epoll_event) * ORANGE_EPOLL_NEVENTS);
    memset(epoll->events, 0, sizeof(struct epoll_event) * ORANGE_EPOLL_NEVENTS);
    epoll->nevents = ORANGE_EPOLL_NEVENTS;

    if(epoll->epoll_fd == -1 || epoll->events == NULL) {
        if (epoll->events) {
            free(epoll->events);
            epoll->events = NULL;
        }
        free(epoll);
        epoll = NULL;
    }

exit:
    return epoll;
}

void orange_epoll_destroy(struct orange_epoll *epoll)
{
    struct orange_epoll_entry *entry;

    if(epoll == NULL) {
        return;
    }

    pthread_mutex_lock(&epoll->lock);

    close(epoll->epoll_fd);

    entry = RB_MIN(orange_epoll_tree, &epoll->tree);
    while(entry != NULL) {
        RB_REMOVE(orange_epoll_tree, &epoll->tree, entry);
        free(entry);
        entry = RB_MIN(orange_epoll_tree, &epoll->tree);
    }

    pthread_mutex_unlock(&epoll->lock);

    __orange_epoll_entry_delete(epoll, 1);
    free(epoll->events);
    epoll->events = NULL;
    free(epoll);

    return;
}

int orange_epoll_register(struct orange_epoll *epoll,
        int fd,
        orange_epoll_onrecv_func  onrecv,
        orange_epoll_onsend_func  onsend,
        orange_epoll_onerror_func onerror,
        orange_epoll_ontimeout_func ontimeout,
        void *data)
{
    int ret = EINVAL;

    if(epoll == NULL) {
        goto exit;
    }

    pthread_mutex_lock(&epoll->lock);
    ret = __orange_epoll_register(epoll, fd, onrecv, onsend, onerror, ontimeout, data);
    pthread_mutex_unlock(&epoll->lock);

exit:
    return ret;
}

int orange_epoll_unregister(struct orange_epoll *epoll, int fd)
{
    int ret = EINVAL;

    if(epoll == NULL) {
        goto exit;
    }

    pthread_mutex_lock(&epoll->lock);
    ret = __orange_epoll_unregister(epoll, fd);
    pthread_mutex_unlock(&epoll->lock);

exit:
    return ret;
}

static int __orange_epoll_process(struct orange_epoll *epoll, unsigned long long timeout_mseconds)
{
    int timeout;
    struct orange_epoll_entry *entry = NULL;
    int i;
    int nevents;

    if(epoll == NULL || epoll->nevents == 0) {
        return 0;
    }

    __orange_epoll_entry_delete(epoll, 0); 

    timeout = timeout_mseconds;
    if(timeout <= 0 || timeout > ORANGE_EPOLL_TIMEOUT_MAX) {
        timeout = ORANGE_EPOLL_TIMEOUT_MAX;
    }

    nevents = epoll_wait(epoll->epoll_fd, epoll->events, epoll->nevents, timeout);

    for(i = 0; i < nevents; i++) {
        int what = epoll->events[i].events;
        entry = (struct orange_epoll_entry *)epoll->events[i].data.ptr;
		if (what & (EPOLLHUP|EPOLLERR)) {
			if(entry->onerror != NULL) {
				entry->onerror(entry->fd, entry->data, 0);
			}
		} else {
			if (what & EPOLLIN) {
				if(entry->onrecv != NULL) {
					entry->onrecv(entry->fd, entry->data, 0);
				}
			}
			if (what & EPOLLOUT) {
				if(entry->onsend != NULL) {
                	entry->onsend(entry->fd, entry->data, 0);
            	}

			}
			if (what & EPOLLRDHUP) {
				if(entry->onerror != NULL) {
					entry->onerror(entry->fd, entry->data, 0);
				}
			}
		}
    }

    return nevents;
}

int orange_epoll_process(struct orange_epoll *epoll, unsigned long long timeout_mseconds)
{
    return __orange_epoll_process(epoll, timeout_mseconds);
}

static int __orange_epoll_module_init(void)
{
	snprintf(orange_epoll_description, 127, "Orange Epoll Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_epoll_version),
			 orange_version_type(orange_epoll_version_type), orange_epoll_build_num, orange_epoll_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_epoll_description);

	return 0;
}

static void __orange_epoll_module_fini(void)
{
	orange_log(ORANGE_LOG_INFO, "Orange Epoll Module unloaded.\n");

	return;
}

static int orange_epoll_modepoll(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_epoll_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_epoll_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_epoll_mod = {"orange_epoll", orange_epoll_modepoll, 0};

ORANGE_DECLARE_MODULE(orange_epoll, orange_epoll_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_epoll, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_epoll);
