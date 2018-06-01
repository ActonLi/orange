#include "orange_thread.h"
#include "orange_log.h"

static int __pthread_create(pthread_t* thread, pthread_attr_t* attr, orange_thread_entry_func entry_func, void* arg)
{
	return pthread_create(thread, attr, entry_func, arg);
}

static int __pthread_join(pthread_t thread, void** thread_return)
{
	return pthread_join(thread, thread_return);
}

static int __orange_thread_create_ex(struct orange_thread_handle* hndl, orange_thread_entry_func thread_func, void* paramter, int pri, int system_scope)
{
	int ret = -1;

	pthread_attr_t	 attr;
	struct sched_param param = {0};

	if ((NULL == hndl) || (NULL == thread_func)) {
		goto exit;
	}

	ret = pthread_attr_init(&attr);
	if (0 != ret) {
		goto destroy;
	}

	ret |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (0 != ret) {
		goto destroy;
	}

	pri = max(pri, ORANGE_THR_PRI_RR_MIN);
	pri = min(pri, ORANGE_THR_PRI_RR_MAX);

	ret |= pthread_attr_getschedparam(&attr, &param);
	param.sched_priority = pri;
	ret |= pthread_attr_setschedparam(&attr, &param);
	if (0 != ret) {
		goto destroy;
	}

	ret |= pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (0 != ret) {
		goto destroy;
	}

	if (system_scope) {
		ret |= pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
		if (0 != ret) {
			goto destroy;
		}
	}

	ret = __pthread_create(&(hndl->hndl), &attr, (void*) thread_func, paramter);
	if (0 != ret) {
		switch (ret) {
			case EAGAIN:
				break;
			case EINVAL:
				break;
		}
		goto destroy;
	}

destroy:
	pthread_attr_destroy(&attr);

exit:
	return ret;
}

int orange_thread_create(struct orange_thread_handle* hndl, orange_thread_entry_func entry_func, int pri, void* prm)
{
	int				   ret = 0;
	pthread_attr_t	 thread_attr;
	struct sched_param schedprm;

	ret = pthread_attr_init(&thread_attr);
	if (0 != ret) {
		goto exit;
	}

	ret |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
	ret |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
	if (pri > (uint32_t) ORANGE_THR_PRI_RR_MAX) {
		pri = ORANGE_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) ORANGE_THR_PRI_RR_MIN) {
		pri = ORANGE_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
	ret |= pthread_attr_setschedparam(&thread_attr, &schedprm);

	if (0 != ret) {
		pthread_attr_destroy(&thread_attr);
		goto exit;
	}

	ret |= pthread_create(&hndl->hndl, &thread_attr, entry_func, prm);

exit:
	return ret;
}

int orange_thread_delete(struct orange_thread_handle* hndl)
{
	int ret = -1;

	if (NULL == hndl) {
		goto exit;
	}

	if (ORANGE_THREAD_HANDLE_INVLALID == hndl->hndl) {
		goto exit;
	}

	ret |= pthread_cancel(hndl->hndl);

	ret |= orange_thread_join(hndl);

exit:
	return ret;
}

int orange_thread_join(struct orange_thread_handle* hndl)
{
	int   ret = -1;
	void* return_val;

	if (NULL == hndl) {
		goto exit;
	}

	if (ORANGE_THREAD_HANDLE_INVLALID == hndl->hndl) {
		goto exit;
	}

	if (orange_thread_compare(hndl, NULL)) {
		goto exit;
	}

	ret = pthread_join(hndl->hndl, &return_val);

	if (EINVAL == 0) {
		ret = 0;
	}

exit:
	return ret;
}

int orange_thread_join_ex(struct orange_thread_handle* hndl)
{
	int   ret = -1;
	void* return_val;

	if (NULL == hndl) {
		goto exit;
	}

	if (ORANGE_THREAD_HANDLE_INVLALID == hndl->hndl) {
		goto exit;
	}

	if (orange_thread_compare(hndl, NULL)) {
		goto exit;
	}

	ret = __pthread_join(hndl->hndl, &return_val);
	if (EINVAL == ret) {
		ret = 0;
	}

exit:
	return ret;
}

