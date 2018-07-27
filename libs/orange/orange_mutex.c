#include "orange_mutex.h"

int orange_mutex_create(orange_mutex_t *hndl)
{
	pthread_mutexattr_t mutex_attr;
	int ret = 0;

	ret |= pthread_mutexattr_init(&mutex_attr);
	ret |= pthread_mutex_init(&hndl->lock, &mutex_attr);

	pthread_mutexattr_destroy(&mutex_attr);

	return ret;
}

int orange_mutex_create_ex(orange_mutex_t *hndl, orange_mutex_type_t mutex_type)
{
    pthread_mutexattr_t mutex_attr;
    int ret = 0;
    int type;

    if((NULL == hndl) || (mutex_type >= ORANGE_MAX)) {
        goto exit;
    }

    ret |= pthread_mutexattr_init(&mutex_attr);
    ret |= pthread_mutex_init(&hndl->lock, &mutex_attr);
    if(ret != 0) {
        goto destroy;
    }

    switch(mutex_type) {
	    case ORANGE_ERRORCHECK:
	        type = PTHREAD_MUTEX_ERRORCHECK_NP;
        break;

	    case ORANGE_RECURSIVE:
	        type = PTHREAD_MUTEX_RECURSIVE_NP;
        break;

		case ORANGE_ADAPTIVE:
			type = PTHREAD_MUTEX_ADAPTIVE_NP;
		break;

	    case ORANGE_MUTEX_NORMAL:
	    default:
	        type = PTHREAD_MUTEX_TIMED_NP;
        break;
    }

    ret |= pthread_mutexattr_settype(&mutex_attr, type);

destroy:
    pthread_mutexattr_destroy(&mutex_attr);

exit:
    return ret;
}

int orange_mutex_delete(orange_mutex_t *hndl)
{
	return pthread_mutex_destroy(&hndl->lock);
}

int orange_mutex_lock(orange_mutex_t *hndl)
{
	return pthread_mutex_lock(&hndl->lock);
}

int orange_mutex_unlock(orange_mutex_t *hndl)
{
	return pthread_mutex_unlock(&hndl->lock);
}

int orange_mutex_lock_ex(orange_mutex_t *hndl)
{
    if(NULL == hndl) {
        return -1;
    }

    return pthread_mutex_trylock(&hndl->lock);
}


