/******************************************************************************
* Copyright 2010-2015 ABB Genway Co.,Ltd.
* FileName: isSyncArbitrate.c
* Desc:  实现IP-GW模式下，户内多分机同步请求仲裁
*
*
* Author: Andy-wei.hou
* Date:   2015/03/13
* Notes:  
*         
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/13, Andy-wei.hou Create this file, 从原始IP-GW的config.c文件中将该部分
			   功能独立出来
******************************************************************************/
#include <osa/osa_time.h>
#include <osa/osa_debug.h>
#include <osa/osa_mutex.h>
#include "osa/osa_timer.h"
#include <network.h>


/*-------------------- Global Definitions and Declarations -------------------*/

/*----------------------- Constant / Macro Definitions -----------------------*/
#define MAX_SYNC_TYPES 	OPTION_Sync_MAX-1			// the value depends on the sync type defined in logic/network.h
#define SYNC_TIMEOUT	3*1000						//default sync_flag hold time is 3s
#define SYNC_MONITOR_TIMEOUT	135*1000			//default sync_flag hold time is 3s

/*------------------------ Type Declarations ---------------------------------*/

typedef struct sync_req_info
{
	char cUsedF;					//Flag that this Sync_Type flag has beed used by one IS
	char User[MAX_ID_SIZE];			//Who use this Sync_flag
	void * szTimer_handle;			//The timer handle used to matain this type sync_request
	OSA_MutexHndl szMutex;
}SYNC_REQ_INFO;



/*------------------------ Variable Declarations -----------------------------*/
struct sync_req_info gszSync_Req_Flag[MAX_SYNC_TYPES];			//the index of element is equal to max type of sync_type


/*------------------------ Function Prototype --------------------------------*/
int sync_req_flag_clear(char cType,char *pID, char cForce);

/*------------------------ Function Implement --------------------------------*/




