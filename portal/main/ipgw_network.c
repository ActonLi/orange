#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>   
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>     
#include <termios.h>    
#include <errno.h>  
#include <linux/sockios.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <netdb.h>

#include "network.h"
#include "osa/osa.h"
#include "osa/osa_thr.h"
#include "osa/osa_mutex.h"
#include "osa/osa_dir.h"
#include "osa/osa_file.h"
#include "osa/osa_time.h"
#include "osa/osa_debug.h"
#include "osa/osa_mem.h"
#include "config.h"
#include "history.h"
#include "setting_interface.h"

LOCAL OSA_ThrHndl hUDPGroupThread;

extern INT32 UDPGroupThreadRunning = 0;
extern char UDPBroadcast_IP[32] = { 0 };
extern char UDPBindlocal_IP[32] = { 0 };
extern int UDPBroadcast_PORT = 0;
LOCAL INT32 UniCastUDPThreadRunning = 0;
LOCAL OSA_ThrHndl hUniCastUDPGroupThread;

LOCAL INT32 AcceptThreadRunning = 0;
LOCAL OSA_ThrHndl hAcceptThread;

//extern OSA_ThrHndl hUDPGroupThread;

VOID ipgwDealTcpCmd(s_PacketMsg *interMsg,BYTE *dataBuf){
	SOCK_DATA_PACKET_T *cmdPacket = NULL;
	BYTE *pData = NULL;
	OSA_DBG_MSGXX("");

	if(!interMsg || !dataBuf)
		return;
	OSA_DBG_MSGXX("");

	cmdPacket = (SOCK_DATA_PACKET_T *)dataBuf;
	pData = dataBuf + sizeof(SOCK_DATA_PACKET_T);

	if(cmdPacket->srcId[0] == 0x10 && cmdPacket->dstId[0] == 0x20) /*cmd from ui to logic*/
		interMsg->order_type = PHONE_ORDER_ENTER_QT;


	if(interMsg->order_type == PHONE_ORDER_ENTER_QT){
		SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
	}
	else
	{
		OSA_DBG_MSGXX("cmdPacket->funcCode %d ",cmdPacket->funcCode);
		switch(cmdPacket->funcCode)
	    {
	
	        case FUNC_SYS_CONFIGURATION:
				SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
				break;
	            
	        case FUNC_MANAGE_APP:
				SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
				break;
	            
	        case FUNC_TRANS_FILE:
	           	if(cmdPacket->operCode == OPTION_CALLHISTORY_REPORT)	/*recving OS/GU reported talk history records*/
				{
				//	dealRcvHistoryRecords(interMsg,dataBuf);

				}
				break;

	        default:
	            OSA_DBG_MSG("\nWARNNING ... %s, %d ... cmdPacket.funcCode(%d) ...\n", __FILE__, __LINE__, cmdPacket->funcCode);
	            close(interMsg->sockfd);
	            break;
		}
	}
}



VOID ipgwDealUdpCmd(s_PacketMsg *interMsg,BYTE *dataBuf){

	if(!interMsg || !dataBuf)
		return -1;
	
	SOCK_DATA_PACKET_T *cmdPacket = dataBuf;
	/*
	按照功能码将命令头，命令数据，命令数据长度发送给相应的功能线程处理。
	*/
	switch(cmdPacket->funcCode)
	{
	   case FUNC_SYS_CONFIGURATION:
	   {
		   switch(cmdPacket->operCode)
		   {
			   case OPER_SET_TIME:
			   case OPER_NET_CONNECTED_DEVICE_SEARCH:
			   //case OPER_GET_MAC_ADDR:				  
			   case OPER_SET_CALL_GUARD_SEQUENCE:
			   case OPER_SET_SOS_DC_LIFT_ONOFF:
			   case OPER_SET_SMH_URL:
			   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
			   break;
		   }
	   }
	   break;


	   
	   case FUNC_MESSAGE:
	   {
		   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
	   }
	   break;

	   default:
		   OSA_DBG_MSG("\nWARNNING ... %s, %d ... cmdPacket.funcCode(%d), ope=(%d) ...\n", __FILE__, __LINE__, cmdPacket->funcCode,cmdPacket->operCode);
		   break;  
	}			
}



