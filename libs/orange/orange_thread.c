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

int orange_thread_create(orange_thread_handle_t* hndl, orange_thread_entry_func thread_func, int pri, void* prm)
{
	int				   iStatus = OSA_SOK;
	pthread_attr_t	 thread_attr;
	struct sched_param schedprm;

	iStatus = pthread_attr_init(&thread_attr);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		return iStatus;
	}

	iStatus |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
#ifdef __linux__ // cygwin only support SCHED_FIFO
	iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
#else
	iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
#endif
	if (pri > (uint32_t) OSA_THR_PRI_RR_MAX) {
		pri = OSA_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) OSA_THR_PRI_RR_MIN) {
		pri = OSA_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
	iStatus |= pthread_attr_setschedparam(&thread_attr, &schedprm);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		goto error_exit;
	}

	iStatus |= pthread_create(&hndl->hndl, &thread_attr, thread_func, prm);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not create thread [%d]\n", iStatus);
	}

error_exit:
	pthread_attr_destroy(&thread_attr);

	return iStatus;
}

int OSA_ThrCreateEx(orange_thread_handle_t* hndl, orange_thread_entry_func thread_func, int pri, void* prm, char* pFunc, int line)
{
	int				   iStatus = OSA_SOK;
	pthread_attr_t	 thread_attr;
	struct sched_param schedprm;

	// initialize thread attributes structure
	iStatus = pthread_attr_init(&thread_attr);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		return iStatus;
	}

	iStatus |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
#ifdef __linux__ // cygwin only support SCHED_FIFO
	iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
#else
	iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
#endif

	if (pri > (uint32_t) OSA_THR_PRI_RR_MAX) {
		pri = OSA_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) OSA_THR_PRI_RR_MIN) {
		pri = OSA_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
	iStatus |= pthread_attr_setschedparam(&thread_attr, &schedprm);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		goto error_exit;
	}

	iStatus = __orange_thread_create(&hndl->hndl, &thread_attr, thread_func, prm);

	if (OSA_SOK != iStatus) {
		OSA_ERROR("OSA_ThrCreate() - Could not create thread [%d]\n", iStatus);
	}

error_exit:
	pthread_attr_destroy(&thread_attr);

	return iStatus;
}

int OSA_ThrJoin(orange_thread_handle_t* hndl)
{
	int   iStatus = OSA_SOK;
	void* returnVal;

	if (NULL == hndl) {
		OSA_ERROR("Invalid parameter...");
		return OSA_EFAIL;
	}

	if (OSA_THREAD_HANDLE_INVLALID == hndl->hndl) {
		OSA_ERROR("Invalid pthread handle...");
		return OSA_EFAIL;
	}

	if (OSA_ThrCompare(hndl, NULL)) {
		OSA_ERROR("Join in the same thread...");
		return OSA_EFAIL;
	}

	// iStatus = __orange_thread_join(hndl->hndl, &returnVal);
	iStatus = pthread_join(hndl->hndl, &returnVal);

	if (EINVAL == iStatus) {
		OSA_ERROR("The target thread 0x%x is Detach", (int) (hndl->hndl));
		iStatus = OSA_SOK;
	}

	return iStatus;
}

int OSA_ThrJoinEx(orange_thread_handle_t* hndl, char* pFunc, int line)
{
	int   iStatus = OSA_SOK;
	void* returnVal;

	if (NULL == hndl) {
		OSA_ERROR("Invalid parameter...");
		return OSA_EFAIL;
	}

	if (OSA_THREAD_HANDLE_INVLALID == hndl->hndl) {
		OSA_ERROR("Invalid pthread handle...");
		return OSA_EFAIL;
	}

	if (OSA_ThrCompare(hndl, NULL)) {
		OSA_ERROR("Join in the same thread...");
		return OSA_EFAIL;
	}

	iStatus = __orange_thread_join(hndl->hndl, &returnVal);

	if (EINVAL == iStatus) {
		OSA_ERROR("The target thread 0x%x is Detach", (int) (hndl->hndl));
		iStatus = OSA_SOK;
	}

	//    OSA_DBG_MSG(" %s iStatus = %d", __func__ , iStatus);

	return iStatus;
}

