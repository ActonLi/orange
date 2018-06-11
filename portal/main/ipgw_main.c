/******************************************************************************
* Copyright 2010-2012 ABB Genway Co.,Ltd.
* FileName: 	 main.c 
* Desc:    
* 
* 
* Author: 	 Nancy-xiaofeng.xu 
* Date: 	 2012/01/09
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2012/01/09, Nancy-xiaofeng.xu create this file
* 
******************************************************************************/ 

/*-------------------------------- Includes ----------------------------------*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <linux/input.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <dirent.h>

#include <osa/osa_timer.h>

#include <osa/osa_time.h>
#include <osa/osa_debug.h>
#include <osa/osa_thr.h>
#include "setting_interface.h"
#include <inside_list_alarm.h>
#include <inside_list_send.h>
#include <id_ip_maintainer.h>
#include <config.h>


#if defined(__Use__HB__miniOS__)

/**  HB系统指令的tag数据  */
LOCAL BYTE hbSysCmdHeader[] = { 0x47, 0x56, 0x53, 0x38, 0x30, 0x31, 0xa5, 0xa5, 0xa5, 0xa5 };
LOCAL OSA_ThrHndl hUDPForwardCommandThread[4];
LOCAL OSA_ThrHndl hUDPForwardVideoThread;


/** HB系统协议命令随机码长度 */
#define HBSysRandomCodeLen      (8)

/** HB系统协议命令密文长度 */
#define HBSysSecretCodeLen      (8)

#define HB_PROTOCOL_HEADER_LEN 42 //数据长度之前

/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  按照单元号，房间号，设备编号生成符合HB系统的设备bcdid\\n
            操作成功返回TRUE\\n
            否则，返回FALSE\\n
            在HB系统中，设备id是6字节的bcd码组成，其中\\n
            第一个字节，0x61表示室内分机，0x62表示二次门口机\\n
            第二个字节，楼栋号，取值范围 1 ~ 99\\n
            第三个字节，单元号，取值范围 1 ~ 9\\n
            第四个字节，楼层号，取值范围 1 ~ 63\\n
            第五个字节，房号，  取值范围 1 ~ 32\\n
            第六个字节，编号，  取值范围 室内机 1 ~ 4，二次门口机 1 ~ 2\\n

            其中楼栋号与单元号按如下规则转为黄河系统的单元号\\n
            10 * HB楼栋号 + HB单元号\\n
            反过来，\\n
            HB楼栋号=YR单元号 / 10
            HB单元号=YR单元号 % 10

    @param[in] dir         设备id转换方向，true表示YR->HB, 否则HB->YR
    @param[in] yrDeviceId  YellowerRiver系统设备id
    @param[in] hbDeviceId  HB系统设备id
    @return                操作成功返回TRUE,否则，返回FALSE
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-11-26
*/
/////////////////////////////////////////////////////////////////////////////////
BOOL ConvertDeviceId(BOOL dir, BYTE *yrDeviceId, BYTE *hbDeviceId)
{  
    UINT16 unitNo = 0;
    if (dir)
    {
        MemCopy(hbDeviceId, yrDeviceId, MAX_ID_SIZE);
        if (yrDeviceId[0] == DT_INDOOR_STATION)
        {
            hbDeviceId[0] = 0x61;
        }
        else if (yrDeviceId[0] == DT_DIGITAL_SECOND_DOOR_STATION)
        {
            hbDeviceId[0] = 0x62;
        }
        else
        {
            return FALSE;
        }
        unitNo = MAKEWORD(yrDeviceId[2], yrDeviceId[1]);
        hbDeviceId[1] = (unitNo >> 4) & 0xFF;
        hbDeviceId[2] = unitNo & 0x0F;
        return TRUE;
    }
    else
    {
        MemCopy(yrDeviceId, hbDeviceId, MAX_ID_SIZE);
        if (hbDeviceId[0] == 0x61)
        {
            yrDeviceId[0] = DT_INDOOR_STATION;
        }
        else if (hbDeviceId[0] == 0x62)
        {
            yrDeviceId[0] = DT_DIGITAL_SECOND_DOOR_STATION;
        }
        else
        {
            return FALSE;
        }
        unitNo = (hbDeviceId[1] << 4) | hbDeviceId[2];
        yrDeviceId[1] = (unitNo >> 8) & 0xFF;
        yrDeviceId[2] = unitNo & 0xFF;
        return TRUE;
    }
}


