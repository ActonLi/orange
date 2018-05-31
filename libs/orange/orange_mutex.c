#include "orange_mutex.h"
#include "orange_log.h"

int orange_mutex_create(struct orange_mutex_tag* handle)
{
	pthread_mutexattr_t mutex_attr;
	int					ret = 0;

	ret |= pthread_mutexattr_init(&mutex_attr);
	ret |= pthread_mutex_init(&handle->lock, &mutex_attr);

	if (ret != 0) {
		orange_log_error("lock init error.\n");
	}

	pthread_mutexattr_destroy(&mutex_attr);

	return ret;
}

int orange_mutex_create_ex(struct orange_mutex_tag* handle, uint8_t mutex_type)
{
	pthread_mutexattr_t mutex_attr;
	int					ret = 0;
	int					type;

	if ((NULL == handle) || (mutex_type >= NUTEX_MAX)) {
		orange_log_error("Invalid parameter...");
		ret = -1;
		goto exit;
	}

	ret |= pthread_mutexattr_init(&mutex_attr);
	ret |= pthread_mutex_init(&handle->lock, &mutex_attr);

	if (ret != 0) {
		orange_log_error("%s() = %d \r\n", __func__, ret);
		goto exit;
	}

	switch (mutex_type) {
		case MUTEX_ERROR_CHECK:
			type = PTHREAD_MUTEX_ERRORCHECK_NP;
			break;

		case MUTEX_RECURSIVE:
			type = PTHREAD_MUTEX_RECURSIVE_NP;
			break;

		case MUTEX_ADAPTIVE:
			type = PTHREAD_MUTEX_ADAPTIVE_NP;
			break;

		case MUTEX_NORMAL:
		/*for pc lint though*/
		default:
			type = PTHREAD_MUTEX_TIMED_NP;
			break;
	}

	ret |= pthread_mutexattr_settype(&mutex_attr, type);
	if (ret != 0) {
		orange_log_error("Create mutex fail...Call SYSTEM API pthread_mutexattr_settype fail, error = %d", ret);
	}

	pthread_mutexattr_destroy(&mutex_attr);

exit:
	return ret;
}

int orange_mutex_delete(struct orange_mutex_tag* handle)
{
	return pthread_mutex_destroy(&handle->lock);
}
int orange_mutex_lock(struct orange_mutex_tag* handle)
{
	return pthread_mutex_lock(&handle->lock);
}

int orange_mutex_unlock(struct orange_mutex_tag* handle)
{
	return pthread_mutex_unlock(&handle->lock);
}

int orange_mutex_lock_ex(struct orange_mutex_tag* handle)
{
	if (NULL == handle) {
		orange_log_error("Invalid parameter...");
		return -1;
	}

	int ret = pthread_mutex_trylock(&handle->lock);

	return (0 == ret);
}