/******************************************************************************
* Name:  TCPCommandProcess
*
* Desc:  处理连接到本机的TCP socket命令
* Param:  
*        arg, 接收数据的socket句柄
*
*
* Return:  
*        none
*
* Global:  所有的TCP命令都是连接一次，处理完之后就释放此连接。
* Note:    接收到的socket包(包括header，data)都放到统一内部消息的数据字段，并添加
*          统一内部消息头，然后发送到各个处理线程。
* Author:  liqun.wei
* -------------------------------------
* Log:   2012/2/6, liqun.wei Create this function
******************************************************************************/
LOCAL char TCPLink_IP[32] = { 0 };
LOCAL int  TCPLink_PORT = 0;

LOCAL VOID TCPCommandProcess(void *arg)
{
    struct timeval tm ;
    fd_set set;
    int result = -1;
    SOCK_DATA_PACKET_T cmdPacket = { { 0 }, { 0 }, 0, 0, { 0 } };
    INT32 len = 0;
    BYTE *dataBuf = NULL;
    INT32 dataLen = 0;
//    int sockfd = (INT32)arg;
//    s_PacketMsg interMsg = { PHONE_ORDER_ENTER_NET, sockfd, 0 };
	s_PacketMsg *pInterMsg = (s_PacketMsg *)arg;
    int sockfd = pInterMsg->sockfd;


    if(sockfd < 0){
		SAFE_DELETE_MEM(arg);
        return;
	}

    OSA_THREAD_ID("prn_thread_id");    
    
__TCPCommandProcess__Read__Next__:

    OSA_DBG_MSGXX("socket(%d) \n", sockfd);
    
    FD_ZERO(&set);
    FD_SET(sockfd,&set); 
    tm.tv_sec  = 5;    
    tm.tv_usec = 0;
    result = select(sockfd + 1, &set,  NULL, NULL, &tm);
    if(result <= 0)
    {
        OSA_ERRORX("connected, but no data in 5s.");
		SAFE_DELETE_MEM(arg);
        close(sockfd);
        return;
    }

    errno = 0;
    len = read(sockfd,  &cmdPacket,  sizeof(cmdPacket));
    if(len != sizeof(cmdPacket))
    {
        OSA_ERRORX("len(%d), errno(%d)", len, errno);
		SAFE_DELETE_MEM(arg);
        close(sockfd);
        return;
    }
    dataLen = MAKEFOURCC_BE(cmdPacket.dataLen[0], cmdPacket.dataLen[1], cmdPacket.dataLen[2], cmdPacket.dataLen[3]);   
    dataBuf = OSA_MemMalloc(dataLen + sizeof(SOCK_DATA_PACKET_T));
    NOT_MEM_PRINT(dataBuf);
    OSA_MemCopy(dataBuf, (VOID *)&cmdPacket, sizeof(SOCK_DATA_PACKET_T));    
    DoRecvDataPacketEx(sockfd, dataLen, dataBuf + sizeof(SOCK_DATA_PACKET_T));
    pInterMsg->datalen = dataLen + sizeof(SOCK_DATA_PACKET_T);
    DisplayNetCmdPacket(&cmdPacket, dataBuf + sizeof(SOCK_DATA_PACKET_T), dataLen);    


	ipgwDealTcpCmd(pInterMsg,dataBuf);
   
    SAFE_DELETE_MEM(dataBuf);
	SAFE_DELETE_MEM(arg);
}   


