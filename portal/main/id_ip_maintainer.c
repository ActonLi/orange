/******************************************************************************
* Copyright 2010-2013 ABB Genway Co.,Ltd.
* FileName: 	 id_ip_maintainer.c 
* Desc:
* 
* 
* Author: 	Andy Hou
* Date: 	 	2013.03.25
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2013.03.25, Andy Hou create this file
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
#include <osa/osa_timer.h>
#include <osa/osa_thr.h>
#include <osa/osa_debug.h>
#include <osa/osa_mem.h>

#include <network.h>
#include "id_ip_maintainer.h"
#include "nat_maintainer.h"
#include "inside_list_alarm.h"
#include "inside_list_send.h"
#include "process_interface.h"
//#include "watchdog.h"
//#include "public.h"
//#include "config.h"

//#define DEBUG_MAITAINER
#ifdef DEBUG_MAITAINER
#define	MAITAINER_DBG(...) \
	do \
	{	\
		 fprintf(stdout,"\n"); \
		 fprintf(stdout, "\nT$ %llu ", GetTimeValEx()); \
		 fprintf(stdout, __VA_ARGS__); \
		 fprintf(stdout,"\n"); \
	} \
	while(0);
#else
#define MAITAINER_DBG(...)
#endif

#define DEFAULT_RECORD_ALIVE 	15
#define TIMER_INTERVAL 			MSEC		//周期Timer，每500ms执行一次
#define DEC_ALIVE_INTERVAL		1
#define SEND_LIST_INTERVAL		5
#define BROCAST_INTERVAL		15
#define MAX_INTERVAL			(DEC_ALIVE_INTERVAL * BROCAST_INTERVAL)

#define ID_LEN		13




struct ID_IP_record {
	struct in_addr 	ip;
	int   			alive;
};


struct record {
	int id_len;
	char user_name[ID_LEN];
	char display_name[ID_LEN];
	struct in_addr ip_addr;
	char ope;
};

#if 0
enum LIST_THREAD_TYPE
{
   ereport = 0,
   esend_inside
};
   
#endif

#pragma pack(1)
struct brocast_buf{
	char id[MAX_ID_SIZE];
	struct in_addr ip;
	char strDomain[16];
};
#pragma pack()


typedef enum deviceType_tag{
	eDevIS = 0x00,
	eDev2ndOS,
	eDevIPA,
}LIST_DEV_TYPE;


/*global variables, the index of array is the ID 
   and the vaule of element is the IP and alive time
*/
struct ID_IP_record gstID_IP_LIST[LIST_MAX+LIST_MAX];	/*[0~31] for IS, [32~63] for 2ndOS*/
BYTE   g_ID_ResoList[LIST_MAX];		/*IS video resolution list; same as gstID_IP_LIST, the element index is mapping to is inner_id */


struct ID_IP_record gstIPA_ID_IP_LIST[LIST_MAX];




// gstID_IP_list_mutex is used to mutual exculusion visit gstID_IP_LIST
OSA_MutexHndl gstID_IP_list_mutex;
OSA_MutexHndl gstIPA_ID_IP_list_mutex;

LOCAL INT32 giList_maintainer_thread_running = 0;
LOCAL OSA_ThrHndl gListMaintainerThr;
LOCAL int giISList_num = 0;
LOCAL int giIPAList_num = 0;

LOCAL OSA_ThrHndl hReportListThread;


LOCAL int giVitrual_time=0;



extern void get_local_ID(char* szLocal_ID);
extern void get_internal_IP(char* szInternal_IP);


#if 0
/************************************************
Name: 	get_stLocal_ip_by_name
Desc: 	根据网卡名获取对应IP地址
Param:
		ip		: 
		dev_name: 网卡名，例如eth0

Return:
	操作成功返回0
	操作失败则返回-1

Global:
Note:
Author: Andy
-------------------------------------------------------
Log: 2012.07.12 Andy-wei.hou Create this function
*************************************************/

int get_stLocal_ip_by_name(struct in_addr *ip,const BYTE * dev_name)
{
	struct sockaddr_in *addr;
	struct ifreq req;
	
	int sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(dev_name==NULL)
	{
		perror("network card interface name is NULL !\n");
		return -1;
	}
	
	bzero(&req,sizeof(req));
	strcpy(req.ifr_name,dev_name);
	
	if(-1==ioctl(sockfd,SIOCGIFADDR,&req))
		{
			perror("getlocalip: ioctl error !\n");

			return -1;
		}
	addr=(struct sockaddr_in *) &req.ifr_addr;
	ip->s_addr= addr->sin_addr.s_addr;
	close(sockfd);

	return 0;
	
	
}


/************************************************
Name: 	bind_socket_UDP_device
Desc: 	创建一个socket，然后将该socket绑定到指定网卡
Param:
		dev_name: 网卡名，例如eth0

Return:
	操作成功返回 创建的socket句柄
	操作失败则返回-1

Global:
Note:
Author: Andy
-------------------------------------------------------
Log: 2012.07.12 Andy-wei.hou Create this function
*************************************************/

int bind_socket_UDP_device(BYTE * dev_name){
	struct sockaddr_in addr;
	struct in_addr local_addr;
	int local_sockfd = -1;	
	local_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(local_sockfd == -1){
		perror("bind_socket_UDP_device: socket create error!\n");
		goto __ERR__INIT__SOCKET__UDP__;
	}
		
	get_stLocal_ip_by_name(&local_addr,dev_name);
	memset(&addr,0,sizeof(addr));	
	addr.sin_family 	 = AF_INET;
	addr.sin_addr.s_addr = local_addr.s_addr;
	addr.sin_port        = htons(YELLOW_RIVER_GROUP_PORT);
	
	if (-1 == bind(local_sockfd, (struct sockaddr *)&addr, sizeof(addr)))
    	{
			perror("InitSocketUDP: socket bind error!\n");
    		goto __ERR__INIT__SOCKET__UDP__;
    	}
		
	return(local_sockfd);
    
__ERR__INIT__SOCKET__UDP__:
    close(local_sockfd);
    return -1;

	
}


