/******************************************************************************
* Copyright 2010-2016 ABB Genway Co.,Ltd.
* FileName: 	 osa_thr.c
* Desc:
* 平台无关的  OSA 抽象层对 Thread 操作的封装实现文件
*
* Author: 	 Davis Chen
* Date: 	 2010/07/18
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2010/07/18, Davis Chen create this file
*             v1.1   2016/04/29, Nancy Xu modify this file
******************************************************************************/
/*-------------------------------- Includes ----------------------------------*/
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sched.h>

#include "osa.h"
#include "osa_thr.h"
#include "osa_time.h"
#include "osa_debug.h"


/*-------------------- Global Definitions and Declarations -------------------*/


/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Type Declarations ---------------------------------*/


/*------------------------ Variable Declarations -----------------------------*/


/*------------------------ LOCAL Function Prototype --------------------------*/


/*------------------------ Function Implement --------------------------------*/

VOID OSA_Sleep(UINT32 u32SleepMs)
{
	#define __SLEEP_CAL_VAL__ 1000//降低睡眠误差值,单位 us
    if(0 == u32SleepMs)
    {
        usleep(0); //退出当前CPU 时间片
        return;
    }
    if(u32SleepMs < 1000) // 1s以下
    {
       usleep(u32SleepMs*1000); //休眠 ms
    }
    else
    {
    	if( (u32SleepMs/1000)>0)
		{
	        sleep(u32SleepMs/1000); // 风险 -- 不准确
		}

        if(u32SleepMs%1000)
    	{
            usleep((u32SleepMs%1000) * 1000 - __SLEEP_CAL_VAL__);
    	}
    }
}


static int testing_pthread_create(pthread_t *thread,
                             pthread_attr_t *attr,
                             OSA_ThrEntryFunc entryFunc,
                             void *arg, char *func, int line)
{
    int rel=0;

    OSA_printf("func:%s(%d), ++00 testing_pthread_create ++\n",func,line);
    rel = pthread_create(thread, attr, entryFunc, arg);
    OSA_printf("++01 testing_pthread_create(%d) ++ thread (%x)\n", rel, *(unsigned int *)thread);
    return (rel);
}

static int testing_pthread_join(pthread_t thread, void **thread_return, char *func, int line)
{
    int rel=0;

    OSA_printf("func:%s(%d), ++00 testing_pthread_join ++ thread (%d)\n", func,line, (int)thread);
    rel = pthread_join (thread, thread_return);
    OSA_printf("++01 testing_pthread_join(%d) ++\n",rel);
    return (rel);
}

int OSA_ThrCreate(OSA_ThrHndl *hndl, OSA_ThrEntryFunc entryFunc,int pri, void *prm)
{
	int iStatus=OSA_SOK;
	pthread_attr_t thread_attr;
    struct sched_param schedprm;

	// initialize thread attributes structure
	iStatus = pthread_attr_init(&thread_attr);

	if(OSA_SOK != iStatus)
	{
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		return iStatus;
	}

	iStatus |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
#ifdef __linux__    // cygwin only support SCHED_FIFO
	iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
#else
    iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
#endif
	if(pri > (UINT32)OSA_THR_PRI_RR_MAX)
    {
        pri = OSA_THR_PRI_RR_MAX;
    }
    else if(pri < (UINT32)OSA_THR_PRI_RR_MIN)
    {
        pri = OSA_THR_PRI_RR_MIN;
    }

    schedprm.sched_priority = pri;
    iStatus |= pthread_attr_setschedparam(&thread_attr, &schedprm);

	if(OSA_SOK != iStatus)
	{
		OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
		goto error_exit;
	}

	iStatus |= pthread_create(&hndl->hndl, &thread_attr, entryFunc, prm);

	if(OSA_SOK != iStatus)
	{
		OSA_ERROR("OSA_ThrCreate() - Could not create thread [%d]\n", iStatus);
	}

	error_exit:
	pthread_attr_destroy(&thread_attr);

	return iStatus;
}

