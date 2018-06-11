#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include<dirent.h>
#include<sys/types.h>


#include <network.h>
#include "osa/osa_debug.h"
#include <osa/osa_ip_acd.h>
#include <osa/osa_thr.h>
#include <osa/osa_mem.h>

#include "edit_file.h"
#include "config.h"
#include "isSyncArbitrate.h"
#include "id_ip_maintainer.h"
#include "SmartHomeAPI.h"
#include "ui_interface.h"
#include "setting_interface.h"



/*-------------------- Global Definitions and Declarations -------------------*/
#define CONFIG_PRINTF      


#define ADDR_LEN                   3
#define UNIT_LEN                   4
#define APARTMENT_LEN              4
#define DT_IPGATEWAY               DT_INDOOR_STATION
#define DEFAULT_INSIDE_ID          0
//#define DEFAULT_USR                "admin"
#define __USE__WLAN__
#if defined(__USE__WLAN__)
#define  INTERFACE_INTERNAL  "wlan0"  
#else
#define  INTERFACE_INTERNAL  "eth1"  
#endif

#define INTERFACE_EXTERNAL         "eth0"
#define GLOBAL_PORT                50600
#define USER_MAX					2		//普通用户个数
#define DEFAULT_IP_GATEWAY_IP       "192.168.1.200"
#define DEFAULT__MASK               "255.255.255.0"
#define DEFAULT_GATEWAY             "192.168.1.1"
#define DEFAULT_USR_PSW			    "123456"


enum PRIV_TYPE
{
	eSURVEY = 1,
	eOPENDOOR ,
	eSWITCHLIGHT,
	eSWITCHMONITOR
};
typedef struct usrpriinfo
{
    char usrname[USERNAME_LENGTH];
	char pass[17];
    char survey;
    char opendoor;
    char switchlight;
    char switchmonitor;
}USER_PRI_INFO;


typedef struct config_info
{
	 int  startMode;
     int  byIP_mode;
     char szInternal_IP[16];
     char szInternal_mask[16];
     char szInternal_gateway[16];
     char szGlobal_address[128];
     char szDNS[16];
     char szUnit[8];
     char szApartment[8];
     char szDevice_ID[8];
     char szExternal_IP[16];
     char szIPS_IP[16];
	 USER_PRI_INFO szUsr_Priv[3];
}CONFIG_INFO; 

typedef struct oslist_info
{
	 int osnum;
     int gsnum;
     int sndosnum;
     int oslist;
     int gslist;
     int sndoslist;
}OSLIST_INFO; 



enum USER_OPER_TYPE{
	eUSER_CFG_NEW,
	eUSER_CFG_DEL,
	eUSER_CFG_MODIFY_PASS,
	eUSER_CFG_MODIFY_PRI
};

enum BASIC_SAVE_OPER_TYPE{
	eCFG_SAVE_START_MODE,
	eCFG_SAVE_NETWORK,
	eCFG_SAVE_GLOBAL_ADDR,
	eCFG_SAVE_ID,
	eCFG_SAVE_USER_PASS,
	eCFG_SAVE_USER_NAME,
	eCFG_SAVE_IPS_IP,
};

enum IP_mode
{
	eStatic = 0,
	eDHCP,
    
};


OSLIST_INFO g_strOsList={0};
LOCAL char s_szPreIPaddr[20] = { 0 };

/*----------------------- Constant / Macro Definitions -----------------------*/

int setUserCfg(USER_PRI_INFO *userInfo, BYTE operType);


/*--------------编译桩函数及宏定义----------*/


/*------------------------ Variable Declarations -----------------------------*/
LOCAL CONFIG_INFO g_stCfg_info ={
								0, 						/*start mode*/
								1,						/*ip Mode*/
                                "192.168.1.200",		/*ip Addr*/
                                "255.255.255.0",		/* mask */
                                "192.168.1.1",			/*gateway*/
                                "",						/*global_address*/
                                "",						/*dns*/
                                "001",					/*unit*/
                                "0101",					/*apartment*/
                                {DT_IPGATEWAY,0x00,0x01,0x01,0x01,0x00},	/*deviceID*/
                                "10.1.65.0",			/*extern ip*/
                                "224.0.23.12",			/*ips ip*/
								{{NULL,NULL,0,0,0,0},{NULL,NULL,0,0,0,0},{NULL,NULL,0,0,0,0}}	/*user config*/
    };

LOCAL pthread_mutex_t  g_Mutex_config = PTHREAD_MUTEX_INITIALIZER;
LOCAL int ipgwConfigServiceThreadRunning = 0;
LOCAL OSA_ThrHndl gIPGWCfgServiceThr;
LOCAL int g_iNeedRestart=0;
int g_dhcp_try_times;

/*------------------------ Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 获取配置接口函数
*
* Desc: 	 
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/27, Lewis-weixin.liu Create this function
*		 2015/03/18, Andy-wei.hou Modify this funcion. 全局变量的修改应该和写配置文件一起被锁保护
 ******************************************************************************/
LOCAL VOID __set_gLocal_id(char *szUnit, char *szApartment)
{
	char szShell_buf[128] ={0};
	char szStrID[20] = { 0 };
	sprintf(g_stCfg_info.szUnit,"%s",szUnit);
	sprintf(g_stCfg_info.szApartment,"%s",szApartment);
	
    sprintf(szShell_buf,"%02x%s%s%02x",DT_IPGATEWAY,szUnit,szApartment,DEFAULT_INSIDE_ID);
	OSA_printf("\r\nszShell_buf = %s\r\n",szShell_buf);	
    generate_bcdid_by_text(szShell_buf, (char *)g_stCfg_info.szDevice_ID);	
    

	sprintf(szStrID,"%02x%s%s%02x",DT_IPGATEWAY,szUnit,szApartment,0x01);
	generate_ip_by_text(szStrID,g_stCfg_info.szExternal_IP);
	OSA_DBG_MSG("extern IP =%s",g_stCfg_info.szExternal_IP);
//	print_config_info();

}

LOCAL VOID __set_gStartMode(BYTE iMode){
	g_stCfg_info.startMode = iMode;
}

LOCAL VOID __set_gNetworkInfo(BYTE ipMode ,char *ip, char *mask, char *gw, char *dns){
	g_stCfg_info.byIP_mode = ipMode;
	if(ip)
		sprintf(g_stCfg_info.szInternal_IP,"%s",ip);
	if(mask)
		sprintf(g_stCfg_info.szInternal_mask,"%s",mask);
	if(gw)
		sprintf(g_stCfg_info.szInternal_gateway,"%s",gw);
	if(dns)
		sprintf(g_stCfg_info.szDNS,"%s",dns);
//	print_config_info();
}

LOCAL VOID __set_gIPSIP(char *ip)
{	if(ip)
		sprintf(g_stCfg_info.szIPS_IP,"%s",ip);
}

LOCAL VOID __set_gGlobalAddr(char *addr)
{
	if(addr)
		sprintf(g_stCfg_info.szGlobal_address,"%s",addr);
}

LOCAL VOID __set_gUser_byIndex(int index,USER_PRI_INFO * userInfo)
{
	if(index >=0 && index < USER_MAX)
	{
		memcpy(&g_stCfg_info.szUsr_Priv[index],userInfo,sizeof(USER_PRI_INFO));
	}
}


LOCAL INT __set_gAdd_user(USER_PRI_INFO *usrPriInfo){
	int i = 0; 
	/*get the first empty slot for new user*/
	for(i = 0; i < USER_MAX ; i++){
		if(strcmp(g_stCfg_info.szUsr_Priv[i].usrname,usrPriInfo->usrname) == 0){
			OSA_DBG_MSGXX("The userName(%s) is exist,: overWrite it",usrPriInfo->usrname);
			break;
		}
		if(strlen(g_stCfg_info.szUsr_Priv[i].usrname) == 0)
			break;
	}

	if(i >= USER_MAX){
		OSA_ERROR("Max Support %d user, Add Fail", USER_MAX);
		return -1;
	}
	OSA_DBG_MSG("Find slot %d empty ",i);
	memcpy(&g_stCfg_info.szUsr_Priv[i],usrPriInfo,sizeof(USER_PRI_INFO));
	return 0;
		
}

LOCAL INT __set_gDel_user(char * userName){
	int i = 0; 
	/*get the first matched user*/
	for(i = 0; i < USER_MAX; i++){
		if(strcmp(userName,g_stCfg_info.szUsr_Priv[i].usrname) == 0)
			break;
	}
	if(i >= USER_MAX){
		OSA_DBG_MSG("Can't Find user : %s", userName);
		return -1;
	}
	memmove(&g_stCfg_info.szUsr_Priv[i],&g_stCfg_info.szUsr_Priv[i+1],(USER_MAX-i-1)*sizeof(USER_PRI_INFO));
	memset(&g_stCfg_info.szUsr_Priv[USER_MAX-1],0x00,sizeof(USER_PRI_INFO));
	
	return 0;
}

LOCAL INT __set_gModify_User_name(char * unit,char * apartment)
{
    int i=0;
    for(i = 0; i < 2; i++){
        sprintf(g_stCfg_info.szUsr_Priv[i].usrname,"user%d_%s%s",i+1,unit,apartment);
        if(i==0)
        {
            g_stCfg_info.szUsr_Priv[i].opendoor = 1;
        }
        else
        {
            g_stCfg_info.szUsr_Priv[i].opendoor = 0;
        }
		g_stCfg_info.szUsr_Priv[i].survey = 1;
		g_stCfg_info.szUsr_Priv[i].switchlight = 1;
		g_stCfg_info.szUsr_Priv[i].switchmonitor = 1;
	}
}

LOCAL INT __set_gModify_user_pass(char * userName, char *pass)
{
	int i = 0; 
	/*get the first matched user*/
	for(i = 0; i < USER_MAX; i++){
		if(strcmp(userName,g_stCfg_info.szUsr_Priv[i].usrname) == 0)
			break;
	}
	if(i >= USER_MAX){
		OSA_DBG_MSG("Can't Find user : %s", userName);
		return -1;
	}
	sprintf(g_stCfg_info.szUsr_Priv[i].pass,"%s",pass);
	return 0;
	
}

LOCAL INT __set_gModify_user_pri(USER_PRI_INFO *userInfo)
{
	int i = 0; 
	/*get the first matched user*/
	for(i = 0; i < USER_MAX; i++){
		if(strcmp(userInfo->usrname,g_stCfg_info.szUsr_Priv[i].usrname) == 0)
			break;
	}
	if(i >= USER_MAX){
		OSA_DBG_MSG("Can't Find user : %s", userInfo->usrname);
		return -1;
	}
	
	sprintf(userInfo->pass,"%s",g_stCfg_info.szUsr_Priv[i].pass);
	memcpy(&g_stCfg_info.szUsr_Priv[i],userInfo,sizeof(USER_PRI_INFO));
	
	return 0;
}

LOCAL VOID InitLocalOsList(VOID)
{
    int i=0;
    for(i=0;i<32;i++)
    {
        memset(&g_strOsList,0,sizeof(g_strOsList));
   }
}

/*read operation should be protected by mutex*/
int GetneedrestartFlexisip(void)
{
    return g_iNeedRestart;
}
void SetneedrestartFlexisip(int flag)
{
    g_iNeedRestart = flag;
}