LOCAL VOID TCPLinkProcessTask(void)
{
	struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int sockfd_tcp = -1;
    int sockfd = -1;
    int ret = -1;
    OSA_ThrHndl  hWorkThread;

    OSA_THREAD_ID("prn_thread_id");
    
    sockfd_tcp = InitSocketTCP(NULL, NET_CMD_PORT2);
	OSA_DBG_MSGXX("");
    // !review by Davis --需判断返回值
    if(-1 == sockfd_tcp)
    {
        OSA_ERROR("Unexpect error ...");
        ASSERT(0);
    }
	OSA_DBG_MSGXX("");

	while(AcceptThreadRunning)
	{
        errno = 0;
        sockfd = accept(sockfd_tcp, (struct sockaddr *)&addr, &len);
        if(sockfd <= 0)
        {
            OSA_ERROR("accept return sockfd = %d errorno=%d\n", sockfd, errno);
            OSA_Sleep(1000);
            continue;
        }
        int flag = 1;
        ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
        if(ret < 0)
        {
            OSA_ERRORX("setsockopt IPPROTO_TCP TCP_NODELAY failed.");
        }
        
		s_PacketMsg *pInterMsg = (s_PacketMsg *)OSA_MemMalloc(sizeof(s_PacketMsg));
		if(!pInterMsg)
        {
			OSA_ERROR("Malloc Fail, errno=%d",errno);
			continue;
		}
		pInterMsg->sockfd = sockfd;

	 	struct sockaddr_in guest;                        
		char guest_ip[20];
		socklen_t guest_len = sizeof(guest);
		getpeername(sockfd, (struct sockaddr *)&guest, &guest_len);
		inet_ntop(AF_INET, &guest.sin_addr, guest_ip, sizeof(guest_ip));
		OSA_DBG_MSGXX("IPGW Connect Remote peer IP=%s",guest_ip); 
		if(strcmp(guest_ip,"127.0.0.1")==0)
			pInterMsg->order_type= PHONE_ORDER_ENTER_QT;
		else
			pInterMsg->order_type = PHONE_ORDER_ENTER_NET;

        ret = OSA_ThreadCreate(&hWorkThread, (VOID *)TCPCommandProcess, (void *)pInterMsg);
        if(ret != OSA_SOK)
        {
            OSA_ERROR("Create pthread error!\n");
            SAFE_DELETE_MEM(pInterMsg);
            continue;
        }
    }

    close(sockfd_tcp);
}


LOCAL VOID dealUdpCommand(s_PacketMsg *interMsg,BYTE *dataBuf)
{
	if(!interMsg || !dataBuf)
	return -1;
	
	SOCK_DATA_PACKET_T *cmdPacket = dataBuf;
	/*
	按照功能码将命令头，命令数据，命令数据长度发送给相应的功能线程处理。
	*/
	if(cmdPacket->operCode != OPER_GET_IPACTUATOR_LIST)
	{
		DisplayNetCmdPacket((SOCK_DATA_PACKET_T *)cmdPacket, dataBuf + sizeof(SOCK_DATA_PACKET_T) , interMsg->datalen - sizeof(SOCK_DATA_PACKET_T));
	}
	
	switch(cmdPacket->funcCode)
	{
	   case FUNC_SYS_CONFIGURATION:
	   {
		   switch(cmdPacket->operCode)
		   {
			   case OPER_SET_TIME:
			   case OPER_NET_CONNECTED_DEVICE_SEARCH:
			   //case OPER_GET_MAC_ADDR:				  
			   case OPER_SET_CALL_GUARD_SEQUENCE:
			   case OPER_SET_SOS_DC_LIFT_ONOFF:
			   case OPER_SET_SMH_URL:
			  // case OPER_GET_IPACTUATOR_LIST:
			   case OPER_GET_IPACTUATOR_LIST_ACK:
			   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
			   break;
			   default:
			   break;
		   }
	   }
	   break;
	   case FUNCCODE_DOORCONTROL:
	   {
		   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
	   }
	   break;	   
	   case FUNC_MESSAGE:
	   {
		   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
	   }
	   break;
	   case FUNC_TRANS_FILE:
	   {
	   	   switch(cmdPacket->operCode)
		   {
			   case OPER_UPDATE_COMPLETE:
		   	   SendMsg2IPGWCfg((BYTE *)interMsg, INTER_PACKET_SIZE, dataBuf, interMsg->datalen);
			   break;
			   default:
			   break;
	   	   }
	   }
	   break;
	   default:
	   OSA_DBG_MSG("\nWARNNING ... %s, %d ... cmdPacket.funcCode(%d), ope=(%d) ...\n", __FILE__, __LINE__, cmdPacket->funcCode,cmdPacket->operCode);
	   break;  
	}	
}







