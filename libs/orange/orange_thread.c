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

	return orange_thread_create_ex(thread, thread_func, pri, paramter);
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

int orange_CondEventInit(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE)
{
	INT32 iRet = orange_SOK;

	pthread_condattr_t  cond_attr;
	pthread_mutexattr_t mutex_attr;

	if (NULL == porange_COND_EVENT_HANDLE) {
		orange_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		return iRet;
	}

	porange_COND_EVENT_HANDLE->bInitialized	= FALSE;
	porange_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;

	iRet = pthread_condattr_init(&cond_attr);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_condattr_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	iRet = pthread_mutexattr_init(&mutex_attr);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutexattr_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

#ifdef __linux__
	// ???? recursive ????(??????)
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif

	iRet = pthread_mutex_init(&(porange_COND_EVENT_HANDLE->stMutex), &mutex_attr);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	iRet = pthread_cond_init(&(porange_COND_EVENT_HANDLE->stCond), &cond_attr);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_cond_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	porange_COND_EVENT_HANDLE->bInitialized = TRUE;

__EXIT__:
	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	if (orange_SOK == iRet) {
		orange_DBG_MSG("orange_CondEventInit call successfully!");
	}

	return iRet;
}

INT32 orange_CondEventDeInit(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE)
{
	INT32 iRet = orange_SOK;

	if (NULL == porange_COND_EVENT_HANDLE) {
		orange_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		goto __EXIT__;
	}

	iRet = pthread_mutex_lock(&(porange_COND_EVENT_HANDLE->stMutex));
	orange_DBG_MSGXX("porange_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int*) &(porange_COND_EVENT_HANDLE->stMutex),
					 (int) &porange_COND_EVENT_HANDLE->stMutex);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	porange_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;
	iRet									   = pthread_cond_destroy(&(porange_COND_EVENT_HANDLE->stCond));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_cond_destroy fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}
	porange_COND_EVENT_HANDLE->bInitialized = FALSE;

	iRet = pthread_mutex_unlock(&(porange_COND_EVENT_HANDLE->stMutex));
	orange_DBG_MSGXX("porange_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int*) &(porange_COND_EVENT_HANDLE->stMutex),
					 (int) &porange_COND_EVENT_HANDLE->stMutex);
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	iRet = pthread_mutex_destroy(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_destroy fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}

__EXIT__:
	return iRet;
}

INT32 orange_SetCondEvent(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE)
{
	INT32 iRet = orange_SOK;

	if (NULL == porange_COND_EVENT_HANDLE) {
		orange_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		goto __EXIT__;
	}
	iRet = pthread_mutex_lock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	if (!porange_COND_EVENT_HANDLE->bInitialized) {
		orange_ERROR("porange_COND_EVENT_HANDLE->stCond is DESTROYED");
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__1__;
	}

	iRet = pthread_cond_signal(&(porange_COND_EVENT_HANDLE->stCond));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_cond_signal fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	} else {
		porange_COND_EVENT_HANDLE->bSpuriouswakeup = FALSE; // ?ڴ˱???Ϊ ????????λ
	}

__EXIT__1__:

	iRet = pthread_mutex_unlock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}

__EXIT__:
	if (orange_SOK == iRet) {
		orange_DBG_MSG("orange_SetCondEvent call successfully!");
		orange_Sleep(0); //?ڴ˽����?ǰ?????̵߳?CPUʱ??Ƭ?ó?
	}

	orange_DBG_MSGXX("aaa");
	return iRet;
}