int get_startMode(){
	int mode;
	pthread_mutex_lock(&g_Mutex_config);
    mode = g_stCfg_info.startMode;
    pthread_mutex_unlock(&g_Mutex_config);
	return mode;
}
void get_local_ID(char* szLocal_ID)
{
    if(szLocal_ID != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szLocal_ID,g_stCfg_info.szDevice_ID,6);
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
void get_external_IP(char* szExternal_IP)
{
    if(szExternal_IP != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szExternal_IP,g_stCfg_info.szExternal_IP,sizeof(g_stCfg_info.szExternal_IP));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
void get_internal_IP(char* szInternal_IP)
{
    if(szInternal_IP != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szInternal_IP,g_stCfg_info.szInternal_IP,sizeof(g_stCfg_info.szInternal_IP));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
void get_internal_gateway(char* szInternal_gateway)
{
    if(szInternal_gateway != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szInternal_gateway,g_stCfg_info.szInternal_gateway,sizeof(g_stCfg_info.szInternal_gateway));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
void get_internal_dns(char* szInternal_dns)
{
    if(szInternal_dns != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szInternal_dns,g_stCfg_info.szDNS,sizeof(g_stCfg_info.szDNS));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
void get_internal_mask(char* szInternal_mask)
{
    if(szInternal_mask != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szInternal_mask,g_stCfg_info.szInternal_mask,sizeof(g_stCfg_info.szInternal_mask));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}


void get_IPS_IP(char* szIPS_IP)
{
    if(szIPS_IP != NULL)
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(szIPS_IP,g_stCfg_info.szIPS_IP,sizeof(g_stCfg_info.szIPS_IP));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}
int  get_ip_mode()
{
	int iIPMode ;
    pthread_mutex_lock(&g_Mutex_config);
    iIPMode = g_stCfg_info.byIP_mode;
    pthread_mutex_unlock(&g_Mutex_config);
	return iIPMode;
}


/******************************************************************************
* Name: 	 get_usr_priv_info
* 
* Desc: 	 获取用户权限信息
* Param: 	pszUsr_priv_info: 保存用户权限信息buffer指针
			i :	查询第i个用户信息
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/27, Lewis-weixin.liu Create this function
 ******************************************************************************/
void get_usr_priv_info(USER_PRI_INFO *pszUsr_priv_info, int i)
{
	if(pszUsr_priv_info != NULL )
    {
        pthread_mutex_lock(&g_Mutex_config);
        memcpy(pszUsr_priv_info,&(g_stCfg_info.szUsr_Priv[i]),sizeof(USER_PRI_INFO));
        pthread_mutex_unlock(&g_Mutex_config);
    }
}




/*******************************-------------CFG File Process------------------*************************/


LOCAL VOID __load_basicCfg()
{
	 /*IP mode*/
    char* endpos = NULL ;
    char szTmp[128] = {0};
	BYTE startMode = 0;
	BYTE byIP_mode = 0;
    BYTE byBCD_ID[6] = {0};
    char szIP_addr1[ADDR_LEN+1];
    char szIP_addr2[ADDR_LEN+1];
    char szIP_addr3[ADDR_LEN+1];
    char szIP_addr4[ADDR_LEN+1];

	GetValueFromEtcFile_new(CFG_BASIC,"MODE","FullMode",szTmp,1);  
	startMode = atoi(szTmp);
	OSA_DBG_MSG("StartMode=%s",startMode == eFullFuncMode ? "FullMode":"ConfigMode");
	__set_gStartMode(startMode);

    GetValueFromEtcFile_new(CFG_BASIC,"IP","mode",szTmp,1);  
   	byIP_mode= atoi(szTmp);
    if(byIP_mode == eStatic) 
    {
        /*eth1 IP*/
		memset(szTmp, 0x00, sizeof(szTmp));
        GetValueFromEtcFile_new(CFG_BASIC,"IP", "IPaddr", szTmp, 16);
		__set_gNetworkInfo(byIP_mode,szTmp,NULL,NULL,NULL);

        
        /*eth1 netmask*/
		memset(szTmp, 0x00, sizeof(szTmp));
        GetValueFromEtcFile_new(CFG_BASIC,"IP", "submask", szTmp, 16);
		__set_gNetworkInfo(byIP_mode,NULL,szTmp,NULL,NULL);


        /*eth1 default gateway*/
		memset(szTmp, 0x00, sizeof(szTmp));
        GetValueFromEtcFile_new(CFG_BASIC,"IP", "gateway", szTmp, 16);
		__set_gNetworkInfo(byIP_mode,NULL,NULL,szTmp,NULL);

        /*DNS*/
		memset(szTmp, 0x00, sizeof(szTmp));
        GetNameserver(szTmp);
		__set_gNetworkInfo(byIP_mode,NULL,NULL,NULL,szTmp);
    }
   
	{
	    /*Unit*/ 	/*Apartment*/
		BYTE szUnit[10] = { 0 };
		BYTE szApartment[10] = { 0 };
		GetValueFromEtcFile_new(CFG_BASIC,"community", "unit", szUnit, UNIT_LEN);
		GetValueFromEtcFile_new(CFG_BASIC,"community", "apartment", szApartment, APARTMENT_LEN);
		__set_gLocal_id(szUnit,szApartment);
	}


    /*IPS IP*/
    memset(szTmp,0,64);
    GetValueFromEtcFile_new(CFG_BASIC,"community", "ipsaddr", szTmp,16);
	__set_gIPSIP(szTmp);
}


/*config.ini  & config.ini config file*/
LOCAL VOID __load_userCfg(){
	
	int i = 0 ; 
    char szTmp[128] = {0};
	char pTmp_key[32]= {0};
	USER_PRI_INFO userInfo;
	for (i = 0; i<USER_MAX; i++)
	{
		memset(&userInfo,0x00, sizeof(USER_PRI_INFO));
        #if 0
		sprintf(pTmp_key,"user_%d",i);
		GetValueFromEtcFile_new(CFG_USER,pTmp_key,"user",userInfo.usrname,USERNAME_LENGTH);
		memset(szTmp,0,128);
		GetValueFromEtcFile_new(CFG_WELCOME,pTmp_key,"opendoor",szTmp,4);
		userInfo.opendoor = (strcmp(szTmp,"yes")==0 ? 1 : 0);
		memset(szTmp,0,128);
		GetValueFromEtcFile_new(CFG_WELCOME,pTmp_key,"surveillance",szTmp,4);
		userInfo.survey= (strcmp(szTmp,"yes")==0 ? 1 : 0);
		memset(szTmp,0,128);
		GetValueFromEtcFile_new(CFG_WELCOME,pTmp_key,"switchlight",szTmp,4);
		userInfo.switchlight= (strcmp(szTmp,"yes")==0 ? 1 : 0);
		memset(szTmp,0,128);
		GetValueFromEtcFile_new(CFG_WELCOME,pTmp_key,"switching",szTmp,4);
		userInfo.switchmonitor= (strcmp(szTmp,"yes")==0 ? 1 : 0);
        #endif

		sprintf(userInfo.usrname,"user%d_%s%s",i+1,g_stCfg_info.szUnit,g_stCfg_info.szApartment);
        GetValueFromEtcFile_new(CFG_USER,pTmp_key,"password",userInfo.pass,16);     
		if(i ==0){
			userInfo.opendoor = 1;
		}
		else{
			userInfo.opendoor = 0;
		}
		userInfo.survey = 1;
		userInfo.switchlight = 1;
		userInfo.switchmonitor = 1;
		setUserCfg(&userInfo,eUSER_CFG_NEW);

		__set_gUser_byIndex(i,&userInfo);
		
	}
}

LOCAL VOID __load_globalAddress(){
 	char* endpos = NULL ;
	char szGlobalAddr[128] =  { 0 };
    char szTmp[128] = {0};
	/*global address*/
    GetValueFromEtcFile_new(CFG_WELCOME,"network", "global-address", szTmp, 128);
    if(szTmp[0] != '\0' ) //外网地址为空
    {
        endpos=strstr(szTmp,":");
        if(endpos != NULL)
        {
            strncpy(szGlobalAddr,szTmp,endpos-szTmp);
			__set_gGlobalAddr(szGlobalAddr);
        }
    }
}

LOCAL VOID __load_AllConfigFromFile(){
	__load_basicCfg();
	__load_globalAddress();
	__load_userCfg();
	
}



LOCAL VOID __save_createDefaultCfg()
{

	SetValueToEtcFile_new(CFG_BASIC,"MODE","FullMode","0");	//start MODE , full mode =0 , only start configure and tcp link func. else start full function mode
	SetValueToEtcFile_new(CFG_BASIC,"IP","mode","1");	//DHCP MODE
	SetValueToEtcFile_new(CFG_BASIC,"IP","IPaddr","192.168.1.200");
	SetValueToEtcFile_new(CFG_BASIC,"IP","submask","255.255.255.0");
	SetValueToEtcFile_new(CFG_BASIC,"IP","gateway","192.168.1.1");

	SetValueToEtcFile_new(CFG_BASIC,"community","unit","0001");
	SetValueToEtcFile_new(CFG_BASIC,"community","apartment","0101");
	SetValueToEtcFile_new(CFG_BASIC,"community","ipsaddr","224.0.23.12");
	SetValueToEtcFile_new(CFG_BASIC,"PASS","pass","admin");

}

LOCAL VOID __save_basicCfg(BYTE operType){
	//OSA_DBG_MSGXX("");
	
	switch(operType){
		case eCFG_SAVE_START_MODE: {
			SetValueToEtcFile_new(CFG_BASIC,"MODE","FullMode",g_stCfg_info.startMode == eConfigMode ? "0" : "1");
		}break;
		case eCFG_SAVE_NETWORK:{
			SetValueToEtcFile_new(CFG_BASIC,"IP","mode",g_stCfg_info.byIP_mode == eDHCP ? "0" : "1");
			SetValueToEtcFile_new(CFG_BASIC,"IP","IPaddr",g_stCfg_info.szInternal_IP);
			SetValueToEtcFile_new(CFG_BASIC,"IP","submask",g_stCfg_info.szInternal_mask);
			SetValueToEtcFile_new(CFG_BASIC,"IP","gateway",g_stCfg_info.szInternal_gateway);
		}break;
		case eCFG_SAVE_ID: {
			SetValueToEtcFile_new(CFG_BASIC,"community","unit",g_stCfg_info.szUnit);
			SetValueToEtcFile_new(CFG_BASIC,"community","apartment",g_stCfg_info.szApartment);
		}break;

        case eCFG_SAVE_USER_NAME:{
            int i = 0;
			char section[8] = { 0 };
			for(i = 0; i < USER_MAX; i++){
				sprintf(section,"user_%d",i);
				if(strlen(g_stCfg_info.szUsr_Priv[i].usrname)!=0)
					SetValueToEtcFile_new(CFG_USER,section,"user",g_stCfg_info.szUsr_Priv[i].usrname);
			}
        }
        break;
		case eCFG_SAVE_USER_PASS: {
			int i = 0;
			char section[8] = { 0 };
			for(i = 0; i < USER_MAX; i++){
				sprintf(section,"user_%d",i);
				if(strlen(g_stCfg_info.szUsr_Priv[i].usrname)!=0){
					SetValueToEtcFile_new(CFG_USER,section,"user",g_stCfg_info.szUsr_Priv[i].usrname);
					SetValueToEtcFile_new(CFG_USER,section,"password",g_stCfg_info.szUsr_Priv[i].pass);
				}
				else
					DeleleSection(CFG_USER,section);
			}
		}break;
		case eCFG_SAVE_IPS_IP: {
			SetValueToEtcFile_new(CFG_BASIC,"community","ipsaddr",g_stCfg_info.szIPS_IP);
			}break;
	}
//	OSA_DBG_MSGXX("");
}



#ifdef CONFIG_PRINTF
void print_config_info()
{
	//pthread_mutex_lock(&g_Mutex_config);
	
    OSA_DBG_MSG("config info => byIP_mode = %s",g_stCfg_info.byIP_mode == eDHCP ? "eDHCP" : "eStatic");
    OSA_DBG_MSG("config info => szInternal_IP = %s",g_stCfg_info.szInternal_IP);
    OSA_DBG_MSG("config info => szInternal_mask = %s",g_stCfg_info.szInternal_mask);
    OSA_DBG_MSG("config info => szInternal_gateway = %s",g_stCfg_info.szInternal_gateway);
    OSA_DBG_MSG("config info => szUnit = %s",g_stCfg_info.szUnit);
    OSA_DBG_MSG("config info => szApartment = %s",g_stCfg_info.szApartment);
    OSA_DBG_MSG("config info => szDevice_ID ");
    printf_array_in_hex(g_stCfg_info.szDevice_ID,6);
    OSA_DBG_MSG("config info => szExternal_IP = %s",g_stCfg_info.szExternal_IP);
    OSA_DBG_MSG("config info => szIPS_IP = %s",g_stCfg_info.szIPS_IP);
    OSA_DBG_MSG("config info => szGlobal_address = %s",g_stCfg_info.szGlobal_address);

	int i = 0;
	for (i = 0; i<USER_MAX ; i++ ){		
		OSA_DBG_MSG("config info => szUsr_Priv[%d]: user(%s) opendoor(%d) survey(%d) switchlight(%d) switchmonitor(%d)"
					,i
					,g_stCfg_info.szUsr_Priv[i].usrname
					,g_stCfg_info.szUsr_Priv[i].opendoor
					,g_stCfg_info.szUsr_Priv[i].survey
					,g_stCfg_info.szUsr_Priv[i].switchlight
					,g_stCfg_info.szUsr_Priv[i].switchmonitor
					);
	}
	
	//pthread_mutex_unlock(&g_Mutex_config);

}
void printUserCfgInfo()
{
//	pthread_mutex_lock(&g_Mutex_config);

	int i = 0;
	for (i = 0; i<USER_MAX ; i++ ){ 	
		OSA_DBG_MSG("config info => szUsr_Priv[%d]: user(%s) opendoor(%d) survey(%d) switchlight(%d) switchmonitor(%d)"
					,i
					,g_stCfg_info.szUsr_Priv[i].usrname
					,g_stCfg_info.szUsr_Priv[i].opendoor
					,g_stCfg_info.szUsr_Priv[i].survey
					,g_stCfg_info.szUsr_Priv[i].switchlight
					,g_stCfg_info.szUsr_Priv[i].switchmonitor
					);
	}
	
//	pthread_mutex_unlock(&g_Mutex_config);

}


void printf_array_in_hex(BYTE* array,int len)
{
    int i =0;
    for(i=0;i<len;i++)
    {
        printf("%02x ",array[i]);
    } 
    printf("\n");
}
#endif




/*Network.conf Process*/
/******************************************************************************
* Name: 	 add_boot_count 
*
* Desc: 	 启动次数累加并保存文件
* Param: 	
* Return: 	返回当前启动次数
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/26, Lewis-weixin.liu Create this function
 ******************************************************************************/
LOCAL int add_boot_count()
{
     char bootCount[16] = {0};
     int count =1;
     GetValueFromEtcFile_new(CFG_BASIC,"bootcount","bootcount",bootCount, 15);  
     if(strcmp(bootCount,"") != 0) 
     {
         count = atoi(bootCount);
         count++;
     }
     memset(bootCount,0,16);
     sprintf(bootCount,"%d",count);
     printf("number of starts is %s\n",bootCount);
     SetValueToEtcFile_new(CFG_BASIC,"bootcount","bootcount",bootCount);   
     return count;
}

/*create flexisip default config when the file do not exist or flexisip crushed after system init*/
LOCAL VOID dump_SIP_server_DefCfg()
{
	char shellStr[256] = { 0 };
	sprintf(shellStr,"flexisip --dump-default-config >%s",CFG_FLEXISIP);
	//do_system1(shellStr,0);
    system(shellStr);

	SetValueToEtcFile_new(CFG_FLEXISIP,"global","dump-corefiles","false");  
	SetValueToEtcFile_new(CFG_FLEXISIP,"global","tls-certificates-dir","/usr/app/userconfig/ipgw/flexisip"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"global","transaction-timeout","6000"); 

	SetValueToEtcFile_new(CFG_FLEXISIP,"stun-server","port","50602"); 

	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","filter","is_request && request.method-name != 'MESSAGE' && request.method-name != 'INVITE'"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","enabled","false"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","db-implementation","file"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","trusted-hosts","10.0.0.0/8"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","auth-domains","*"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","datasource","/usr/app/userconfig/ipgw/flexisip/auth_db"); 

	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Registrar","reg-domains","*"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Registrar","max-contacts-by-aor","8"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Registrar","static-records-file","/usr/app/userconfig/ipgw/flexisip/route.ini"); 

	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","use-global-domain","true"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","fork-late","true"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","fork-no-global-decline","true"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","treat-decline-as-urgent","true"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","preroute","ipgw"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::Router","acl-file","/usr/app/userconfig/ipgw/flexisip/acl.list"); 


	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","sdp-port-range-min","50603"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","sdp-port-range-max","50800"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","bye-orphan-dialogs","true"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","max-calls","2"); 
#if 0
	/*do not know why,should check the code*/
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","h264-filtering-bandwidth","256"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","h264-iframe-decim","1"); 
	SetValueToEtcFile_new(CFG_FLEXISIP,"module::MediaRelay","drop-telephone-event","true"); 
#endif	
}

VOID IPGWConfigInit()
{
	/*SIP SERVER CFG FILE*/
	OSA_DBG_MSGXX("");
	if(!OSA_DirIsExist(CFG_FLEXISIP_DIR))
	{
		OSA_DBG_MSGXX("");
		OSA_DirCreateEx(CFG_FLEXISIP_DIR);
	}

	if(!OSA_FileIsExist(CFG_FLEXISIP))
	{
		dump_SIP_server_DefCfg();		/*add by Andy inorder to avoid flexisip crushed by config file problem*/
	}

	char global_addr [4] = {0};
	__save_SIP_server_transport(global_addr,SystemDeviceInfo_GetCommunityIPAddress());
}

/*config web_auth file*/
LOCAL void __save_web_server_auth(USER_PRI_INFO userInfoList[])
{
    int i = 0;
    char strShell[128] = {0};

    sprintf(strShell,"rm %s -f",CFG_WEB_AUTH);
    //do_system1(strShell,0);
    system(strShell);
    memset(strShell,0,sizeof(strShell));
    sprintf(strShell,"touch %s ",CFG_WEB_AUTH);
    //do_system1(strShell,0);
    system(strShell);
    
    for(i=0;i<USER_MAX;i++)
    {
	    memset(strShell,0x00,sizeof(strShell));
		sprintf(strShell,"httpPassword -p %s %s ABB %s",userInfoList[i].pass,CFG_WEB_AUTH,userInfoList[i].usrname);
        //do_system1(strShell,0);
        system(strShell);

    }
}

/*config welcome related config file config.ini */

char*  generateSIPURLEx(char *deviceID)
{
    static char URI[64] ={0};
    char ipaddr1[16] = {0};
    char mask[16] = {0};
    generate_ip_by_text(deviceID, ipaddr1);
    sprintf(URI,"sip:%s@%s:5070",deviceID,ipaddr1);
    return URI;
}

char * generateSIPURLEx2(char *deviceID,char *ip){
    static char URI[64] ={0};
    sprintf(URI,"sip:%s@%s:5070",deviceID,ip);
    return URI;
}
void GenerateQRcode(char *usrname,char* password, char *ipaddr,int index)
{
	char strShell[128] = {0};
	sprintf(strShell,"qrencode -m 1 -o %suser%d.png -t PNG -k 'URL=http://%s/config.ini USER=%s PW=%s'",CFG_IPGW_CFG_DIR,index+1,ipaddr,usrname,password);
	system(strShell);
}

LOCAL void __save_welcome_app_userQrCode(USER_PRI_INFO usrInfo[],char *szInternalIP){
	char shellStr[128] = { 0 };
	int i = 0; 
	
	for(i = 0; i<  USER_MAX; i++)
	{
		if(strlen(usrInfo[i].usrname)!=0)
		{
            sprintf(shellStr,"rm user%d.png",CFG_IPGW_CFG_DIR,i+1);
        	//do_system1(shellStr,0);	
        	system(shellStr);
			GenerateQRcode(usrInfo[i].usrname,usrInfo[i].pass,szInternalIP,i);
		}
	}
}




/*Os in section 0~31 while gs in section 32~63*/
LOCAL void __del_welcome_app_monitorOSList(INT OsType ){
	char section[64] = { 0 };
	int base = 0;
    int count = 0;
    
	switch(OsType){
		case DT_DOOR_STATION : 
			base = 0;
            count = 64;
			break;
		case DT_GATE_STATION :
			base = 64;
            count = 32;
			break;
		case DT_DIGITAL_SECOND_DOOR_STATION : 
			base = 96;
            count = 32;
			break;
	}

	int i = 0;
	for(i = 0; i < count ; i++ ){
		memset(section,0x00, sizeof(section));
		sprintf(section,"outdoorstation_%d",i+base);
//		OSA_DBG_MSGXX(" %s",section);
		DeleleSection(CFG_WELCOME,section);
	}
}



LOCAL void __save_welcome_app_monitoryOS(char *deviceid)
{
	char deviceType[4] = {0}; 
	char deviceName[128] = {0};
	char id[4] = {0};
	char section[32] = {0};
	strncpy(deviceType,deviceid,2);
	  
	int  iDevice = strtol(deviceType,NULL,16);
	if(iDevice == DT_DOOR_STATION)
	{
		
		strncpy(id,deviceid+8,2);
		sprintf(section,"outdoorstation_%d",atoi(id)-1);
		sprintf(deviceName,"OS%d",atoi(id));
		
	}
	else if(iDevice == DT_GATE_STATION)
	{
		strncpy(id,deviceid+4,2);
		sprintf(section,"outdoorstation_%d",atoi(id)+31);
		sprintf(deviceName,"GS%d",atoi(id));
	}

//	OSA_DBG_MSGXX("%s:%s",section,deviceid);
	SetValueToEtcFile_new(CFG_WELCOME,section, "name",deviceName);
	SetValueToEtcFile_new(CFG_WELCOME,section, "type", "door");
	SetValueToEtcFile_new(CFG_WELCOME,section, "address", (char*)generateSIPURLEx(deviceid));  
	SetValueToEtcFile_new(CFG_WELCOME,section, "screenshot", "yes"); 
	SetValueToEtcFile_new(CFG_WELCOME,section, "surveillance", "yes"); 	
}

LOCAL void __save_welcome_app_monitory2ndOS(int num, char* pAddrArry){
	int i =0;
	char deviceName[128] = {0};
	char section[32] = {0};
	char ipAddr[16] = { 0 };
	char strDeviceID[16] = { 0 };
	char deviceID[MAX_ID_SIZE] = { 0 };
	struct in_addr *pIPAddr = (struct in_addr *)pAddrArry;

	memcpy(deviceID,g_stCfg_info.szDevice_ID,MAX_ID_SIZE);

//	OSA_DBG_MSGXX(" LOCAL_ID=%02x %02x %02x %02x %02x %02x",deviceID[0],deviceID[1],deviceID[2],deviceID[3],deviceID[4],deviceID[5]);
	for(i = 1; i <= num; i++ ){
		if(pIPAddr[i-1].s_addr == 0)
			continue;
		
		sprintf(ipAddr,"%s",inet_ntoa(pIPAddr[i-1]));		
		sprintf(section,"outdoorstation_%d",i+95); // 63);
		sprintf(deviceName,"2ndOS%d",i);

		
		sprintf(strDeviceID,"%02X%02X%02X%02X%02X%02X",DT_DIGITAL_SECOND_DOOR_STATION,deviceID[1],deviceID[2],deviceID[3],deviceID[4],i);
		
		SetValueToEtcFile_new(CFG_WELCOME,section, "name",deviceName);
		SetValueToEtcFile_new(CFG_WELCOME,section, "type", "door");
		SetValueToEtcFile_new(CFG_WELCOME,section, "address", generateSIPURLEx2(strDeviceID,ipAddr));  
		SetValueToEtcFile_new(CFG_WELCOME,section, "screenshot", "yes"); 
		SetValueToEtcFile_new(CFG_WELCOME,section, "surveillance", "yes"); 
		
	}
}

LOCAL void __save_welcome_app_GardUnit()
{
    char deviceType[4] = {0}; 
    char deviceName[128] = {0};
    char id[4] = {0};
    char section[32] = {0};
    int i=0,j=0;
    char deviceid[20]={0};

    for(i=0;i<4;i++)
    {
        sprintf(deviceid,"7000%02d000000",i+1);
        sprintf(section,"outdoorstation_%d",i+1+95);
        sprintf(deviceName,"PC%d",i+1);
        SetValueToEtcFile_new(CFG_WELCOME,section, "name",deviceName);
        SetValueToEtcFile_new(CFG_WELCOME,section, "type", "gu");
        SetValueToEtcFile_new(CFG_WELCOME,section, "address", (char*)generateSIPURLEx(deviceid));  
        SetValueToEtcFile_new(CFG_WELCOME,section, "screenshot", "no"); 
        SetValueToEtcFile_new(CFG_WELCOME,section, "surveillance", "no"); 
    } 
     
    for(j=0;j<4;j++)
    {
        sprintf(deviceid,"7100%02d000000",j+1);
        sprintf(section,"outdoorstation_%d",i+1+95);
        sprintf(deviceName,"GU%d",j+1);
        SetValueToEtcFile_new(CFG_WELCOME,section, "name",deviceName);
        SetValueToEtcFile_new(CFG_WELCOME,section, "type", "gu");
        SetValueToEtcFile_new(CFG_WELCOME,section, "address", (char*)generateSIPURLEx(deviceid));  
        SetValueToEtcFile_new(CFG_WELCOME,section, "screenshot", "no"); 
        SetValueToEtcFile_new(CFG_WELCOME,section, "surveillance", "no"); 
        i++;
    } 
     
}


void __save_welcome_app_url(const char* szInternal_IP)
{
    char szTmp[128] = {0};

    if(szInternal_IP == NULL)
        return;
    /*config.ini */
    sprintf(szTmp,"%s:%d",szInternal_IP,SIPS_PORT);
    SetValueToEtcFile_new(CFG_WELCOME,"network","local-address",szTmp );    
     
    sprintf(szTmp,"http://%s/cert.der",szInternal_IP); 
    SetValueToEtcFile_new(CFG_WELCOME,"network","der-certificate",szTmp); 

    memset(szTmp,0,sizeof(szTmp));
    sprintf(szTmp,"http://%s/cert.pem",szInternal_IP);
    SetValueToEtcFile_new(CFG_WELCOME,"network","tls-certificate",szTmp);    

    memset(szTmp,0,sizeof(szTmp));
    sprintf(szTmp,"https://%s:%d/history.ini",szInternal_IP,HTTPS_PORT);
    SetValueToEtcFile_new(CFG_WELCOME,"network","local-history",szTmp);  
}


void __save_welcome_global_history(char* szGlobal_IP)
{
    char szTmp[128] = {0};
    memset(szTmp,0,sizeof(szTmp));
    if(szGlobal_IP !=NULL )
    {
        if( strlen(szGlobal_IP) == 0)//外网IP可以为空
        {
            SetValueToEtcFile_new(CFG_WELCOME,"network","global-history",szTmp);  
            SetValueToEtcFile_new(CFG_WELCOME,"network","global-address",szTmp);  
        }
        else
        {
            sprintf(szTmp,"https://%s:%d/history.ini",szGlobal_IP,HTTPS_PORT);
            SetValueToEtcFile_new(CFG_WELCOME,"network","global-history",szTmp);  

            memset(szTmp,0,sizeof(szTmp));
            sprintf(szTmp,"%s:%d",szGlobal_IP,REMOTE_SIPS_PORT);
            SetValueToEtcFile_new(CFG_WELCOME,"network","global-address",szTmp);  
        }
    }
}


void __save_welcome_app_domain(const char* szExternal_IP)
{
   SetValueToEtcFile_new(CFG_WELCOME,"network","domain",szExternal_IP);  
}

void __save_welcome_app_user(USER_PRI_INFO userInfoList[]){
	int i = 0; 
	char section[32] = { 0 };
	for(i = 0; i < USER_MAX; i++){
		sprintf(section,"%s_%d","user",i);
		if(strlen(userInfoList[i].usrname)!= 0){
			SetValueToEtcFile_new(CFG_WELCOME,section, "user", userInfoList[i].usrname); 
		    SetValueToEtcFile_new(CFG_WELCOME,section, "opendoor", userInfoList[i].opendoor == 1 ? "yes" : "no");  
		    SetValueToEtcFile_new(CFG_WELCOME,section, "surveillance", userInfoList[i].survey== 1 ? "yes" : "no");  
		    //printf(" %s %s %s %s \n",usrinfo.opendoor, usrinfo.survey,usrinfo.switchlight,usrinfo.switchmonitor);
		    SetValueToEtcFile_new(CFG_WELCOME,section, "switchlight", userInfoList[i].switchlight== 1 ? "yes" : "no");  
		    SetValueToEtcFile_new(CFG_WELCOME,section, "switching", userInfoList[i].switchmonitor== 1 ? "yes" : "no");  
		}
		else
			DeleleSection(CFG_WELCOME,section);
	}

}




/*flexisp config file flexisip.conf*/
void __save_SIP_server_nortpproxy(char* szExternal_IP)
{ 
    char szTmp[128] = {0};
    SetValueToEtcFile_new(CFG_FLEXISIP, "module::MediaRelay","nortpproxy",szExternal_IP);
}

void __save_SIP_server_transport(char* szGlobal_IP,char* szInternal_IP)
{
    char szTransports[256] ={0};
	OSA_DBG_MSGXX("");
    if(strcmp(szGlobal_IP,"")==0)
    {
     sprintf(szTransports,"sip:*:5070  sips:*");
    }
    else
    {
     sprintf(szTransports,"sip:*:5070 sips:* sips:%s:%d;maddr=%s;transport=tls",szGlobal_IP,GLOBAL_PORT,szInternal_IP);
    }
	OSA_DBG_MSGXX("");
    SetValueToEtcFile_new(CFG_FLEXISIP, "global","transports",szTransports);
}

void __save_SIP_server_route(char* szExternal_IP, char *szUnit, char * szApartment, USER_PRI_INFO userInfo[])
{
    char szFileContent[512] = {0};
    char szText_IS_ID[16] = {0};
	
	char bHasUsers = 0;
    FILE* pRoute_file= fopen(CFG_ROUTE,"w+");
    if(pRoute_file ==NULL)
    {
        OSA_ERROR("open %s failed!",CFG_ROUTE);
        return ;
    }

	/*the caller should using mutex pro*/
    sprintf(szText_IS_ID,"%02x%s%s%02x",DT_IPGATEWAY,szUnit,szApartment,0x01);
    sprintf(szFileContent,"<sip:%s@%s>",szText_IS_ID,szExternal_IP);

	int i = 0;
    for(i=0;i<USER_MAX && strlen(userInfo[i].usrname) != 0;i++)
    {
		if(i == 0)
			sprintf(szFileContent,"%s <sip:%s@%s:%d>",szFileContent,userInfo[i].usrname,szExternal_IP,DEF_SIPLOCALPORT);
		else
			sprintf(szFileContent,"%s,<sip:%s@%s:%d>",szFileContent,userInfo[i].usrname,szExternal_IP,DEF_SIPLOCALPORT); 
       	bHasUsers = TRUE;
    }

#ifdef IP_GATEWAY_AND_IS_7INCH
	/*this route Record is used for maijor IS1 to fork a call*/

	if(bHasUsers)
		sprintf(szFileContent,"%s,<sip:%s@%s:%d>",szFileContent,"root",szExternal_IP,5060);
	else
		sprintf(szFileContent,"%s <sip:%s@%s:%d>",szFileContent,"root",szExternal_IP,5060);
#endif

    if(i>0)
    {
         fprintf(pRoute_file, "%s\n", szFileContent);
    }

    fclose(pRoute_file);
}


void __save_SIP_server_auth(char* szExternal_IP, char *szUnit, char *szApartment, USER_PRI_INFO userInfo[])
{
    FILE *authFile =NULL;

    //do_system1("rm /usr/app/web/auth_db -rf",0);
    system("rm /usr/app/web/auth_db -rf");

    char authRecord[64] = {0};
    char szText_IS_ID[16] = {0};
 	char szText_2ndOS_ID[16] = {0};
    int i =0;
	
    sprintf(szText_IS_ID,"%02x%s%s%02x",DT_IPGATEWAY,szUnit,szApartment,0x01);
	sprintf(szText_2ndOS_ID,"%02x%s%s%02x",DT_DIGITAL_SECOND_DOOR_STATION,szUnit,szApartment,0x01);
	
//    GetValueFromEtcFile_new(CFG_BASIC, "community","externIP",szExternal_IP, 16);
    authFile = fopen(CFG_FLEXISIP_AUTH,"w+");
    if(authFile)  
    {
        for(i=0;i< USER_MAX && strlen(userInfo[i].usrname)!=0;i++)
        {
			sprintf(authRecord,"%s@%s %s\n",userInfo[i].usrname,szExternal_IP,userInfo[i].pass);
            fwrite(authRecord,1,strlen(authRecord),authFile);			
        }
		/*for IS default userName & password*/
        sprintf(authRecord,"%s@%s %s\n",szText_IS_ID,szExternal_IP,szText_IS_ID);
        fwrite(authRecord,1,strlen(authRecord),authFile);

		/*for 2nd default userName & password*/
        sprintf(authRecord,"%s@%s %s\n",szText_2ndOS_ID,szExternal_IP,szText_2ndOS_ID);
        fwrite(authRecord,1,strlen(authRecord),authFile);

    }
    fclose(authFile);

}


/*-------Global Cfg Info changed process------------*/


/*user delete or added*/
void __deal_userChanged(){
	OSA_DBG_MSGXX("");
	__save_basicCfg(eCFG_SAVE_USER_PASS);
	__save_welcome_app_user(g_stCfg_info.szUsr_Priv); //config.ini 中usr1和usr2信息
	__save_web_server_auth(g_stCfg_info.szUsr_Priv);  //webauth.ini 
	__save_SIP_server_route(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv); //route.ini
	__save_SIP_server_auth(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv); //auth.db
	__save_welcome_app_userQrCode(g_stCfg_info.szUsr_Priv,g_stCfg_info.szInternal_IP);
	
}
void __deal_userPassChanged(){
	__save_basicCfg(eCFG_SAVE_USER_PASS);
	__save_web_server_auth(g_stCfg_info.szUsr_Priv);
	__save_SIP_server_auth(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv);
	__save_welcome_app_userQrCode(g_stCfg_info.szUsr_Priv,g_stCfg_info.szInternal_IP);
}

void __deal_userPriChanged(){
	__save_welcome_app_user(g_stCfg_info.szUsr_Priv);
}


/*inter IP changed*/
void __deal_interIPChanged(){
	__save_basicCfg(eCFG_SAVE_NETWORK);
	__save_welcome_app_url(g_stCfg_info.szInternal_IP);
	__save_SIP_server_transport(g_stCfg_info.szGlobal_address,g_stCfg_info.szInternal_IP);
	__save_welcome_app_userQrCode(g_stCfg_info.szUsr_Priv,g_stCfg_info.szInternal_IP);

}

void __deal_interDNSChanged(){
	SaveNameserver(g_stCfg_info.szDNS);
}


/*ID & externIP changed*/
void __deal_apartmentIDChanged(){
	__save_basicCfg(eCFG_SAVE_ID);
    __save_basicCfg(eCFG_SAVE_USER_NAME);  //add by nancyxu usr.ini
    __save_welcome_app_user(g_stCfg_info.szUsr_Priv); //add by nancyxu config.ini
    __save_web_server_auth(g_stCfg_info.szUsr_Priv); //add by nancyxu webauth.ini 
	__save_welcome_app_domain(g_stCfg_info.szExternal_IP);
	__save_SIP_server_nortpproxy(g_stCfg_info.szExternal_IP);
	__save_SIP_server_route(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv);
	__save_SIP_server_auth(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv);
    __save_welcome_app_userQrCode(g_stCfg_info.szUsr_Priv,g_stCfg_info.szInternal_IP); //add by nancyxu


}

/*global Address changed*/
void __deal_globalAddressChanged(){
	__save_SIP_server_transport(g_stCfg_info.szGlobal_address,g_stCfg_info.szInternal_IP);
	__save_welcome_global_history(g_stCfg_info.szGlobal_address);
}
LOCAL BOOL _CompareOsListIfchange(int devicetype, BYTE bcdIDList[][MAX_ID_SIZE], int num)
{
    BOOL ret =FALSE; //表没有更改
    int i=0;
    int tmplist=0;
    
    switch(devicetype){
		case DT_DOOR_STATION : 
		{
            if(num>32)
            {
                return ret;
            }
            printf("1111 g_strOsList.oslist=%d g_strOsList.osnum=%d\n",g_strOsList.oslist,g_strOsList.osnum);
            if(num == g_strOsList.osnum) //数量相同
            {
                for(i=0;i<num;i++)
                {
                    int id = ((bcdIDList[i][4]&0xF0)>>4)*10 + (bcdIDList[i][4]&0x0f);;
                    int tmp=(g_strOsList.oslist >>(id-1))&0x00000001;
                    if( tmp != 0x01)
                    {
                        ret = TRUE; 
                        break;
                    }
                }
            }
            else
            {
                ret = TRUE;
            }
            if(ret)
            {
                g_strOsList.oslist = 0;
                for(i=0;i<num;i++)
                {
                    int osid=((bcdIDList[i][4]&0xF0)>>4)*10 + (bcdIDList[i][4]&0x0f);
                    g_strOsList.oslist = (g_strOsList.oslist | (0x01 << (osid-1)));
                    printf("g_strOsList.oslist=%d\n",g_strOsList.oslist);
               }
                g_strOsList.osnum = num;

            }
		}
		break;
        
		case DT_GATE_STATION :
        {
            if(num>32)
            {
                return ret;
            }
            printf("1111 sg_strOsList.gslist=%ld g_strOsList.gsnum=%d\n",g_strOsList.gslist,g_strOsList.gsnum);
            if(num == g_strOsList.gsnum) //数量相同
            {
                for(i=0;i<num;i++)
                {
                    int id = ((bcdIDList[i][2]&0xF0)>>4)*10 + (bcdIDList[i][2]&0x0f);;
                    int tmp=(g_strOsList.gslist >>(id-1))&0x00000001;
                    if( tmp != 0x01)
                    {
                        ret = TRUE; 
                        break;
                    }
                }
            }
            else
            {
                ret = TRUE;
            }
            if(ret)
            {
                g_strOsList.gslist = 0;
                for(i=0;i<num;i++)
                {
                    int gsid = ((bcdIDList[i][2]&0xF0)>>4)*10 + (bcdIDList[i][2]&0x0f);
                    g_strOsList.gslist = (g_strOsList.gslist | (0x01 << (gsid-1)));
                    printf("g_strOsList.gslist=%ld gsid=%d\n",g_strOsList.gslist,gsid);
               }
                g_strOsList.gsnum = num;
            }
		}
		break;
		case DT_DIGITAL_SECOND_DOOR_STATION : 
        {
		}
		break;
        
        default:
            break;
	}
    return ret;
}

/*welcome monitor list */
void __deal_monitorListChanged(BYTE bcdIDList[][MAX_ID_SIZE], int num){
	char tmpStrID[16] = { 0 };
	int i = 0;
    BOOL ret=0;
	BYTE bDeviceType = bcdIDList[0][0];
    //printf("check os list if change\n");

    ret = _CompareOsListIfchange(bDeviceType,bcdIDList,num);
    if(ret)
    {
        printf("os list is change,and rewrite to config.ini now\n");
	OSA_DBG_MSGX("%s : OSType=%02X, num=%d ",__func__,bDeviceType,num);
	/*delete old list */
	__del_welcome_app_monitorOSList(bDeviceType);

	for(i = 0; i < num; i++){
		memset(tmpStrID,0x00, sizeof(tmpStrID));
		sprintf(tmpStrID,"%02X%02X%02X%02X%02X%02X",bcdIDList[i][0],bcdIDList[i][1],bcdIDList[i][2],bcdIDList[i][3],bcdIDList[i][4],bcdIDList[i][5]);
		__save_welcome_app_monitoryOS(tmpStrID);
	}
    }
	
}

void __deal_monitory2ndOsListChanged(int num, char *pIPArry){

	OSA_DBG_MSGX("%s : num=%d ",__func__,num);
	/*delete old list */
	__del_welcome_app_monitorOSList(DT_DIGITAL_SECOND_DOOR_STATION);
	__save_welcome_app_monitory2ndOS(num,pIPArry);

}



void __deal_ipsIPChanged(){
	__save_basicCfg(eCFG_SAVE_IPS_IP);
}


/******************************************************************************
* Name: 	 local_config_init 
*
* Desc: 	 配置文件初始化
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/26, Lewis-weixin.liu Create this function
 ******************************************************************************/
VOID create_dftUser();

void local_config_init()
{
//during startup, only main thread running, so do not need pthread_mutex
	if(!OSA_DirIsExist(CFG_IPGW_CFG_DIR))
	{
		OSA_DirCreate(CFG_IPGW_CFG_DIR);
	}
	
	/*SIP SERVER CFG FILE*/
	if(!OSA_DirIsExist(CFG_FLEXISIP_DIR))
	{
		OSA_DirCreate(CFG_FLEXISIP_DIR);
	}
	/*web dir*/
	if(!OSA_DirIsExist(CFG_IPGW_WEB_DIR))
	{
		OSA_DirCreate(CFG_IPGW_WEB_DIR);
	}
	/*
	if(!OSA_FileIsExist(CFG_FLEXISIP_AUTH))
	{
		char shell_str[100]= {0};
		sprintf(shell_str,"touch %s",CFG_FLEXISIP_AUTH);
		do_system1(shell_str,0);	
		OSA_Sleep(100);
	}
	if(!OSA_FileIsExist(CFG_ROUTE))
	{
		char shell_str[100]= {0};
		sprintf(shell_str,"touch %s",CFG_ROUTE);
		do_system1(shell_str,0);		
		OSA_Sleep(100);
	}
*/
	if(!OSA_FileIsExist(CFG_BASIC))
	{
	   __save_createDefaultCfg();
	   OSA_Sleep(100);
	}
	

	int bootcount = add_boot_count();
	if(bootcount == 1 || access(CFG_WEB_CERT,F_OK) != 0)
	{	
		//生成证书
		//do_system1(SHELL_GENE_CERT,0);
        system(SHELL_GENE_CERT);
	}
	
	__load_AllConfigFromFile();



    sync_req_flag_init();


	return 0;
}




void cfg_internal_network_protected(BYTE mode,char *ip, char *mask, char *gw, char*dns)
{

	OSA_DBG_MSG("Set to address: mode=%d ip=%s, mask=%s, gw=%s, dns=%s",mode,ip,mask,gw,dns);
	pthread_mutex_lock(&g_Mutex_config);
	__set_gNetworkInfo(mode,ip,mask,gw,dns);
	pthread_mutex_unlock(&g_Mutex_config);	
}



/******************************************************************************
* Name: 	 wait_networkReady
*
* Desc: 	 等待网卡配置就绪	
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 	As the network is configured by Hdi06_main process, IPGW wait for network ready, 
*			Then, start the function thread
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2015/05/18, Andy-wei.hou Create this function
 ******************************************************************************/
void wait_networkReady(){
    char szIP_addr[16] = {0};
    char szMask[16] = {0};
	char szGw[16] = {0};
	char szDns[16] = {0};
	int iCount = 0;
	while(1){
		if(read_ip_addr(szIP_addr,szMask,INTERFACE_INTERNAL) == 0 )
		{
		 	get_gatewayip(szGw, 20);
		    GetNameserver(szDns);
			printf("===============szIP_addr = %s mask=%s gw=%s dns=%s",szIP_addr,szMask,szGw,szDns);
			cfg_internal_network_protected(eDHCP,szIP_addr,szMask,szGw,szDns);
		    break;
		}
		iCount++;
		if(iCount>5)
		{
			break;
		}
#if 0
		else
		{
			iCount ++;
			OSA_DBG_MSGX("Network of %s not ready, loop wait !!!",INTERFACE_INTERNAL);
			if(iCount >= 10){
				OSA_ERROR("Can't get IP address of %s", INTERFACE_INTERNAL);
				break;
			}
		}
#endif		
		OSA_Sleep(1000);
	}
}
/******************************************************************************
* Name: 	 set_internal_network
*
* Desc: 	 设置内部网卡
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/27, Lewis-weixin.liu Create this function
 ******************************************************************************/
void set_internal_network()
{
	return ; 	/*Add By Andy-wei.hou 2015.05.18; The actually network setting done by hdi06_main process*/
	
    char szIP_addr[16] = {0};
    char szMask[16] = {0};
	char szGw[16] = {0};
	char szDns[16] = {0};
    char szShell_buf[128] ={0};
	//if(get_startMode() == eConfigMode)
	//	return;

    /*Internal interface*/
    if(get_ip_mode()== eDHCP) 
    {
    	/*
    			Sometimes, ifconfg eth1 down will lead mrouted process exit abnormally.
    			So, using ifconfig eth1 0.0.0.0 to avoid this problem
    			--Andy 2013.08.13
    		*/
    	sprintf(szShell_buf,"ifconfig %s 0.0.0.0",INTERFACE_INTERNAL);
		//if( do_system1("ifconfig eth1 0.0.0.0",0) < 0)
		if(system(szShell_buf) < 0)
		{
			OSA_ERROR("ifconfig %s down fail!\n",INTERFACE_INTERNAL);
			return;
		}
        if( system("killall udhcpc")<0)
		{
            OSA_ERROR("killall udhcpc fail!\n");
            return;
		}

		/*
		As if there is no DHCP server, the waiting for DHCP discover will be almost 8s. 
		In order to aviod watdog reset. Just feed it before and after starting dhcpc
		*/
//		FeedWatchDogEx(eDealCgiConfigThr);

		sprintf(szShell_buf,"udhcpc  -H  ip-gateway -i %s -a -b&",INTERFACE_INTERNAL);

		if(system(szShell_buf) < 0)
		{
			OSA_ERROR("Can't get IP address dynamic\n");
			return;
		}
//		FeedWatchDogEx(eDealCgiConfigThr);

        /* if we dhcp failed, we just use the default ip 192.168.1.200*/
		/*
		As after using popen and pclose, udhcpc will blocked until it get the local ip 
		otherwise, the -b parameter of udhcpc will force the process to background.
		So, we do not need to wait 8 times for IP obatin.
		*/

        if(read_ip_addr(szIP_addr,szMask,INTERFACE_INTERNAL) == 0 )
        {
		 	get_gatewayip(szGw, 20);
            GetNameserver(szDns);
			OSA_DBG_MSG("===============szIP_addr = %s mask=%s gw=%s dns=%s",szIP_addr,szMask,szGw,szDns);
			cfg_internal_network_protected(eDHCP,szIP_addr,szMask,szGw,szDns);
            //break;
        }
		else
        {
            OSA_ERROR("dhcp failed , set to default address");
			cfg_internal_network_protected(eStatic,DEFAULT_IP_GATEWAY_IP,DEFAULT_MASK,DEFAULT_GATEWAY,DEFAULT_GATEWAY);
			set_internal_network();
        }

		
    }
    else if(get_ip_mode()== eStatic) 
    {
	    /*
    			Sometimes, ifconfg eth1 down will lead mrouted process exit abnormally.
    			So, using ifconfig eth1 0.0.0.0 to avoid this problem
    			--Andy 2013.08.13
    		*/

		get_internal_IP(szIP_addr);
		get_internal_mask(szMask);
		get_internal_gateway(szGw);
		get_internal_dns(szDns);

		
        sprintf(szShell_buf,"ifconfig %s 0.0.0.0",INTERFACE_INTERNAL);
        if(system(szShell_buf) < 0)
        {
            OSA_ERROR("set internal network down faled");
        }

        sprintf(szShell_buf,"ifconfig %s %s netmask %s",INTERFACE_INTERNAL,szIP_addr,szMask);
        if(system(szShell_buf) < 0)
        {
            OSA_ERROR("set internal static ip fail!");
        }
		//Start ip address conflicted detect thread
		OSA_ipv4_acd_start(INTERFACE_INTERNAL,szIP_addr,ASYNNCH);
		
		OSA_printf("g_stCfg_info.szInternal_gateway = %s",szGw);
        sprintf(szShell_buf,"route add default gw %s",szGw);
		if(system(szShell_buf) < 0)
        {
            OSA_ERROR("set route ip fail!");
        }
    }
	get_internal_IP(szIP_addr);
	get_internal_mask(szMask);
	get_internal_gateway(szGw);	
	get_internal_dns(szDns);
	OSA_DBG_MSG("IP set to %d %s %s %s %s \n",get_ip_mode(), szIP_addr,szMask, szGw, szDns);
}


void setExternalNetwork()
{
	char shellBuff[256] = { 0 };
	char ex_ip[20]= { 0 };
	get_external_IP(ex_ip);
	sprintf(shellBuff,"ifconfig eth0 %s",ex_ip);
	//do_system1(shellBuff,0);
    system(shellBuff);
}
/******************************************************************************
* Name: 	 network_init 
*
* Desc: 	 网络初始化
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/26, Lewis-weixin.liu Create this function
 ******************************************************************************/

void network_init()
{
	/*should be remove after released, as eth0 ip is configured by hdi06_main*/
//	setExternalNetwork();
	
    set_internal_network();
	
	//wait_networkReady();
	
#ifdef CONFIG_PRINTF
    print_config_info();
#endif   

	system("route add -net 239.0.0.0 netmask 255.0.0.0 eth1");
#if defined(__Use__HB__miniOS__)
    system("route add -net 255.255.255.255 netmask 255.255.255.255 dev eth0 metric 1");
#endif
}





/******************************************************************************
* Name: 	 config_SIP_server 
*
* Desc: 	 更新配置服务器文件config.ini
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/28, Lewis-weixin.liu Create this function
 ******************************************************************************/
void config_welcome_app()
{
	pthread_mutex_lock(&g_Mutex_config);
	__save_welcome_app_url(g_stCfg_info.szInternal_IP);
    __save_welcome_app_domain(g_stCfg_info.szExternal_IP);
	__save_welcome_app_userQrCode(g_stCfg_info.szUsr_Priv,g_stCfg_info.szInternal_IP);
    __del_welcome_app_monitorOSList(DT_DOOR_STATION); //开机删除该单元所有门口机列表
    __del_welcome_app_monitorOSList(DT_GATE_STATION); //开机删除该单元所有门口机列表
	pthread_mutex_unlock(&g_Mutex_config);
}

/******************************************************************************
* Name: 	 config_SIP_server 
*
* Desc: 	 更新配置服务器文件flexisip.conf
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/28, Lewis-weixin.liu Create this function
 ******************************************************************************/

void config_SIP_server()
{ 
	pthread_mutex_lock(&g_Mutex_config);
	if(!OSA_FileIsExist(CFG_FLEXISIP))
	{
		dump_SIP_server_DefCfg();		/*add by Andy inorder to avoid flexisip crushed by config file problem*/
	}

    else //sip通话不进行认证
    {
        SetValueToEtcFile_new(CFG_FLEXISIP,"module::Authentication","filter","is_request && request.method-name != 'MESSAGE' && request.method-name != 'INVITE'"); 
    }
    __save_SIP_server_transport(g_stCfg_info.szGlobal_address,g_stCfg_info.szInternal_IP);
    __save_SIP_server_route(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv);
    __save_SIP_server_auth(g_stCfg_info.szExternal_IP,g_stCfg_info.szUnit,g_stCfg_info.szApartment,g_stCfg_info.szUsr_Priv);
    __save_SIP_server_nortpproxy(g_stCfg_info.szExternal_IP);
	pthread_mutex_unlock(&g_Mutex_config);
}






/******************************************************************************
* Name: 	 app_config_init 
*
* Desc: 	 配置其他应用程序
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/26, Lewis-weixin.liu Create this function
 ******************************************************************************/
void app_config_init()
{   
    config_welcome_app();
    config_SIP_server();
	//do_system1("sync",0);
    system("sync");
}



void start_web()
{
    char szShell[128] = {0};
	int iPid = 0;
	if(get_startMode()== eConfigMode)
		return;
	
	printf("\nStart Appweb\n");
#if 0
    do_system1("killall appweb",0);
#endif
/*
	sometimes killall can not kill the proces. So we change the way using kill -9 pid
	---Andy-wei.hou 2013.08.15
*/
	if(find_pid_by_name("appweb",&iPid) == 0)
		{
			sprintf(szShell,"kill -9 %d",iPid);
			if(system(szShell) < 0)
				OSA_ERROR("Kill appweb fail, errno(%d)",errno);
		}

    sprintf(szShell,"exec appweb --config %s &",CFG_APPWEB);    
    if(0 > system(szShell))
		OSA_ERROR("Start appweb fail, errno(%d)",errno);

/*
	using renice command to startup appweb with lower priroity inorder to avoide the problem
	that if too many welcome APPs download history files simultaneously, the CPU occupancy of appweb
	process will reach about 90%, this will lead another realtime process can't using CPU at this moment
	and bring some packets discard or video delay. So, the priroity of APPweb process should be lower 
	than this realtime process.
	---Andy-wei.hou 2013.10.10

	As upnp thread will change the priority of appweb, so we have to renice the priority of appweb afater
	start upnp. in upnp_thr.h
*/

}
void start_flexisip()
{
 
	char szShell[128] = {0};
	int iPid = 0;

	printf("\nStart Flexisip\n");
	if(find_pid_by_name("flexisip",&iPid) == 0)
	{
		sprintf(szShell,"kill -9 %d",iPid);
		if(system(szShell) < 0)
		{
			OSA_ERROR("Kill flexisip fail, errno(%d)",errno);
		}
	}

//	sleep(1);
	OSA_Sleep(100);
	sprintf(szShell,"exec flexisip --configfile %s &",CFG_FLEXISIP);    
   	if( 0 > system(szShell))
   	{
		OSA_ERROR("Start flexisip fail, errno(%d)",errno)
   	}
	OSA_Sleep(200);
	printf("\nStart Flexisip end\n");
}

void start_b2bsip()
{
 
	char szShell[128] = {0};
	int iPid = 0;
	perror("\nStart b2bsip\n");
	if(find_pid_by_name("b2bsip",&iPid) == 0)
	{
		sprintf(szShell,"kill -9 %d",iPid);
		if(system(szShell) < 0)
		{
			OSA_ERROR("Kill b2bsip fail, errno(%d)",errno);
		}
	}

	OSA_Sleep(100);
	sprintf(szShell,"%s","exec b2bsip &");    
   	if( 0 > system(szShell))
   	{
		OSA_ERROR("Start b2bsip fail, errno(%d)",errno)
   	}
}


void start_mrouted(void){
	char szShell[128] = {0};
	int iPid = 0;
	int networkType = Engineer_Settings_GetDeviceType(); //
	int netport = Engineer_Settings_GetDomesticNetPort();

	OSA_DBG_MSGXX("networkType %d \n",networkType);
	
	printf("\nStart Mrouted\n");
	if(find_pid_by_name("mrouted",&iPid) == 0)
	{
		sprintf(szShell,"kill -9 %d",iPid);
		if(system(szShell) < 0)
		{
			OSA_ERROR("Kill mrouted fail, errno(%d)",errno);
		}
	}

	sleep(1);
	
	if(networkType ==  0)  //LAN+LAN
	{
		if(netport ==  eNetPortLan1)
		{
			sprintf(szShell,"%s","mrouted -c /etc/mrouted_lan1.conf &");
		}
		else if(netport ==  eNetPortLan2)
		{
			sprintf(szShell,"%s","mrouted -c /etc/mrouted_lan2.conf &");
		}
	}
	else if(networkType == 1) //LAN+WIFI
	{
		sprintf(szShell,"%s","mrouted -c /etc/mrouted.conf &");
	}
		
	if(0 > system(szShell))
	{
		OSA_ERROR("Start Mrouted fail, errno(%d)",errno);
	}
	
}




/*恢复出厂设置*/
void RecoverFactorySetting(void)
{
    char szReset_script[32] = {0};
    sprintf(szReset_script,"%s",SHELL_RESET);
    //do_system1(szReset_script,0);
    system(szReset_script);
}



#if 0
/******************************************************************************
* Name: 	 detect_network_info 
*
* Desc: 	 检测网络信息
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Lewis-weixin.liu
* -------------------------------------
* Log: 	 2013/03/26, Lewis-weixin.liu Create this function
 ******************************************************************************/
int detect_network_info()
{
    char bufip[20] = {0};
    char bufmask[20] = {0};
    char bufgw[20] = {0};
    char buftmp[128] = {0};

	char oldIp[20] = {0};
    char oldMask[20] = {0};
    char oldGw[20] = {0};
	int  oldIPmode = 0;

	INT iGetIpRet = -1;

	oldIPmode = get_ip_mode();
	sprintf(oldIp,"%s",SystemDeviceInfo_GetHomeIPAddress());
	iGetIpRet = read_ip_addr(bufip, bufmask,GetSecondPhyType());
	OSA_DBG_MSGXX("new ipaddr %s oldIp %s \n",bufip,oldIp);
	if(oldIPmode == eDHCP){
		if(iGetIpRet == -1 ){
			OSA_DBG_MSG("dhcp lease expire \n");
            sprintf(g_stCfg_info.szInternal_IP,"%s",""); //DHCP过程中没有获取到IP，将全局变量清空
			g_dhcp_try_times ++;
		}

		if(8 < g_dhcp_try_times )
		{
			g_dhcp_try_times = 0;
			// if dhcp can not allocate ip, using the default ones	-- Andy  08.09
			OSA_DBG_MSG("DHCP get IP fail, recover to default setting");
			cfg_internal_network_protected(eStatic,DEFAULT_IP_GATEWAY_IP,DEFAULT_MASK,DEFAULT_GATEWAY,DEFAULT_GATEWAY);
			set_internal_network();
		}
	}

	/*process ip changed*/
    if((iGetIpRet == 0 ) && strcmp(bufip, oldIp) != 0)
    {	/*ip changed*/
		OSA_DBG_MSGX("Detect IP changed");
		if(get_gatewayip(bufgw, 20) )
        {	
        	if( strcmp(bufgw,"0.0.0.0")==0) 
			{//说明由于IP的改变删除了路由表中dfault gw 要重新设置
				memset(bufgw,0x00, sizeof(bufgw));
				get_internal_gateway(bufgw);
				OSA_DBG_MSG("set defalut gw %s",bufgw);
				sprintf(buftmp,"route add default gw %s",bufgw);
				//do_system1(buftmp,0);
				system(buftmp);
			}
        }

		oldIPmode = get_ip_mode();
//		OSA_DBG_MSGX("mode = %d",oldIPmode);
		
		pthread_mutex_lock(&g_Mutex_config);
		__set_gNetworkInfo(oldIPmode,bufip,bufmask,bufgw,NULL);
//		OSA_DBG_MSGXX("modify cfg file");
		__deal_interIPChanged();
		
		pthread_mutex_unlock(&g_Mutex_config);	

	    if(GetneedrestartFlexisip())
	    {
            start_flexisip();
    		start_mrouted();
	    }

//		OSA_ipv4_acd_start(INTERFACE_INTERNAL,bufip,ASYNNCH);

    }
}

#endif

/*
	用户权限校验函数
*/

int user_priv_check(char *pUsr_name, char cPriv_type )
{
	int i = 0 ;
	USER_PRI_INFO szUsr_priv = {NULL,0,0,0,0};
	char pLoc_bcd_ID[MAX_ID_SIZE]= {0};
	char pCom_bcd_id[MAX_ID_SIZE] = {0};
	//if the user_name is the ID of this apartment, we think it is the IS, so allow all the priviledge
	get_local_ID(pLoc_bcd_ID);
	generate_bcdid_by_text(pUsr_name, (char *)pCom_bcd_id);
	pLoc_bcd_ID[5] = 0;
	pCom_bcd_id[5] = 0;

	if(strcmp(pLoc_bcd_ID,pCom_bcd_id) == 0){
		OSA_DBG_MSGX("Check Usr privilege by IS, return true to it");
		return 1;
		}

	
	for ( i = 0; i< USER_MAX; i++ ){
		get_usr_priv_info(&szUsr_priv,i);
		OSA_DBG_MSG("Get Local userinfo: name=(%s),Survey(%d),opendoor(%d),switchlight(%d),swithchmontor(%d)"
			,szUsr_priv.usrname
			,szUsr_priv.survey
			,szUsr_priv.opendoor
			,szUsr_priv.switchlight
			,szUsr_priv.switchmonitor
			);
		if( strcmp(pUsr_name,szUsr_priv.usrname)==0){
			switch(cPriv_type)
				{
					case eSURVEY :{
						return szUsr_priv.survey;
					}break;
					case eOPENDOOR : {
						return szUsr_priv.opendoor;
					}break;
					case eSWITCHLIGHT : {
						return szUsr_priv.switchlight;
					}break;
					case eSWITCHMONITOR : {
						return szUsr_priv.switchmonitor;
					}break;
					
					default:
						OSA_ERROR("Unknow Privilege Type (%d)",cPriv_type);
				}
		}		
	}
	if(i >= USER_MAX){
		OSA_DBG_MSG("Can't Find username(%s)",pUsr_name);
		return 0;
	}
}


/******************************************************************************
* Name: 	 deal_usr_priv_check
*
* Desc: 	 响应主机检查用户权限请求
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2013/06/28,Andy-wei.hou Create this function
 ******************************************************************************/
void deal_usr_priv_check(s_PacketMsg *interMsg, BYTE *data,UINT32 len)
{
	SOCK_DATA_PACKET_T *netMsg = (SOCK_DATA_PACKET_T *)data;
	BYTE dataBuf[16]= {0};
	BYTE dataLen = 0;
    BOOL bRet = FALSE;
	char cPriv_type ;
	char pUser_name[USERNAME_LENGTH] = {0};
	char *pData_buf = (data+ sizeof(SOCK_DATA_PACKET_T));

	/*
		pData_buf[0] is the privilege type
		pData_buf[1...33] is the username
	*/
	cPriv_type = pData_buf[0];
	sprintf(pUser_name,"%s",(pData_buf+1));
	OSA_DBG_MSGX(" Username (%s),privilege type(%d)",pUser_name,cPriv_type);
		
    switch(netMsg->operCode)
    {
    	case OPER_PRIVILEGE_REQUEST:       
    	{
			dataLen = 1 + 1 + 1 ;
			dataBuf[0] = SOCK_ACK_OK;
			dataBuf[1] = cPriv_type;
			dataBuf[2] = user_priv_check(pUser_name,cPriv_type);

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

void setStartModeByUI(BYTE mode, char*resp){
	pthread_mutex_lock(&g_Mutex_config);
	__set_gStartMode(mode);
	__save_basicCfg(eCFG_SAVE_START_MODE);
	pthread_mutex_unlock(&g_Mutex_config);
	OSA_DBG_MSGX("StartMode Changed to %s", mode == eConfigMode ? "ConfigMode" : "FullMode");
	resp = 0;

}

/***************************************************************
*FUN: 		setNetworkByUI
*DESC:		处理UI发送的配置网络请求
*INPUT: 		networkBuf,  网络配置数据，格式参考通讯协议中(FunCode=0x0D, Oper=0x01)中从DHCP开始的字段
			respBuf,	     配置反馈结果，格式参考通讯协议中(FunCode=0x0D, Oper=0x02)中从DHCP开始的字段
*OUTPUT:	-1， 输入参数错误
			-2，IP冲突
			0，  配置成功，但是无修改
			=1，配置成功
*NOTE:		当返回值>0 时，需要调用start_flexisip(); start_mrouted(); 重启SIP服务器和多播路由器
			
*Author:		Andy-wei.hou
*LOG:		2015/03/17, Created by Andy-wei.hou
			2015/05/12, Network setting shoule be enabled after reboot. So remove the restart_flexisip(), restart_mrouted() process
****************************************************************/
int setNetworkByUI(char *networkBuf,char *respBuf)
{
#define CONFIG_IP_STR_LEN 16
#define CONFIG_HOST_DNS_LEN 256

enum cfgNetworkIndex_tag{
	 eCfgNetworkMode = 0,
	 eCfgNetworkIP,
	 eCfgNetworkMask,
	 eCfgNetworkGateway,
	 eCfgNetworkDNS,
	 eCfgNetworkHost
};

	char mode = 0;
    char netmask[16] = {0};
    char localIp[16] = {0};
    char gateway[16] = {0};
    char szGlobal_addr[256] = { 0 };
    char szDNS[16] = {0};
    int changed	= 0;
	int iRet = 0;

	if(!networkBuf && !respBuf){
		OSA_ERROR("Invalid parameter");
		return -1;
	}

    //analysis the network info from recving packets buffer
	mode = networkBuf[0];
	if(eStatic == mode)
	{		
		memcpy(localIp, &networkBuf[1],CONFIG_IP_STR_LEN);
	    localIp[15] = '\0';
		memcpy(netmask, &networkBuf[1+CONFIG_IP_STR_LEN], CONFIG_IP_STR_LEN);
		netmask[15] = '\0';

		memcpy(gateway, &networkBuf[1+2*CONFIG_IP_STR_LEN], CONFIG_IP_STR_LEN);
		gateway[15] = '\0';
		memcpy(szDNS,   &networkBuf[1+3*CONFIG_IP_STR_LEN], CONFIG_IP_STR_LEN);
		szDNS[15] = '\0';
	}
	memcpy(szGlobal_addr, &networkBuf[1+4*CONFIG_IP_STR_LEN], CONFIG_HOST_DNS_LEN);	
	szGlobal_addr[255] = '\0';
	memset(respBuf, 0x00, 6);	
	
	OSA_DBG_MSG("\r\nLocal Network     : mode=%d, ip=%s, mask=%s,gw=%s, dns=%s, globalAddr=%s\r\n",g_stCfg_info.byIP_mode,g_stCfg_info.szInternal_IP,g_stCfg_info.szInternal_mask,g_stCfg_info.szInternal_gateway,g_stCfg_info.szDNS,g_stCfg_info.szGlobal_address);
	OSA_DBG_MSG("\r\nSet NetWork By UI : mode=%d, ip=%s, mask=%s,gw=%s, dns=%s, globalAddr=%s\r\n",mode,localIp,netmask,gateway,szDNS,szGlobal_addr);
	
	//if IP changed and the mode is static ip mode , start ip address conflict detect thread
	pthread_mutex_lock(&g_Mutex_config);
	
	if(eStatic == mode){
		if(strcmp(g_stCfg_info.szInternal_IP, localIp) !=0){
			/*测试IP是否可用*/
			if(1==OSA_ipv4_acd_start(INTERFACE_INTERNAL,localIp,SEQUENCE))
			{
				//if the ipconflicted detected 
				respBuf[eCfgNetworkIP] = 1;
				iRet = -2;	/*IP Addr Confilicte*/
				goto out;
			}
		}
#if 0
	   if(g_stCfg_info.byIP_mode  == eDHCP)
        {
            do_system1("killall udhcpc",0);
        }
#endif		
		if((strcmp(g_stCfg_info.szInternal_IP, localIp)) 
			|| (strcmp(g_stCfg_info.szInternal_mask, netmask))
			|| (strcmp(g_stCfg_info.szInternal_gateway, gateway))
			|| g_stCfg_info.byIP_mode != mode)
	    {
	        __set_gNetworkInfo(mode,localIp,netmask,gateway,szDNS);
			__deal_interIPChanged();
			changed = 1; 
	    }    

		if((strcmp(szDNS,g_stCfg_info.szDNS)))
		{
			__set_gNetworkInfo(mode,NULL,NULL,NULL,szDNS);
			__deal_interDNSChanged();
		}
		

		
	}
	else if(eDHCP == mode)
	{
	   if(eStatic == g_stCfg_info.byIP_mode)
	    {
			__set_gNetworkInfo(mode,NULL,NULL,NULL,NULL);
			__deal_interIPChanged();
			changed = 1;
	        
	    }
		
	}
    
out:
    if(strcmp(g_stCfg_info.szGlobal_address, szGlobal_addr))
    {
    	__set_gGlobalAddr(szGlobal_addr);
		 __deal_globalAddressChanged();
    }
  
	pthread_mutex_unlock(&g_Mutex_config);
	
//	if(changed == 1)/*As set ip address in DHCP mode might exhault too many times, so ,let it alone protected by mutex*/
//		set_internal_network(); 

	return iRet < 0 ? iRet: changed;
}

int setGlobalAddressByUI(char *globalAddr){
    int change = 0;
	if(!globalAddr){
		OSA_ERROR("Invalid Parameter");
		return -1;
	}
	
	pthread_mutex_lock(&g_Mutex_config);
    if(strcmp(globalAddr,g_stCfg_info.szGlobal_address) != 0  )
    {
		__set_gGlobalAddr(globalAddr);
		__deal_globalAddressChanged();
		change = 1;
    }
   
	pthread_mutex_unlock(&g_Mutex_config);
	return change;
}


/***************************************************************
*FUN: 		setBasicIDByUI
*DESC:		处理UI发送的配置ID请求
*INPUT: 		idStr，传入ASCII 码字符串ID, 例如"750001010102"
*OUTPUT:	-1， 输入参数错误
			0，  配置成功，但是无修改
			=1，配置成功
*NOTE:		当返回值>0 时，需要调用start_flexisip(); 重启SIP服务器
*Author:		Andy-wei.hou
*LOG:		2015/03/17, Created by Andy-wei.hou
****************************************************************/
int setBasicIDByUI(char *idStr){
	char unit[8] = {0};
    char apartment[8] = {0};
    int change = 0;
	if(!idStr){
		OSA_ERROR("Invalid Parameter");
		return -1;
	}
	
    strncpy(unit,idStr+2,4);
    strncpy(apartment,idStr+6,4);
	
	pthread_mutex_lock(&g_Mutex_config);
    if(strcmp(unit,g_stCfg_info.szUnit) || strcmp(apartment,g_stCfg_info.szApartment) )
    {
        if(strcmp(unit,g_stCfg_info.szUnit)) //单元号改变
        {
            //删除config.ini中的主机列表
            __del_welcome_app_monitorOSList(DT_DOOR_STATION);
        }
    	__set_gLocal_id(unit,apartment);
        __set_gModify_User_name(unit,apartment);
		__deal_apartmentIDChanged();
		change = 1;
		OSA_printf("ID set to %s %s" ,g_stCfg_info.szUnit,g_stCfg_info.szApartment);		
    }
   
	pthread_mutex_unlock(&g_Mutex_config);
	return change;
}

#if 0
int setIPSIPFromUI(char *ipsIP){
	char ipsaddr[16] = {0};
	int change = 0; 
	if(!ipsIP){
		OSA_ERROR("Invalid Parameter");
		return -1;
	}
    memcpy(ipsaddr,ipsIP,16);
	ipsaddr[15] = 0;
    OSA_DBG_MSGX("Set IPS addr by UI :ipsaddr = %s",ipsaddr);
	
	pthread_mutex_lock(&g_Mutex_config);
    if(strcmp(ipsaddr,g_stCfg_info.szIPS_IP))
    {
    	__set_gIPSIP(ipsaddr);
        __deal_ipsIPChanged();
		change = 1;
    }
	pthread_mutex_unlock(&g_Mutex_config);
	if(change)
		clear_EIB_SEARCH_SUCCESS();
	
	return 0;
}

#endif
/***************************************************************
*FUN: 		setEngCfgFromUI
*DESC:		处理UI发送的工程配置请求
*INPUT: 		engCfgBuf,	输入的工程配置参数，参见通信协议，地址设置命令中地址数据字段
			respBuf,		配置结果，参见通信协议，地址设置响应中的应答数据
*OUTPUT:	-1， 输入参数错误
			0，  配置成功
*NOTE:		Set Engineer configuration just write the configure file . do reboot to enable
*Author:		Andy-wei.hou
*LOG:		2015/03/17, Created by Andy-wei.hou
			2015/05/12, Modified by Andy-wei.hou. Just remove the enable network process & flexisip & mrouted process
****************************************************************/
int setEngCfgFromUI(char *engCfgBuf, char *respBuf){
	if(!engCfgBuf && !respBuf){

		OSA_ERROR("Invalid Parameter");
		return -1;
	}
	int bRestartFlexisip = 0;
	int bRestartMrouted = 0;
	BYTE  mode = engCfgBuf[0]; 
	char *pID = &engCfgBuf[2];
	char *pIDResp = &respBuf[2];
	char *pNetworkCfg = &engCfgBuf[2+2*16];
	char *pNetResp = &respBuf[4];
	int  iRet = -1;

	setStartModeByUI(mode,&respBuf[0]);

	iRet = setBasicIDByUI(pID);
	*pIDResp = iRet >= 0 ? 0 : 1;
	bRestartFlexisip = iRet>0 ? 1 : 0;
	
	if((iRet = setNetworkByUI(pNetworkCfg,pNetResp)) > 0 ){
		bRestartFlexisip = 1;
		bRestartMrouted = 1 ;
	}

#if 0
	if(bRestartFlexisip)
		start_flexisip();
	if(bRestartMrouted)
		start_mrouted();
#endif	

	return iRet;
	
}


/***************************************************************
*FUN: 		setUserCfg
*DESC:		处理用户配置请求
*INPUT: 		
*OUTPUT:	-1， 输入参数错误
			0，  配置成功
*NOTE:		
*Author:		Andy-wei.hou
*LOG:		2015/03/17, Created by Andy-wei.hou
****************************************************************/
int setUserCfg(USER_PRI_INFO *userInfo, BYTE operType)
{
	BOOL changed = 0;
	BOOL needRestartFlexisip = 0;
	BOOL needRestartAppweb = 0;
	if(!userInfo){
		OSA_ERROR("Invalid Parameter");
		return -1;
	}
	printf("OperType = %d, user=%s \r\n",operType,userInfo->usrname);
	pthread_mutex_lock(&g_Mutex_config);
	/*modify the global variablies*/
	switch(operType){
		case eUSER_CFG_NEW:{
			changed = __set_gAdd_user(userInfo) == 0 ? 1 : 0;
		}break;
		case eUSER_CFG_DEL: {
			changed = __set_gDel_user(userInfo->usrname) == 0 ? 1: 0;
		}break;
		case eUSER_CFG_MODIFY_PASS: {
			changed = __set_gModify_user_pass(userInfo->usrname,userInfo->pass)==0 ? 1 : 0;
		}break;
		case eUSER_CFG_MODIFY_PRI:{
			changed = __set_gModify_user_pri(userInfo) == 0 ? 1:0;
		}break;
		default : {
			OSA_ERROR("unknow OperType =%02X", operType);
		}
	}

	
	/*save to CFG file*/
	if(changed){
		switch(operType){
			case eUSER_CFG_DEL:
			case eUSER_CFG_NEW: {
				__deal_userChanged();
				needRestartFlexisip = 1;
				needRestartAppweb = 1;
			}break;
			case eUSER_CFG_MODIFY_PASS: {
				__deal_userPassChanged();
				needRestartFlexisip = 1;
				needRestartAppweb = 1;
			}break;
			case eUSER_CFG_MODIFY_PRI:{
				__deal_userPriChanged();
			}break;
			
		}
	}
	pthread_mutex_unlock(&g_Mutex_config);

    if(GetneedrestartFlexisip())
    {
    	if(needRestartFlexisip)
    		start_flexisip();
    	if(needRestartAppweb)
    		start_web();
    }
	
	return changed == 0 ? -1 : 0 ;
}


VOID create_dftUser(){
	if(OSA_FileIsExist(CFG_USER) == FALSE){
		OSA_DBG_MSGX("Test user config file %s fail, cretate default ",CFG_USER);
		USER_PRI_INFO userInfo[2];
		int i=0;
		for(i =0; i<2; i++){
			sprintf(userInfo[i].usrname,"user%d_00010101",i+1);
			sprintf(userInfo[i].pass,DEFAULT_USR_PSW);
			if(i ==0){
				userInfo[i].opendoor = 1;
			}
			else{
				userInfo[i].opendoor = 0;
			}
			userInfo[i].survey = 1;
			userInfo[i].switchlight = 1;
			userInfo[i].switchmonitor = 1;
			setUserCfg(&userInfo[i],eUSER_CFG_NEW);
		}
	}
}
#if 0
void dealConfigFromUI(s_PacketMsg *interMsg, BYTE *data,UINT32 dataLen)
{
	BYTE ackData[10] = { 0 };
	INT ackLen = 0;
	SOCK_DATA_PACKET_T * netMsg = (SOCK_DATA_PACKET_T *)data;
	BYTE *pData = data + sizeof(SOCK_DATA_PACKET_T);
	DisplayNetCmdPacket(netMsg,pData,dataLen);
	
	if(UI_FUNC_ENGINEERMODE == netMsg->funcCode){
		/*Engineer Config*/
		if(netMsg->operCode == UI_OPER_ENMODE_IN_ADDRESS){
			/*basic address configuration*/
			setEngCfgFromUI(pData,ackData);
			
			netMsg->operCode = UI_OPER_ENMODE_OUT_ADDRESS_REPLY;	/*engineer configuration response operation*/
			MAKE_DATALEN(netMsg->dataLen,10);
			ackLen = 10;
			
		}
	}
	else if(UI_FUNC_SMARTHOME == netMsg->funcCode)
	{
		/*智能家居*/
		if(netMsg->operCode == UI_OPER_SH_SET_IP){
			/*配置KNX地址*/
			//setIPSIPFromUI(pData+1);

			/*send resp to UI*/
			netMsg->operCode = UI_OPER_SH_OUT_SET_IP_REPLY;	/*ips ip setting resp operation*/
			MAKE_DATALEN(netMsg->dataLen,1);
			
			ackData[0] = 0;
			ackLen = 1;

			
		}
	}
	else if (UI_FUNC_VILLAOS == netMsg->funcCode && UI_OPER_VILLAOS_IN_UPDATECERTIFICATE == netMsg->operCode)
	{
		/*generate certification*/
		//do_system1("/etc/appweb/ssl/gererate_certification",0);
        system("/usr/app/userconfig/appweb/ssl/gererate_certification");
		netMsg->operCode = 0x10;	/*ips ip setting resp operation*/
		MAKE_DATALEN(netMsg->dataLen,1);

		ackData[0] = 0;
		ackLen = 1;
		
	}
	/*else if (UI_FUNC_SETTING == netMsg->funcCode && UI_OPER_SETTING_IN_SETPASSWORD == netMsg->operCode){
		USER_PRI_INFO user;
			int i =0 ;
			for(i =0; i < 2; i++){
				memset(&user,0x00,sizeof(user));
				sprintf(user.usrname,"user%d",i+1);
				sprintf(user.pass,DEFAULT_USR_PSW);
				setUserCfg(&user,eUSER_CFG_MODIFY_PASS);
			}
		ackData[0] = 0;
		ackLen = 1;
	}*/
	else if (UI_FUNC_SETTING == netMsg->funcCode && UI_OPER_SETTING_IN_SETPASSWORD == netMsg->operCode)
	{/*Process user Config*/
	
		USER_PRI_INFO user;
		BYTE type = pData[0];
		if(type == 1){/*user pass changed*/
			int i =0 ;
			for(i =0; i < 2; i++){
				memset(&user,0x00,sizeof(user));
				sprintf(user.usrname,"user%d_%s%s",i+1,g_stCfg_info.szUnit,g_stCfg_info.szApartment);
				memcpy(user.pass,pData+1,10);
				setUserCfg(&user,eUSER_CFG_MODIFY_PASS);
			}
			ackData[0] = 0;
		}
		else
			ackData[0] = 1;
		ackLen = 1;
	}
	else if (UI_FUNC_SETTING == netMsg->funcCode && UI_OPER_SETTING_IN_RESETPWD == netMsg->operCode)
	{//process reset to factory password
        USER_PRI_INFO user;
		BYTE type = pData[0];
		int i =0 ;
		for(i =0; i < 2; i++){
			memset(&user,0x00,sizeof(user));
			sprintf(user.usrname,"user%d_%s%s",i+1,g_stCfg_info.szUnit,g_stCfg_info.szApartment);
			strcpy(user.pass,"123456");
			setUserCfg(&user,eUSER_CFG_MODIFY_PASS);
		}
		ackData[0] = 0;
		ackLen = 1;
	}
	else if (UI_FUNC_SETTING == netMsg->funcCode && UI_OPER_SETTING_IN_RESTORE == netMsg->operCode)
	{	/*Process reset to factory*/
		RecoverFactorySetting();
		netMsg->operCode = UI_OPER_SETTING_OUT_RESTORE_REPLY;	/*ips ip setting resp operation*/
		MAKE_DATALEN(netMsg->dataLen,1);
		ackData[0] = 0;
		ackLen = 1;
	}
	//do_system1("sync",0);
	system("sync");

	/*send resp to requester with */
	if(interMsg->sockfd > 0)
	{
		BOOL bRet = FALSE;
		bRet = SendAckTCP(netMsg, ackData, ackLen, interMsg->sockfd);
		if(bRet == FALSE)
		{
			OSA_ERROR("SendAckTCP FAILED. errno(%d)",errno);
		}
		close(interMsg->sockfd);
	}
}

#endif


void setOSListFromNet(BYTE bcdIDList[ ][ MAX_ID_SIZE ],int num)
{
	if(!bcdIDList)
		return;
	pthread_mutex_lock(&g_Mutex_config);
	__deal_monitorListChanged(bcdIDList,num);
	pthread_mutex_unlock(&g_Mutex_config);
}

void set2ndOSLIst(int num, char *pIpArry){
	if(!pIpArry)
		return;
	pthread_mutex_lock(&g_Mutex_config);
	__deal_monitory2ndOsListChanged(num,pIpArry);
	pthread_mutex_unlock(&g_Mutex_config);
}



LOCAL BOOLEAN ipgwSendDeviceVersion(SOCK_DATA_PACKET_T *cmdPacket, BYTE *dataBuf, INT32 dataLen, INT32 iSockFd)
{
    BYTE localDeviceInfo[256] = { 0 };
    INT32 localDeviceSize = sizeof(localDeviceInfo);
    
    localDeviceInfo[0] = SOCK_ACK_OK;

	get_local_ID(&localDeviceInfo[1]);


    if(SendAckTCP(cmdPacket, localDeviceInfo, localDeviceSize + 7, iSockFd) == FALSE)
    {
        OSA_ERROR("SendAckTCP FAILED.\n");
    }
    
    return TRUE;
}

void dealSet2ndOSFromNet(char *pDataBuf){
	int num;
	if(!pDataBuf)
		return;
	num = pDataBuf[0];
	set2ndOSLIst(num,&pDataBuf[1]);
}


void dealConfigFromNet(s_PacketMsg *interMsg, BYTE *data,UINT32 dataLen)
{
#define MAKEFOURCC_BE(a,b,c,d) ( ((char)(a) << 24) | ((char)(b) << 16) | ((char)(c) << 8) | ((char)(d) << 0) )

	SOCK_DATA_PACKET_T * netMsg = (SOCK_DATA_PACKET_T *)data;

	if(FUNC_MANAGE_APP == netMsg->funcCode){
		switch (netMsg->operCode)
		{
			case OPER_INQUERY_INDOOR_LIST:
				{
					//OSA_DBG_MSG("Receive requir command for ID_IP_LIST");
					inside_alarm_deal_net_msg(interMsg, netMsg,dataLen);

				}break;
			case OPER_PRIVILEGE_REQUEST:
				{//开锁权限校验 APP 通过2833和sip message给os开锁后，os发送该条信息请求权限。
					//respone User privilege request
					deal_usr_priv_check(interMsg, netMsg,dataLen);
					
				}break;
			case OPER_SYNC_REQUEST: //同步标志仲裁
				{
					sync_req_flag_process(interMsg, netMsg,dataLen);
					
				}break;
			case OPER_2ND_OS_LIST_IDIP : {
						//dealSet2ndOSFromNet(data+sizeof(SOCK_DATA_PACKET_T));
				}break;
			default: break;
		}
	}
	else if(FUNC_SYS_CONFIGURATION== netMsg->funcCode){
		switch(netMsg->operCode & 0x7f){
			case OPER_GET_OUTDOOR_LIST: {
				/*Get Monitory OS list */
                //is获取os列表后，os应答的list，发给ipgw，ipgw将该信息保存到config.ini中
				int iLen = MAKEFOURCC_BE(netMsg->dataLen[0],netMsg->dataLen[1],netMsg->dataLen[2],netMsg->dataLen[3]);
				int num = (iLen -1 )/6; 
				char **pList =(data+sizeof(SOCK_DATA_PACKET_T)+1);
				OSA_DBG_MSGXX("Get OS List num=%d",num);
				setOSListFromNet(pList,num);
			}
			break;

			case OPER_SET_TIME:
			case OPER_SET_CALL_GUARD_SEQUENCE:	
            case OPER_SET_SOS_DC_LIFT_ONOFF:
			case OPER_GET_IPACTUATOR_LIST_ACK:	
				/*Forward set time and callGUSeq*/
                // 239组播的转发
				transfer_packet_to_inner_network(YELLOW_RIVER_GROUP_ADDR,YELLOW_RIVER_GROUP_PORT,data,interMsg->datalen); //对于IP-Gateway校时命令从内网转发出去
				break;
				
	
			case OPER_NET_CONNECTED_DEVICE_SEARCH:
				/*process on-line device searching request*/
				ipgwConnectedDeviceSearch(netMsg,data + sizeof(SOCK_DATA_PACKET_T),interMsg->datalen);
				break;
    	    case OPER_GET_MAC_ADDR:    
				/*process */
				//ipgwSendLocalMacAddr(netMsg,data + sizeof(SOCK_DATA_PACKET_T),interMsg->datalen);
				break;

			case OPER_GET_DEV_VERSION:
                {
					/*read Version*/
                    ipgwSendDeviceVersion(netMsg, data + sizeof(SOCK_DATA_PACKET_T), dataLen - sizeof(SOCK_DATA_PACKET_T), interMsg->sockfd);
                    close(interMsg->sockfd);
                }
				break;

				
		}
		
		
	}
	else if (netMsg->funcCode== FUNC_MESSAGE && netMsg->operCode == OPER_SEND_BULLETIN_MSG)
	{	
		/*Forward boardcast msg*/
        //239 广播的公告信息，不带图片的，带图片的由mroute转发
		transfer_packet_to_inner_network(YELLOW_RIVER_GROUP_ADDR,YELLOW_RIVER_GROUP_PORT,data,interMsg->datalen); 
	}
	else if(FUNCCODE_DOORCONTROL== netMsg->funcCode)
	{
		transfer_packet_to_inner_network(YELLOW_RIVER_GROUP_ADDR,YELLOW_RIVER_GROUP_PORT,data,interMsg->datalen); //对于IP-Gateway校时命令从内网转发出去
	}
	else if(FUNC_TRANS_FILE == netMsg->funcCode)
	{
		if(OPER_UPDATE_COMPLETE == netMsg->operCode)
		{
			transfer_packet_to_inner_network(YELLOW_RIVER_GROUP_ADDR,YELLOW_RIVER_GROUP_PORT,data,interMsg->datalen); 
		}
	}
}




/******************************************************************************
* Name: 	 IpgwConfigService
*
* Desc: 	 处理IPGW服务相关的配置
* Param: 	
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy-wei.hou
* -------------------------------------
* Log: 	 2015/03/17, Andy-wei.hou Create this function
 ******************************************************************************/

VOID IpgwConfigService(VOID *arg)
{
    s_PacketMsg interMsg = { 0 };
    INT32 dataLen = 0;
    BYTE *dataBuf = NULL;
    SOCK_DATA_PACKET_T *netMsg = NULL;
    int sockfd = -1;
    
	ipgwConfigServiceThreadRunning = 1;

    InitLocalOsList();

    sockfd = LocalSocketUDPServer(kPORT_MSG2IPGATEWAYCFG);
    if(sockfd < 0)
    {
        perror ("InitSocketUDP(NULL, PORT_MSG2SYSTEMCONFIGURATION, UDP_SINGLE_RECV) failed!\n");
        exit(1);//直接退出应用程序
    }
    
    OSA_THREAD_ID("prn_thread_id");
//	OSA_thrPrintSelfPrio();
    while(ipgwConfigServiceThreadRunning)
    {

        INT32 iRet = ReadFullPacket(sockfd, &interMsg, INTER_PACKET_SIZE, &dataBuf, &dataLen);
        if(iRet >= 0)
        {
        	netMsg = (SOCK_DATA_PACKET_T *)dataBuf;
			OSA_DBG_MSGXX("recv Msg, orderType=%d, socket=%d, dataLen=%d",interMsg.order_type,interMsg.sockfd,interMsg.datalen);
			DisplayNetCmdPacket(netMsg, dataBuf + sizeof(SOCK_DATA_PACKET_T), dataLen - sizeof(SOCK_DATA_PACKET_T));
			
            if(interMsg.order_type == PHONE_ORDER_ENTER_QT)
            {            		
				/*config from ui*/
		        //dealConfigFromUI(&interMsg,dataBuf,dataLen); 
				
            }
			else if(interMsg.order_type == PHONE_ORDER_ENTER_NET)
			{
				/*config or req from YR protocol*/
				dealConfigFromNet(&interMsg,dataBuf,dataLen);                                                

			}
            /*
            处理命令
            */
            SAFE_DELETE_MEM(dataBuf);
        }
	
		
    }
    
    close(sockfd);
}

#if 0
void StartnetStausDetcDaemon(){
	INT32 iCount = 0;
   	char cLinkRecovery = 0;			//Flag used to decide the network connect link is reconnected
	while(1)
	{
//		OSA_DBG_MSGXX("%d", iCount);
		/*misc control*/
        iCount++;			
		if(iCount % 2 ==0)
            detect_network_info();
		 
		/*
			detect the network link state, After network link recovery, start  ip addr conflict detect
			Detect interval is 5s in order to avoid the end user plug in and plug out cable very frequently
			---Andy-wei.hou 2013.08.08
		*/
#if 0
		if(iCount %25 == 0)
		{
			if (0 == cLinkRecovery ){	
				if ( 1==GetNetworkLinkStatusEx(INTERFACE_INTERNAL))
					{
						cLinkRecovery = 1;
						char pIp_buf[32] = {0};
						get_internal_IP(pIp_buf);
						OSA_DBG_MSG("=============Network Link Recovered, IP(%s)",pIp_buf);
						OSA_ipv4_acd_start(INTERFACE_INTERNAL,pIp_buf,ASYNNCH);		
					}
			}
			else
			{
				if ( 0==GetNetworkLinkStatusEx(INTERFACE_INTERNAL)){
						cLinkRecovery = 0;
						OSA_DBG_MSG("Detect Network Link disconnected");
					}
			}
		}
#endif
		OSA_Sleep(5000);
    }
}
#endif

void ipgwServiceCfgInit(){
    //配置初始化
  //  local_config_init();

 	//其他进程的配置初始化
	//app_config_init();
}


void initIPGWConfigSevice(){
	INT iRet = -1;
	iRet = OSA_ThrCreate(&gIPGWCfgServiceThr,(void *)IpgwConfigService,OSA_THR_PRI_RR_MIN,(void *)0);	/*启动IPGW 配置响应线程*/
	if(iRet!=OSA_SOK)
	{
		perror ("Create pthread error!\n");
		exit(1);
	}


	//cfgTest();

}


void cfgTest(){
#define TEST_USER_MAX 1
#define TEST_OS_NUM_MAX 2
#define TEST_USER_CFG
//#define TEST_MISC_CFG
//#define TEST_OSLIST_CFG

	USER_PRI_INFO userInfo[TEST_USER_MAX];;
	int i = 0; 

	OSA_Sleep(1000);

#ifdef TEST_USER_CFG
	/*-----USER Deal Test-------*/
	for(i = 0; i < TEST_USER_MAX; i ++){
		sprintf(userInfo[i].usrname,"user%d",i);
		sprintf(userInfo[i].pass,"123456");
		userInfo[i].opendoor = 1;
		userInfo[i].survey = 1;
		userInfo[i].switchlight = 1;
		userInfo[i].switchmonitor = 1;

		setUserCfg(&userInfo[i],eUSER_CFG_NEW);
	}
	printUserCfgInfo();

	/*change pass*/
	sprintf(userInfo[0].pass,"000000");
	setUserCfg(&userInfo[0],eUSER_CFG_MODIFY_PASS);
	/*change pri*/
	userInfo[0].survey = 0;
	setUserCfg(&userInfo[0],eUSER_CFG_MODIFY_PRI);
	/*del user*/
	setUserCfg(&userInfo[1],eUSER_CFG_DEL);
	printUserCfgInfo();
#endif 

#ifdef TEST_MISC_CFG
	setIPSIPFromUI("10.0.65.5");
	setBasicIDByUI("750001010101");
	setGlobalAddressByUI("abb.cn_cndex");


#endif	

#ifdef TEST_OSLIST_CFG
	BYTE bcdIDList[TEST_OS_NUM_MAX][MAX_ID_SIZE];
	for(i = 0; i< TEST_OS_NUM_MAX; i++){
		bcdIDList[i][0] = DT_GATE_STATION;
		bcdIDList[i][1] = 0x00;
		bcdIDList[i][2] = i+1;
		bcdIDList[i][3] = 0x00;
		bcdIDList[i][4] = 0x00;
		bcdIDList[i][5] = 0x00;
	}
	setOSListFromNet(bcdIDList,TEST_OS_NUM_MAX);
#if 1	
	OSA_Sleep(10000);
	for(i = 0; i< TEST_OS_NUM_MAX; i++){
		bcdIDList[i][0] = DT_DOOR_STATION;
		bcdIDList[i][1] = 0x00;
		bcdIDList[i][2] = 0x00;
		bcdIDList[i][3] = 0x00;
		bcdIDList[i][4] = i+1;
		bcdIDList[i][5] = 0x00;
	}
	setOSListFromNet(bcdIDList,TEST_OS_NUM_MAX);
#endif

#endif
	
	
}



