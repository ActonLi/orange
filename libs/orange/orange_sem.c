#include "orange_sem.h"

int orange_sem_create(struct orange_sem_hanle* hndl, uint32_t max_count, uint32_t init_val)
{
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t  cond_attr;

	pthread_mutexattr_init(&mutex_attr);
	pthread_condattr_init(&cond_attr);

	pthread_mutex_init(&hndl->lock, &mutex_attr);
	pthread_cond_init(&hndl->cond, &cond_attr);

	hndl->count		= init_val;
	hndl->max_count = max_count;

	if (hndl->max_count == 0) {
		hndl->max_count = 1;
	}

	if (hndl->count > hndl->max_count) {
		hndl->count = hndl->max_count;
	}

	return 0;
}

int orange_sem_wait(struct orange_sem_hanle* hndl, uint32_t timeout)
{
	int ret = -1;

	pthread_mutex_lock(&hndl->lock);
	while (1) {
		if (hndl->count > 0) {
			hndl->count--;
			ret = 0;
			break;
		} else {
			if (timeout == ORANGE_TIMEOUT_NONE) {
				break;
			}
			pthread_cond_wait(&hndl->cond, &hndl->lock);
		}
	}
	pthread_mutex_unlock(&hndl->lock);

	return ret;
}

int orange_sem_timed_wait(struct orange_sem_hanle* hndl, uint32_t timeoutms)
{
	int				ret = -1;
	struct timespec sttime;
	struct timeval  stcurtime = {0};
	uint32_t		ms;

	sttime.tv_sec  = timeoutms / 1000;
	ms			   = timeoutms - (uint32_t) sttime.tv_sec * 1000;
	sttime.tv_nsec = (long) ms * 1000 * 1000;
	gettimeofday(&stcurtime, NULL);
	sttime.tv_nsec = ((stcurtime.tv_usec + timeoutms * 1000) % 1000000) * 1000;
	sttime.tv_sec  = stcurtime.tv_sec + (stcurtime.tv_usec + timeoutms * 1000) / 1000000;

	pthread_mutex_lock(&hndl->lock);
	while (1) {
		if (hndl->count > 0) {
			hndl->count--;
			ret = 0;
			break;
		} else {
			pthread_cond_timedwait(&hndl->cond, &hndl->lock, &sttime);
			break;
		}
	}
	pthread_mutex_unlock(&hndl->lock);

	return ret;
}
int orange_sem_try_wait(struct orange_sem_hanle* hndl)
{
	int ret = -1;

	pthread_mutex_lock(&hndl->lock);
	while (1) {
		if (hndl->count > 0) {
			ret = 0;
			break;
		} else {
			pthread_cond_wait(&hndl->cond, &hndl->lock);
		}
	}
	pthread_mutex_unlock(&hndl->lock);

	return ret;
}
int orange_sem_signal(struct orange_sem_hanle* hndl, uint32_t count)
{
	int ret = -1;

	pthread_mutex_lock(&hndl->lock);

	if ((count < hndl->max_count) && (count > 0)) {
		hndl->count = count;
		pthread_cond_signal(&hndl->cond);
		ret = 0;
	}

	pthread_mutex_unlock(&hndl->lock);

	return ret;
}

int orange_sem_delete(struct orange_sem_hanle* hndl)
{
	pthread_cond_destroy(&hndl->cond);
	pthread_mutex_destroy(&hndl->lock);

	return 0;
}

int orange_sem_set_count(struct orange_sem_hanle* hndl, uint32_t count)
{
	int ret = -1;

	pthread_mutex_lock(&hndl->lock);

	if (count < hndl->max_count) {
		hndl->count = count;
		ret			= 0;
	}

	pthread_mutex_unlock(&hndl->lock);

	return ret;
}