int OSA_ThrCreateEx(OSA_ThrHndl *hndl, OSA_ThrEntryFunc entryFunc,int pri, void *prm,char *pFunc, int line)
{
    int iStatus=OSA_SOK;
    pthread_attr_t thread_attr;
    struct sched_param schedprm;

    // initialize thread attributes structure
    iStatus = pthread_attr_init(&thread_attr);

    if(OSA_SOK != iStatus)
    {
        OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
        return iStatus;
    }


    iStatus |= pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
#ifdef __linux__    // cygwin only support SCHED_FIFO    
    iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
#else
    iStatus |= pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
#endif

    if(pri > (UINT32)OSA_THR_PRI_RR_MAX)
    {
        pri = OSA_THR_PRI_RR_MAX;
    }
    else if(pri < (UINT32)OSA_THR_PRI_RR_MIN)
    {
        pri = OSA_THR_PRI_RR_MIN;
    }

    schedprm.sched_priority = pri;
    iStatus |= pthread_attr_setschedparam(&thread_attr, &schedprm);

    if(OSA_SOK != iStatus)
    {
        OSA_ERROR("OSA_ThrCreate() - Could not initialize thread attributes\n");
        goto error_exit;
    }

    iStatus = testing_pthread_create(&hndl->hndl, &thread_attr, entryFunc, prm, pFunc,line);

    if(OSA_SOK != iStatus)
    {
        OSA_ERROR("OSA_ThrCreate() - Could not create thread [%d]\n", iStatus);
    }

    error_exit:
    pthread_attr_destroy(&thread_attr);

    return iStatus;
}


int OSA_ThrJoin(OSA_ThrHndl *hndl)
{
	int iStatus = OSA_SOK;
	void *returnVal;

	if(NULL == hndl)
	{
		OSA_ERROR("Invalid parameter...");
		return OSA_EFAIL;
	}

	if(OSA_THREAD_HANDLE_INVLALID == hndl->hndl )
	{
		OSA_ERROR("Invalid pthread handle...");
		return OSA_EFAIL;
	}

	if(OSA_ThrCompare(hndl, NULL))
	{
		OSA_ERROR("Join in the same thread...");
		return OSA_EFAIL;
	}

	//iStatus = testing_pthread_join(hndl->hndl, &returnVal, (char *)__FUNCTION__,__LINE__);
	iStatus = pthread_join (hndl->hndl, &returnVal);

	if(EINVAL == iStatus)
	{
		OSA_ERROR("The target thread 0x%x is Detach", (int)(hndl->hndl));
		iStatus = OSA_SOK;
	}

	return iStatus;
}

int OSA_ThrJoinEx(OSA_ThrHndl *hndl,char *pFunc, int line)
{
    int iStatus = OSA_SOK;
    void *returnVal;

    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return OSA_EFAIL;
    }

    if(OSA_THREAD_HANDLE_INVLALID == hndl->hndl )
    {
        OSA_ERROR("Invalid pthread handle...");
        return OSA_EFAIL;
    }

    if(OSA_ThrCompare(hndl, NULL))
    {
        OSA_ERROR("Join in the same thread...");
        return OSA_EFAIL;
    }

    iStatus = testing_pthread_join(hndl->hndl, &returnVal, pFunc,line);

    if(EINVAL == iStatus)
    {
        OSA_ERROR("The target thread 0x%x is Detach", (int)(hndl->hndl));
        iStatus = OSA_SOK;
    }

//    OSA_DBG_MSG(" %s iStatus = %d", __func__ , iStatus);

    return iStatus;
}


int OSA_ThrDelete(OSA_ThrHndl *hndl)
{
    int iStatus = OSA_SOK;

    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return OSA_EFAIL;
    }

    if(OSA_THREAD_HANDLE_INVLALID == hndl->hndl )
    {
        OSA_ERROR("Invalid pthread handle...");
        return OSA_EFAIL;
    }

    iStatus |= pthread_cancel(hndl->hndl);
	OSA_DBG_MSG(" %s cancel 0x%x handle iStatus = %d", __func__, (int)hndl->hndl, iStatus);

    iStatus |= OSA_ThrJoin(hndl);
	OSA_DBG_MSG(" %s cancel 0x%x handle iStatus = %d", __func__, (int)hndl->hndl, iStatus);

    return iStatus;
}


int OSA_ThrChangePri(OSA_ThrHndl *hndl, UINT32 pri)
{
    int iStatus = OSA_SOK;
    struct sched_param schedprm;

    if(pri>(UINT32)OSA_THR_PRI_RR_MAX)
    {
        pri=OSA_THR_PRI_RR_MAX;
    }
    else if(pri<(UINT32)OSA_THR_PRI_RR_MIN)
    {
        pri=OSA_THR_PRI_RR_MIN;
    }

    schedprm.sched_priority = pri;
#ifdef __linux__    // cygwin only support SCHED_FIFO
    iStatus |= pthread_setschedparam(hndl->hndl, SCHED_RR, &schedprm);
#else
    iStatus |= pthread_setschedparam(hndl->hndl, SCHED_FIFO, &schedprm);
#endif
    return iStatus;
}


