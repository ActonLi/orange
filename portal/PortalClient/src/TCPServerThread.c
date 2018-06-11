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

#include "osa/osa.h"
#include "osa/osa_mem.h"
#include "osa/osa_debug.h"
#include "osa/osa_thr.h"
#include "osa/osa_mutex.h"
#include "osa/osa_time.h"

#include "id_ip_conf.h"
#include "network.h"
#include "PortalClient.h"

LOCAL OSA_ThrHndl hPortalClientTCPServerThread;
BYTE g_u8Type = 255;

#define TYPE_POS 0
#define TYPE_LEN 1
#define SESSION_ID_POS TYPE_POS + TYPE_LEN
#define SESSION_ID_LEN 32
#define TIME_STAMP_OFFSET_POS SESSION_ID_POS + SESSION_ID_LEN
#define TIME_STAMP_OFFSET_LEN 1
#define LOCALID_POS TIME_STAMP_OFFSET_POS + TIME_STAMP_OFFSET_LEN 
#define LOCALID_LEN 16
#define REMOTEID_POS LOCALID_POS + LOCALID_LEN
#define REMOTEID_LEN 128
#define IMAGELEN_POS REMOTEID_POS + REMOTEID_LEN
#define IMAGELEN_LEN 4
#define IMAGEDATA_POS IMAGELEN_POS + IMAGELEN_LEN

char* PortalClientGetISO8601Time(BYTE bOffsetInSencond)
{
    static char szTimeBuf[32] = {0};
    struct timeval tv;
    struct timezone tz;
    struct tm *tm;
    memset(szTimeBuf,0,32);
    gettimeofday(&tv,&tz);
	tv.tv_sec = tv.tv_sec + bOffsetInSencond;
    tm = localtime(&tv.tv_sec);
    strftime(szTimeBuf,sizeof(szTimeBuf),"%FT%T%z",tm);
	return szTimeBuf;
}

LOCAL VOID DealEvent(BYTE *pMsgBuf)
{
	BYTE u8TimeStampOffset = 0;
	UINT32 u32ImageLen = 0;
	InternalEventInfo stinfo;
	memset(&stinfo,0,sizeof(stinfo));
	
	stinfo.type = pMsgBuf[TYPE_POS];
	OSA_MemCopy(stinfo.sessionID,&pMsgBuf[SESSION_ID_POS],SESSION_ID_LEN);
	u8TimeStampOffset = pMsgBuf[TIME_STAMP_OFFSET_POS];
	sprintf(stinfo.iostime,"%s",PortalClientGetISO8601Time(u8TimeStampOffset));
	OSA_MemCopy(stinfo.localid,&pMsgBuf[LOCALID_POS],LOCALID_LEN);
	OSA_MemCopy(stinfo.remoteid,&pMsgBuf[REMOTEID_POS],REMOTEID_LEN);
	u32ImageLen = MAKEFOURCC_BE(pMsgBuf[IMAGELEN_POS], pMsgBuf[IMAGELEN_POS+1], pMsgBuf[IMAGELEN_POS+2], pMsgBuf[IMAGELEN_POS+3]);
	stinfo.snapshot.imglength = u32ImageLen;
	if(u32ImageLen > 0)
	{
		stinfo.snapshot.imgbuf = (char *)&pMsgBuf[IMAGEDATA_POS];
	}
	SendEventToPortalServer(&stinfo);	
}

