#include "orange_thread.h"

void orange_thread_sleep(uint32_t ms)
{
#define __SLEEP_CAL_VAL__ 1000
	if (0 == ms) {
		usleep(0);
		return;
	}
	if (ms < 1000) {
		usleep(ms * 1000);
	} else {
		if ((ms / 1000) > 0) {
			sleep(ms / 1000);
		}

		if (ms % 1000) {
			usleep((ms % 1000) * 1000 - __SLEEP_CAL_VAL__);
		}
	}

	return;
}

static int __orange_thread_create(pthread_t* thread, pthread_attr_t* attr, orange_thread_entry_func thread_func, void* arg)
{
	return pthread_create(thread, attr, thread_func, arg);
}

static int __orange_thread_join(pthread_t thread, void** thread_return)
{
	return pthread_join(thread, thread_return);
}

int orange_thread_join(orange_thread_handle_t* hndl)
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

	pthread_join(hndl->hndl, &return_val);

	ret = 0;
exit:
	return ret;
}

int orange_thread_delete(orange_thread_handle_t* hndl)
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

	return ret;
}

int orange_thread_change_pri(orange_thread_handle_t* hndl, uint32_t pri)
{
	int				   ret = -1;
	struct sched_param schedprm;

	if (pri > (uint32_t) ORANGE_THR_PRI_RR_MAX) {
		pri = ORANGE_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) ORANGE_THR_PRI_RR_MIN) {
		pri = ORANGE_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
	ret						= pthread_setschedparam(hndl->hndl, SCHED_RR, &schedprm);

	return ret;
}

void orange_thread_change_self_pri(uint32_t pri)
{
	orange_thread_handle_t self_hndl;
	orange_thread_get_self_handle(&self_hndl);
	orange_thread_change_pri(&self_hndl, pri);
	return;
}

int orange_thread_exit(void* value)
{
	pthread_exit(value);
	return 0;
}

int orange_thread_get_self_handle(orange_thread_handle_t* hndl)
{
	int ret = 0;

	if ((NULL == hndl)) {
		ret = -1;
		goto exit;
	}

	hndl->hndl = pthread_self();
	if ((hndl->hndl == ORANGE_THREAD_HANDLE_INVLALID) || (hndl->hndl == 0)) {
		ret = -1;
	}

exit:
	return ret;
}

int orange_thread_set_pri(orange_thread_handle_t* thread, int new_pri)
{
	int				   ret = -1;
	int				   policy;
	struct sched_param sched_param;

	if ((NULL == thread)) {
		goto exit;
	}

	new_pri = (new_pri < 0 /*ORANGE_THR_PRI_MIN*/) ? 0 /*ORANGE_THR_PRI_MIN*/ : pri;
	new_pri = (new_pri > ORANGE_THR_PRI_RR_MAX) ? ORANGE_THR_PRI_RR_MAX : pri;

	memset(&sched_param, 0, sizeof(struct sched_param));
	ret = pthread_getschedparam(thread->hndl, &policy, &sched_param);
	if (0 != ret) {
		goto exit;
	}

	sched_param.sched_priority = new_pri;

	ret = pthread_setschedparam(thread->hndl, policy, &sched_param);

exit:
	return ret;
}

int orange_thread_get_pri(orange_thread_handle_t* thread)
{
	int				   ret = -1;
	int				   policy;
	struct sched_param sched_param;

	if ((NULL == thread)) {
		goto exit;
	}

	memset(&sched_param, 0, sizeof(struct sched_param));
	ret = pthread_getschedparam(thread->hndl, &policy, &sched_param);
	if (0 != ret) {
		ret = -1;
	} else {
		ret = sched_param.sched_priority;
	}

exit:
	return iRet;
}

int orange_thread_get_stack_size(void)
{
	pthread_attr_t thread_attr;
	int			   ret  = -1;
	int			   size = 0;

	int ret = pthread_attr_init(&thread_attr);
	if (0 != ret) {
		ret = -1;
		goto exit;
	}

	ret = pthread_attr_getstacksize(&thread_attr, (size_t*) &size);
	if (0 != ret) {
		ret = -1;
		goto exit;
	}

	ret = size;
exit:
	return ret;
}

int orange_thread_set_stack_size(uint32_t size)
{
	pthread_attr_t thread_attr;
	int			   ret = 0;

	int ret = pthread_attr_init(&thread_attr);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_attr_setstacksize(&thread_attr, (int) size);

exit:
	return ret;
}

int orange_thread_detach(orange_thread_handle_t* hndl)
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

int orange_thread_compare(orange_thread_handle_t* a, orange_thread_handle_t* b)
{
	orange_thread_handle_t tmp;
	int					   ret = -1;

	if ((NULL == a)) {
		goto exit;
	}

	tmp.hndl = (NULL == b) ? pthread_self() : b->hndl;

	ret = pthread_equal(a->hndl, tmp.hndl);

	return ret;
}

int orange_thread_create(orange_thread_handle_t* thread, orange_thread_entry_func thread_func, void* paramter)
{
	int					   pri = 0;
	orange_thread_handle_t tmp;

	if (orange_thread_get_self_handle(&tmp)) {
		pri = orange_thread_get_pri(&tmp);
		if (-1 == pri) {
			pri = 1;
		}
	} else {
		pri = 1;
	}

	return orange_thread_create_ex(thread, thread_func, paramter, pri, 0);
}