void OSA_ThrChangeSelfPri(UINT32 pri)
{
	OSA_ThrHndl pSelfThrHndl;
	OSA_GetSelfThrHandle(&pSelfThrHndl);
	OSA_ThrChangePri(&pSelfThrHndl,pri);
}


int OSA_ThrExit(void *returnVal)
{
    pthread_exit(returnVal);
    return OSA_SOK;
}

BOOL OSA_GetSelfThrHandle(OSA_ThrHndl *pSelfThrHndl)
{
    if((NULL == pSelfThrHndl))
    {
        OSA_ERROR("Invalid paremeter 1");
        return FALSE;
    }

    pSelfThrHndl->hndl = pthread_self();
    if((pSelfThrHndl->hndl == OSA_THREAD_HANDLE_INVLALID)||(pSelfThrHndl->hndl == 0))
	{
        return FALSE;
	}
    else
	{
        return TRUE;
	}
}

BOOL OSA_SetThrPri(OSA_ThrHndl *pThr, INT iNewPri)
{
    BOOL bRet = FALSE;
    int iRet = ERR_FAIL;
    int iPolicy;
    struct sched_param stSched_param = {0};

    if((NULL == pThr))
    {
        OSA_ERROR("Invalid paremeter 1");
        return bRet;
    }

    iNewPri = (iNewPri < 0 /*OSA_THR_PRI_MIN*/) ? 0 /*OSA_THR_PRI_MIN*/ : iNewPri;
    iNewPri = (iNewPri > OSA_THR_PRI_RR_MAX) ? OSA_THR_PRI_RR_MAX : iNewPri;

    iRet = pthread_getschedparam(pThr->hndl, &iPolicy, &stSched_param);
    if(OSA_SOK != iRet)//出错
    {
        OSA_ERROR("call pthread_getschedparam fail... error=%d", iRet);
        bRet = FALSE;
        return bRet ;
    }

    OSA_DBG_MSG("Get current pri=%d", stSched_param.sched_priority);

    stSched_param.sched_priority = iNewPri;

    OSA_DBG_MSG("the new pri=%d, Policy=%d", iNewPri, iPolicy);

    iRet = pthread_setschedparam(pThr->hndl, iPolicy, &stSched_param);

    if(OSA_SOK != iRet)
    {//出错
        OSA_ERROR("call pthread_setschedparam fail... error=%d", iRet);
        switch(iRet)
        {
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
    }
    else
    {
        bRet = TRUE;
    }

    return bRet ;
}


INT OSA_GetThrPri(OSA_ThrHndl *pThr)
{
    int iRet = ERR_FAIL;
    int iPolicy;
    struct sched_param stSched_param = {0};

    if((NULL == pThr))
    {
        OSA_ERROR("Invalid paremeter...");
        return iRet;
    }

    iRet = pthread_getschedparam(pThr->hndl, &iPolicy, &stSched_param);

    if(OSA_SOK != iRet)
    {//出错
        OSA_ERROR("call pthread_getschedparam fail... error=%d",iRet);
    }
    else
    {
        iRet = stSched_param.sched_priority;
    }

    return iRet;
}

int OSA_GetStackSize(VOID)
{
    pthread_attr_t thread_attr;
    int iStackSize = 0;

    int iRet = pthread_attr_init(&thread_attr);

    if(OSA_SOK != iRet)
    {
        OSA_ERROR("Could not initialize thread attributes\n");
        return -1;
    }

    iRet = pthread_attr_getstacksize(&thread_attr, (size_t *)&iStackSize);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("pthread_attr_getstacksize() - Could not pthread_attr_getstacksize");
        return -1;
    }

    OSA_DBG_MSG("Current Thread stacksize=%d", iStackSize);

    return iStackSize;

}

BOOL OSA_SetThrStackSize(UINT32 uStackSize)
{
    pthread_attr_t thread_attr;
    int iStackSize = 0;

    int iRet = pthread_attr_init(&thread_attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("Could not initialize thread attributes\n");
        return FALSE;
    }

    iRet = pthread_attr_setstacksize(&thread_attr, (int)uStackSize);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("pthread_attr_setstacksize() - Could not pthread_attr_getstacksize");
        return FALSE;
    }

    iRet = pthread_attr_getstacksize(&thread_attr, (size_t *)&iStackSize);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("pthread_attr_getstacksize() - Could not call pthread_attr_getstacksize");
    }
    else
    {
        OSA_DBG_MSG("New stacksize=%d\n", iStackSize);
    }

    return TRUE;
}

