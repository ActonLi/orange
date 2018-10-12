#include "events.h"
#include "osa_mutex.h"
#include "osa_sem.h"
#include "orangelog.h"
#include "osa_thr.h"
#include "osa_time.h"

#if 0
#define DEBUGP orangelog_log_error
#else
#define DEBUGP(format, args...)
#endif

typedef struct tag_STRSEMElemType {
    U32			semid;
    OSA_SemHndl sem;

    struct tag_STRSEMElemType* pre;
    struct tag_STRSEMElemType* next;
} STRSEMElemType;

#define QUEUE_MAX_COUNT 128
static OSA_MutexHndl	 g_event_lk;
static STRSEMElemType*   g_semqueue   = NULL;
static STREventElemType* g_eventqueue = NULL;

static OSA_SemHndl* findsembyid(U32 semid)
{
    OSA_SemHndl*	handle = NULL;
    STRSEMElemType* elem   = g_semqueue;
    while (elem != NULL) {
        if (semid == elem->semid) {
            handle = &elem->sem;
            break;
        }
        elem = elem->next;
    }
    return handle;
}

void event_get(U32 semid, STREventElemType** elem, U32* size)
{
    if (size == NULL) {
        return;
    }

    int				  index = 0;
    int				  count = 0;
    STREventElemType* evt   = NULL;
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);

    DEBUGP("%s:%d lock.\n", __func__, __LINE__);
    STREventElemType* evtelem = g_eventqueue;
    while (evtelem != NULL) {
        if (semid == evtelem->semid) {
            count++;
        }
        evtelem = evtelem->next;
    }
    if (count > 0) {
        evt   = (STREventElemType*) malloc(sizeof(STREventElemType) * count);
        *elem = evt;
        *size = count;

        evtelem = g_eventqueue;
        while (evtelem != NULL) {
            if (semid == evtelem->semid) {
                memcpy(&evt[index], evtelem, sizeof(STREventElemType));
                index++;
            }
            evtelem = evtelem->next;
        }
    }
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
}

void event_wait(U32 semid)
{
    OSA_SemHndl* sem = NULL;
    sem				 = findsembyid(semid);
    if (NULL != sem) {
        OSA_SemTryWait(sem);
    } else {
        OSA_Sleep(500);
    }
}
void event_timedwait(U32 semid, U32 timeoutms)
{
    OSA_SemHndl* sem = NULL;
    sem				 = findsembyid(semid);
    if (NULL != sem) {
        DEBUGP("semid event wait=%d\n", semid);
        OSA_SemTimedWait(sem, timeoutms);
    } else {
        OSA_Sleep(500);
    }
}

void event_init(void)
{
    if (g_semqueue != NULL || g_eventqueue != NULL) {
        event_uninit();
    }
    OSA_MutexCreate(&g_event_lk);
    g_semqueue   = NULL;
    g_eventqueue = NULL;
}

void event_uninit(void)
{
    OSA_MutexLock(&g_event_lk);

    STREventElemType* evtelem = g_eventqueue;
    while (evtelem != NULL) {
        if (evtelem->params != NULL) {
            free(evtelem->params);
            evtelem->params = NULL;
        }
        STREventElemType* tmp = evtelem;
        evtelem				  = evtelem->next;
        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
    }
    g_eventqueue = NULL;

    STRSEMElemType* semelem = g_semqueue;
    while (semelem != NULL) {
        if (semelem->semid > 0) {
            semelem->semid = 0;
            OSA_SemDelete(&semelem->sem);
        }
        STRSEMElemType* tmp = semelem;
        semelem				= semelem->next;
        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
    }
    g_semqueue = NULL;

    OSA_MutexUnlock(&g_event_lk);
    OSA_MutexDelete(&g_event_lk);
}