/* 接收内网工程模式命令255.255.255.255的数据 */
VOID UDPOSCommandForward2IS(void* param)
{
    struct timeval tm;
    fd_set set;
    int result = -1;
    SOCK_DATA_PACKET_T cmdPacket = { { 0 }, { 0 }, 0, 0, { 0 } };  //数据头
    INT32 len = 0;
    BYTE *dataBuf = NULL;   //保存完整的一条数据
    
    static BYTE tmpDataSave[2048] = { 0 };   //上次接收数据临时缓冲区
    UINT64 tmpDataAtTime = 0;                //上次接收时间，防止短时间内重复接收
    int devid = (int)(param);
    BYTE tmpData[2048] = { 0 };    //接收数据的临时缓冲区         
    char ipaddr[32] = { 0 };
    char internalIP[16] = {0};
	char externalIP[16] = {0};
	int funcCode = 0;
	int operCode = 0;
	char *dstIP = NULL;
	BYTE srcDeviceId[MAX_ID_SIZE] = { 0 };
    BYTE dstDeviceId[MAX_ID_SIZE] = { 0 };
    OSA_THREAD_ID("prn_thread_id");   
	char miniOSAddr[16] = {0};
    int sock = InitSocketUDPSingleRecv(kHbCommandPort1+(devid-1)*5,NULL);
	struct in_addr szIp_list = {0} ;
    if(sock < 0)
    {
        OSA_DBG_MSG("\n(%s, %d) UDP broadcast process thread failed.\n", __FILE__, __LINE__);
        return;
    }
	int value = 1;
	OSA_DBG_MSG("UDPOSCommandForward2IS (%d)\n",devid);    


    while(1)
    {
        //FEEDWATCHDOG(10, eUDPConnectThr);
        FD_ZERO(&set);
        FD_SET(sock,&set); 
        tm.tv_sec  = 0;    
        tm.tv_usec = 500 * 1000;
        result = select(sock + 1, &set, NULL, NULL, &tm);
        
        if(result <= 0)
        {
            //OSA_DBG_MSG("\n(%s, %d)....................\n", __FUNCTION__, __LINE__);
            continue;
        }
        
        if (FD_ISSET(sock, &set))
        {
        	struct sockaddr_in from;
        	socklen_t fromlen = sizeof(from);
            len = recvfrom(sock, tmpData, sizeof(tmpData), 0,  (struct sockaddr *)&from, &fromlen);
            if(len <= 0)
            {
                OSA_ERRORX("len(%d, %d)\n", len, sizeof(tmpData));
                continue;
            }
					

			/*过滤本机全网广播包*/
			char *srcIP = inet_ntoa(from.sin_addr);
			if(NULL != srcIP)
			{

				get_external_IP(externalIP);
				if(strcmp(srcIP,externalIP) == 0 )
				{
					continue;
				}
			}
			#if 0
			struct   sockaddr_in   me;   
			struct   sockaddr_in   server; 
			socklen_t   addrlen   =   sizeof(server);   
           	bzero(&me   ,   sizeof(me));
			getsockname(sock   ,   (struct   sockaddr   *)&me   ,   &addrlen); 
			getpeername(sock   ,   (struct   sockaddr   *)&server,   &addrlen);
			printf("getsockname (%s) getpeername(%s) \n",inet_ntoa(me.sin_addr),inet_ntoa(server.sin_addr));
			#endif
			OSA_DBG_MSG("srcIP(%s) internalIP(%s) externalIP(%s)\n",srcIP,internalIP,externalIP);
			if(memcmp(hbSysCmdHeader,tmpData,sizeof(hbSysCmdHeader)) == 0)
			{
				funcCode = tmpData[sizeof(hbSysCmdHeader) + MAX_ID_SIZE * 2 + HBSysRandomCodeLen + HBSysSecretCodeLen + 0];
	   			operCode = tmpData[sizeof(hbSysCmdHeader) + MAX_ID_SIZE * 2 + HBSysRandomCodeLen + HBSysSecretCodeLen + 1];
				// 源设备id，从hb系统转yr失败则丢弃
			    if (! ConvertDeviceId(FALSE, srcDeviceId, tmpData + sizeof(hbSysCmdHeader) + MAX_ID_SIZE))
			    {
			        continue;
			    }
			    
			    if (! ConvertDeviceId(FALSE, dstDeviceId, tmpData + sizeof(hbSysCmdHeader)))
			    {
			        continue;
			    }

				if(funcCode == 0x10 && operCode == 0x11)
				{
					memcpy(tmpData+42+5,GetMAC(SystemDeviceInfo_GetCommunityPhyName()),6);
				}
				if(strncmp(srcIP,"10.",3) == 0) //如果源IP是10.向内网转发
				{
					if(dstDeviceId[0] == 0x76 || dstDeviceId[1] == 0xFF || dstDeviceId[2] == 0xFF
					|| dstDeviceId[3] == 0xFF || dstDeviceId[4] == 0xFF || dstDeviceId[5] == 0xFF)
					{
						continue;
					}
					if( (funcCode == 0x03 && operCode == 0x01)
						|| (funcCode == 0x03 && operCode == 0x51))
					{
						transfer_packet_to_inner_network("239.0.0.1",kHbCommandPort2,tmpData,len);
						continue;
					}
				
					get_IP_by_ID(devid,(BYTE *)&szIp_list);
				
					dstIP = inet_ntoa(szIp_list);
					if(dstIP != NULL)
					{
						transfer_packet_to_inner_network(dstIP,kHbCommandPort2,tmpData,len);
					}
				}
				else
				{
					if(dstDeviceId[0] == 0x76 && dstDeviceId[1] == 0x0F && dstDeviceId[2] == 0xFF
					&& dstDeviceId[3] == 0xFF && dstDeviceId[4] == 0xFF && dstDeviceId[5] == 0xFF)
					{
						transfer_multicast_to_uplink_network("255.255.255.255",kHbCommandPort1,tmpData,len);
					}
					else
					{
						generate_ip_by_bcdid(dstDeviceId,miniOSAddr);
						printf("minios-id(%d) address(%s)\n",dstDeviceId[5],miniOSAddr);
						transfer_multicast_to_uplink_network(miniOSAddr,kHbCommandPort1,tmpData,len);
					}
				}
			}
            //if(len > 0)
            //{
            //    OSA_Sleep(1000);
            //}
        }
    }
    close(sock);
}