BOOL OSA_ThrIsValid(OSA_ThrHndl *hndl)
{
    int iRet = OSA_SOK;
    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return FALSE;
    }

    if(hndl->hndl==OSA_THREAD_HANDLE_INVLALID)
    {
        OSA_ERROR("Invalid parameter: hndl->hndl==OSA_THREAD_HANDLE_INVLALID  ");
        return FALSE;
    }

    iRet = pthread_kill(hndl->hndl, 0);

    if(OSA_SOK != iRet)
    {
        OSA_ERROR("Call SYSTEM API: pthread_kill fail... error=%d", iRet);
        return FALSE;
    }

    return (OSA_SOK == iRet);
}


int OSA_ThrDetach(OSA_ThrHndl *hndl)
{
    int iStatus = OSA_SOK;

    if(NULL == hndl)
    {
        OSA_ERROR("Invalid parameter...");
        return FALSE;
    }

    if(OSA_THREAD_HANDLE_INVLALID== hndl->hndl)
    {
        OSA_ERROR("Invalid thread handle...");
        return FALSE;
    }

    iStatus = pthread_detach(hndl->hndl);
    if(OSA_SOK != iStatus)
    {
        OSA_ERROR("Call SYSTEM API : pthread_detach fail... error=%d", errno);
        return FALSE;
    }

    return TRUE;
}


BOOL OSA_ThrCompare(OSA_ThrHndl *pThr1, OSA_ThrHndl * pThr2)
{
    OSA_ThrHndl hThreadTemp;
    int iRet ;

    if((NULL == pThr1))
    {
        OSA_ERROR("Invalid paremeter 1");
        return FALSE;
    }

    hThreadTemp.hndl = (NULL == pThr2) ? pthread_self() : pThr2->hndl;

    iRet = pthread_equal(pThr1->hndl, hThreadTemp.hndl);

    return (0 != iRet );
}

INT32 OSA_ThreadCreate(OSA_ThrHndl *pThrHandle, OSA_ThrEntryFunc ThreadFunc, void * pParamter)
{
    INT32 iPri = 0;
    OSA_ThrHndl hThr;

    if (OSA_GetSelfThrHandle(&hThr))
    {
        iPri = OSA_GetThrPri(&hThr); //与当前线程的优先级同
        if(ERR_FAIL == iPri)
        {
            iPri = 1;  //使用 1 优先级(低)
        }
    }
    else
    {
        iPri = 1;  //使用 1 优先级(低)
    }

    return OSA_ThreadCreateEx(pThrHandle, ThreadFunc, pParamter, iPri, FALSE);
}