int OSA_ThrDelete(orange_thread_handle_t* hndl)
{
	int iStatus = OSA_SOK;

	if (NULL == hndl) {
		OSA_ERROR("Invalid parameter...");
		return OSA_EFAIL;
	}

	if (OSA_THREAD_HANDLE_INVLALID == hndl->hndl) {
		OSA_ERROR("Invalid pthread handle...");
		return OSA_EFAIL;
	}

	iStatus |= pthread_cancel(hndl->hndl);
	OSA_DBG_MSG(" %s cancel 0x%x handle iStatus = %d", __func__, (int) hndl->hndl, iStatus);

	iStatus |= OSA_ThrJoin(hndl);
	OSA_DBG_MSG(" %s cancel 0x%x handle iStatus = %d", __func__, (int) hndl->hndl, iStatus);

	return iStatus;
}

int OSA_ThrChangePri(orange_thread_handle_t* hndl, uint32_t pri)
{
	int				   iStatus = OSA_SOK;
	struct sched_param schedprm;

	if (pri > (uint32_t) OSA_THR_PRI_RR_MAX) {
		pri = OSA_THR_PRI_RR_MAX;
	} else if (pri < (uint32_t) OSA_THR_PRI_RR_MIN) {
		pri = OSA_THR_PRI_RR_MIN;
	}

	schedprm.sched_priority = pri;
#ifdef __linux__ // cygwin only support SCHED_FIFO
	iStatus |= pthread_setschedparam(hndl->hndl, SCHED_RR, &schedprm);
#else
	iStatus |= pthread_setschedparam(hndl->hndl, SCHED_FIFO, &schedprm);
#endif
	return iStatus;
}

void OSA_ThrChangeSelfPri(uint32_t pri)
{
	orange_thread_handle_t pSelfThrHndl;
	OSA_GetSelfThrHandle(&pSelfThrHndl);
	OSA_ThrChangePri(&pSelfThrHndl, pri);
}

int OSA_ThrExit(void* returnVal)
{
	pthread_exit(returnVal);
	return OSA_SOK;
}