VOID UDPCommandProcess(VOID)
{
    struct timeval tm;
    fd_set set;
    int result = -1;
    s_PacketMsg interMsg = { 0 };
    INT32 len = 0;
    BYTE *dataBuf = NULL;
    
    static BYTE tmpDataSave[2048] = { 0 };
    UINT64 tmpDataAtTime = 0;
    
    BYTE tmpData[2048] = { 0 };
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

    
	OSA_THREAD_ID("prn_thread_id");
	OSA_DBG_MSGXX("SystemDeviceInfo_GetCommunityIPAddress %s \n",SystemDeviceInfo_GetCommunityIPAddress());
    int sock = InitSocketUDPMultiRecv(YELLOW_RIVER_GROUP_PORT, YELLOW_RIVER_GROUP_ADDR, SystemDeviceInfo_GetCommunityIPAddress());
    if(sock < 0)
    {
        OSA_ERRORX("UDP broadcast process thread failed.\n");
        return;
    }
	OSA_DBG_MSGXX("");
    while(UDPGroupThreadRunning)
    {
        FD_ZERO(&set);
        FD_SET(sock,&set); 
        tm.tv_sec  = 0;    
        tm.tv_usec = 500 * 1000;
        result = select(sock + 1, &set, NULL, NULL, &tm);
        
        if(result <= 0)
        {
            continue;
        }
        
        if (FD_ISSET(sock, &set))
        {   
            len = recvfrom(sock, tmpData, sizeof(tmpData), 0, (struct sockaddr *)&from, &fromlen);
            if(len <= 0)
            {
                OSA_ERRORX("len(%d, %d)\n", len, sizeof(tmpData));
                continue;
            }
			
       
			if( (strcmp(inet_ntoa(from.sin_addr),SystemDeviceInfo_GetHomeIPAddress()) == 0) ||	(strcmp(inet_ntoa(from.sin_addr),SystemDeviceInfo_GetCommunityIPAddress()) == 0) )
			{
				OSA_DBG_MSGXX("discard my own packet");
				continue;
			}
					
		   if(((OSA_TimeGetTimeValEx() - tmpDataAtTime) < 500) && (memcmp(tmpData, tmpDataSave, len) == 0))
		   {
			   continue;
		   }
            OSA_MemCopy(tmpDataSave, tmpData, len);
            tmpDataAtTime = OSA_TimeGetTimeValEx();
            dataBuf = OSA_MemMalloc(len);
			NOT_MEM_PRINT(dataBuf);            
            if(dataBuf == NULL)
            {
                OSA_ERROR("dataBuf MemMalloc FAILED (%d)", len);
                continue;
            }
			
            //DisplayNetCmdPacket((SOCK_DATA_PACKET_T *)tmpData, tmpData + sizeof(SOCK_DATA_PACKET_T), len - sizeof(SOCK_DATA_PACKET_T));
            OSA_MemCopy(dataBuf, tmpData, len);
            interMsg.datalen = len;
            interMsg.sockfd = -1;
            interMsg.order_type = PHONE_ORDER_ENTER_NET;

			OSA_DBG_MSGXX("");
			/*
			           按照功能码将命令头，命令数据，命令数据长度发送给相应的功能线程处理。
			*/
		   	dealUdpCommand(&interMsg,dataBuf);
            SAFE_DELETE_MEM(dataBuf);
        }
    }
    close(sock);
}





