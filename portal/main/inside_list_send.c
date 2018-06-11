/******************************************************************************
* Copyright 2010-2013 ABB Genway Co.,Ltd.
* FileName: 	 inside_list_send.c 
* Desc:
* 
* 
* Author: 	Andy Hou
* Date: 	 	2013.03.25
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2013.04.10, Andy Hou create this file
* 
******************************************************************************/
#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>



#include <osa/osa_mutex.h>
#include <osa/osa_time.h>
#include <osa/osa_debug.h>
#include <osa/osa_mem.h>
#include <network.h>
#include <osa/osa_thr.h>
#include <ipgwCommonDefy.h>
#include "id_ip_maintainer.h"
#include "inside_list_send.h"


struct Send_Flg {
	OSA_MutexHndl szListSendMutex;		//mutex
	unsigned int S2ISs_Fail;			//32bit中的每一位都表示当前推送列表给该IS是否失败,1,失败；0，成功
	char List_changed;					//户内列表是否发生变化
};



static OSA_ThrHndl hSendListThread;
LOCAL int giSendListRunning = 0;

/*send inside IS list*/
struct Send_Flg gsz_SenFlg = { 0 };
static OSA_ThrHndl hTmpSendListThread;
LOCAL int giTmpSend_list_thr_running= 0 ;

/*send inside 2nd OS list*/
struct Send_Flg gsz_2ndSenFlg =  { 0 };
static OSA_ThrHndl hTmpSend2ndOSListThread;
LOCAL int giTmpSend2ndOSList_thr_running= 0 ;


/*send IPA  list*/
struct Send_Flg gsz_IPASenFlg =  { 0 };
static OSA_ThrHndl hTmpSendIPAListThread;
LOCAL int giTmpSend3ndOSList_thr_running= 0 ;