VOID UDPForwardVideo2IS(void * param)
{
    struct timeval tm;
    fd_set set;
    int result = -1;
    SOCK_DATA_PACKET_T cmdPacket = { { 0 }, { 0 }, 0, 0, { 0 } };  //数据头
    INT32 len = 0;
    BYTE *dataBuf = NULL;   //保存完整的一条数据
    
    static BYTE tmpDataSave[2048] = { 0 };   //上次接收数据临时缓冲区
    UINT64 tmpDataAtTime = 0;                //上次接收时间，防止短时间内重复接收
    
    BYTE tmpData[2048] = { 0 };    //接收数据的临时缓冲区         
    char ipaddr[32] = { 0 };
    char internalIP[16] = {0};
	char externalIP[16] = {0};
	int funcCode = 0;
	int operCode = 0;
	char *dstIP = NULL;  
    int devid = (int)(param);
	struct in_addr szIp_list ;
	int value = 1;
    int sock = InitSocketUDPSingleRecv(kHbVideoPort1,NULL);
	int sock1 = InitSocketUDPSingleSend(NULL, NULL); 


	struct sockaddr_in addr = {0};
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port	= htons(kHbVideoPort2);
	inet_pton(AF_INET, "239.0.0.1", &addr.sin_addr.s_addr);


	//InitSocketUDP(ipaddr, kHbCommandPort, UDP_MULTI_RECV);
    if(sock < 0)
    {
        OSA_DBG_MSG("\n(%s, %d) UDP broadcast process thread failed.\n", __FILE__, __LINE__);
        return;
    }

	
    while(1)
    {
        //FEEDWATCHDOG(10, eUDPConnectThr);
 #if 1
        FD_ZERO(&set);
        FD_SET(sock,&set); 
        tm.tv_sec  = 0;    
        tm.tv_usec = 500 * 1000;
        result = select(sock + 1, &set, NULL, NULL, &tm);
        
        if(result <= 0)
        {
            //OSA_DBG_MSG("\n(%s, %d)....................\n", __FUNCTION__, __LINE__);
            continue;
        }
        
        if (FD_ISSET(sock, &set))
        {
        	struct sockaddr_in from;
        	socklen_t fromlen = sizeof(from);
            len = recvfrom(sock, tmpData, sizeof(tmpData), 0,  (struct sockaddr *)&from, &fromlen);
            if(len <= 0)
            {
                OSA_ERRORX("len(%d, %d)\n", len, sizeof(tmpData));
                continue;
            }
			//char *srcIP = inet_ntoa(from.sin_addr);

#endif
					//szIp_list = get_IP_by_ID_Ex(devid);
					//dstIP = get_IP_by_ID_Ex(devid);
					//if(dstIP != NULL)
					{
						//transfer_multicast_to_uplink_network(dstIP,8303,tmpData,len);
						 //SocketSendUDPMsg(sock1, "239.0.0.1", 8303, tmpData, sizeof(tmpData));
						//long long starttime = GetTimeValEx();
					
						#if 1
						int sendlen = sendto(sock1, tmpData, len , 0, (struct sockaddr *)&addr, addrlen);
						/*if(sendlen != len )
						{
							printf(" send data error \n");
						}*/
						//printf("(:%llu) \n",GetTimeValEx() - starttime);

						//printf("forward video (%d)\n",sendlen);
						#endif
						
					}
#if 0
				}
			}
#endif
       }
    }
    close(sock);
	close(sock1);
}