BOOL OSA_GetSelfThrHandle(orange_thread_handle_t* pSelfThrHndl)
{
	if ((NULL == pSelfThrHndl)) {
		OSA_ERROR("Invalid paremeter 1");
		return FALSE;
	}

	pSelfThrHndl->hndl = pthread_self();
	if ((pSelfThrHndl->hndl == OSA_THREAD_HANDLE_INVLALID) || (pSelfThrHndl->hndl == 0)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

BOOL OSA_SetThrPri(orange_thread_handle_t* pThr, INT iNewPri)
{
	BOOL			   bRet = FALSE;
	int				   iRet = ERR_FAIL;
	int				   iPolicy;
	struct sched_param stSched_param = {0};

	if ((NULL == pThr)) {
		OSA_ERROR("Invalid paremeter 1");
		return bRet;
	}

	iNewPri = (iNewPri < 0 /*OSA_THR_PRI_MIN*/) ? 0 /*OSA_THR_PRI_MIN*/ : iNewPri;
	iNewPri = (iNewPri > OSA_THR_PRI_RR_MAX) ? OSA_THR_PRI_RR_MAX : iNewPri;

	iRet = pthread_getschedparam(pThr->hndl, &iPolicy, &stSched_param);
	if (OSA_SOK != iRet) //????
	{
		OSA_ERROR("call pthread_getschedparam fail... error=%d", iRet);
		bRet = FALSE;
		return bRet;
	}

	OSA_DBG_MSG("Get current pri=%d", stSched_param.sched_priority);

	stSched_param.sched_priority = iNewPri;

	OSA_DBG_MSG("the new pri=%d, Policy=%d", iNewPri, iPolicy);

	iRet = pthread_setschedparam(pThr->hndl, iPolicy, &stSched_param);

	if (OSA_SOK != iRet) { //????
		OSA_ERROR("call pthread_setschedparam fail... error=%d", iRet);
		switch (iRet) {
			case EINVAL:
				OSA_DBG_MSG("error = EINVAL");
				break;

			case ENOTSUP:
				OSA_DBG_MSG("error = ENOTSUP");
				break;

			default:
				OSA_DBG_MSG("error = Unknow");
				break;
		}
		bRet = FALSE;
	} else {
		bRet = TRUE;
	}

	return bRet;
}

INT OSA_GetThrPri(orange_thread_handle_t* pThr)
{
	int				   iRet = ERR_FAIL;
	int				   iPolicy;
	struct sched_param stSched_param = {0};

	if ((NULL == pThr)) {
		OSA_ERROR("Invalid paremeter...");
		return iRet;
	}

	iRet = pthread_getschedparam(pThr->hndl, &iPolicy, &stSched_param);

	if (OSA_SOK != iRet) { //????
		OSA_ERROR("call pthread_getschedparam fail... error=%d", iRet);
	} else {
		iRet = stSched_param.sched_priority;
	}

	return iRet;
}

int OSA_GetStackSize(void)
{
	pthread_attr_t thread_attr;
	int			   iStackSize = 0;

	int iRet = pthread_attr_init(&thread_attr);

	if (OSA_SOK != iRet) {
		OSA_ERROR("Could not initialize thread attributes\n");
		return -1;
	}

	iRet = pthread_attr_getstacksize(&thread_attr, (size_t*) &iStackSize);
	if (OSA_SOK != iRet) {
		OSA_ERROR("pthread_attr_getstacksize() - Could not pthread_attr_getstacksize");
		return -1;
	}

	OSA_DBG_MSG("Current Thread stacksize=%d", iStackSize);

	return iStackSize;
}

BOOL OSA_SetThrStackSize(uint32_t uStackSize)
{
	pthread_attr_t thread_attr;
	int			   iStackSize = 0;

	int iRet = pthread_attr_init(&thread_attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("Could not initialize thread attributes\n");
		return FALSE;
	}

	iRet = pthread_attr_setstacksize(&thread_attr, (int) uStackSize);
	if (OSA_SOK != iRet) {
		OSA_ERROR("pthread_attr_setstacksize() - Could not pthread_attr_getstacksize");
		return FALSE;
	}

	iRet = pthread_attr_getstacksize(&thread_attr, (size_t*) &iStackSize);
	if (OSA_SOK != iRet) {
		OSA_ERROR("pthread_attr_getstacksize() - Could not call pthread_attr_getstacksize");
	} else {
		OSA_DBG_MSG("New stacksize=%d\n", iStackSize);
	}

	return TRUE;
}

BOOL OSA_ThrIsValid(orange_thread_handle_t* hndl)
{
	int iRet = OSA_SOK;
	if (NULL == hndl) {
		OSA_ERROR("Invalid parameter...");
		return FALSE;
	}

	if (hndl->hndl == OSA_THREAD_HANDLE_INVLALID) {
		OSA_ERROR("Invalid parameter: hndl->hndl==OSA_THREAD_HANDLE_INVLALID  ");
		return FALSE;
	}

	iRet = pthread_kill(hndl->hndl, 0);

	if (OSA_SOK != iRet) {
		OSA_ERROR("Call SYSTEM API: pthread_kill fail... error=%d", iRet);
		return FALSE;
	}

	return (OSA_SOK == iRet);
}

int OSA_ThrDetach(orange_thread_handle_t* hndl)
{
	int iStatus = OSA_SOK;

	if (NULL == hndl) {
		OSA_ERROR("Invalid parameter...");
		return FALSE;
	}

	if (OSA_THREAD_HANDLE_INVLALID == hndl->hndl) {
		OSA_ERROR("Invalid thread handle...");
		return FALSE;
	}

	iStatus = pthread_detach(hndl->hndl);
	if (OSA_SOK != iStatus) {
		OSA_ERROR("Call SYSTEM API : pthread_detach fail... error=%d", errno);
		return FALSE;
	}

	return TRUE;
}

BOOL OSA_ThrCompare(orange_thread_handle_t* pThr1, orange_thread_handle_t* pThr2)
{
	orange_thread_handle_t hThreadTemp;
	int					   iRet;

	if ((NULL == pThr1)) {
		OSA_ERROR("Invalid paremeter 1");
		return FALSE;
	}

	hThreadTemp.hndl = (NULL == pThr2) ? pthread_self() : pThr2->hndl;

	iRet = pthread_equal(pThr1->hndl, hThreadTemp.hndl);

	return (0 != iRet);
}

INT32 OSA_ThreadCreate(orange_thread_handle_t* pThrHandle, orange_thread_entry_func ThreadFunc, void* pParamter)
{
	INT32				   iPri = 0;
	orange_thread_handle_t hThr;

	if (OSA_GetSelfThrHandle(&hThr)) {
		iPri = OSA_GetThrPri(&hThr); //?뵱ǰ?̵߳????ȼ?ͬ
		if (ERR_FAIL == iPri) {
			iPri = 1; //ʹ?? 1 ???ȼ?(??)
		}
	} else {
		iPri = 1; //ʹ?? 1 ???ȼ?(??)
	}

	return OSA_ThreadCreateEx(pThrHandle, ThreadFunc, pParamter, iPri, FALSE);
}

INT32 OSA_ThreadCreateEx(orange_thread_handle_t* pThrHandle, orange_thread_entry_func ThreadFunc, void* pParamter, INT32 i32Pri, BOOL bSystemScope)
{
	INT32 iRet = OSA_SOK;

	pthread_attr_t	 attr;
	struct sched_param param = {0};

	if ((NULL == pThrHandle) || (NULL == ThreadFunc)) {
		OSA_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		return iRet;
	}

	iRet = pthread_attr_init(&attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_attr_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}
#ifdef __linux__ // cygwin only support SCHED_FIFO
	iRet |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
#else
	iRet |= pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
#endif
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_attr_setschedpolicy fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	//???????ȼ?
	i32Pri = OSA_Max(i32Pri, OSA_THR_PRI_RR_MIN);
	i32Pri = OSA_Min(i32Pri, OSA_THR_PRI_RR_MAX);

	iRet |= pthread_attr_getschedparam(&attr, &param);
	OSA_DBG_MSG("the default pri=%d, Target pri=%d", param.sched_priority, i32Pri);
	param.sched_priority = i32Pri;
	iRet |= pthread_attr_setschedparam(&attr, &param);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_attr_setschedparam fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	iRet |= pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_attr_setdetachstate fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	if (bSystemScope) {
		//?????߳̾??�����
		iRet |= pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
		if (OSA_SOK != iRet) {
			OSA_ERROR("call pthread_attr_setdetachstate fail... error=%d", iRet);
			iRet = ERR_SYS_API_CALLFAIL;
			goto __EXIT__;
		}
	}

	//????
	iRet = __orange_thread_create(&(pThrHandle->hndl), &attr, (void*) ThreadFunc, pParamter);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_create fail... error=%d", iRet);
		switch (iRet) {
			case EAGAIN:
				break;
			case EINVAL:
				break;
		}
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

__EXIT__:
	pthread_attr_destroy(&attr);

	return iRet;
}

/*----------------????Ϊ cond??????��?ķ?װ-------------------------------*/
INT32 OSA_CondEventInit(OSA_COND_EVENT_HANDLE* pOSA_COND_EVENT_HANDLE)
{
	INT32 iRet = OSA_SOK;

	pthread_condattr_t  cond_attr;
	pthread_mutexattr_t mutex_attr;

	if (NULL == pOSA_COND_EVENT_HANDLE) {
		OSA_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		return iRet;
	}

	pOSA_COND_EVENT_HANDLE->bInitialized	= FALSE;
	pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;

	iRet = pthread_condattr_init(&cond_attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_condattr_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	iRet = pthread_mutexattr_init(&mutex_attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutexattr_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

#ifdef __linux__
	// ???? recursive ????(??????)
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif

	iRet = pthread_mutex_init(&(pOSA_COND_EVENT_HANDLE->stMutex), &mutex_attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	iRet = pthread_cond_init(&(pOSA_COND_EVENT_HANDLE->stCond), &cond_attr);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_cond_init fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__;
	}

	pOSA_COND_EVENT_HANDLE->bInitialized = TRUE;

__EXIT__:
	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	if (OSA_SOK == iRet) {
		OSA_DBG_MSG("OSA_CondEventInit call successfully!");
	}

	return iRet;
}

INT32 OSA_CondEventDeInit(OSA_COND_EVENT_HANDLE* pOSA_COND_EVENT_HANDLE)
{
	INT32 iRet = OSA_SOK;

	if (NULL == pOSA_COND_EVENT_HANDLE) {
		OSA_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		goto __EXIT__;
	}

	iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int*) &(pOSA_COND_EVENT_HANDLE->stMutex), (int) &pOSA_COND_EVENT_HANDLE->stMutex);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;
	iRet									= pthread_cond_destroy(&(pOSA_COND_EVENT_HANDLE->stCond));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_cond_destroy fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}
	pOSA_COND_EVENT_HANDLE->bInitialized = FALSE;

	iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int*) &(pOSA_COND_EVENT_HANDLE->stMutex), (int) &pOSA_COND_EVENT_HANDLE->stMutex);
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	iRet = pthread_mutex_destroy(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_destroy fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}

