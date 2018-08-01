#include "orange_timer.h"
#include "../orange/orange_log.h"
#include "../orange/orange_mutex.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_thread.h"
#include "../orange/orange_tree.h"
#include "orange_timer_version.h"

ORANGE_VERSION_GENERATE(orange_timer, 1, 1, 1, ORANGE_VERSION_TYPE_ALPHA);

typedef struct orange_timer_cmp {
	int timer_id;
} orange_timer_cmp_t;

typedef struct orange_timer {
	int					 timer_id;
	uint32_t			 interval;
	uint32_t			 elapse;
	orange_timer_func_t* timeout_func;
	void*				 data;
	int					 data_size;
	uint32_t			 index;
	uint32_t			 times;

	RB_ENTRY(orange_timer) entry;
	TAILQ_ENTRY(orange_timer) list;
} orange_timer_t;

typedef struct orange_timer_disc {
	uint32_t	   num;
	uint32_t	   max_num;
	uint32_t	   ms_seconds;
	orange_mutex_t lock;

	RB_HEAD(orange_timer_tree, orange_timer) timer_tree;
	TAILQ_HEAD(orange_timer_head, orange_timer) list_head;
} orange_timer_disc_t;

static inline int __orange_timer_cmp(struct orange_timer* a, struct orange_timer* b);
RB_PROTOTYPE(orange_timer_tree, orange_timer, entry, __orange_timer_cmp);
RB_GENERATE(orange_timer_tree, orange_timer, entry, __orange_timer_cmp);

static struct orange_timer_disc	timer_disc;
static struct orange_thread_handle thread_handle;
static int						   timer_func_running = 0;

static void __orange_timer_lock(void)
{
	orange_mutex_lock(&timer_disc.lock);
}

static void __orange_timer_unlock(void)
{
	orange_mutex_unlock(&timer_disc.lock);
}

static inline int __orange_timer_cmp(struct orange_timer* a, struct orange_timer* b)
{
	return (a->timer_id - b->timer_id);
}

static void __orange_timer_destroy(struct orange_timer* timer)
{
	if (NULL == timer) {
		return;
	}

	free(timer);

	return;
}

static struct orange_timer* __orange_timer_zalloc(void)
{
	struct orange_timer* timer = NULL;

	timer = malloc(sizeof(struct orange_timer));
	if (timer) {
		memset(timer, 0, sizeof(struct orange_timer));
	}

	return timer;
}

static struct orange_timer* __orange_timer_find(int timer_id)
{
	struct orange_timer_cmp cmp;
	cmp.timer_id = timer_id;

	return RB_FIND(orange_timer_tree, &(timer_disc.timer_tree), (struct orange_timer*) &cmp);
}

static int __orange_timer_set(uint32_t interval, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data, int data_size)
{
	int					 timer_id = INVALID_TIMER_ID;
	struct orange_timer* timer	= NULL;

	timer = __orange_timer_zalloc();
	if (NULL == timer) {
		goto exit;
	}

	timer_id			= ++(timer_disc.num);
	timer->timer_id		= timer_id;
	timer->timeout_func = timeout_func;
	timer->interval		= interval;
	timer->elapse		= 0;
	timer->times		= timer_type;
	timer->data			= data;
	timer->data_size	= data_size;
	timer->index++;
	if (timer->index >= 300) {
		timer->index = 0;
	}

	__orange_timer_lock();
	RB_INSERT(orange_timer_tree, &(timer_disc.timer_tree), timer);
	TAILQ_INSERT_TAIL(&(timer_disc.list_head), timer, list);
	__orange_timer_unlock();

exit:
	return timer_id;
}

static void* __orange_timer_process(void* param)
{
	struct orange_timer* timer = NULL;
	struct orange_timer* tmp   = NULL;
	struct timeval		 timeout;
	int					 ret;

	while (timer_func_running) {
		timeout.tv_sec  = 0;
		timeout.tv_usec = 1000 * timer_disc.ms_seconds;
		ret				= select(0, 0, 0, 0, &timeout);

		if (0 == ret) {
			TAILQ_FOREACH_SAFE(timer, &(timer_disc.list_head), list, tmp)
			{
				if (timer) {
					timer->elapse++;
					if (timer->elapse == timer->interval) {
						if (NULL != timer->timeout_func) {
							orange_timer_func_t* timeout_func;
							__orange_timer_lock();
							timeout_func = timer->timeout_func;
							__orange_timer_unlock();
							if (timeout_func) {
								timeout_func(timer->timer_id, timer->data, timer->data_size);
							}
							__orange_timer_lock();
							if (timer->times == ORANGE_TIMER_ONCE) {
								RB_REMOVE(orange_timer_tree, &(timer_disc.timer_tree), timer);
								TAILQ_REMOVE(&(timer_disc.list_head), timer, list);
								__orange_timer_destroy(timer);
							} else if (timer->times == ORANGE_TIMER_CONTINUED) {
								timer->elapse = 0;
							}
							__orange_timer_unlock();
						}
					}
				}
			}
		}
	}

	return NULL;
}