void event_register(U32 semid)
{
    BOOL res = FALSE;
    OSA_MutexLock(&g_event_lk);

    STRSEMElemType* semelem = g_semqueue;
    while (semelem != NULL) {
        if (semid == semelem->semid) {
            res = TRUE;
            break;
        }
        semelem = semelem->next;
    }
    if ((res == FALSE)) {
        DEBUGP("event register=%d\n", semid);
        STRSEMElemType* last = NULL;
        last				 = (STRSEMElemType*) malloc(sizeof(STRSEMElemType));
        if (TRUE == OSA_SemCreate(&last->sem, SEM_MAX_COUNT, SEM_INIT_COUNT)) {
            last->semid = semid;
            if (g_semqueue != NULL) {
                last->pre			  = g_semqueue->pre;
                last->next			  = NULL;
                g_semqueue->pre->next = last;
                g_semqueue->pre		  = last;
            } else {
                g_semqueue		= last;
                g_semqueue->pre = last;
                last->next		= NULL;
            }
        } else
            free(last);
    }
    OSA_MutexUnlock(&g_event_lk);
}

void event_post(U32 semid, char* module, EnumEventType eventtype, void* param, U32 size, U32 timeout)
{
    U32 count;
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);

    DEBUGP("%s:%d lock.\n", __func__, __LINE__);

    STREventElemType* last = NULL;
    last				   = (STREventElemType*) malloc(sizeof(STREventElemType));
    last->this			   = last;
    if (g_eventqueue != NULL) {
        last->pre				= g_eventqueue->pre;
        last->next				= NULL;
        g_eventqueue->pre->next = last;
        g_eventqueue->pre		= last;
    } else {
        g_eventqueue	  = last;
        g_eventqueue->pre = last;
        last->next		  = NULL;
    }
    OSA_SemHndl* sem;
    sem = findsembyid(semid);
    if (NULL != sem) {
        if (module != NULL) {
            DEBUGP("%s:%d event post modulename=%s\n", __func__, __LINE__, module);
            last->semid = semid;
            snprintf(last->module, sizeof(last->module), "%s", module);
            last->type   = eventtype;
            last->params = malloc(size);
            memcpy(last->params, param, size);
            last->size		= size;
            last->occurtime = OSA_TimeGetTimeValEx();
            if (last->occurtime > 0) {
                last->occurtime--;
            }
            last->expiredtime = (last->occurtime + timeout);
            last->flags		  = 0xffffffff;
        }
    }

    count = 0;

    STREventElemType* evtelem = g_eventqueue;
    while (evtelem != NULL) {
        if (semid == evtelem->semid) {
            count++;
        }
        evtelem = evtelem->next;
    }

    OSA_SemSignal(sem, count);
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
}

void event_clear(U32 semid, U32 modulecnt)
{
    U64  curtime;
    BOOL finish;
    U32  flags;
    U32  count = 0;
    U32  mask  = ((U32) 1 << modulecnt) - 1;
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);

    DEBUGP("%s:%d lock.\n", __func__, __LINE__);

    STREventElemType* tmp	 = NULL;
    STREventElemType* evtelem = g_eventqueue;
    while (evtelem != NULL) {
        DEBUGP("%s:%d in while.\n", __func__, __LINE__);
        DEBUGP("%s:%d evtelem: %p, evtelem->next: %p, evtelem->pre: %p.\n", __func__, __LINE__, evtelem, evtelem->next, evtelem->pre);
        if (semid == evtelem->semid) {
            flags  = evtelem->flags & mask;
            finish = FALSE;
            if (0 == flags) {
                finish = TRUE;
            } else {
                curtime = OSA_TimeGetTimeValEx();
                if (curtime < evtelem->occurtime) {
                    finish = TRUE;
                } else {
                    if (curtime > evtelem->expiredtime) {
                        finish = TRUE;
                    }
                }
            }
            if (finish) {
                evtelem->semid = 0;
                if (evtelem->next != NULL) {
                    DEBUGP("%s:%d 111.\n", __func__, __LINE__);
                    if (g_eventqueue == evtelem) {
                        evtelem->next->pre = g_eventqueue->pre;
                        g_eventqueue	   = evtelem->next;
                    }
                    else {
                        evtelem->pre->next = evtelem->next;
                        evtelem->next->pre = evtelem->pre;
                    }
                } else {
                    DEBUGP("%s:%d 222.\n", __func__, __LINE__);
                    g_eventqueue->pre  = evtelem->pre;
                    evtelem->pre->next = NULL;
                    if (g_eventqueue == evtelem) {
                        g_eventqueue = NULL;
                    }
                }
                if (evtelem->params != NULL) {
                    free(evtelem->params);
                    evtelem->params = NULL;
                }
                tmp = evtelem;
            } else {
                count++;
            }
        }
        evtelem = evtelem->next;
        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
    }
    DEBUGP("%s:%d .\n", __func__, __LINE__);
    OSA_SemHndl* sem;
    sem = findsembyid(semid);
    if (NULL != sem) {
        DEBUGP("%s:%d .\n", __func__, __LINE__);
        OSA_SemSetCount(sem, count);
        DEBUGP("%s:%d .\n", __func__, __LINE__);
    }
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
}