#endif


/*临时编译桩函数*/
/***************编译桩函数*******************/


/*------------------------------------*/

/*------------------------ Function Implement --------------------------------*/
void signal_ipconflict()
{
	//signal UI the ip is conflicted
}

void signal_ipunconflict()
{
	
}



LOCAL int hBoot_IPGW_OK = -1;
LOCAL VOID Boot_IPGW_OK(void)
{
    hBoot_IPGW_OK = LocalSocketUDPServer(kPORT_BootIPGWSuccess);
	OSA_ERRORX("IPGW boot ok.\n");
}




/*start ipgw configuration & misc function thread*/
VOID startWorkThread_ConfiMode(){
	initIPGWConfigSevice();	
	ipgwInitYellowRiverProtocol(eConfigMode); 
}


VOID startWorkThread_FullMode()
{
	OSA_DBG_MSGXX("");

	PortalClientModuleInit();
	ipgwInitYellowRiverProtocol();
	initIPGWConfigSevice();		/*start ipgw configuration & misc function thread*/
	//init_inside_list_alarm();	/*start offline-alarm thread*/

	nat_init_rule();
	
	start_flexisip();
	start_b2bsip();
	start_mrouted();
	InitSendListProcess();		/*start send inside list thread*/
	start_id_ip_maintainer();	/*start id_ip_maintainer thread*/
	//InitIndoorSyncDataProcess();

	//Boot_IPGW_OK();

	
	//InitTalkHistoryProcess();	/*start welcome talk history process thread*/
	//InitSmartHomeModule();		/*start ipgw smarthome thread*/
	//ipgwInitYellowRiverProtocol(eFullFuncMode); 

	/*three service process*/
	//start_mrouted();
	//start_flexisip();

   // SetneedrestartFlexisip(1);

}

VOID stopWorkThread(){
	deInit_inside_list_alarm();
	DeInitSendListProcess();
}



void deal_kill_ipgw_signal(int sig){
	OSA_DBG_MSG("\nreceive kill signal, prepare to exit\n");
	(void) signal(SIGTERM,SIG_DFL);  
	//do_system1("killall flexisip",0);
	//do_system1("killall appweb",0);
	//do_system1("killall mrouted",0);
	//do_system1("killall udhcpc",0);

    system("killall flexisip");
    system("killall appweb");
    system("killall mrouted");
    system("killall udhcpc");

	kill(getpid(),SIGKILL);
	
}
void init_ipgw_deal_signal(void){
	(void)signal(SIGTERM,deal_kill_ipgw_signal);			//deal with terminate signal
}

LOCAL VOID UniqueApp(const char *app)
{
#define __LOCAL_IP__ "127.0.0.1"
#define __KEEP_PORT__ 56788
    int sockfd_tcp = -1;
    sockfd_tcp = InitSocketTCP(__LOCAL_IP__, __KEEP_PORT__);

    if(-1 == sockfd_tcp)
    {
        OSA_ERRORX("Unexpect error ... The Application maybe run duplicate ...app(%s)", app);
        exit(-1);
   }
}

/******************************************************************************
* Name: 	 main 
*
* Desc: 	 
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/20, Lewis-weixin.liu Create this function
 ******************************************************************************/
int main(int argc, char *argv[])
{

    OSA_BULID_INFO();   
   // UniqueApp(argv[0]);
	init_ipgw_deal_signal();
	Settings_Init();
	SystemDeviceInfo_Init();  
    IniGlobalInit(6);
    OSA_MemInit();
    OSA_InitTimer();

	//OSA_initMAC();
	
	if(Engineer_Settings_GetDevicemode() == eDeviceModeMaster)
	{
		printf("IPGWConfigInit \n");
		IPGWConfigInit();
		//启动工作线程
		startWorkThread_FullMode();
		while(1)
			sleep(10);
		//网络状态检测
		//StartnetStausDetcDaemon();
	}
	else
	{
		//startWorkThread_ConfiMode();
		//	Boot_IPGW_OK();
		//while(1)
			//OSA_Sleep(10000);
	}
}