void send_list_fail_set(int iIndex,LIST_SEND_OPE_TYPE type)
{
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		gsz_SenFlg.S2ISs_Fail |= (1<<iIndex);			
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else{
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		gsz_2ndSenFlg.S2ISs_Fail |= (1<<iIndex);			
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
}

void send_list_fail_clr(int iIndex,LIST_SEND_OPE_TYPE type)
{
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		gsz_SenFlg.S2ISs_Fail &= ~(1<<iIndex);			
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else
	{
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		gsz_2ndSenFlg.S2ISs_Fail &= ~(1<<iIndex);			
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
}

void set_list_changed(LIST_SEND_OPE_TYPE type)
{
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		gsz_SenFlg.List_changed = 1;
		gsz_SenFlg.S2ISs_Fail = 0;
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else if(type == eSEND_2ndOS_LIST)
	{
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		gsz_2ndSenFlg.List_changed = 1;
		gsz_2ndSenFlg.S2ISs_Fail = 0;
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
	else if(type == eSEND_IPA_LIST)
	{
		OSA_MutexLock(&gsz_IPASenFlg.szListSendMutex);
		gsz_IPASenFlg.List_changed = 1;
		gsz_IPASenFlg.S2ISs_Fail = 0;
		OSA_MutexUnlock(&gsz_IPASenFlg.szListSendMutex);


	}
}
void clr_list_changed(LIST_SEND_OPE_TYPE type)
{
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		gsz_SenFlg.List_changed = 0;
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else if(type == eSEND_2ndOS_LIST){ 
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		gsz_2ndSenFlg.List_changed = 0;
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
	else if(type == eSEND_IPA_LIST)
	{
		OSA_MutexLock(&gsz_IPASenFlg.szListSendMutex);
		gsz_IPASenFlg.List_changed = 0;
		OSA_MutexUnlock(&gsz_IPASenFlg.szListSendMutex);
	}
	

}

char  get_list_changed(LIST_SEND_OPE_TYPE type)
{
	char Flg = 0;
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		Flg = gsz_SenFlg.List_changed;
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else if(type == eSEND_2ndOS_LIST){
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		Flg = gsz_2ndSenFlg.List_changed;
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
	else if(type == eSEND_IPA_LIST)
	{
		gsz_IPASenFlg.List_changed;
		OSA_MutexUnlock(&gsz_IPASenFlg.szListSendMutex);

	}
	return Flg;
}

unsigned int get_send_fail_flg(LIST_SEND_OPE_TYPE type)
{
	unsigned int Flg = 0;
	
	if(type == eSEND_IS_LIST){
		OSA_MutexLock(&gsz_SenFlg.szListSendMutex);
		Flg = gsz_SenFlg.S2ISs_Fail;
		OSA_MutexUnlock(&gsz_SenFlg.szListSendMutex);
	}
	else  if(type == eSEND_2ndOS_LIST)
	{
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		Flg = gsz_2ndSenFlg.S2ISs_Fail;
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);
	}
	else 
	{
		OSA_MutexLock(&gsz_2ndSenFlg.szListSendMutex);
		Flg = gsz_2ndSenFlg.S2ISs_Fail;
		OSA_MutexUnlock(&gsz_2ndSenFlg.szListSendMutex);

	}
	return Flg;

}





/******************************************************************************
* Name: 	 send_inside_is_list 
*
* Desc: 	根据户内列表，向户内IS逐个推送户内分机列表
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.04.18, Create this function by Andy Hou
*		 2013.04.27, Andy Hou modify this function by adding a timeout mechnism for wating for IS reply
				   after ip-gateway send a list to this is.
 ******************************************************************************/

void send_inside_is_list(void)
{
	int sockfd;
	int iSend_all = 0;										//用于判断是推送全部列表还是只推送失败的列表
	char pTag_ID[MAX_ID_SIZE];				/*tmp saving target ID */
	struct in_addr szIp_list[LIST_MAX+LIST_MAX];		/*tmp saving inside ISs List*/
    struct in_addr sz2OsIp_list[LIST_MAX+LIST_MAX];
    char pSend_buf[1 +5*LIST_MAX+18]={0};					//指向数据段指针，指向发送buf指针, modify by Andy 15.03.26, add video resolution list
	char pRcv_buf[1+18]={0};

	char *pData_buf = NULL;
	SOCK_DATA_PACKET_T *pHeader,*pRcvHeader;				//pointer to send header
	BYTE ucData_size, ucBuf_size,ucRcv_size;
	int i=0,iRet;	
	
	fd_set fSets;
	struct timeval time_out;

	//printf("2ndos list change :send indoor list to indoor and 2ndos  start\n");
	ucData_size = 1 +5*LIST_MAX;
	ucBuf_size = ucData_size + sizeof(SOCK_DATA_PACKET_T);
	ucRcv_size = 1+sizeof(SOCK_DATA_PACKET_T);
	
	pData_buf = (char *)(pSend_buf+sizeof(SOCK_DATA_PACKET_T));		//设置数据指针指向跳过头的长度
	pHeader = (SOCK_DATA_PACKET_T *)pSend_buf;
	pRcvHeader = (SOCK_DATA_PACKET_T *)pRcv_buf;

	memset(szIp_list,0x00,sizeof(struct in_addr)*LIST_MAX);	
	get_ID_IP_list((char *)szIp_list);
    get2ndOsIdIpList((char *)sz2OsIp_list);

	//construct the protocol msg
	//construct procotol header
	get_local_ID((char *)&(pHeader->srcId));
	//as dstID is diffent, construct it later using cTage_ID
	get_local_ID((char *)pTag_ID);
	pHeader->funcCode = FUNC_MANAGE_APP;
	pHeader->operCode = OPER_INDOORLIST_IPID_TABLE;
	pHeader->dataLen[0] = 0;
	pHeader->dataLen[1] = 0;
	pHeader->dataLen[2] = 0;
	pHeader->dataLen[3] = ucData_size;

	pData_buf[0] = LIST_MAX;
	memcpy(&pData_buf[1],szIp_list,4*LIST_MAX);
	get_ID_ReSoluList(&pData_buf[1+4*LIST_MAX]);

	iSend_all = get_list_changed(eSEND_IS_LIST);				//	只有当list变化时，才推送全部，否则只推送之前失败的
	OSA_DBG_MSGX("Send list to %s",iSend_all==1? "all ISs":"IS which send fail before");

	clr_list_changed(eSEND_IS_LIST);
	for(i = 0;i < LIST_MAX && giTmpSend_list_thr_running; i++ )
	{
	    //printf("2ndos list change :send indoor list to indoor and 2ndos  start  1111 szIp_list[i].s_addr=%d,sz2OsIp_list[i].s_addr=%d\n",szIp_list[i].s_addr,sz2OsIp_list[i].s_addr);
		//若IP为空，则未连接，直接continue
		//if(szIp_list[i].s_addr == 0 && sz2OsIp_list[i].s_addr == 0)		
		//	continue;
		
        if(szIp_list[i].s_addr!=0)
		{
		//若不是整个list变化，且之前推送成功，则直接continue
		if(iSend_all == 0 && (get_send_fail_flg(eSEND_IS_LIST)& (1<<i))==0)
			continue ;
        //printf("222222\n");
		memset(pRcv_buf,0x00,ucRcv_size);
		pTag_ID[5] = (i+1);
//		OSA_DBG_MSG("dest_ID=%02x %02x %02x %02x %02x %02x",pTag_ID[0],pTag_ID[1],pTag_ID[2],pTag_ID[3],pTag_ID[4],pTag_ID[5]);
		memcpy(&(pHeader->dstId),pTag_ID,MAX_ID_SIZE);


		//创建socket并尝试与目标IS建立tcp连接

		sockfd = TcpipConnect(inet_ntoa(szIp_list[i]),NET_CMD_PORT,500);
		if(sockfd < 0)
		{
			OSA_DBG_MSGX("Connect with IS%d %s : %d fail,errno(%d)",i+1,inet_ntoa(szIp_list[i]),NET_CMD_PORT,errno);
			send_list_fail_set(i,eSEND_IS_LIST);
			continue;	
		}
        //printf("333333\n");

		OSA_DBG_MSGX("connect IS%d: %s:%d success. sockfd(%d)\n",i+1,inet_ntoa(szIp_list[i]),NET_CMD_PORT, sockfd);
		if(iSend_all == 0 )
			send_list_fail_clr(i,eSEND_IS_LIST);
		
//		display_packets(pSend_buf,ucBuf_size);
	//printf("2ndos list change :send indoor list to indoor and 2ndos 1111 \n");

	//	if(write(sockfd,pSend_buf,ucBuf_size) >0)
		if(send(sockfd,pSend_buf,ucBuf_size,MSG_NOSIGNAL) >0)   //inorder to avoid broken pipe problem if the socket is close or do not existes  ---Andy 2013.06.27
		{
			DisplayNetCmdPacket(pHeader,pData_buf,ucData_size);
			FD_ZERO(&fSets);
		    FD_SET(sockfd,&fSets); 
    		time_out.tv_sec  = 0;    
    		time_out.tv_usec = 200*1000;
		    iRet= select(sockfd + 1, &fSets,  NULL, NULL, &time_out);
		    if(iRet <= 0)
    		{
        		OSA_DBG_MSGX("send the list, but no reply in 200ms.");
    		}
			else
			{
				if(FD_ISSET(sockfd,&fSets)){
					read(sockfd,pRcv_buf,ucRcv_size);
					print_proto_header(pRcvHeader);
				}
			}
		
    	}
		else
			{
			 OSA_ERROR("send fail , errno(%d)", errno);
			}
		close(sockfd);
        usleep(50*1000);
		}
        
        // printf("4444\n");
        
	//printf("2ndos list change :send indoor list to indoor and 2ndos 2222 \n");
        //增加将分机列表发送给二次门口机
        pHeader->dstId[0] = DT_DIGITAL_SECOND_DOOR_STATION;
        if(sz2OsIp_list[i].s_addr == 0 )		
			continue;
        
        sockfd = TcpipConnect(inet_ntoa(sz2OsIp_list[i]),NET_CMD_PORT,500);
		if(sockfd < 0)
		{
			OSA_DBG_MSGX("Connect with IS%d %s : %d fail,errno(%d)",i+1,inet_ntoa(szIp_list[i]),NET_CMD_PORT,errno);
			send_list_fail_set(i,eSEND_IS_LIST);
			continue;	
		}

        if(send(sockfd,pSend_buf,ucBuf_size,MSG_NOSIGNAL) >0)   //inorder to avoid broken pipe problem if the socket is close or do not existes  ---Andy 2013.06.27
		{
			DisplayNetCmdPacket(pHeader,pData_buf,ucData_size);
			FD_ZERO(&fSets);
		    FD_SET(sockfd,&fSets); 
    		time_out.tv_sec  = 0;    
    		time_out.tv_usec = 200*1000;
		    iRet= select(sockfd + 1, &fSets,  NULL, NULL, &time_out);
		    if(iRet <= 0)
    		{
        		OSA_DBG_MSGX("send the list, but no reply in 200ms.");
    		}
			else
			{
				if(FD_ISSET(sockfd,&fSets)){
					read(sockfd,pRcv_buf,ucRcv_size);
					print_proto_header(pRcvHeader);
				}
			}
		
    	}
		else
			{
			 OSA_ERROR("send fail , errno(%d)", errno);
			}
		close(sockfd);

	}

    

}



void send_2ndOS_list(void)
{
	int sockfd;
	int iSend_all = 0;										//用于判断是推送全部列表还是只推送失败的列表
	char pTag_ID[MAX_ID_SIZE];
	struct in_addr szIp_list[LIST_MAX];

	char pSend_buf[1 +4*LIST_MAX+18]={0};					//指向数据段指针，指向发送buf指针, modify by Andy 15.03.26, add video resolution list
	char pRcv_buf[1+18]={0};
	
	char *pData_buf = NULL;
	SOCK_DATA_PACKET_T *pHeader,*pRcvHeader;				//pointer to send header
	BYTE ucData_size, ucBuf_size,ucRcv_size;
	
	int i=0,iRet;	
	fd_set fSets;
	struct timeval time_out;
	ucData_size = 1 +4*LIST_MAX;
	ucBuf_size = ucData_size + sizeof(SOCK_DATA_PACKET_T);
	ucRcv_size = 1+sizeof(SOCK_DATA_PACKET_T);
	
	pData_buf = (char *)(pSend_buf+sizeof(SOCK_DATA_PACKET_T));		//设置数据指针指向跳过头的长度
	pHeader = (SOCK_DATA_PACKET_T *)pSend_buf;
	pRcvHeader = (SOCK_DATA_PACKET_T *)pRcv_buf;

	memset(szIp_list,0x00,sizeof(struct in_addr)*LIST_MAX);	
	get_ID_IP_list((char *)szIp_list);

	//construct the protocol msg
	//construct procotol header
	get_local_ID((char *)&(pHeader->srcId));
	//as dstID is diffent, construct it later using cTage_ID
	get_local_ID((char *)pTag_ID);
	pHeader->funcCode = FUNC_MANAGE_APP;
	pHeader->operCode = OPER_2ND_OS_LIST_IDIP;
	pHeader->dataLen[0] = 0;
	pHeader->dataLen[1] = 0;
	pHeader->dataLen[2] = 0;
	pHeader->dataLen[3] = ucData_size;

	pData_buf[0] = LIST_MAX;
	get2ndOsIdIpList(&pData_buf[1]);

	iSend_all = get_list_changed(eSEND_2ndOS_LIST);				//	只有当list变化时，才推送全部，否则只推送之前失败的
	OSA_DBG_MSGX("Send list to %s",iSend_all==1? "all ISs":"IS which send fail before");

	clr_list_changed(eSEND_2ndOS_LIST);
	for(i = 0;i < LIST_MAX && giTmpSend2ndOSList_thr_running; i++ )
	{
	
		//若IP为空，则未连接，直接continue
		if(szIp_list[i].s_addr == 0 )		
			continue;
		//若不是整个list变化，且之前推送成功，则直接continue
		if(iSend_all == 0 && (get_send_fail_flg(eSEND_2ndOS_LIST)& (1<<i))==0)
			continue ;
		
		memset(pRcv_buf,0x00,ucRcv_size);
		pTag_ID[5] = (i+1);
//		OSA_DBG_MSG("dest_ID=%02x %02x %02x %02x %02x %02x",pTag_ID[0],pTag_ID[1],pTag_ID[2],pTag_ID[3],pTag_ID[4],pTag_ID[5]);
		memcpy(&(pHeader->dstId),pTag_ID,MAX_ID_SIZE);


		//创建socket并尝试与目标IS建立tcp连接

		sockfd = TcpipConnect(inet_ntoa(szIp_list[i]),NET_CMD_PORT,500);
		if(sockfd < 0)
		{
			OSA_DBG_MSGX("Connect with IS%d %s : %d fail,errno(%d)",i+1,inet_ntoa(szIp_list[i]),NET_CMD_PORT,errno);
			send_list_fail_set(i,eSEND_2ndOS_LIST);
			continue;	
		}

		OSA_DBG_MSGX("connect IS%d: %s:%d success. sockfd(%d)\n",i+1,inet_ntoa(szIp_list[i]),NET_CMD_PORT, sockfd);
		if(iSend_all == 0 )
			send_list_fail_clr(i,eSEND_2ndOS_LIST);
		
//		display_packets(pSend_buf,ucBuf_size);

	//	if(write(sockfd,pSend_buf,ucBuf_size) >0)
		if(send(sockfd,pSend_buf,ucBuf_size,MSG_NOSIGNAL) >0)   //in order to avoid broken pipe problem if the socket is close or do not existes  ---Andy 2013.06.27
		{
			DisplayNetCmdPacket(pHeader,pData_buf,ucData_size);
			FD_ZERO(&fSets);
		    FD_SET(sockfd,&fSets); 
    		time_out.tv_sec  = 0;    
    		time_out.tv_usec = 200*1000;
		    iRet= select(sockfd + 1, &fSets,  NULL, NULL, &time_out);
		    if(iRet <= 0)
    		{
        		OSA_DBG_MSGX("send the list, but no reply in 200ms.");
    		}
			else
			{
				if(FD_ISSET(sockfd,&fSets)){
					read(sockfd,pRcv_buf,ucRcv_size);
					print_proto_header(pRcvHeader);
				}
			}
		
    	}
		else
			{
			 OSA_ERROR("send fail , errno(%d)", errno);
			}
		close(sockfd);

	}

}





void send_list_process(void *argv)
{
	OSA_DBG_MSGXX("");
    s_PacketMsg interMsg = { 0 };
    INT32 dataLen = 0;
    BYTE *dataBuf = NULL;

	int sockfd = LocalSocketUDPServer(kPOST_MSG2SENDLIST);
	if(sockfd < 0){
		OSA_DBG_MSGXX("Create socket fail, errno=%d",errno);
		return;
	}
	giSendListRunning = 1;
	giTmpSend_list_thr_running = 0;
	giTmpSend2ndOSList_thr_running = 0;
	
	while(giSendListRunning){
        
        INT32 iRet = ReadFullPacket(sockfd, &interMsg, INTER_PACKET_SIZE, &dataBuf, &dataLen);
        if(iRet >= 0)
        {
			OSA_DBG_MSGXX("recv Msg, orderType=%d, socket=%d, dataLen=%d",interMsg.order_type,interMsg.sockfd,interMsg.datalen);
			
            if(interMsg.order_type == PHONE_ORDER_ENTER_IPGW)
            {            		
				INT opeType = dataBuf[0];
				switch(opeType) {
					/*
					There is  a timer periodictly send SEND command to this thread per 5s to enable send list function
					if the list changed, the send_inside_list thread will send list to all ISs
					else, the send_inside_list thread only send list to the ISs which send fail before
					*/
					case (eSEND_IS_LIST): {
						int iChanged = get_list_changed(opeType);
						int iSendFlag = get_send_fail_flg(opeType);
						int iList_num = get_list_num();
						if ( (iChanged|| iSendFlag ) && iList_num)
						{//只有list发生变化或者之前有推送失败的分机才激活临时推送线程
					
							OSA_DBG_MSG("Receive cmd to sendlist, iChanged =%x iSendFlg =%4x iList_num=%d",iChanged,iSendFlag,iList_num);		
							if(giTmpSend_list_thr_running == 1)
							{
								giTmpSend_list_thr_running = 0; 	//tell send_list_thr exit
								OSA_ThrJoin(&hTmpSendListThread);	//wait for sendlist exit
								OSA_DBG_MSGX("====Old List send thread cancelled successfully");	
							}
							giTmpSend_list_thr_running = 1;
							OSA_ThrCreate(&hTmpSendListThread,(void *)send_inside_is_list,OSA_THR_PRI_NONE, NULL);	
						}
					}break;
					case eSEND_2ndOS_LIST : {
						int iChanged = get_list_changed(opeType);
						int iSendFlag = get_send_fail_flg(opeType);
						int iList_num = get_list_num();
						if ( (iChanged|| iSendFlag ) && iList_num)
						{//只有list发生变化或者之前有推送失败的分机才激活临时推送线程
					
							OSA_DBG_MSG("Receive cmd to sendlist, iChanged =%x iSendFlg =%4x iList_num=%d",iChanged,iSendFlag,iList_num);		
                            if(giTmpSend2ndOSList_thr_running == 1)
							{
								giTmpSend2ndOSList_thr_running = 0; 	//tell send_list_thr exit
								OSA_ThrJoin(&hTmpSend2ndOSListThread);	//wait for sendlist exit
								OSA_DBG_MSGX("====Old List send thread cancelled successfully");	

							}
							giTmpSend2ndOSList_thr_running = 1;
							OSA_ThrCreate(&hTmpSend2ndOSListThread,(void *)send_2ndOS_list,OSA_THR_PRI_NONE, NULL);	

                            
                            //当二次门口机列表发生改变时，也发送室内机列表 add by nancy 
                            if(giTmpSend_list_thr_running == 1)
							{
								giTmpSend_list_thr_running = 0; 	//tell send_list_thr exit
								OSA_ThrJoin(&hTmpSendListThread);	//wait for sendlist exit
								OSA_DBG_MSGX("====Old List send thread cancelled successfully");	

							}
                            int i=0;
                            for(i=0;i<LIST_MAX;i++)
                            {
                                send_list_fail_set(i,eSEND_IS_LIST);
                            }
							giTmpSend_list_thr_running = 1;
							OSA_ThrCreate(&hTmpSendListThread,(void *)send_inside_is_list,OSA_THR_PRI_NONE, NULL);	

						}
					}break;

					default: {
						OSA_DBG_MSGXX("invaild cmd");
					}
										
			    }
				
            }
			
            /*
            */
            SAFE_DELETE_MEM(dataBuf);
        }

        #if 0
		//deal with message from flexisip
		FD_ZERO(&fSets);
		FD_SET(sockfd,&fSets); 
		time_out.tv_sec  = 0;	 
		time_out.tv_usec = 500*1000;
		
		iRet= select(sockfd + 1, &fSets,  NULL, NULL, &time_out);

//		FEEDWATCHDOG(10, eSendList2ISIPGWThr);
		if(iRet > 0)
		{
			if(FD_ISSET(sockfd,&fSets)){
                
				read(sockfd,pRcv_buf, 1);
				INT opeType = pRcv_buf[0];
				switch(opeType) {
					/*
					There is  a timer periodictly send SEND command to this thread per 5s to enable send list function
					if the list changed, the send_inside_list thread will send list to all ISs
					else, the send_inside_list thread only send list to the ISs which send fail before
					*/
					case (eSEND_IS_LIST): {
						int iChanged = get_list_changed(opeType);
						int iSendFlag = get_send_fail_flg(opeType);
						int iList_num = get_list_num();
						if ( (iChanged|| iSendFlag ) && iList_num)
						{//只有list发生变化或者之前有推送失败的分机才激活临时推送线程
					
							OSA_DBG_MSG("Receive cmd to sendlist, iChanged =%x iSendFlg =%4x iList_num=%d",iChanged,iSendFlag,iList_num);		
							if(giTmpSend_list_thr_running == 1)
							{
								giTmpSend_list_thr_running = 0; 	//tell send_list_thr exit
								OSA_ThrJoin(&hTmpSendListThread);	//wait for sendlist exit
								OSA_DBG_MSGX("====Old List send thread cancelled successfully");	

							}
							giTmpSend_list_thr_running = 1;
							OSA_ThrCreate(&hTmpSendListThread,(void *)send_inside_is_list,OSA_THR_PRI_MIN,64*1024,(void *)0);	

						}
						
					}break;
					case eSEND_2ndOS_LIST : {
						int iChanged = get_list_changed(opeType);
						int iSendFlag = get_send_fail_flg(opeType);
						int iList_num = get_list_num();
						if ( (iChanged|| iSendFlag ) && iList_num)
						{//只有list发生变化或者之前有推送失败的分机才激活临时推送线程
					
							OSA_DBG_MSG("Receive cmd to sendlist, iChanged =%x iSendFlg =%4x iList_num=%d",iChanged,iSendFlag,iList_num);		
							if(giTmpSend2ndOSList_thr_running == 1)
							{
								giTmpSend2ndOSList_thr_running = 0; 	//tell send_list_thr exit
								OSA_ThrJoin(&hTmpSend2ndOSListThread);	//wait for sendlist exit
								OSA_DBG_MSGX("====Old List send thread cancelled successfully");	

							}
							giTmpSend2ndOSList_thr_running = 1;
							OSA_ThrCreate(&hTmpSend2ndOSListThread,(void *)send_2ndOS_list,OSA_THR_PRI_MIN,64*1024,(void *)0);	

						}
					}break;

					default: {
						OSA_DBG_MSGXX("invaild cmd");
					}
						
				}
				
			}
		}
        #endif
	
	}
	close(sockfd);
}

/******************************************************************************
* Name:      InitSendListProcess 
*
* Desc:      初始化推送分机列表线程
* Param:     none
* Return:    none
* Global:    
* Note:      
* Author:    Andy-wei.hou
* -------------------------------------
* Log:   2013/04/01, Create this function by Andy-wei.hou
 ******************************************************************************/
VOID InitSendListProcess(VOID)
{
    int iRet = OSA_EFAIL;
	OSA_MutexCreate(&gsz_2ndSenFlg.szListSendMutex);
	OSA_MutexCreate(&gsz_SenFlg.szListSendMutex);
    iRet = OSA_ThreadCreate(&hSendListThread, (VOID *)send_list_process, NULL);
    if (OSA_SOK != iRet)
    {
        perror (" OSA_ThreadCreate(&hSendListThread, (VOID *)send_list_process, NULL) error! \n");
        //exit(1);//直接退出应用程序
    }    
}


/******************************************************************************
* Name:      DeInitSendListProcess 
*
* Desc:      反初始化推送户内分机列表线程
* Param:     none
* Return:    none
* Global:    
* Note:      
* Author:    daniel-qinghua.huang
* -------------------------------------
* Log:   2013/04/01, Create this function by daniel-qinghua.huang
 ******************************************************************************/
VOID DeInitSendListProcess(VOID)
{
    int iRet = OSA_EFAIL;
	giSendListRunning = 0;
    iRet = OSA_ThrDelete(&hSendListThread);
    if (OSA_SOK != iRet)
    {
        perror ("OSA_thrDelete1(&hSendListThread)\n");
        //exit(1);//直接退出应用程序
    }
	OSA_MutexDelete(&gsz_2ndSenFlg.szListSendMutex);
	OSA_MutexDelete(&gsz_SenFlg.szListSendMutex);
}