int orange_thread_create_ex(orange_thread_handle_t* thread, orange_thread_entry_func thread_func, void* paramter, int pri, int system_scope)
{
	int ret = -1;

	pthread_attr_t	 attr;
	struct sched_param param;

	if ((NULL == thread) || (NULL == thread_func)) {
		orange_log_error("%s;%d parameter error.\n", __func__, __LINE__);
		return -1;
	}

	ret = pthread_attr_init(&attr);
	if (0 != ret) {
		goto exit;
	}

	ret |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
	if (0 != ret) {
		goto exit;
	}

	pri = MAX(pri, orange_THR_PRI_RR_MIN);
	pri = MIN(pri, orange_THR_PRI_RR_MAX);

	memset(param, 0, sizeof(struct sched_param));
	ret |= pthread_attr_getschedparam(&attr, &param);

	param.sched_priority = pri;
	ret |= pthread_attr_setschedparam(&attr, &param);
	if (0 != ret) {
		goto exit;
	}

	ret |= pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (0 != ret) {
		goto exit;
	}

	if (system_scope) {
		ret |= pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
		if (0 != ret) {
			goto exit;
		}
	}

	ret = __orange_thread_create(&(thread->hndl), &attr, (void*) thread_func, paramter);

exit:
	pthread_attr_destroy(&attr);
	return ret;
}

int orange_thread_cond_event_init(orange_cond_event_handle_t* handle)
{
	int ret = -1;

	pthread_condattr_t  cond_attr;
	pthread_mutexattr_t mutex_attr;

	if (NULL == handle) {
		return ret;
	}

	handle->initialized		= 0;
	handle->spurious_wakeup = 1;

	ret = pthread_condattr_init(&cond_attr);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_mutexattr_init(&mutex_attr);
	if (0 != ret) {
		goto exit;
	}

	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);

	ret = pthread_mutex_init(&(handle->mutex), &mutex_attr);
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_cond_init(&(handle->cond), &cond_attr);
	if (0 != ret) {
		goto exit;
	}

	handle->initialized = 1;

exit:
	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	return ret;
}

int orange_thread_cond_event_de_init(orange_cond_event_handle_t* handle)
{
	int ret = -1;

	if (NULL == handle) {
		goto exit;
	}

	ret = pthread_mutex_lock(&(handle->mutex));
	if (0 != ret) {
		goto exit;
	}

	handle->spurious_wakeup = 1;
	ret						= pthread_cond_destroy(&(handle->cond));
	if (0 != ret) {
		goto exit;
	}
	handle->initialized = 0;

	ret = pthread_mutex_unlock(&(handle->mutex));
	if (0 != ret) {
		goto exit;
	}

	ret = pthread_mutex_destroy(&(handle->mutex));

exit:
	return ret;
}

int orange_thread_set_cond_event(orange_cond_event_handle_t* handle)
{
	int ret = -1;

	if (NULL == handle) {
		goto exit;
	}

	ret = pthread_mutex_lock(&(handle->mutex));
	if (0 != ret) {
		goto exit;
	}

	if (!handle->initialized) {
		goto unlock;
	}

	ret = pthread_cond_signal(&(handle->cond));
	if (0 == ret) {
		handle->spurious_wakeup = 0;
	}

unlock:
	ret = pthread_mutex_unlock(&(handle->mutex));

exit:
	if (0 == ret) {
		orange_Sleep(0);
	}

	return ret;
}

int orange_thread_wait_cond_event(orange_cond_event_handle_t* handle, int ms)
{
	int				ret = -1;
	struct timespec waittime;
	uint32_t		tmp;
	struct timeval  time_val;

	if ((NULL == handle)) {
		return ret;
	}

	if (ms < (int) ORANGE_TIMEOUT_FOREVER) {
		return ret;
	}

	ret = pthread_mutex_lock(&(handle->mutex));
	if (0 != ret) {
		goto exit;
	}

	switch (ms) {
		case ORANGE_TIMEOUT_FOREVER: {
			while (1) {
				ret = pthread_cond_wait(&(handle->cond), &(handle->mutex));
				if (0 != ret) {
					goto exit;
				} else {
					if (!handle->spurious_wakeup) {
						break;
					} else {
						continue;
					}
				}
			}
		} break;

		case ORANGE_TIMEOUT_NONE: {
		} break;

		default: {
			gettimeofday(&time_val, NULL);
			tmp				 = (tv_sec.tv_usec / 1000) + (ms % 1000);
			waittime.tv_sec  = tv_sec.tv_sec + ms / 1000 + tmp / 1000;
			waittime.tv_nsec = (tmp % 1000) * 1000000 + (tv_sec.tv_usec % 1000) * 1000;

			while (1) {
				ret = pthread_cond_timedwait(&(handle->cond), &(handle->mutex), &waittime);
				if (ETIMEDOUT == ret) {
					break;
				} else {
					if (0 == ret) {
						if (!handle->spurious_wakeup) {
							break;
						} else {
							continue;
						}
					} else {
						goto exit;
					}
				}
			}

		} break;
	}

	handle->spurious_wakeup = 1;

exit:
	pthread_mutex_unlock(&(handle->mutex));
	return ret;
}

int orange_thread_is_cond_event_trig(orange_cond_event_handle_t* handle)
{
	int ret = -1;
	int sw  = 0;

	if (NULL == handle) {
		goto exit;
	}

	ret = pthread_mutex_lock(&(handle->mutex));
	if (0 != ret) {
		goto exit;
	}

	sw = !(handle->spurious_wakeup);

	pthread_mutex_unlock(&(handle->mutex));
exit:
	return sw;
}