#endif

/*2nd OS List process functions*/
/*stored from gstID-IP_LIST[32~63]*/
int get2ndOsIdIpList(char * pData_buf){
	int i;
	if(pData_buf == NULL)
		return -1;
		//P(LIST_MUTEX);	
	OSA_MutexLock(&gstID_IP_list_mutex);	
	for(i=0; i<LIST_MAX; i++)
		memcpy((BYTE *)(pData_buf+i*sizeof(struct in_addr)),&(gstID_IP_LIST[LIST_MAX+i].ip),sizeof(struct in_addr));
	//V(LIST_MUTEX);	
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return 0;
}

int  get_IP_by_ID(int id , char * pData_buf){
	if(pData_buf == NULL)
		return -1;
		//P(LIST_MUTEX);	
	OSA_MutexLock(&gstID_IP_list_mutex);	
	
	memcpy((BYTE *)(pData_buf),&(gstID_IP_LIST[id -1].ip),sizeof(struct in_addr));
	//V(LIST_MUTEX);	
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return 0;
}

/******************************************************************************
* Name: 	 get_ID_IP_list 
*
* Desc: 	get the  global variables gstID_IP_LIST 
* Param: 	pData_buf, pointer to the return buffer, the buffer size should at least sizieof(LIST_MAX*sizeof(struct in_addr));
*
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

int  get_ID_IP_list(char * pData_buf){
	int i;
	if(pData_buf == NULL)
		return -1;
		//P(LIST_MUTEX);	
	OSA_MutexLock(&gstID_IP_list_mutex);	
	for(i=0; i<LIST_MAX; i++)
		memcpy((BYTE *)(pData_buf+i*sizeof(struct in_addr)),&(gstID_IP_LIST[i].ip),sizeof(struct in_addr));
	//V(LIST_MUTEX);	
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return 0;
}


int get_list_num(void){
	int iRet = 0;
	OSA_MutexLock(&gstID_IP_list_mutex);	
	iRet = giISList_num;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return iRet;
}

int get_ID_ReSoluList(char *pDataBuf)
{
	if(!pDataBuf)
		return -1;
	OSA_MutexLock(&gstID_IP_list_mutex);	
	memcpy(pDataBuf,g_ID_ResoList,sizeof(g_ID_ResoList));
	OSA_MutexUnlock(&gstID_IP_list_mutex);	
	return 0;
}


/******************************************************************************
* Name: 	 init_ID_IP_list 
*
* Desc: 	init the global variables gstID_IP_LIST 
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void init_ID_IP_list(void){
	int i;
	//init the mutex used for global variables ID_IP_LISTS
	OSA_MutexCreate(&gstID_IP_list_mutex);
	//P(LIST_MUTEX);	
	OSA_MutexLock(&gstID_IP_list_mutex);
	for(i=0;i<LIST_MAX+LIST_MAX; i++){
		gstID_IP_LIST[i].alive=0;
		gstID_IP_LIST[i].ip.s_addr=0;
	}
	//V(LIST_MUTEX);
	giISList_num = 0;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
}



/******************************************************************************
* Name: 	 dec_record_alive 
*
* Desc: 	decrease the alive time for gstID_IP_LIST[id], and return it's current alive time
* Param: 	 
* Return: 	-1, this id record is empty and with no data 
             	>=0, alive time of this record id
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
		 2015.04.20, Modified by Andy-wei.hou . Add support for 2ndOS
 ******************************************************************************/

int dec_record_alive(LIST_DEV_TYPE type,int id){
	int iTime=0;
	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;
	//P(LIST_MUTEX);
	OSA_MutexLock(&gstID_IP_list_mutex);
	if(gstID_IP_LIST[id+index].ip.s_addr !=0 ){
		gstID_IP_LIST[id+index].alive--;
		iTime = gstID_IP_LIST[id+index].alive;
	}else{
		iTime = -1;
	}
	//V(LIST_MUTEX)
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return iTime;
}



void setGListInfoByID(LIST_DEV_TYPE type,int id, int ip, int alive)
{
	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;

	OSA_MutexLock(&gstID_IP_list_mutex);
	gstID_IP_LIST[id+index].ip.s_addr = ip;
	gstID_IP_LIST[id+index].alive = alive;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
}
#if 0

void setGResoListInfoByID(LIST_DEV_TYPE type,int id,BYTE resoType)
{
	if(type != eDevIS)
		return;
	BYTE vResoType = (resoType == DT_INDOOR_STATION) ? VRESO_VGA_30 : resoType;
	OSA_MutexLock(&gstID_IP_list_mutex);
	g_ID_ResoList[id] = vResoType;	
	OSA_MutexUnlock(&gstID_IP_list_mutex);
}
void clearGResoListInfoByID(LIST_DEV_TYPE type,int id)
{
	if(type != eDevIS)
		return;

	OSA_MutexLock(&gstID_IP_list_mutex);
	g_ID_ResoList[id] = 0x00;
	OSA_MutexUnlock(&gstID_IP_list_mutex);

}


#endif