LOCAL OSA_ThrHndl hTransfer239GroupProcess = { .hndl = OSA_THREAD_HANDLE_INVLALID };
LOCAL INT iTransfer239GroupProcess = 0;
VOID Init239TransferGroupRecvProcess(VOID)
{
    s_PacketMsg interMsg = { 0 };
    SOCK_DATA_PACKET_T *header = NULL;
    BYTE *cmdData = NULL;
    INT32 cmdDataLen = 0;

    BYTE ackData[256] = { 0 };
    INT32 ackDataLen = 0;
    
    INT32 dataLen = 0;
    BYTE *dataBuf = NULL;
    int sockfd = -1;

    OSA_THREAD_ID("prn_thread_id");
    sockfd = LocalSocketUDPServer(kPORT_Transfer239GroupProcess);
    if(sockfd < 0)
    {
        OSA_ERROR("LocalSocketUDPServer(kPORT_Transfer239GroupProcess)!\n");
        exit(1);//直接退出应用程序
    }
    
    while(iTransfer239GroupProcess)
    {
        INT32 iRet = ReadFullPacket(sockfd, &interMsg, INTER_PACKET_SIZE, &dataBuf, &dataLen);
        
        //OSA_ERRORX("iRet(%d)\n\n", iRet);
        
        if (iRet < 0) continue;

        header = (SOCK_DATA_PACKET_T *)dataBuf;
        cmdData = dataBuf + sizeof(SOCK_DATA_PACKET_T);
        cmdDataLen = dataLen - sizeof(SOCK_DATA_PACKET_T);
        DisplayNetCmdPacket(header, cmdData, cmdDataLen);
        
    	switch(header->funcCode)
    	{
    	  case FUNC_SYS_CONFIGURATION:
    	  {
    		  switch(header->operCode)
    		  {
    			  case OPER_SET_TIME:
    			  case OPER_NET_CONNECTED_DEVICE_SEARCH:
    			  case OPER_GET_MAC_ADDR:				 
    			  case OPER_SET_CALL_GUARD_SEQUENCE:
                  case OPER_SET_SOS_DC_LIFT_ONOFF:
    			  case OPER_SET_SMH_URL:
				  case OPER_GET_IPACTUATOR_LIST_ACK:	
    			  SendMsg2SystemConfiguration((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
    			  break;
    		  }
    	  }
    	  break;

    	  case FUNCCODE_DOORCONTROL:
    	  {
		  	   SendMsg2UnlockPort((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
    	  }
    	  break;

    	  case FUNC_ALARM_SYSTEM:
    	  {
    		  if(header->operCode == OPER_DISCONNECT_DETECT_ONOFF)
    		  {
    			  //SendMsg2Phone((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
    		  }
    	  }
    	  break;
    	  
    	  case FUNC_MESSAGE:
    	  {
    		  SendMsg2DealInfo((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
    	  }
    	  break;

    	  // for IpGateWay2，接收ipgateway广播的ip，id表，发送监视线程处理。
    	  case FUNC_MANAGE_APP:
    	  {
    		  SendMsg2SystemConfiguration((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
    	  }
    	  break;

    	  case FUNC_ELEC_APP_CONTROL:
    	  {
    		  if(header->operCode == OPER_CALL_ELEVATOR_STATUS)
    		  {
    			  //DisplayNetCmdPacket(header, dataBuf + sizeof(SOCK_DATA_PACKET_T), interMsg->datalen - sizeof(SOCK_DATA_PACKET_T));
    		  }
    	  }
    	  break;
		  case FUNC_TRANS_FILE:
		  {
			if(OPER_UPDATE_COMPLETE == header->operCode)
			{
				SendMsg2Update((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
			}
		  }
		  break;
    	}
		   SAFE_DELETE_MEM(dataBuf);
    }
}

VOID Init239TransferGroupRecv(VOID)
{
    INT32 iRet = -1;
    if (iTransfer239GroupProcess == 0)
    {
        iTransfer239GroupProcess = 1;
        iRet = OSA_ThreadCreate(&hTransfer239GroupProcess, Init239TransferGroupRecvProcess, NULL);
        if(OSA_SOK != iRet)
        {
            perror ("OSA_ThreadCreate(&hTransfer239GroupProcess, (void *)Init239TransferGroupRecvProcess, NULL) error!\n");
            exit(1);//直接退出应用程序
        }
    }
}



void ipgwInitYellowRiverProtocol()
{
	char extIP[16] = { 0 };
	int devicemode = Engineer_Settings_GetDevicemode();
	int iRet;
#if 0
	get_external_IP(extIP);
	AcceptThreadRunning = 1;
   int iRet = OSA_ThreadCreate(&hAcceptThread, (VOID *)TCPLinkProcessTask, NULL);
   if(OSA_SOK != iRet)
   {
	   perror ("OSA_ThreadCreate(&hAcceptThread, (void *)TCPLinkProcessTask, NULL) error!\n");
	   exit(1);//直接退出应用程序
   }
#endif
	if(devicemode == eDeviceModeMaster){
		//InitYellowRiverBroadCast(YELLOW_RIVER_GROUP_ADDR,extIP,YELLOW_RIVER_GROUP_PORT,ipgwDealUdpCmd);	
		UDPGroupThreadRunning = 1;
		iRet = OSA_ThreadCreate(&hUDPGroupThread, (OSA_ThrEntryFunc)UDPCommandProcess, NULL);
	    if(OSA_SOK != iRet)
	    {
	        perror ("OSA_ThreadCreate(&hUDPGroupThread, (void *)UDPCommandProcess, NULL) error!\n");
	        exit(1);//直接退出应用程序
	    }

		AcceptThreadRunning = 1;
		iRet = OSA_ThreadCreate(&AcceptThreadRunning, (OSA_ThrEntryFunc)TCPLinkProcessTask, NULL);
	    if(OSA_SOK != iRet)
	    {
	        perror ("OSA_ThreadCreate(&hUDPGroupThread, (void *)UDPCommandProcess, NULL) error!\n");
	        exit(1);//直接退出应用程序
	    }
		OSA_DBG_MSGXX("");
		Init239TransferGroupRecv();
	}
	

}