INT32 OSA_ThreadCreateEx(OSA_ThrHndl *pThrHandle, OSA_ThrEntryFunc ThreadFunc,void * pParamter, INT32 i32Pri, BOOL bSystemScope)
{
    INT32 iRet = OSA_SOK;

    pthread_attr_t attr;
    struct sched_param param = {0};

    if((NULL == pThrHandle)||(NULL == ThreadFunc))
    {
        OSA_ERROR("Invalid parameter...");
        iRet = ERR_INVALID_PARAM;
        return iRet;
    }

    iRet = pthread_attr_init(&attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_attr_init fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }
#ifdef __linux__    // cygwin only support SCHED_FIFO
    iRet |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
#else
    iRet |= pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
#endif
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_attr_setschedpolicy fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

    //设置优先级
    i32Pri = OSA_Max(i32Pri,OSA_THR_PRI_RR_MIN);
    i32Pri = OSA_Min(i32Pri,OSA_THR_PRI_RR_MAX);

    iRet |= pthread_attr_getschedparam (&attr, &param);
    OSA_DBG_MSG("the default pri=%d, Target pri=%d", param.sched_priority, i32Pri);
    param.sched_priority = i32Pri;
    iRet |= pthread_attr_setschedparam(&attr, &param);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_attr_setschedparam fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

	iRet |= pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_attr_setdetachstate fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

    if(bSystemScope)
    {
        //设置线程竞争场景
        iRet |= pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        if(OSA_SOK != iRet)
        {
            OSA_ERROR("call pthread_attr_setdetachstate fail... error=%d",iRet);
            iRet = ERR_SYS_API_CALLFAIL;
            goto __EXIT__;
        }

    }

    //创建
	iRet = testing_pthread_create(&(pThrHandle->hndl), &attr, (void *)ThreadFunc, pParamter, (char *)__FUNCTION__,__LINE__);
	if(OSA_SOK != iRet)
	{
	    OSA_ERROR("call pthread_create fail... error=%d",iRet);
        switch(iRet)
        {
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

/*----------------以下为 cond条件变量的封装-------------------------------*/
INT32 OSA_CondEventInit(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    INT32 iRet = OSA_SOK;

    pthread_condattr_t cond_attr;
    pthread_mutexattr_t mutex_attr;

    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = ERR_INVALID_PARAM;
        return iRet;
    }

    pOSA_COND_EVENT_HANDLE->bInitialized = FALSE;
    pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;

    iRet = pthread_condattr_init(&cond_attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_condattr_init fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

    iRet = pthread_mutexattr_init(&mutex_attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutexattr_init fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

#ifdef __linux__
    // 设置 recursive 属性(互斥锁)
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif

    iRet = pthread_mutex_init(&(pOSA_COND_EVENT_HANDLE->stMutex), &mutex_attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_init fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

    iRet = pthread_cond_init(&(pOSA_COND_EVENT_HANDLE->stCond), &cond_attr);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_init fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__;
    }

    pOSA_COND_EVENT_HANDLE->bInitialized = TRUE;

__EXIT__:
    pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

    if(OSA_SOK == iRet)
    {
        OSA_DBG_MSG("OSA_CondEventInit call successfully!");
    }

    return iRet;

}

INT32 OSA_CondEventDeInit(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    INT32 iRet = OSA_SOK;

    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = ERR_INVALID_PARAM;
        goto __EXIT__;
    }

    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;

        goto __EXIT__;
    }

    pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE;
    iRet = pthread_cond_destroy(&(pOSA_COND_EVENT_HANDLE->stCond));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_destroy fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
    }
    pOSA_COND_EVENT_HANDLE->bInitialized = FALSE;

    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    OSA_DBG_MSGXX("pOSA_COND_EVENT_HANDLE->stMutex(%d), (%d)", *(unsigned int *)&(pOSA_COND_EVENT_HANDLE->stMutex), (int)&pOSA_COND_EVENT_HANDLE->stMutex);
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;

        goto __EXIT__;
    }

    iRet = pthread_mutex_destroy(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_destroy fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
    }

__EXIT__:
    return iRet;
}

INT32 OSA_SetCondEvent(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    INT32 iRet = OSA_SOK;

    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        iRet = ERR_INVALID_PARAM;
        goto __EXIT__;
    }
    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;

        goto __EXIT__;
    }

    if(! pOSA_COND_EVENT_HANDLE->bInitialized)
    {
        OSA_ERROR("pOSA_COND_EVENT_HANDLE->stCond is DESTROYED");
        iRet = ERR_SYS_API_CALLFAIL;
        goto __EXIT__1__;
    }

    iRet = pthread_cond_signal(&(pOSA_COND_EVENT_HANDLE->stCond));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_cond_signal fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
    }
    else
    {
        pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = FALSE; // 在此标记为 真正的置位
    }

__EXIT__1__:

    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet);
        iRet = ERR_SYS_API_CALLFAIL;
    }

__EXIT__:
    if(OSA_SOK == iRet)
    {
        OSA_DBG_MSG("OSA_SetCondEvent call successfully!");
        OSA_Sleep(0); //在此仅将当前调用线程的CPU时间片让出
    }

    OSA_DBG_MSGXX("aaa");
    return iRet;
}