void resetGListAliveByID(LIST_DEV_TYPE type, int id)
{
	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;

	OSA_MutexLock(&gstID_IP_list_mutex);
	gstID_IP_LIST[id+index].alive = DEFAULT_RECORD_ALIVE;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
}
void clearGListRecByID(LIST_DEV_TYPE type, int id)
{
	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;

	OSA_MutexLock(&gstID_IP_list_mutex);
	gstID_IP_LIST[id+index].ip.s_addr = 0;
	gstID_IP_LIST[id+index].alive = 0;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
}	


int getGListIPByID(LIST_DEV_TYPE type, int id)
{
	int ip = 0;
	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;
	OSA_MutexLock(&gstID_IP_list_mutex);
	ip = gstID_IP_LIST[id+index].ip.s_addr;
	OSA_MutexUnlock(&gstID_IP_list_mutex);
	return ip;
}
/////////////////////////////////////////////////////////////////////////////////////////





void init_IPA_ID_IP_list(void){
	int i;
	OSA_MutexCreate(&gstIPA_ID_IP_list_mutex);
	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	for(i=0;i<LIST_MAX; i++){
		gstIPA_ID_IP_LIST[i].alive=0;
		gstIPA_ID_IP_LIST[i].ip.s_addr=0;
	}
	giIPAList_num = 0;
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);

}

int IPA_dec_record_alive(int id){
	int iTime=0;
	int index = 0;

	//P(LIST_MUTEX);
	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	if(gstIPA_ID_IP_LIST[id+index].ip.s_addr !=0 ){
		gstIPA_ID_IP_LIST[id+index].alive--;
		iTime = gstIPA_ID_IP_LIST[id+index].alive;
	}else{
		iTime = -1;
	}
	//V(LIST_MUTEX)
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);
	return iTime;
}



void IPA_setGListInfoByID(LIST_DEV_TYPE type,int id, int ip, int alive)
{
	int index = 0;

	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	gstIPA_ID_IP_LIST[id+index].ip.s_addr = ip;
	gstIPA_ID_IP_LIST[id+index].alive = alive;
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);
}


void IPA_resetGListAliveByID(LIST_DEV_TYPE type, int id)
{
	int index = 0;

	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	gstIPA_ID_IP_LIST[id+index].alive = DEFAULT_RECORD_ALIVE;
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);
}
void IPA_clearGListRecByID( int id)
{
	int index = 0;

	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	gstIPA_ID_IP_LIST[id+index].ip.s_addr = 0;
	gstIPA_ID_IP_LIST[id+index].alive = 0;
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);
}	


int IPA_getGListIPByID(int id)
{
	int ip = 0;
	int index = 0;

	OSA_MutexLock(&gstIPA_ID_IP_list_mutex);
	ip = gstIPA_ID_IP_LIST[id+index].ip.s_addr;
	OSA_MutexUnlock(&gstIPA_ID_IP_list_mutex);
	return ip;
}



	

int UDPSocketSendMsg(const char *name,char *dataBuf,int dataLen)
{
     s_PacketMsg interMsg = { PHONE_ORDER_ENTER_IPGW,-1,0};
     interMsg.datalen = dataLen;
	return LocalSocketSendMsg(name,&interMsg,sizeof(s_PacketMsg),dataBuf,dataLen);
	
}


/******************************************************************************
* Name: 	 transfer_multicast_to_inner_network 
*
* Desc: 	 向内网发送组播
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/04/1, Lewis-weixin.liu Create this function
 ******************************************************************************/
int transfer_packet_to_inner_network(char* IPAddress,int port,char* pPacket,int iPacket_len)
{
    struct sockaddr_in addr = { 0 };
    int addrlen = sizeof(addr);
    int fd = -1;
    char szInternal_IP[16] = {0};
    int bytes = 0;
    char *sendBuf = NULL;
	OSA_DBG_MSGXX("");
    if(pPacket == NULL || iPacket_len<= 0 )
    {
        OSA_ERROR("pPacket to transfered is empty");
        return -1;
    }

    if (port == YELLOW_RIVER_GROUP_PORT)
    {
        s_PacketMsg interMsg = { PHONE_ORDER_ENTER_NET, -1,  iPacket_len};
        //DisplayNetCmdPacket(NULL, pPacket, iPacket_len);
        SendTransfer239GroupProcess(&interMsg, INTER_PACKET_SIZE, pPacket, iPacket_len);
        OSA_ERRORX("IPGW send to localsocket, SendTransfer239GroupProcess\n\n");
    }

	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1)
	{
		OSA_ERROR("Create socket fail, errno(%d)",errno);
        //exit(1);//直接退出应用程序
       	return -1;
	}    
	OSA_DBG_MSGXX("");

    addr.sin_family = AF_INET;
	//addr.sin_port   = htons(7879);		//do not need to specific src port  ---Andy 08.09
	inet_aton(SystemDeviceInfo_GetHomeIPAddress(), &addr.sin_addr);
	OSA_DBG_MSGXX("SystemDeviceInfo_GetHomeIPAddress %s \n",SystemDeviceInfo_GetHomeIPAddress());

    if (-1 == bind(fd, (struct sockaddr *)&addr, addrlen))
    {
       	OSA_ERROR("Bind socket error,errno(%d)",errno);
		close(fd);				   //mush close the socket if bind error, ---Andy 08.09
        return -1;
    }
    addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	inet_aton(IPAddress, &addr.sin_addr);

	OSA_DBG_MSGXX("");
    bytes = sendto(fd, pPacket, iPacket_len, 0, (struct sockaddr *)&addr, addrlen);
    if (-1 == bytes || bytes != iPacket_len)
    {
        OSA_ERRORX("--ERROR: bytes=%d, sendBufLen=%d port(%d)",bytes,iPacket_len, port);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
    
}


