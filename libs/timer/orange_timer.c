#include "orange_timer.h"
#include "../orange/orange.h"
#include "../orange/orange_queue.h"
#include "../orange/orange_spinlock.h"
#include "../orange/orange_tree.h"

typedef struct orange_timer_cmp {
	int timer_id;
} orange_timer_cmp_t;

typedef struct orange_timer {
	int					 timer_id;
	uint32_t			 interval;
	uint32_t			 elapse;
	orange_timer_func_t* timeout_func;
	void*				 data;
	uint32_t			 index;
	uint32_t			 times;

	RB_ENTRY(orange_timer) entry;
	TAILQ_ENTRY(orange_timer) list;
} orange_timer_t;

typedef struct orange_timer_disc {
	uint32_t		  num;
	uint32_t		  max_num;
	uint32_t		  ms_seconds;
	orange_spinlock_t lock;

	RB_HEAD(orange_timer_tree, orange_timer) timer_tree;
	TAILQ_HEAD(orange_timer_head, orange_timer) list_head;
} orange_timer_disc_t;

static inline int __orange_timer_cmp(struct orange_timer* a, struct orange_timer* b);
RB_PROTOTYPE(orange_timer_tree, orange_timer, entry, __orange_timer_cmp);
RB_GENERATE(orange_timer_tree, orange_timer, entry, __orange_timer_cmp);

struct orange_timer_disc timer_disc;

static void __orange_timer_lock(void)
{
	orange_spinlock_lock(&timer_disc.lock);
}

static void __orange_timer_unlock(void)
{
	orange_spinlock_unlock(&timer_disc.lock);
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

static int __orange_timer_set(uint32_t interval, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data)
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

int orange_timer_kill(int timer_id)
{
	if (timer_id < 0 || timer_id > MAX_TIMER_NUM) {
		return -1;
	}
}

int orange_timer_set(uint32_t timeout, orange_timer_type_t timer_type, orange_timer_func_t* timeout_func, void* data)
{
	int timer_id = INVALID_TIMER_ID;

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

	timer_id = __orange_timer_set(timeout, timer_type, timeout_func, data);

exit:
	return timer_id;
}

int orange_timer_init(int ms_seconds)
{
	memset(&timer_disc, 0, sizeof(struct orange_timer_disc));

	timer_disc.num		  = 0;
	timer_disc.max_num	= MAX_TIMER_NUM;
	timer_disc.ms_seconds = ms_seconds;

	TAILQ_INIT(&timer_disc.list_head);
	orange_spinlock_init(&timer_disc.lock, "timer list lock");

	return 0;
}

void orange_timer_fini(void)
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

	orange_spinlock_destroy(&timer_disc.lock);
	memset(&timer_disc, 0, sizeof(struct orange_timer_disc));

	return;
}