INT32 OSA_WaitCondEvent(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE, INT32 iTimeOutMilliSec)
{
    INT32 iRet = OSA_SOK;
    INT32 iResult = OSA_SOK;
    struct timespec waittime;
    TIME_VAL stTimeVal = {0};
    UINT32 uTemp;

    if((NULL == pOSA_COND_EVENT_HANDLE))
    {
        OSA_ERROR("Handle Invalid !");
        iRet = ERR_INVALID_PARAM;

        return iRet;
    }

    if(iTimeOutMilliSec < (INT32)OSA_TIMEOUT_FOREVER)
    {
        OSA_ERROR("Invalid time out value!");
        iRet = ERR_INVALID_PARAM;

        return iRet;
    }

    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet);
        iResult = ERR_SYS_API_CALLFAIL;

        goto __EXIT__;
    }

    switch(iTimeOutMilliSec)
    {
        case OSA_TIMEOUT_FOREVER://永久等待
        {
            while(1) //!pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
            {// 避免 Spurious wakeup
                iRet = pthread_cond_wait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex));

                if(OSA_SOK != iRet)
                {
                    OSA_ERROR("Call System API pthread_cond_wait return val=%d",iRet);
                    iResult = ERR_SYS_API_CALLFAIL;
                    goto __EXIT__ ;
                }
                else
                {
                    if(FALSE == pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
                    {// 测试是否为 假唤醒
                        iResult = OSA_SOK;
                        break;
                    }
                    else
                    {//假唤醒
                        continue;
                    }
                }
            }

            if(OSA_SOK != iRet)
            {
                OSA_ERROR("pthread_cond_wait return val=%d",iRet);
                iResult = ERR_SYS_API_CALLFAIL;
            }
            else
            {
                iResult = OSA_SOK;
            }
        }
            break;

        case OSA_TIMEOUT_NONE:// 0 等待
        {
            OSA_DBG_MSG("OSA_WaitCondEvent wait 0 ms...");
        }
            break;

        default://正常超时值
        {
            if(! OSA_TimeGetCurVal(&stTimeVal))
            {//获取当前时间value
                iResult = ERR_SYS_API_CALLFAIL;
                OSA_ERROR("Unexpect error...");
                break;
            }

            uTemp = stTimeVal.uMilliSec + (iTimeOutMilliSec % 1000); // ms部分累计值
            waittime.tv_sec = stTimeVal.uSecs + iTimeOutMilliSec/1000 + uTemp/1000 ; //考虑到ms部分累计溢出
            waittime.tv_nsec = (uTemp%1000)*1000000 + stTimeVal.uMicroSec * 1000; //剩余ms部分转换为 ns

            OSA_DBG_MSG("timeout is %u, wait time=%u.%u", iTimeOutMilliSec,(UINT32)waittime.tv_sec, (UINT32)waittime.tv_nsec);

            while(1) //pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
            {// 避免 Spurious wakeup
                iRet = pthread_cond_timedwait(&(pOSA_COND_EVENT_HANDLE->stCond), &(pOSA_COND_EVENT_HANDLE->stMutex),&waittime);
                if( ETIMEDOUT == iRet )
                {//超时
                    iResult = ERR_COND_EVENT_WAIT_TIMEOUT;
                    break;
                }
                else
                {
                    if(OSA_SOK == iRet)//在超时时间内得到 singal
                    {
                        if(FALSE == pOSA_COND_EVENT_HANDLE->bSpuriouswakeup)
                        {// 测试是否为 假唤醒
                            iResult = OSA_SOK;
                            break;
                        }
                        else
                        {//假唤醒
                            continue;
                        }
                    }
                    else
                    {//其他错误
                        OSA_ERROR("call pthread_cond_timedwait fail... error=%d",iRet);
                        iResult = ERR_SYS_API_CALLFAIL;

                        goto __EXIT__ ;
                    }
                }
            }


        }
            break;
    }

    pOSA_COND_EVENT_HANDLE->bSpuriouswakeup = TRUE; //需要将该标志复位

__EXIT__:
    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet);
        iResult = ERR_SYS_API_CALLFAIL;
    }

    if(ERR_COND_EVENT_WAIT_TIMEOUT == iResult)
    {//超时要重点返回给调用者
        return iResult;
    }

    return iResult;
}

BOOL OSA_IsCondEventTrig(OSA_COND_EVENT_HANDLE *pOSA_COND_EVENT_HANDLE)
{
    BOOL bRet = FALSE;
    INT32 iRet = OSA_SOK;

    if(NULL == pOSA_COND_EVENT_HANDLE)
    {
        OSA_ERROR("Invalid parameter...");
        goto __EXIT__;
    }

    iRet = pthread_mutex_lock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_lock fail... error=%d",iRet);
        bRet = FALSE;

        goto __EXIT__;
    }

    //测试 是否为假唤醒
    bRet = !(pOSA_COND_EVENT_HANDLE->bSpuriouswakeup) ;

    iRet = pthread_mutex_unlock(&(pOSA_COND_EVENT_HANDLE->stMutex));
    if(OSA_SOK != iRet)
    {
        OSA_ERROR("call pthread_mutex_unlock fail... error=%d",iRet);
    }

__EXIT__:
    return bRet;
}