#if defined(__Use__HB__miniOS__)
/******************************************************************************
* Name: 	 transfer_multicast_to_uplink_network 
*
* Desc: 	 向内网发送组播
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2016/03/18, Lewis-weixin.liu Create this function
 ******************************************************************************/
int transfer_multicast_to_uplink_network(char* IPAddress,int port,char* pPacket,int iPacket_len)
{
    struct sockaddr_in addr = { 0 };
    int addrlen = sizeof(addr);
    int fd = -1;
    char szExternal_IP[16] = {0};
    int bytes = 0;
    char *sendBuf = NULL;
    if(pPacket == NULL || iPacket_len<= 0 )
    {
        OSA_ERROR("pPacket to transfered is empty");
        return -1;
    }
    get_external_IP(szExternal_IP);
    
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1)
	{
		OSA_ERROR("Create socket fail, errno(%d)",errno);
        //exit(1);//直接退出应用程序
       	return -1;
	}    

    addr.sin_family = AF_INET;
	//addr.sin_port   = htons(7879);		//do not need to specific src port  ---Andy 08.09
	inet_aton(szExternal_IP, &addr.sin_addr);

    if (-1 == bind(fd, (struct sockaddr *)&addr, addrlen))
    {
       	OSA_ERROR("Bind socket error,errno(%d)",errno);
		close(fd);				   //mush close the socket if bind error, ---Andy 08.09
        return -1;
    }

	int value = 1;
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));

	
    addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	inet_aton(IPAddress, &addr.sin_addr);


    bytes = sendto(fd, pPacket, iPacket_len, 0, (struct sockaddr *)&addr, addrlen);
    if (-1 == bytes || bytes != iPacket_len)
    {
        OSA_ERRORX("--ERROR: bytes=%d, sendBufLen=%d port(%d)",bytes,iPacket_len, port);
        close(fd);
        return -1;
    }

	printf("dstaddr(%s) port(%d) iPacket_len(%d) err(%d)\n",IPAddress,port,iPacket_len,errno);

	OSA_DBG_MSG("forward broadcast data: \n");
	DisplayNetCmdPacket(NULL,pPacket,iPacket_len);
	//display_packets(pPacket,iPacket_len);
    close(fd);
    return 0;
}
#endif