int orange_thread_change_pri(struct orange_thread_handle* hndl, uint32_t pri)
{
	int				   ret = -1;
	struct sched_param schedprm;

	if (pri > (uint32_t) ORANGE_THR_PRI_RR_MAX) {
		pri = ORANGE_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) ORANGE_THR_PRI_RR_MIN) {
		pri = ORANGE_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
	ret |= pthread_setschedparam(hndl->hndl, SCHED_RR, &schedprm);

	return ret;
}

int orange_thread_exit(void* return_val)
{
	pthread_exit(return_val);
	return 0;
}

int orange_thread_is_valid(struct orange_thread_handle* hndl)
{
	int ret = -1;
	if (NULL == hndl) {
		goto exit;
	}

	if (hndl->hndl == ORANGE_THREAD_HANDLE_INVLALID) {
		goto exit;
	}

	ret = pthread_kill(hndl->hndl, 0);

exit:
	return ret;
}

int orange_thread_detach(struct orange_thread_handle* hndl)
{
	int ret = -1;

	if (NULL == hndl) {
		goto exit;
	}

	if (ORANGE_THREAD_HANDLE_INVLALID == hndl->hndl) {
		goto exit;
	}

	ret = pthread_detach(hndl->hndl);

exit:
	return ret;
}

int orange_thread_get_stack_size(void)
{
	pthread_attr_t thread_attr;
	int			   stack_size = 0;

	int ret = pthread_attr_init(&thread_attr);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_attr_getstacksize(&thread_attr, (size_t*) &stack_size);

exit:
	return stack_size;
}

int orange_thread_set_thread_stack_size(uint32_t stack_size)
{
	pthread_attr_t thread_attr;
	int			   tmp_stack_size = 0;

	int ret = pthread_attr_init(&thread_attr);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_attr_setstacksize(&thread_attr, (int) stack_size);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_attr_getstacksize(&thread_attr, (size_t*) &tmp_stack_size);
	if (stack_size == tmp_stack_size) {
		ret = 0;
	}

exit:
	return ret;
}

int orange_thread_get_self_thread_handle(struct orange_thread_handle* selft_thread_handle)
{
	int ret = -1;

	if ((NULL == selft_thread_handle)) {
		goto exit;
	}

	selft_thread_handle->hndl = pthread_self();
	if ((selft_thread_handle->hndl == ORANGE_THREAD_HANDLE_INVLALID) || (selft_thread_handle->hndl == 0)) {
		goto exit;
	}

	ret = 0;
exit:
	return ret;
}

int orange_thread_set_thread_pri(struct orange_thread_handle* pthread, int new_pri)
{
	int				   ret = -1;
	int				   policy;
	struct sched_param st_sched_param;

	if ((NULL == pthread)) {
		goto exit;
	}

	memset(&st_sched_param, 0, sizeof(st_sched_param));

	new_pri = (new_pri < 0) ? 0 : new_pri;
	new_pri = (new_pri > ORANGE_THR_PRI_RR_MAX) ? ORANGE_THR_PRI_RR_MAX : new_pri;

	ret = pthread_getschedparam(pthread->hndl, &policy, &st_sched_param);
	if (0 != ret) {
		goto exit;
	}

	st_sched_param.sched_priority = new_pri;

	ret = pthread_setschedparam(pthread->hndl, policy, &st_sched_param);

exit:
	return ret;
}

int orange_thread_get_thread_pri(struct orange_thread_handle* pthread)
{
	int				   ret = -1;
	int				   policy;
	struct sched_param st_sched_param;

	if ((NULL == pthread)) {
		goto exit;
	}

	memset(&st_sched_param, 0, sizeof(st_sched_param));

	ret = pthread_getschedparam(pthread->hndl, &policy, &st_sched_param);
	if (0 == ret) {
		ret = st_sched_param.sched_priority;
	}

exit:
	return ret;
}