static int __orange_timer_init(int count, uint32_t ms_seconds)
{
	int ret = -1;

	if (count < 0 || count > MAX_TIMER_NUM) {
		goto exit;
	}

	timer_func_running = 1;
	ret				   = orange_thread_create(&thread_handle, __orange_timer_process, NULL);

exit:
	return ret;
}

int orange_timer_kill(int timer_id)
{
	struct orange_timer* timer = NULL;

	if (timer_id < 0 || timer_id > MAX_TIMER_NUM) {
		return -1;
	}

	__orange_timer_lock();
	timer = __orange_timer_find(timer_id);
	if (NULL == timer) {
		__orange_timer_unlock();
		return -1;
	}
	RB_REMOVE(orange_timer_tree, &(timer_disc.timer_tree), timer);
	TAILQ_REMOVE(&(timer_disc.list_head), timer, list);
	__orange_timer_unlock();

	__orange_timer_destroy(timer);

	return 0;
}

int orange_timer_set(uint32_t timeout, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data, int data_size)
{
	int timer_id = INVALID_TIMER_ID;

	orange_log(ORANGE_LOG_INFO, "%s:%d begin.\n", __func__, __LINE__);

	if (timer_disc.num >= timer_disc.max_num) {
		goto exit;
	}

	if ((timer_type <= ORANGE_TIMER_NONE) || (timer_type >= ORANGE_TIMER_TYPE_MAX)) {
		goto exit;
	}

	if (timeout == 0) {
		goto exit;
	}

	if (NULL == timeout_func) {
		goto exit;
	}

	timer_id = __orange_timer_set(timeout, timer_type, timeout_func, data, data_size);

exit:
	orange_log(ORANGE_LOG_INFO, "%s:%d end timer_id: %d.\n", __func__, __LINE__, timer_id);
	return timer_id;
}

int orange_timer_init_ex(uint32_t ms_seconds)
{
	return __orange_timer_init(MAX_TIMER_NUM, ms_seconds);
}

int orange_timer_init(void)
{
	return orange_timer_init_ex(100);
}

static int __orange_timer_module_init(void)
{

	snprintf(orange_timer_description, 127, "Orange Timer Module " ORANGE_VERSION_FORMAT "-%s #%u: %s", ORANGE_VERSION_QUAD(orange_timer_version),
			 orange_version_type(orange_timer_version_type), orange_timer_build_num, orange_timer_build_date);

	orange_log(ORANGE_LOG_INFO, "%s\n", orange_timer_description);

	memset(&timer_disc, 0, sizeof(struct orange_timer_disc));

	timer_disc.num		  = 0;
	timer_disc.max_num	= MAX_TIMER_NUM;
	timer_disc.ms_seconds = 100;

	TAILQ_INIT(&timer_disc.list_head);
	orange_mutex_create(&timer_disc.lock);

	return 0;
}

static void __orange_timer_module_fini(void)
{
	struct orange_timer* timer = NULL;
	struct orange_timer* tmp   = NULL;

	TAILQ_FOREACH_SAFE(timer, &(timer_disc.list_head), list, tmp)
	{
		if (timer) {
			TAILQ_REMOVE(&(timer_disc.list_head), timer, list);
			__orange_timer_destroy(timer);
		}
	}

	orange_mutex_delete(&timer_disc.lock);
	memset(&timer_disc, 0, sizeof(struct orange_timer_disc));

	orange_log(ORANGE_LOG_INFO, "Orange timer Module unloaded.\n");

	return;
}

static int orange_timer_modevent(orange_module_t mod, int type, void* data)
{
	int ret = 0;

	switch (type) {
		case ORANGE_MOD_LOAD:
			ret = __orange_timer_module_init();
			break;
		case ORANGE_MOD_UNLOAD:
			__orange_timer_module_fini();
			break;
		default:
			return (EOPNOTSUPP);
	}
	return ret;
}

static orange_moduledata_t orange_timer_mod = {"orange_timer", orange_timer_modevent, 0};

ORANGE_DECLARE_MODULE(orange_timer, orange_timer_mod, ORANGE_SI_SUB_PSEUDO, ORANGE_SI_ORDER_ANY);

ORANGE_MODULE_VERSION(orange_timer, 1);
ORANGE_DECLARE_MODULE_EXTENSION(orange_timer);