/******************************************************************************
* Name: 	 brocast_ID_ServIP
*
* Desc: 	Construct a brocast packages, and brocast it to all ISs
*		The brocast data packages construction is shown below:
*		-------------------------
*		|	6Bytes	|	4Bytes	|	
*		-------------------------
*		|	ID		|	IP_eth1	|	
*		-------------------------
*		ID,6Bytes		:	ID of this apartment
*		IP_eth1,4Bytes	:	IP of eth1 of IP-Gateway, used for setting sip proxy server address of ISs
*
* Param: 	 
* Return: 	 
             
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void brocast_ID_ServIP(void){
	//construct a brocast packages and brocast it to all ISs
	struct in_addr stLocal_ip;
	char *pData_buf,*pSend_buf;					
	SOCK_DATA_PACKET_T *pHeader;				//pointer to send header
	BYTE ucData_size, ucBuf_size;
	char szInternal_IP[16] = {0};
	
	ucData_size = sizeof(struct brocast_buf);
	ucBuf_size = ucData_size + sizeof(SOCK_DATA_PACKET_T);

	OSA_DBG_MSGXX("brocase ID and serIP");
	
	pSend_buf=(char *)malloc(ucBuf_size);
	if (!pSend_buf){
		OSA_ERROR("malloc sendbuf of brocast list fail\n");
	}
	pData_buf = (char *)(pSend_buf+sizeof(SOCK_DATA_PACKET_T));
	pHeader = (SOCK_DATA_PACKET_T *)pSend_buf;

//	OSA_DBG_MSGX("BroadCast ID & SIP server ADDR");
	//construct sendbuf
	//construct protocol header

	OSA_DBG_MSGXX("brocase ID and serIP");

	memcpy(pHeader->srcId,SystemDeviceInfo_GetLocalBCDID(),MAX_ID_SIZE);
	memset(&(pHeader->dstId),0xFF,MAX_ID_SIZE);
	pHeader->funcCode = FUNC_MANAGE_APP;
	pHeader->operCode = OPER_SIPSERVER_IPID;
	pHeader->dataLen[0] = 0;
	pHeader->dataLen[1] = 0;
	pHeader->dataLen[2] = 0;
	pHeader->dataLen[3] = ucData_size;

	//construct databuf
	OSA_DBG_MSGXX("brocase ID and serIP %s \n",SystemDeviceInfo_GetHomeIPAddress());
	memcpy(pData_buf,SystemDeviceInfo_GetLocalBCDID(),MAX_ID_SIZE);
	
	inet_aton((char *)SystemDeviceInfo_GetHomeIPAddress(), &stLocal_ip);
	memcpy((BYTE *)(pData_buf+MAX_ID_SIZE),&stLocal_ip,sizeof(struct in_addr));
	strncpy(pData_buf+MAX_ID_SIZE+4,GetDeviceDomain(),16);
	
	DisplayNetCmdPacket(pHeader,pData_buf,ucData_size);
	OSA_DBG_MSGXX("brocase ID and serIP ====================  ");
	transfer_packet_to_inner_network(YELLOW_RIVER_GROUP_ADDR,YELLOW_RIVER_GROUP_PORT,pSend_buf,ucBuf_size);
 
	free(pSend_buf);
	OSA_DBG_MSGXX("brocase ID and serIP");
}



/******************************************************************************
* Name: 	 dec_list_alive 
*
* Desc: 	call this function per second to dec alive of each element in list and doing the necessary operation
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void dec_list_alive(void){
	//mutex visit ID_IP_LIST
	int i;
	struct record del_rec = { 0 };
	for(i=0; i < LIST_MAX; i++){
			if(dec_record_alive(eDevIS,i) == 0){
				del_rec.ope = DEL;
				del_rec.display_name[0] = i+1;  //复用diplayname,当是删除list指令时，秩序在displayname的最后一个字节填对应的ID即可
				del_rec.display_name[1] = eDevIS;
				UDPSocketSendMsg(kPORT_FLEXISIP2IPGATEWAY,(char *)&del_rec,sizeof(del_rec));
			}
	}
	for(i=0; i < LIST_MAX; i++){
		if(dec_record_alive(eDev2ndOS,i) == 0){
			del_rec.ope = DEL;
			del_rec.display_name[0] = i+1;  //复用diplayname,当是删除list指令时，秩序在displayname的最后一个字节填对应的ID即可
			del_rec.display_name[1] = eDev2ndOS;
			UDPSocketSendMsg(kPORT_FLEXISIP2IPGATEWAY,(char *)&del_rec,sizeof(del_rec));
		}
	}

	for(i=0; i < LIST_MAX; i++){
		if(IPA_dec_record_alive(i) == 0){
			del_rec.ope = DEL;
			del_rec.display_name[0] = i+1;	//复用diplayname,当是删除list指令时，秩序在displayname的最后一个字节填对应的ID即可
			del_rec.display_name[1] = eDevIPA;
			UDPSocketSendMsg(kPORT_FLEXISIP2IPGATEWAY,(char *)&del_rec,sizeof(del_rec));
		}
	}

}


/******************************************************************************
* Name: 	 Timer_ID_IP_list 
*
* Desc:  a periodic timer used to deal with id_ip_list
*		per 1s,		dec_list_alive
*		per 15s,		brocast the list
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/
 void Timer_ID_IP_list(void){

   #if 0

	if(giVitrual_time % DEC_ALIVE_INTERVAL == 0){
		//dec alive time per second
		dec_list_alive();
	}



	if((giVitrual_time+2) % SEND_LIST_INTERVAL == 0){
		//tell send_list_process to send inside 2ndOS list to all or pre-fail sended IS
		char cmd = eSEND_2ndOS_LIST;
		UDPSocketSendMsg(kPOST_MSG2SENDLIST,&cmd,1);
	}
		
	#endif

	if(giVitrual_time % DEC_ALIVE_INTERVAL == 0){
		//dec alive time per second
		dec_list_alive();
	}

	if(giVitrual_time % SEND_LIST_INTERVAL == 0){
		//tell send_list_process to send list to all or pre-fail sended IS
		char cmd = eSEND_IS_LIST;
		UDPSocketSendMsg(kPOST_MSG2SENDLIST,&cmd,1);
	}

	if((giVitrual_time+2) % SEND_LIST_INTERVAL == 0){
		//tell send_list_process to send inside 2ndOS list to all or pre-fail sended IS
		char cmd = eSEND_2ndOS_LIST;
		UDPSocketSendMsg(kPOST_MSG2SENDLIST,&cmd,1);
	}
		
	giVitrual_time++;
	if(giVitrual_time % BROCAST_INTERVAL	== 0) {
		brocast_ID_ServIP();
	}
	

	#if 0
	if(giVitrual_time % MAX_INTERVAL == 0){
		giVitrual_time=0;
	}
 	#endif
}

LOCAL BOOL netAddrMatch(char *ipAddr, char *netPreFix){
	char *p = strstr(ipAddr,netPreFix);
	if(p == ipAddr)
		return TRUE;
	else
		return FALSE;
	
}


void report2ndOSList(){
	char dataBuf[250] = { 0 };
	s_PacketMsg interMsg = {PHONE_ORDER_ENTER_NET, -1, sizeof(SOCK_DATA_PACKET_T)+1+LIST_MAX*4};
	
	SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)dataBuf;
	char *pData = dataBuf+sizeof(SOCK_DATA_PACKET_T);

	netMsg->funcCode = FUNC_MANAGE_APP;
	netMsg->operCode = OPER_2ND_OS_LIST_IDIP;
	netMsg->dataLen[0] = 0;
	netMsg->dataLen[1] = 0;
	netMsg->dataLen[2] = 0;
	netMsg->dataLen[3] = 1+LIST_MAX*4;
	pData[0] = LIST_MAX;
	get2ndOsIdIpList(&pData[1]);
	SendMsg2IPGWCfg((BYTE *)&interMsg, INTER_PACKET_SIZE, dataBuf, interMsg.datalen);
//	DisplayNetCmdPacket(netMsg,pData,sizeof(SOCK_DATA_PACKET_T)+1+LIST_MAX*4);
}


/******************************************************************************
* Name: 	 ID_IP_maintainer 
*
* Desc: 	 matain the global ID_IP_list according to the in put operation, if the LIST changs, adjust the 
*		nat_rules and brocast the newest list to ISs
* Param: 	 id		:	 id of operated element of list
*			operation	:	operation type
			pIP_addr	:	new ip addr of this element
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
		2013.04.27, Andy Hou modify this function. Inorder to avoid stucking in send and report 
		inside list, create a tmp thread to do these two things
		2015.03.25, Modify by Andy-wei.hou.  Add support for video resolution type
 ******************************************************************************/