/******************************************************************************
* Name: 	 deal_sync_req_flag_timeout
*
* Desc: 	分机同步仲裁结构初始化
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/
 void deal_sync_req_flag_timeout(void *arg)
{
	int iType = (int )arg;
	OSA_DBG_MSGX("No Release syn_req_flag(%d) in %d s, Force release it",iType,SYNC_TIMEOUT);
	sync_req_flag_clear(iType,"000000",1);	
}


/******************************************************************************
* Name: 	 setup_sync_req_timer
*
* Desc: 	分机同步仲裁结构初始化
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/
 void setup_sync_req_timer(char cType)
{
	OSA_DBG_MSG("Setup Timer for SyncFlag(%d)",cType);
	int iType = cType;		// the parameter used for timeout callback fucntion should be multiple of interger type, else the value read by callback function is wrong, so, force the parameter to interger type -----Andy 2013.07.24

    #if defined(__Use__HB__miniOS__)
    if(cType == OPTION_SyncMonitorMiniOS)
	{
		gszSync_Req_Flag[cType-1].szTimer_handle = OSA_SetTimer(SYNC_MONITOR_TIMEOUT,eTimerOnce,deal_sync_req_flag_timeout,NULL);
	}
	else
    #endif    
	{
	    gszSync_Req_Flag[cType-1].szTimer_handle = OSA_SetTimer(SYNC_TIMEOUT,eTimerOnce,deal_sync_req_flag_timeout,NULL);
	}
}


/******************************************************************************
* Name: 	 sync_req_flag_used_test
*
* Desc: 	分机同步仲裁测试标记是否被占用
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/

int sync_req_flag_used_test(char cType)
{
	int iRet = 0;
	OSA_MutexLock(&gszSync_Req_Flag[cType-1].szMutex);
	iRet = gszSync_Req_Flag[cType-1].cUsedF;
	OSA_MutexUnlock(&gszSync_Req_Flag[cType-1].szMutex);
	return iRet;
	
}

/******************************************************************************
* Name: 	 sync_req_flag_set 
*
* Desc: 	分机同步仲裁结构占用设置
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/
 int sync_req_flag_set(char cType,char *pID)
{
	//check the Flag is used or not , if used return fail
	if(sync_req_flag_used_test(cType)){
		OSA_DBG_MSG("The Flag has been used,return busy");
		return 0;
	}
	OSA_DBG_MSG("The Flag is empty,can be used");
	OSA_MutexLock(&gszSync_Req_Flag[cType-1].szMutex);
	gszSync_Req_Flag[cType-1].cUsedF = 1;
	memcpy(&(gszSync_Req_Flag[cType-1].User),pID,MAX_ID_SIZE);
	//setup an timer for this sync_flag, 
	setup_sync_req_timer(cType);

	OSA_MutexUnlock(&gszSync_Req_Flag[cType-1].szMutex);	
	return 1;
	
}

/******************************************************************************
* Name: 	 sync_req_flag_check_user 
*
* Desc: 	分机同步仲裁测试当前是否是某用户在使用同步标记
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/

int sync_req_flag_check_user(char cType,char *pUser)
{
	int iRet = 1;
	int i;
	char pID[MAX_ID_SIZE]= {0};
	OSA_MutexLock(&gszSync_Req_Flag[cType-1].szMutex);
	memcpy(pID,gszSync_Req_Flag[cType-1].User,MAX_ID_SIZE);
	OSA_MutexUnlock(&gszSync_Req_Flag[cType-1].szMutex);

	for ( i=0 ; i<MAX_ID_SIZE; i++)
	{
		if(pUser[i] != pID[i])
		{	
			iRet = 0;
			break;
		}
	}
	return iRet;
}


/******************************************************************************
* Name: 	 sync_req_flag_clear
*
* Desc: 	分机同步仲裁标记结构释放
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/
 int sync_req_flag_clear(char cType,char *pID, char cForce)
{
	if(cForce || sync_req_flag_check_user(cType,pID))
	{
		OSA_MutexLock(&gszSync_Req_Flag[cType-1].szMutex);
		
		gszSync_Req_Flag[cType-1].cUsedF = 0;
		memset(gszSync_Req_Flag[cType-1].User,0,MAX_ID_SIZE);

		//if the timer still alive, kill it 
		if(gszSync_Req_Flag[cType-1].szTimer_handle)
			OSA_KillTimer(gszSync_Req_Flag[cType-1].szTimer_handle);

		gszSync_Req_Flag[cType-1].szTimer_handle = NULL;
		OSA_MutexUnlock(&gszSync_Req_Flag[cType-1].szMutex);			
		return 1;
	}
	else
	{
		OSA_DBG_MSGX("The user ID check fail:ID(%02x %02x %02x %02x %02x %02x)",pID[0],pID[1],pID[2],pID[3],pID[4],pID[5]);
		return 0;
	}

}


/******************************************************************************
* Name: 	 sync_req_flag_process
*
* Desc: 	分机同步仲裁结构设置
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/

void sync_req_flag_process(s_PacketMsg *interMsg, BYTE *data,UINT32 len)
{
	SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)data;
	BYTE dataBuf[16]= {0};
	BYTE dataLen = 0;
    BOOL bRet = FALSE;
	char pID[MAX_ID_SIZE] = {0};
	char cSync_type ;
	char cReq_or_clear;
	char *pData_buf = (data+ sizeof(SOCK_DATA_PACKET_T));

	/*
		pData_buf[0] is the sync_type
		pData_buf[1] is the Request or Release option
	*/
	cSync_type = pData_buf[0];
	cReq_or_clear = pData_buf[1];
	memcpy(pID,netMsg->srcId,MAX_ID_SIZE);
	OSA_DBG_MSGX("SYNC %s From ID(%02x %02x %02x %02x %02x %02x), type(%d)",cReq_or_clear==0 ? "Release":"Request"
				,pID[0],pID[1],pID[2],pID[3],pID[4],pID[5],cSync_type);
		
    switch(netMsg->operCode)
    {
    	case OPER_SYNC_REQUEST:       
    	{
			dataLen = 1 + 1 + 1 ;
			//dataBuf[0] = SOCK_ACK_OK;
			dataBuf[1] = cSync_type;
			dataBuf[2] = cReq_or_clear;
	
			if(cReq_or_clear)
			{
				//sync request
				if (sync_req_flag_set(cSync_type,pID))
					dataBuf[0] = SOCK_ACK_OK;
				else
					dataBuf[0] = SOCK_ACK_SYSTEM_BUSY;
			}
			else
			{
				//sync release
				if(sync_req_flag_clear(cSync_type,pID,0))
					dataBuf[0] = SOCK_ACK_OK;
				else
					dataBuf[0] = SOCK_ACK_OPER_FAILED;
			}
					
			if(interMsg->sockfd > 0)
            {
                bRet = SendAckTCP(netMsg, dataBuf, dataLen, interMsg->sockfd);
                if(bRet == FALSE)
                {
                    OSA_ERROR("SendAckTCP FAILED. errno(%d)",errno);
                }
                close(interMsg->sockfd);
            }
            else
            {
                OSA_ERRORX("interMsg->sockfd(%d)", interMsg->sockfd);
            }
			
    	}break;

		 default:
        {
            OSA_ERROR("unknown operCode(%d)", netMsg->operCode);
			 close(interMsg->sockfd);
        }
    }


}



/******************************************************************************
* Name: 	 sync_req_flag_init 
*
* Desc: 	分机同步仲裁结构初始化
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/07/18, Andy-wei.hou Create this function
 ******************************************************************************/
 void sync_req_flag_init()
{
	int i = 0;
	OSA_DBG_MSG("Max Sync Req Flags are %d",MAX_SYNC_TYPES);
	for(i=0 ; i<=MAX_SYNC_TYPES; i++)
	{
		gszSync_Req_Flag[i].szTimer_handle= NULL;
		gszSync_Req_Flag[i].cUsedF = 0;
		memset(gszSync_Req_Flag[i].User,0,MAX_ID_SIZE);
		OSA_MutexCreate(&gszSync_Req_Flag[i].szMutex);
	}
}