__EXIT__:
	return iRet;
}

INT32 OSA_SetCondEvent(OSA_COND_EVENT_HANDLE* pOSA_COND_EVENT_HANDLE)
{
	INT32 iRet = OSA_SOK;

	if (NULL == pOSA_COND_EVENT_HANDLE) {
		OSA_ERROR("Invalid parameter...");
		iRet = ERR_INVALID_PARAM;
		goto __EXIT__;
	}
	iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	if (!pOSA_COND_EVENT_HANDLE->bInitialized) {
		OSA_ERROR("pOSA_COND_EVENT_HANDLE->stCond is DESTROYED");
		iRet = ERR_SYS_API_CALLFAIL;
		goto __EXIT__1__;
	}

	iRet = pthread_cond_signal(&(pOSA_COND_EVENT_HANDLE->stCond));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_cond_signal fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	} else {
		pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = FALSE; // ?ڴ˱???Ϊ ????????λ
	}

__EXIT__1__:

	iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iRet = ERR_SYS_API_CALLFAIL;
	}

__EXIT__:
	if (OSA_SOK == iRet) {
		OSA_DBG_MSG("OSA_SetCondEvent call successfully!");
		OSA_Sleep(0); //?ڴ˽����?ǰ?????̵߳?CPUʱ??Ƭ?ó?
	}

	OSA_DBG_MSGXX("aaa");
	return iRet;
}