void event_clearall(U32 semid)
{
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);
    DEBUGP("%s:%d lock.\n", __func__, __LINE__);
    STREventElemType* tmp	 = NULL;
    STREventElemType* evtelem = g_eventqueue;
    while (evtelem != NULL) {
        if (semid == evtelem->semid) {
            evtelem->semid = 0;
            if (evtelem->next != NULL) {
                /*
                   evtelem->next->pre = evtelem->pre;
                   if (evtelem->next != evtelem->pre)
                   evtelem->pre->next = evtelem->next;
                   if (g_eventqueue == evtelem) {
                   evtelem->next->pre = g_eventqueue->pre;
                   g_eventqueue	   = evtelem->next;
                   }
                   */
                if (g_eventqueue == evtelem) {
                    evtelem->next->pre = g_eventqueue->pre;
                    g_eventqueue	   = evtelem->next;
                }
                else {
                    evtelem->pre->next = evtelem->next;
                    evtelem->next->pre = evtelem->pre;
                }
            } else {
                g_eventqueue->pre  = evtelem->pre;
                evtelem->pre->next = NULL;
                if (g_eventqueue == evtelem)
                    g_eventqueue = NULL;
            }
            if (evtelem->params != NULL) {
                free(evtelem->params);
                evtelem->params = NULL;
            }
            tmp = evtelem;
        }
        evtelem = evtelem->next;
        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
    }

    OSA_SemHndl* sem;
    sem = findsembyid(semid);
    if (NULL != sem) {
        OSA_SemSetCount(sem, 0);
    }
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
}

void event_clearflags(U32 semid, STREventElemType* event, U32 moduleid)
{
    U32 mask;
    mask = (U32) ~((U32) 1 << moduleid);
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);
    DEBUGP("%s:%d lock.\n", __func__, __LINE__);
    if (event->this != NULL) {
        DEBUGP("%s:%d semid: %u, moduleid: %u, flags: %x, event: %p.\n", 
                __func__, __LINE__, semid, moduleid, event->this->flags, event);
        event->this->flags &= mask;
        DEBUGP("%s:%d semid: %u, moduleid: %u, flags: %x, event: %p.\n", 
                __func__, __LINE__, semid, moduleid, event->this->flags, event);
    }
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
}

void event_signal(U32 semid)
{
    DEBUGP("%s:%d begin.\n", __func__, __LINE__);
    OSA_MutexLock(&g_event_lk);
    DEBUGP("%s:%d lock.\n", __func__, __LINE__);
    OSA_SemHndl* sem;
    sem = findsembyid(semid);
    OSA_SemSignal(sem, 1);
    OSA_MutexUnlock(&g_event_lk);
    DEBUGP("%s:%d end.\n", __func__, __LINE__);
    return;
}

BOOL event_get_jsonmsg(void* params, MSG_DATA_HEADER* header, cJSON** json, U32* msgid)
{
    BOOL  res = FALSE;
    char* payload;

    *msgid = (U32) * ((U32*) params);
    memcpy(header, (char*) (((char*) params) + 4), sizeof(MSG_DATA_HEADER));
    payload = (char*) (((char*) params) + 4 + MSG_HEADER_LENTH);
    *json   = cJSON_Parse((char*) payload);
    if (NULL != *json) {
        res = TRUE;
    }
    return (res);
}