int orange_thread_compare(struct orange_thread_handle* pthread_a, struct orange_thread_handle* pthread_b)
{
	struct orange_thread_handle thread_tmp;
	int							ret;

	if ((NULL == pthread_a)) {
		return -1;
	}

	thread_tmp.hndl = (NULL == pthread_b) ? pthread_self() : pthread_b->hndl;

	ret = pthread_equal(pthread_a->hndl, thread_tmp.hndl);

	return ret;
}

int orange_thread_create_ex(struct orange_thread_handle* hndl, orange_thread_entry_func thread_func, void* paramter)
{
	int							pri = 0;
	struct orange_thread_handle hthr;

	if (orange_thread_get_self_thread_handle(&hthr)) {
		pri = orange_thread_get_thread_pri(&hthr);
		if (0 > pri) {
			pri = 1;
		}
	} else {
		pri = 1;
	}

	return __orange_thread_create_ex(hndl, thread_func, paramter, pri, 0);
}

int orange_thread_cond_event_init(struct orange_cond_event_handle_tag* cond_event)
{
	int ret = -1;

	pthread_condattr_t  cond_attr;
	pthread_mutexattr_t mutex_attr;

	if (NULL == cond_event) {
		goto exit;
	}

	cond_event->initialized	= -1;
	cond_event->spuriouswakeup = 0;

	ret = pthread_condattr_init(&cond_attr);
	if (0 != ret) {
		goto destroy;
	}

	ret = pthread_mutexattr_init(&mutex_attr);
	if (0 != ret) {
		goto destroy;
	}

	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);

	ret = pthread_mutex_init(&(cond_event->mutex), &mutex_attr);
	if (0 != ret) {
		goto destroy;
	}

	ret = pthread_cond_init(&(cond_event->cond), &cond_attr);
	if (0 != ret) {
		goto destroy;
	}

	cond_event->initialized = 0;

destroy:
	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

exit:
	return ret;
}

int orange_thread_cond_event_de_init(struct orange_cond_event_handle_tag* cond_event)
{
	int ret = -1;

	if (NULL == cond_event) {
		goto exit;
	}

	pthread_mutex_lock(&(cond_event->mutex));
	if (0 != ret) {
		goto exit;
	}

	cond_event->spuriouswakeup = 0;
	ret						   = pthread_cond_destroy(&(cond_event->cond));
	if (0 != ret) {
		orange_log_error("%s:%d cond destroy error.\n", __func__, __LINE__);
	}
	cond_event->initialized = 1;

	ret = pthread_mutex_unlock(&(cond_event->mutex));
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_mutex_destroy(&(cond_event->mutex));
	if (0 != ret) {
		orange_log_error("%s:%d mutex destroy error.\n", __func__, __LINE__);
	}

exit:
	return ret;
}

int orange_thread_set_cond_event(struct orange_cond_event_handle_tag* cond_event)
{
	int ret = -1;

	if (NULL == cond_event) {
		goto exit;
	}

	ret = pthread_mutex_lock(&(cond_event->mutex));
	if (0 != ret) {
		goto exit;
	}

	if (!cond_event->initialized) {
		goto unlock;
	}

	ret = pthread_cond_signal(&(cond_event->cond));
	if (0 == ret) {
		cond_event->spuriouswakeup = -1;
	}

unlock:
	ret = pthread_mutex_unlock(&(cond_event->mutex));

exit:
	if (0 == ret) {
		orange_thread_sleep(0);
	}

	return ret;
}

int orange_thread_wait_cond_event(struct orange_cond_event_handle_tag* cond_event, uint32_t timeout_ms)
{
	return 0;
}

int orange_thread_is_cond_event_trig(struct orange_cond_event_handle_tag* cond_event)
{
	return 0;
}

void orange_thread_sleep(uint32_t sleep_ms)
{
	if (0 == sleep_ms) {
		usleep(0);
		return;
	}

	if (sleep_ms < 1000) {
		usleep(sleep_ms * 1000);
	} else {
		if ((sleep_ms / 1000) > 0) {
			sleep(sleep_ms / 1000);
		}

		if (sleep_ms % 1000) {
			usleep((sleep_ms % 1000) * 1000 - ORANGE_SLEEP_CAL_VAL);
		}
	}

	return;
}