INT32 OSA_WaitCondEvent(OSA_COND_EVENT_HANDLE* pOSA_COND_EVENT_HANDLE, INT32 iTimeOutMilliSec)
{
	INT32			iRet	= OSA_SOK;
	INT32			iResult = OSA_SOK;
	struct timespec waittime;
	TIME_VAL		stTimeVal = {0};
	uint32_t		uTemp;

	if ((NULL == pOSA_COND_EVENT_HANDLE)) {
		OSA_ERROR("Handle Invalid !");
		iRet = ERR_INVALID_PARAM;

		return iRet;
	}

	if (iTimeOutMilliSec < (INT32) OSA_TIMEOUT_FOREVER) {
		OSA_ERROR("Invalid time out value!");
		iRet = ERR_INVALID_PARAM;

		return iRet;
	}

	iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		iResult = ERR_SYS_API_CALLFAIL;

		goto __EXIT__;
	}

	switch (iTimeOutMilliSec) {
		case OSA_TIMEOUT_FOREVER: //?��õȴ?
		{
			while (1) //! pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
			{		  // ???? Spurious wakeup
				iRet = pthread_cond_wait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex));

				if (OSA_SOK != iRet) {
					OSA_ERROR("Call System API pthread_cond_wait return val=%d", iRet);
					iResult = ERR_SYS_API_CALLFAIL;
					goto __EXIT__;
				} else {
					if (FALSE == pOSA_COND_EVENT_HANDLE->bSpuriouswakeup) { // ?????Ƿ?Ϊ ?ٻ???
						iResult = OSA_SOK;
						break;
					} else { //?ٻ???
						continue;
					}
				}
			}

			if (OSA_SOK != iRet) {
				OSA_ERROR("pthread_cond_wait return val=%d", iRet);
				iResult = ERR_SYS_API_CALLFAIL;
			} else {
				iResult = OSA_SOK;
			}
		} break;

		case OSA_TIMEOUT_NONE: // 0 ?ȴ?
		{
			OSA_DBG_MSG("OSA_WaitCondEvent wait 0 ms...");
		} break;

		default: //??????ʱֵ
		{
			if (!OSA_TimeGetCurVal(&stTimeVal)) { //??ȡ??ǰʱ??value
				iResult = ERR_SYS_API_CALLFAIL;
				OSA_ERROR("Unexpect error...");
				break;
			}

			uTemp			 = stTimeVal.uMilliSec + (iTimeOutMilliSec % 1000);			 // ms?????ۼ?ֵ
			waittime.tv_sec  = stTimeVal.uSecs + iTimeOutMilliSec / 1000 + uTemp / 1000; //???ǵ?ms?????ۼ?????
			waittime.tv_nsec = (uTemp % 1000) * 1000000 + stTimeVal.uMicroSec * 1000;	//ʣ??ms????ת??Ϊ ns

			OSA_DBG_MSG("timeout is %u, wait time=%u.%u", iTimeOutMilliSec, (uint32_t) waittime.tv_sec, (uint32_t) waittime.tv_nsec);

			while (1) // pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
			{		  // ???? Spurious wakeup
				iRet = pthread_cond_timedwait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex), &waittime);
				if (ETIMEDOUT == iRet) { //??ʱ
					iResult = ERR_COND_EVENT_WAIT_TIMEOUT;
					break;
				} else {
					if (OSA_SOK == iRet) //?ڳ?ʱʱ???ڵõ? singal
					{
						if (FALSE == pOSA_COND_EVENT_HANDLE->bSpuriouswakeup) { // ?????Ƿ?Ϊ ?ٻ???
							iResult = OSA_SOK;
							break;
						} else { //?ٻ???
							continue;
						}
					} else { //????????
						OSA_ERROR("call pthread_cond_timedwait fail... error=%d", iRet);
						iResult = ERR_SYS_API_CALLFAIL;

						goto __EXIT__;
					}
				}
			}

		} break;
	}

	pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE; //??Ҫ???ñ?־??λ

__EXIT__:
	iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
		iResult = ERR_SYS_API_CALLFAIL;
	}

	if (ERR_COND_EVENT_WAIT_TIMEOUT == iResult) { //??ʱҪ?ص㷵?ظ???????
		return iResult;
	}

	return iResult;
}

BOOL OSA_IsCondEventTrig(OSA_COND_EVENT_HANDLE* pOSA_COND_EVENT_HANDLE)
{
	BOOL  bRet = FALSE;
	INT32 iRet = OSA_SOK;

	if (NULL == pOSA_COND_EVENT_HANDLE) {
		OSA_ERROR("Invalid parameter...");
		goto __EXIT__;
	}

	iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_lock fail... error=%d", iRet);
		bRet = FALSE;

		goto __EXIT__;
	}

	//???? ?Ƿ?Ϊ?ٻ???
	bRet = !(pOSA_COND_EVENT_HANDLE->bSpuriouswakeup);

	iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
	if (OSA_SOK != iRet) {
		OSA_ERROR("call pthread_mutex_unlock fail... error=%d", iRet);
	}

__EXIT__:
	return bRet;
}