LOCAL VOID PortalClientTCPCommandProcess(void *arg)
{
    struct timeval tm ;
    fd_set set;
    int result = -1;
    SOCK_DATA_PACKET_T cmdPacket = { { 0 }, { 0 }, 0, 0, { 0 } };
    INT32 len = 0;
    BYTE *dataBuf = NULL;
    INT32 dataLen = 0;
	s_PacketMsg *pInterMsg = (s_PacketMsg *)arg;
    int sockfd = pInterMsg->sockfd;
	BOOL bRet = FALSE;
	BYTE pu8AckData[2];

	if(sockfd < 0)
    {
		SAFE_DELETE_MEM(arg);
        return;
	}
     
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

    len = read(sockfd,  &cmdPacket,  sizeof(cmdPacket));
    if(len != sizeof(cmdPacket))
    {
    	SAFE_DELETE_MEM(arg);
        close(sockfd);
        return;
    }
    dataLen = MAKEFOURCC_BE(cmdPacket.dataLen[0], cmdPacket.dataLen[1], cmdPacket.dataLen[2], cmdPacket.dataLen[3]);   

	if(pInterMsg->order_type == PHONE_ORDER_ENTER_NET)
	{
	    if(cmdPacket.funcCode == FUNC_TRANS_FILE)
	    {
	        switch(cmdPacket.operCode)
	        {
	            case OPET_PUSH_EVENT_TO_IPGATEWAY:
				{
					dataBuf = OSA_MemMalloc(dataLen + sizeof(SOCK_DATA_PACKET_T));
					NOT_MEM_PRINT(dataBuf);
					OSA_MemCopy(dataBuf, (VOID *)&cmdPacket, sizeof(SOCK_DATA_PACKET_T));	 
					bRet = DoRecvDataPacketEx(sockfd, dataLen, dataBuf + sizeof(SOCK_DATA_PACKET_T));
					pInterMsg->datalen = dataLen + sizeof(SOCK_DATA_PACKET_T);
					if(pInterMsg->datalen < 512)
					{
						DisplayNetCmdPacket(&cmdPacket, dataBuf + sizeof(SOCK_DATA_PACKET_T), dataLen);    
					}
					
					pu8AckData[0] = dataBuf[sizeof(SOCK_DATA_PACKET_T)];
						
					if(bRet == TRUE)
					{
						pu8AckData[1] = 0;
					}
					else
					{
						pu8AckData[1] = 1;
					}

					if(! SendAckTCP(&cmdPacket, pu8AckData, 2, sockfd))
					{
						OSA_ERRORX("Portal Client Server Send TCP Ack");
					}

					DealEvent(&dataBuf[sizeof(SOCK_DATA_PACKET_T)]);

					SAFE_DELETE_MEM(dataBuf);
					SAFE_DELETE_MEM(arg);
					close(sockfd);
					return;
	            }
	            break;

	        }
	    }
	}
    
}


LOCAL VOID PortalClientTCPServerProcessTask(void)
{
	struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int sockfd_tcp = -1;
    int sockfd = -1;
    int ret = -1;
    OSA_ThrHndl  hWorkThread;
 
    sockfd_tcp = InitSocketTCP(NULL, 8001);

    // !review by Davis --需判断返回值
    if(-1 == sockfd_tcp)
    {
        OSA_ERROR("Unexpect error ...");
        ASSERT(0);
    }

	while(1)
	{
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
		pInterMsg->order_type = PHONE_ORDER_ENTER_NET;

	 	struct sockaddr_in guest;                        
		char guest_ip[20];
		socklen_t guest_len = sizeof(guest);
		getpeername(sockfd, (struct sockaddr *)&guest, &guest_len);
		inet_ntop(AF_INET, &guest.sin_addr, guest_ip, sizeof(guest_ip));
		OSA_DBG_MSGXX("Connect Remote peer IP=%s",guest_ip);
		
		ret = OSA_ThreadCreate(&hWorkThread, (VOID *)PortalClientTCPCommandProcess, (void *)pInterMsg);
        if(ret != OSA_SOK)
        {
            OSA_ERROR("Create pthread error!\n");
            SAFE_DELETE_MEM(pInterMsg);
            continue;
        }
    }

    close(sockfd_tcp);
}



VOID fnCreatePortalClientTCPServerThread(VOID)
{
    int iRet = OSA_EFAIL;

	iRet = OSA_ThreadCreate(&hPortalClientTCPServerThread, (VOID *)PortalClientTCPServerProcessTask, NULL);
    if(OSA_SOK != iRet)
    {
		perror ("OSA_ThreadCreate(&hPortalClientTCPServerThread, (void *)PortalClientTCPServerProcessTask, NULL) error!\n");
		exit(1);//直接退出应用程序
    }
}

