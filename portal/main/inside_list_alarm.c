/******************************************************************************
* Copyright 2010-2013 ABB Genway Co.,Ltd.
* FileName: 	 inside_list_alarm.c 
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
#include <network.h>
#include <osa/osa_thr.h>
#include <ipgwCommonDefy.h>


#include "id_ip_maintainer.h"

static OSA_ThrHndl hListAlarmThread;
LOCAL BOOL gbDevice_pinged = TRUE;//本设备是否被上级设备PING成功
LOCAL BOOL gbInside_device_exsit_running = 0;




extern void get_external_IP(char* szExternal_IP);
/******************************************************************************
* Name: 	 print_proto_header 
*
* Desc: 	打印扬子协议中的协议头
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.04.18, Create this function by Andy Hou
 ******************************************************************************/

void print_proto_header(SOCK_DATA_PACKET_T *szProto_header)
{
#if 0
		OSA_DBG_MSG("SRC_ID: %02x %02x %02x %02x %02x %02x"
			,szProto_header->srcId[0]
			,szProto_header->srcId[1]
			,szProto_header->srcId[2]
			,szProto_header->srcId[3]
			,szProto_header->srcId[4]
			,szProto_header->srcId[5]
			);
		OSA_DBG_MSG("DST_ID: %02x %02x %02x %02x %02x %02x"
			,szProto_header->dstId[0]
			,szProto_header->dstId[1]
			,szProto_header->dstId[2]
			,szProto_header->dstId[3]
			,szProto_header->dstId[4]
			,szProto_header->dstId[5]
			);
		OSA_DBG_MSG("FunCode:OpeCode : (%02x :%02x)",szProto_header->funcCode,szProto_header->operCode);
		OSA_DBG_MSG("datalen: %d",szProto_header->dataLen[3]);
#endif
}




/******************************************************************************
* Name: 	 gene_local_os_id 
*
* Desc: 	根据本户的ID生成该单元主分机ID
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.04.18, Create this function by Andy Hou
 ******************************************************************************/

void gene_local_os_id(char *cLocal_id){
	//单元门口机id由本户id处理后得到
	get_local_ID(cLocal_id);
	cLocal_id[0]			  = DT_DOOR_STATION;	
	cLocal_id[MAX_ID_SIZE-3]  = 0x00;
	cLocal_id[MAX_ID_SIZE-2]  = 0x01;
	cLocal_id[MAX_ID_SIZE-1]  = 0x00;

}


/******************************************************************************
* Name: 	 alarm_report_inside_list 
*
* Desc: 	向os上报本户的断线报警信息
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.04.18, Create this function by Andy Hou
 ******************************************************************************/