void ID_IP_maintainer(BYTE opeaation,LIST_DEV_TYPE type, int id, char *pIP_addr, BYTE vResoType){
	BYTE c2ndOS_Changed = 0;
	BYTE cRecord_Changed=0;
	BYTE cReport2OS = 0;
	struct in_addr stNew_ip;
	inet_aton(pIP_addr,&stNew_ip);

	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;
	switch(opeaation){
		case(NEW):	{
	            id = BCD2VAL(id); 
				//if(gstID_IP_LIST[id].ip.s_addr == 0){
				if(getGListIPByID(type,id)==0){
					//this is a new record
					MAITAINER_DBG("new record ID=%d,ip=%s added",id,pIP_addr);
					/*
					gstID_IP_LIST[id].ip.s_addr = stNew_ip.s_addr;
					gstID_IP_LIST[id].alive = DEFAULT_RECORD_ALIVE;
					*/
					//setGResoListInfoByID(type,id,vResoType);
					setGListInfoByID(type,id,stNew_ip.s_addr,DEFAULT_RECORD_ALIVE);
					if(type == eDevIS){
						giISList_num ++;
						cRecord_Changed=1;
					}else{
						c2ndOS_Changed = 1;
					}
					if(netAddrMatch(pIP_addr,"10.") == FALSE)	/*ipaddr belong 10.x.x.x do not need nat rules*/
						nat_rule_maintainer(ADD,id+index,pIP_addr);
					cReport2OS = 1;					//only the operation of  adding a new indoorstation or the breakdown indoorstation restoring, we need to report to OS. The breakdown do not need to report to os  
					
				}//else if(gstID_IP_LIST[id].ip.s_addr != stNew_ip.s_addr){
				else if(getGListIPByID(type,id) != stNew_ip.s_addr){
					//this is a old record, but the ip have changed
					MAITAINER_DBG("old record ID=%d ip changed  to %s",id,pIP_addr);
					/*
					gstID_IP_LIST[id].ip.s_addr = stNew_ip.s_addr;
					gstID_IP_LIST[id].alive = DEFAULT_RECORD_ALIVE;
					*/
					//setGResoListInfoByID(type,id,vResoType);
					
					setGListInfoByID(type,id,stNew_ip.s_addr,DEFAULT_RECORD_ALIVE);

					if(netAddrMatch(pIP_addr,"10.") == FALSE)	/*ipaddr belong 10.x.x.x do not need nat rules*/
						nat_rule_maintainer(MODIFY,id+index,pIP_addr);
					if(type == eDevIS){
						cRecord_Changed=1;
					}else{
						c2ndOS_Changed = 1;
					}
				
				}else{
					//old record, just reset it's repire time
					MAITAINER_DBG("old record ID=%d received, reset timer",id);
					//gstID_IP_LIST[id].alive = DEFAULT_RECORD_ALIVE;	
					//setGResoListInfoByID(type,id,vResoType);
					resetGListAliveByID(type,id);
				}
				break;
			}
		case(DEL): {
				//if(gstID_IP_LIST[id].ip.s_addr != 0){
				if(getGListIPByID(type,id) != 0 ){
					MAITAINER_DBG("old record ID=%d time expired",id);
					/*
					gstID_IP_LIST[id].ip.s_addr = 0;
					gstID_IP_LIST[id].alive = 0;
					*/
					
					//clearGResoListInfoByID(type,id);
					clearGListRecByID(type,id);
					if(type == eDevIS){
						giISList_num --;
						cRecord_Changed=1;
					}
					else
					{
						c2ndOS_Changed = 1;
					}

					nat_rule_maintainer(DEL,id+index,pIP_addr);

				}
				break;
			}
	}


	if(cRecord_Changed || c2ndOS_Changed){
		//如果有变化，广播并且上报给os
		//为防止阻塞，推送采用单独分离出来的线程进行推送
		OSA_DBG_MSGX("====LIST Changed, try to report to ISs");
		if(cRecord_Changed){
			set_list_changed(eSEND_IS_LIST);
			set_list_changed(eSEND_2ndOS_LIST);
		}
		if(c2ndOS_Changed){
			set_list_changed(eSEND_2ndOS_LIST);
			//report2ndOSList();
		}
		
		if (cReport2OS)
			OSA_ThreadCreate(&hReportListThread,(void *)alarm_report_inside_list,(void *)0);
	}
}