INT32 orange_WaitCondEvent(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE, INT32 iTimeOutMilliSec)
{
	INT32			iRet	= orange_SOK;
	INT32			iResult = orange_SOK;
	struct timespec waittime;
	TIME_VAL		stTimeVal = {0};
	uint32_t		uTemp;

	if ((NULL == porange_COND_EVENT_HANDLE)) {
		orange_ERROR("Handle Invalid !");
		iRet = ERR_INVALID_PARAM;

		return iRet;
	}

	if (iTimeOutMilliSec < (INT32) orange_TIMEOUT_FOREVER) {
		orange_ERROR("Invalid time out value!");
		iRet = ERR_INVALID_PARAM;

		return iRet;
	}

	iRet = pthread_mutex_lock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iResult = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	switch (iTimeOutMilliSec) {
		case orange_TIMEOUT_FOREVER: //?��õȴ?
		{
			while (1) //! porange_COND_EVENT_HANDLE->bSpuriouswakeup)
			{		  // ???? Spurious wakeup
				iRet = pthread_cond_wait(&(porange_COND_EVENT_HANDLE->stCond), &(porange_COND_EVENT_HANDLE->stMutex));

				if (orange_SOK != iRet) {
					orange_ERROR("Call System API pthread_cond_wait return val=%d", iRet);
					iResult = ERR_SYS_API_CALLFAIL;
					goto __EXIT__;
				} else {
					if (FALSE == porange_COND_EVENT_HANDLE->bSpuriouswakeup) { // ?????Ƿ?Ϊ ?ٻ???
						iResult = orange_SOK;
						break;
					} else { //?ٻ???
						continue;
					}
				}
			}

			if (orange_SOK != iRet) {
				orange_ERROR("pthread_cond_wait return val=%d", iRet);
				iResult = ERR_SYS_API_CALLFAIL;
			} else {
				iResult = orange_SOK;
			}
		} break;

		case orange_TIMEOUT_NONE: // 0 ?ȴ?
		{
			orange_DBG_MSG("orange_WaitCondEvent wait 0 ms...");
		} break;

		default: //??????ʱֵ
		{
			if (!orange_TimeGetCurVal(&stTimeVal)) { //??ȡ??ǰʱ??value
				iResult = ERR_SYS_API_CALLFAIL;
				orange_ERROR("Unexpect error...");
				break;
			}

			uTemp			 = stTimeVal.uMilliSec + (iTimeOutMilliSec % 1000);			 // ms?????ۼ?ֵ
			waittime.tv_sec  = stTimeVal.uSecs + iTimeOutMilliSec / 1000 + uTemp / 1000; //???ǵ?ms?????ۼ?????
			waittime.tv_nsec = (uTemp % 1000) * 1000000 + stTimeVal.uMicroSec * 1000;	//ʣ??ms????ת??Ϊ ns

			orange_DBG_MSG("timeout is %u, wait time=%u.%u", iTimeOutMilliSec, (uint32_t) waittime.tv_sec, (uint32_t) waittime.tv_nsec);

			while (1) // porange_COND_EVENT_HANDLE->bSpuriouswakeup)
			{		  // ???? Spurious wakeup
				iRet = pthread_cond_timedwait(&(porange_COND_EVENT_HANDLE->stCond), &(porange_COND_EVENT_HANDLE->stMutex), &waittime);
				if (ETIMEDOUT == iRet) { //??ʱ
					iResult = ERR_COND_EVENT_WAIT_TIMEOUT;
					break;
				} else {
					if (orange_SOK == iRet) //?ڳ?ʱʱ???ڵõ? singal
					{
						if (FALSE == porange_COND_EVENT_HANDLE->bSpuriouswakeup) { // ?????Ƿ?Ϊ ?ٻ???
							iResult = orange_SOK;
							break;
						} else { //?ٻ???
							continue;
						}
					} else { //????????
						orange_ERROR("call pthread_cond_timedwait fail... error=%d", iRet);
						iResult = ERR_SYS_API_CALLFAIL;

						goto __EXIT__;
					}
				}
			}

		} break;
	}

	porange_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE; //??Ҫ???ñ?־??λ

__EXIT__:
	iRet = pthread_mutex_unlock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iResult = ERR_SYS_API_CALLFAIL;
	}

	if (ERR_COND_EVENT_WAIT_TIMEOUT == iResult) { //??ʱҪ?ص㷵?ظ???????
		return iResult;
	}

	return iResult;
}

BOOL orange_IsCondEventTrig(orange_COND_EVENT_HANDLE* porange_COND_EVENT_HANDLE)
{
	BOOL  bRet = FALSE;
	INT32 iRet = orange_SOK;

	if (NULL == porange_COND_EVENT_HANDLE) {
		orange_ERROR("Invalid parameter...");
		goto __EXIT__;
	}

	iRet = pthread_mutex_lock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		bRet = FALSE;

		goto __EXIT__;
	}

	//???? ?Ƿ?Ϊ?ٻ???
	bRet = !(porange_COND_EVENT_HANDLE->bSpuriouswakeup);

	iRet = pthread_mutex_unlock(&(porange_COND_EVENT_HANDLE->stMutex));
	if (orange_SOK != iRet) {
		orange_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
	}

__EXIT__:
	return bRet;
}