void alarm_report_inside_list(void){

	if(gbDevice_pinged == FALSE)
		return ;
	
	char cOs_id[MAX_ID_SIZE];
	char cOs_ip_str[36] = {0};
	int sockfd;
	struct sockaddr_in szSev_addr;
	int iRetry = 0;
	INT32 iIndoor_ip_addr[LIST_MAX] = { 0 };
	int i=0;	
	char *pData_buf,*pSend_buf;					//指向数据段指针，指向发送buf指针
	SOCK_DATA_PACKET_T *pHeader;				//pointer to send header
	BYTE ucData_size, ucBuf_size;
	ucData_size = 1 + (1 + LIST_MAX )+1 + (1+ LIST_MAX); /*1ACK +1IsNum + MIs + 1Mode + 1_2ndNum + M2ndOs*/
	ucBuf_size = ucData_size + sizeof(SOCK_DATA_PACKET_T);
	
	pSend_buf=(char *)malloc(ucBuf_size);
	if (!pSend_buf){
		OSA_ERROR("malloc sendbuf of report inside list fail\n");
		return;
	}
		//创建socket并尝试与os-adapter建立tcp连接
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0){
		OSA_ERROR("report_inside_list: create socket fail");
		return;
	}
	
	pData_buf = (char *)(pSend_buf+sizeof(SOCK_DATA_PACKET_T));		//设置数据指针指向跳过头的长度
	pHeader = (SOCK_DATA_PACKET_T *)pSend_buf;

	//获取本单元一号os的ip
	gene_local_os_id(cOs_id);
	generate_ip_by_bcdid(cOs_id,cOs_ip_str);
	OSA_DBG_MSG("default os ip= %s\n",cOs_ip_str);


	bzero(&szSev_addr,sizeof(szSev_addr));
	szSev_addr.sin_family = AF_INET;
	szSev_addr.sin_port = htons(NET_CMD_PORT);
	szSev_addr.sin_addr.s_addr = inet_addr(cOs_ip_str);
	for(iRetry=0; iRetry <3;iRetry++)
	{
		if(connect(sockfd,&szSev_addr,sizeof(szSev_addr)) < 0)	{
			OSA_DBG_MSG("connect %s:%d fail\n",cOs_ip_str,NET_CMD_PORT);
			sleep(1);
		}
		else{
			break;
		}
	}
	if (iRetry >= 3){
		//report fail, disable offline alarm
		gbDevice_pinged = FALSE;
		free(pSend_buf);
		close(sockfd);
		return;
	}

	OSA_DBG_MSG("connect os_adapter %s:%d success\n",cOs_ip_str,NET_CMD_PORT);
	get_local_ID((char *)&(pHeader->dstId));
	memcpy(&(pHeader->srcId),cOs_id,MAX_ID_SIZE);	
	pHeader->funcCode = FUNC_MANAGE_APP;
	pHeader->operCode = OPER_INQUERY_INDOOR_LIST_ACK;
    
	//pHeader->dataLen[0] = 0;
	//pHeader->dataLen[1] = 0;
	//pHeader->dataLen[2] = 0;
	//pHeader->dataLen[3] = ucData_size+1;
    MAKE_DATALEN(pHeader->dataLen, (ucData_size+1));


	pData_buf[0] = SOCK_ACK_OK;
	pData_buf[1] = LIST_MAX;	

	/*IS*/
	get_ID_IP_list((char *)iIndoor_ip_addr);
	for(i = 0; i < LIST_MAX; i ++)
	{
		pData_buf[i + 2] =	((iIndoor_ip_addr[i] == 0) ? 0 : 1);
	}
	pData_buf[i + 2] = YR_HOUSE_TYPE4;

	/*2ndOs*/
	pData_buf[2+LIST_MAX+1] = LIST_MAX;
	memset(iIndoor_ip_addr,0x00,sizeof(iIndoor_ip_addr));
	get2ndOsIdIpList((char *)iIndoor_ip_addr);
	for(i = 0; i < LIST_MAX; i ++)
	{
		pData_buf[i + 2 + LIST_MAX +2] =	((iIndoor_ip_addr[i] == 0) ? 0 : 1);
	}
	

    if(SendAckTCP(pHeader, (BYTE *)pData_buf, ucData_size, sockfd) == FALSE)
    {
        OSA_ERROR("SendAckTCP FAILED.");
    }
    close(sockfd);
	free(pSend_buf);
	
}



/******************************************************************************
* Name:  inside_device_exist_listen
*
* Desc:  对于设备编号不为1的设备，启动此线程监听DEV_EXIST_PORT端口的连接
* Param:  
*        none
* Return:  
*        none
*
* Global:  
*          
* Note: 
* Author:  Andy-wei.hou
* -------------------------------------
* Log:   2013/04/10, Andy-wei.hou Create this function
******************************************************************************/
LOCAL VOID inside_device_exist_listen(void)
{
	struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int sockfd_tcp = -1;
    char cExternal_IP[18] = {0};
    int sockfd = -1;

    OSA_THREAD_ID("prn_thread_id");
	get_external_IP((char *)cExternal_IP);
	OSA_DBG_MSG("extern_ip = %s",cExternal_IP);

    sockfd_tcp = InitSocketTCP(cExternal_IP, DEV_EXIST_PORT);

    if(-1 == sockfd_tcp)
    {
        OSA_ERROR("Unexpect error ...");
    }
    
	while(gbInside_device_exsit_running)
	{
        sockfd = accept(sockfd_tcp, (struct sockaddr *)&addr, &len);
        if(sockfd <= 0)
        {
            OSA_ERROR("accept return sockfd = %d errno=%d\n", sockfd, errno);
            OSA_Sleep(5 * MSEC);
            continue;
        }
        close(sockfd);
        gbDevice_pinged = TRUE;
		OSA_DBG_MSG("listen up device connected \n");
        //OSA_DBG_MSG("\n[%u],(%s, %d),accepted from (%s) socket(%d) \n", GetTimeValEx(), __FUNCTION__, __LINE__, inet_ntoa(addr.sin_addr), sockfd);
    }

    close(sockfd_tcp);
}


VOID init_inside_list_alarm( void ){
	int iRet = -1;
	gbInside_device_exsit_running = 1;
	gbDevice_pinged = FALSE;
	iRet = OSA_ThreadCreate(&hListAlarmThread, (VOID *)inside_device_exist_listen, NULL);
	if(iRet!=OSA_SOK)
	{
		perror ("Create pthread error!\n");
		exit(1);
	}
	
}

VOID deInit_inside_list_alarm(void){
	gbInside_device_exsit_running= 0;
	gbDevice_pinged = FALSE;	
}