void ID_IP_maintainer_IPA(BYTE opeaation,LIST_DEV_TYPE type, int id, char *pIP_addr){
	BYTE c2ndOS_Changed = 0;
	BYTE cRecord_Changed=0;
	BYTE cReport2OS = 0;
	struct in_addr stNew_ip;
	inet_aton(pIP_addr,&stNew_ip);

	int index = 0;
	if(type == eDev2ndOS){
		index = LIST_MAX;
	}
	else
		index = 0;
	switch(opeaation){
		case(NEW):	{
	            id = BCD2VAL(id); 
				//if(gstID_IP_LIST[id].ip.s_addr == 0){
				if(IPA_getGListIPByID(id)==0){
					//this is a new record
					MAITAINER_DBG("new record ID=%d,ip=%s added",id,pIP_addr);
					/*
					gstID_IP_LIST[id].ip.s_addr = stNew_ip.s_addr;
					gstID_IP_LIST[id].alive = DEFAULT_RECORD_ALIVE;
					*/
					//setGResoListInfoByID(type,id,vResoType);
					IPA_setGListInfoByID(type,id,stNew_ip.s_addr,DEFAULT_RECORD_ALIVE);
		
					giIPAList_num ++;
					cRecord_Changed=1;
									
				}//else if(gstID_IP_LIST[id].ip.s_addr != stNew_ip.s_addr){
				else if(IPA_getGListIPByID(id) != stNew_ip.s_addr){
					//this is a old record, but the ip have changed
					MAITAINER_DBG("old record ID=%d ip changed  to %s",id,pIP_addr);
					/*
					gstID_IP_LIST[id].ip.s_addr = stNew_ip.s_addr;
					gstID_IP_LIST[id].alive = DEFAULT_RECORD_ALIVE;
					*/
					//setGResoListInfoByID(type,id,vResoType);
					
					IPA_setGListInfoByID(type,id,stNew_ip.s_addr,DEFAULT_RECORD_ALIVE);

					giIPAList_num ++;
					cRecord_Changed=1;
				
				}else{
					//old record, just reset it's repire time
					MAITAINER_DBG("old record ID=%d received, reset timer",id);
					IPA_resetGListAliveByID(type,id);
				}
				break;
			}
		case(DEL): {
				//if(gstID_IP_LIST[id].ip.s_addr != 0){
				if(getGListIPByID(type,id) != 0 ){
					MAITAINER_DBG("old record ID=%d time expired",id);
					/*
					gstID_IP_LIST[id].ip.s_addr = 0;
					gstID_IP_LIST[id].alive = 0;
					*/
					
					//clearGResoListInfoByID(type,id);
					clearGListRecByID(type,id);
					if(type == eDevIS){
						giISList_num --;
						cRecord_Changed=1;
					}
					else
					{
						c2ndOS_Changed = 1;
					}

					nat_rule_maintainer(DEL,id+index,pIP_addr);

				}
				break;
			}
	}


	if(cRecord_Changed || c2ndOS_Changed){
		//如果有变化，广播并且上报给os
		//为防止阻塞，推送采用单独分离出来的线程进行推送
		OSA_DBG_MSGX("====LIST Changed, try to report to ISs");
		if(cRecord_Changed){
			set_list_changed(eSEND_IS_LIST);
			set_list_changed(eSEND_2ndOS_LIST);
		}
		if(c2ndOS_Changed){
			set_list_changed(eSEND_2ndOS_LIST);
			report2ndOSList();
		}
		
		if (cReport2OS)
			OSA_ThreadCreate(&hReportListThread,(void *)alarm_report_inside_list,(void *)0);
	}
}



void showRecords(char *pBuf){
#if 1
	struct record *pRec = (struct record *)pBuf;
	printf("Receive Records: \r\n");
	printf("OPE=%d \n",pRec->ope);
	if(pRec->ope == DEL)
		printf("Dis_name=%d \n",pRec->display_name[0]);	
	else
		printf("Dis_name=%s \n",pRec->display_name);
	printf("Usr_name=%s \n",pRec->user_name);
	printf("IP_addr = %4x \n",pRec->ip_addr.s_addr);
#endif
}


/******************************************************************************
* Name: 	 thread_list_matainer 
*
* Desc: 	this thread is used to receive register record from flexisip and matain the ID_IP_LIST and nat Rules
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void thread_list_matainer(void *arg){
  
    s_PacketMsg interMsg = { 0 };
    INT32 dataLen = 0;
    BYTE *dataBuf = NULL;

	//this thread is used to receive register package from flexisip via udp socket
	struct record *rcv_record = NULL;
	char cIS_id[MAX_ID_SIZE] = {0};
	INT Type = 0;
	
	int sockfd_list = LocalSocketUDPServer(kPORT_FLEXISIP2IPGATEWAY);
	if(sockfd_list < 0){
		OSA_DBG_MSGXX("Can't init socket for receiving udp data from Flexisip,errno=%d",errno);
		return;
	}
	OSA_DBG_MSGXX("");

	//init the id_ip_list and the nat rules	
	giList_maintainer_thread_running = 1;
	
	while(giList_maintainer_thread_running){
        INT32 iRet = ReadFullPacket(sockfd_list, &interMsg, INTER_PACKET_SIZE, &dataBuf, &dataLen);
		//OSA_DBG_MSGXX("ret %d \n",iRet);
        if(iRet >= 0)
        {
            if(NULL == dataBuf)    
            {
                continue;
            }
			
			OSA_DBG_MSGX("recv Msg, orderType=%d, socket=%d, dataLen=%d PHONE_ORDER_ENTER_LOGIC %d",interMsg.order_type,interMsg.sockfd,interMsg.datalen,PHONE_ORDER_ENTER_LOGIC);
			
            if(interMsg.order_type == PHONE_ORDER_ENTER_IPGW)
            {            		
				rcv_record= dataBuf;
				printf("showRecords \n");
                showRecords((char *)rcv_record);
				
				switch(rcv_record->ope){
					case NEW : {
						if(generate_bcdid_by_text(rcv_record->display_name, cIS_id)){	
							/*should add or modify resolution info for each IS */
							if(cIS_id[0] == DT_DIGITAL_SECOND_DOOR_STATION)
								Type = eDev2ndOS;
							else
								Type = eDevIS;
							ID_IP_maintainer(NEW,Type,cIS_id[MAX_ID_SIZE-1]-1,inet_ntoa(rcv_record->ip_addr),cIS_id[0]);						
						}
					}break;
					case DEL : {
						Type = rcv_record->display_name[1];
						ID_IP_maintainer(DEL,Type,rcv_record->display_name[0]-1,"0.0.0.0",cIS_id[0]);						
					}break;
					default:
						OSA_ERROR("Invalid operation %d",rcv_record->ope);
					}
                
            }
			else if (interMsg.order_type ==  PHONE_ORDER_ENTER_LOGIC)
			{
					SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)dataBuf;
					char* pReq_data = (char *)(dataBuf + sizeof(SOCK_DATA_PACKET_T));
					OSA_DBG_MSGXX("");
					switch(netMsg->funcCode)
					{
						case PROCESS_FUNC_LOGIC:
						{
							switch(netMsg->operCode)
							{
								case PROCESS_OPER_LOGIC_IP_CHANGED:
								{
										start_flexisip();
										start_b2bsip();
		    							start_mrouted();
								
								}
								break;
								default:
								break;	
							}
						}
						break;
						case PROCESS_FUNC_IPGW:
						{
							switch(netMsg->operCode)
							{
								case PROCESS_OPER_IPGW_FLEXISIP_ROUTE_CHANGED:
								{
									system("killall -SIGUSR1 flexisip");
									OSA_DBG_MSGXX("update route ini");
								}
								break;

								case PROCESS_OPER_IPGW_B2BSIPCONF_CHANGED:
								{
									start_b2bsip();
									OSA_DBG_MSGXX("b2bsip conf changed");
								}
								break;
			
								default:
								break;
							}
						}
						break;
						default:
						break;
					}
				
			}
			else  if (interMsg.order_type ==  PHONE_ORDER_ENTER_NET)
			{
					SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)dataBuf;
					char* pReq_data = (char *)(dataBuf + sizeof(SOCK_DATA_PACKET_T));

					switch(netMsg->funcCode)
					{
						case FUNC_MANAGE_APP:
						{
							if(netMsg->operCode == OPER_IPA_REGISTER)
							{
								int insideNum = pReq_data[5];
								struct in_addr ip_addr;
								memcpy((BYTE *)(&ip_addr),&pReq_data[6],sizeof(struct in_addr));
								ID_IP_maintainer_IPA(NEW,eDevIPA,insideNum-1,inet_ntoa(ip_addr));						
				

							}
						}
						break;
					}
				
			}
        }
        SAFE_DELETE_MEM(dataBuf);
        #if 0
		//deal with message from flexisip
		FD_ZERO(&fSets);
		FD_SET(sockfd_list,&fSets); 
		time_out.tv_sec  = 0;	 
		time_out.tv_usec = 500*1000;
		
		iRet= select(sockfd_list + 1, &fSets,  NULL, NULL, &time_out);

