#include "orange_thread_pool.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_thread_pool_version.h"

ORANGE_VERSION_GENERATE(orange_thread_pool, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

typedef struct task_queue_node {
	void *(*task_func)(void *arg);
	void *arg;
    size_t arg_size;
	TAILQ_ENTRY(task_queue_node) list;
} task_queue_node_t; 

typedef struct thread_queue_node {
	pthread_t 	thread_id;
	TAILQ_ENTRY(thread_queue_node) list;
} thread_queue_node_t;

typedef struct orange_thread_pool {
	int					shutdown;
	int 				thread_count;
	int 				task_count;
	int 				revoke_count;

	pthread_mutex_t 	thread_lock;
	pthread_mutex_t 	task_lock;

	pthread_cond_t		notify;

	TAILQ_HEAD(thread_head, thread_queue_node) thread_list_head;
	TAILQ_HEAD(task_head, task_queue_node) task_list_head;
} orange_thread_pool_t;


static struct orange_thread_pool* g_orange_thread_pool = NULL;

static int __orange_thread_pool_thread_queue_insert(pthread_t thread_id, int* count)
{
    struct thread_queue_node* thread_node;

    thread_node = malloc(sizeof(struct thread_queue_node));
    if (NULL != thread_node) {
        thread_node->thread_id = thread_id;
        TAILQ_INSERT_HEAD(&(g_orange_thread_pool->thread_list_head), thread_node, list);
        (*count)++;
    }

    return 0;
}

static void __orange_thread_pool_task_queue_destroy(void)
{
    struct task_queue_node* task;
    struct task_queue_node* temp;

    TAILQ_FOREACH_SAFE(task, &(g_orange_thread_pool->task_list_head), list, temp) {
        if (task) {
            TAILQ_REMOVE(&(g_orange_thread_pool->task_list_head), task, list);
            if (task->arg) {
                free(task->arg);
            }
            free(task);
        }
    }

    return;
}

static void __orange_thread_pool_thread_queue_destroy(void)
{
    struct thread_queue_node* thread_node;
    struct thread_queue_node* tmp;

    TAILQ_FOREACH_SAFE(thread_node, &(g_orange_thread_pool->thread_list_head), list, tmp) {
        if (thread_node) {
            TAILQ_REMOVE(&(g_orange_thread_pool->thread_list_head), thread_node, list);
            free(thread_node);
        }
    }

    return;
}

static int __orange_thread_pool_thread_queue_remove(pthread_t thread_id, int* count)
{
    struct thread_queue_node* thread_node;
    struct thread_queue_node* tmp;

    if (!TAILQ_EMPTY(&(g_orange_thread_pool->thread_list_head))) {
        TAILQ_FOREACH_SAFE(thread_node, &(g_orange_thread_pool->thread_list_head), list, tmp) {
            if (thread_node && thread_node->thread_id == thread_id) {
                TAILQ_REMOVE(&(g_orange_thread_pool->thread_list_head), thread_node, list);
                free(thread_node);
                (*count)--;
            }
        }
    }

    return 0;
}

static void* __orange_thread_pool_func(void *arg) 
{
    struct task_queue_node* task;

	for(;;) {
		pthread_mutex_lock(&(g_orange_thread_pool->thread_lock));

		while(g_orange_thread_pool->task_count == 0 && !g_orange_thread_pool->shutdown) {
			pthread_cond_wait(&(g_orange_thread_pool->notify), &(g_orange_thread_pool->thread_lock));
		}

		if(g_orange_thread_pool->shutdown) {
			__orange_thread_pool_thread_queue_remove(pthread_self(), &(g_orange_thread_pool->thread_count));
			pthread_mutex_unlock(&(g_orange_thread_pool->thread_lock));
			pthread_exit(NULL);
		}

		if(g_orange_thread_pool->revoke_count > 0) {
			__orange_thread_pool_thread_queue_remove(pthread_self(), &(g_orange_thread_pool->thread_count));
            g_orange_thread_pool->revoke_count--;
			pthread_mutex_unlock(&(g_orange_thread_pool->thread_lock));
			pthread_exit(NULL);
		}

        pthread_mutex_lock(&(g_orange_thread_pool->task_lock));
        task = TAILQ_FIRST(&(g_orange_thread_pool->task_list_head));
        if (NULL == task) {
            pthread_mutex_unlock(&(g_orange_thread_pool->task_lock));
        } else {
            g_orange_thread_pool->task_count--;
            (*(task->task_func))(task->arg);
            TAILQ_REMOVE(&(g_orange_thread_pool->task_list_head), task, list);
            pthread_mutex_unlock(&(g_orange_thread_pool->task_lock));

            if (NULL != task->arg) {
                free(task->arg);
            }
            free(task);
        }

        pthread_mutex_unlock(&(g_orange_thread_pool->thread_lock));
	}

	pthread_exit(NULL);
}

static void __orange_thread_pool_add_thread(int count)
{
	int i;
	pthread_attr_t attr;
    pthread_t thread_id;
    int ret;

	ret = pthread_attr_init(&attr);
	if(ret != 0) {
        goto exit;
	}

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(ret != 0) {
        goto exit;
	}
	
	for(i = 0; i < count; i++) {
		pthread_create(&thread_id, &attr, __orange_thread_pool_func, NULL);
		__orange_thread_pool_thread_queue_insert(thread_id, &(g_orange_thread_pool->thread_count));
	}

	pthread_attr_destroy(&attr);

exit:
    return;
}

static void __orange_thread_pool_del_thread(int thread_num)
{
	if(thread_num == 0) {
		return;
	}

	g_orange_thread_pool->revoke_count = thread_num;
	pthread_cond_broadcast(&(g_orange_thread_pool->notify));

    return;
}

static void* __orange_thread_pool_manage(void *arg)
{
    int thread_num;

    for(;;) {
        if(g_orange_thread_pool->task_count > THREAD_WORKER_HIGH_RATIO * g_orange_thread_pool->thread_count) {
            thread_num =(g_orange_thread_pool->task_count - THREAD_WORKER_HIGH_RATIO * g_orange_thread_pool->thread_count) / THREAD_WORKER_HIGH_RATIO;
            if(g_orange_thread_pool->thread_count + thread_num > THREAD_POOL_MAX_NUM) {
                thread_num = THREAD_POOL_MAX_NUM - g_orange_thread_pool->thread_count;
            }
            __orange_thread_pool_add_thread(thread_num);
        }
        else if (g_orange_thread_pool->task_count * THREAD_WORKER_LOW_RATIO < g_orange_thread_pool->thread_count) {
            thread_num =(g_orange_thread_pool->thread_count - THREAD_WORKER_LOW_RATIO * g_orange_thread_pool->task_count) / THREAD_WORKER_LOW_RATIO;
            if(g_orange_thread_pool->thread_count - thread_num < THREAD_POOL_MIN_NUM) {
                thread_num = g_orange_thread_pool->thread_count - THREAD_POOL_MIN_NUM;
            }
            __orange_thread_pool_del_thread(thread_num);
        }

        sleep(THREAD_POOL_MANAGE_INTERVAL);
    }

    return NULL;
}

int orange_thread_pool_add_task(void*(*task_func)(void *arg), void *arg, size_t size)
{
	struct task_queue_node *new_task = malloc(sizeof(struct task_queue_node));
	if(new_task == NULL) {
		return -1;
	}

    new_task->arg = NULL;
    new_task->arg_size = 0;
	
    if (arg != NULL && size > 0) {
        new_task->arg = malloc(size);
        if (NULL == new_task->arg) {
            free(new_task);
            return -1;
        }

        new_task->task_func = task_func;
        new_task->arg_size = size;
        memcpy(new_task->arg, arg, size);
    }

    pthread_mutex_lock(&(g_orange_thread_pool->task_lock));

    TAILQ_INSERT_TAIL(&(g_orange_thread_pool->task_list_head), new_task, list);
	g_orange_thread_pool->task_count++;

    pthread_mutex_unlock(&(g_orange_thread_pool->task_lock));

	pthread_cond_signal(&(g_orange_thread_pool->notify));

	return 0;
}

static int __orange_thread_pool_init(int thread_num)
{
    int ret = -1;
	pthread_t manage_tid;

    g_orange_thread_pool = malloc(sizeof(struct orange_thread_pool));
    if (g_orange_thread_pool) {
        if (thread_num > THREAD_POOL_MAX_NUM) {
            thread_num  = THREAD_POOL_MAX_NUM;
        } else if (thread_num < THREAD_POOL_MIN_NUM) {
            thread_num  = THREAD_POOL_MIN_NUM;
        }

        pthread_mutex_init(&(g_orange_thread_pool->thread_lock),NULL);
        pthread_mutex_init(&(g_orange_thread_pool->task_lock),NULL);
        pthread_cond_init(&(g_orange_thread_pool->notify),NULL);

        g_orange_thread_pool->task_count = 0;
        g_orange_thread_pool->thread_count = 0;
        g_orange_thread_pool->shutdown = 0;

		TAILQ_INIT(&(g_orange_thread_pool->thread_list_head));
		TAILQ_INIT(&(g_orange_thread_pool->task_list_head));

        __orange_thread_pool_add_thread(thread_num);

        ret = 0;
    }

	pthread_create(&manage_tid, NULL, __orange_thread_pool_manage, NULL);

exit:
    return ret;
}

static int __orange_thread_pool_fini(void)
{
	if(g_orange_thread_pool->shutdown) {
		return -1;
	}

    __orange_thread_pool_task_queue_destroy();
    __orange_thread_pool_thread_queue_destroy();
	
	g_orange_thread_pool->shutdown = 1;

	pthread_cond_broadcast(&(g_orange_thread_pool->notify));

	pthread_mutex_destroy(&(g_orange_thread_pool->task_lock));
	pthread_mutex_destroy(&(g_orange_thread_pool->thread_lock));
	pthread_cond_destroy(&(g_orange_thread_pool->notify));
	
	free(g_orange_thread_pool);
	g_orange_thread_pool = NULL;

	return 0;
}

static int orange_thread_pool_modevent(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_thread_pool_init(64);
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_thread_pool_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_thread_pool_mod = {"orange_thread_pool", orange_thread_pool_modevent, 0};

ORANGE_DECLARE_MODULE(orange_thread_pool, orange_thread_pool_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_thread_pool, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_thread_pool);