//		FEEDWATCHDOG(10, eListMatainerThr);
		if(iRet > 0)
		{
			if(FD_ISSET(sockfd_list,&fSets)){
				read(sockfd_list,(BYTE *)&rcv_record, sizeof(rcv_record));
				showRecords((char *)&rcv_record);
			
				switch(rcv_record.ope){
					case NEW : {
						if(generate_bcdid_by_text(rcv_record.display_name,cIS_id)){	
							/*should add or modify resolution info for each IS */
							if(cIS_id[0] == DT_DIGITAL_SECOND_DOOR_STATION)
								Type = eDev2ndOS;
							else
								Type = eDevIS;
							
							ID_IP_maintainer(NEW,Type,cIS_id[MAX_ID_SIZE-1]-1,inet_ntoa(rcv_record.ip_addr),cIS_id[0]);						
						}
					}break;
					case DEL : {
						Type = rcv_record.display_name[1];
						ID_IP_maintainer(DEL,Type,rcv_record.display_name[0]-1,"0.0.0.0",cIS_id[0]);						
					}break;
			
					default:
						OSA_ERROR("Invalid operation %d",rcv_record.ope);
					}
				
			}
		}		
		else 
		{
			//当IP-GW户内的分机数为0时，禁止IP-GW的功能，当户内有室内机时，恢复其功能
//			detect_fun_status();
		}
        #endif

	}
	close(sockfd_list);
	
}

/******************************************************************************
* Name: 	 start_id_ip_maintainer 
*
* Desc: 	init the id_ip_list and  start matain it
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/
void start_id_ip_maintainer(void){
	INT32 iRet = -1;
	
	//nat_init_rule();
	init_ID_IP_list();
	init_IPA_ID_IP_list();
	brocast_ID_ServIP();
	OSA_SetTimer(10,eTimerContinued,Timer_ID_IP_list,NULL);		/*启动周期定时Timer*/


	#if 1
	iRet = OSA_ThrCreate(&gListMaintainerThr,(void *)thread_list_matainer,OSA_THR_PRI_NONE, NULL);	/*启动列表维护线程*/
	if(iRet!=OSA_SOK)
	{
		perror ("Create pthread error!\n");
		exit(1);
	}
	#endif
}



/******************************************************************************
* Name: 	 inside_alarm_deal_net_msg
*
* Desc: 	 响应主机/PC获取本地ID_IP_LIST请求
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/04/10,Andy-wei.hou Create this function
		should be modified name to dealGetInsideListReq()   --Andy
 ******************************************************************************/
void inside_alarm_deal_net_msg(s_PacketMsg *interMsg, BYTE *data,UINT32 len)
{
	SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)data;
    BYTE dataBuf[256] = { 0 };
    INT32 dataLen = 1 + 1 + 7;
    BOOL bRet = FALSE;

    switch(netMsg->operCode)
    {
    	case OPER_INQUERY_INDOOR_LIST:        // 获取室内机列表
    	{
			dataBuf[0] = SOCK_ACK_OK;
			dataLen = 1 + 1 + LIST_MAX+1;
            dataBuf[1] = LIST_MAX;

            INT32 indoorIpAddr1[LIST_MAX] = { 0 };
			get_ID_IP_list((char *)indoorIpAddr1);
            for(len = 0; len < LIST_MAX; len ++)
            {
                dataBuf[len + 2] =  ((indoorIpAddr1[len] == 0) ? 0 : 1);
            }
            dataBuf[len + 2] = YR_HOUSE_TYPE2;


			if(interMsg->sockfd > 0)
            {
                bRet = SendAckTCP(netMsg, dataBuf, dataLen, interMsg->sockfd);
                if(bRet == FALSE)
                {
                    OSA_ERROR("SendAckTCP FAILED.");
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


