/******************************************************************************
* Copyright 2016-2017 ABB Genway Co.,Ltd.
* FileName: 	 cfg.h 
* Desc:
* 
* 
* Author:	 Shelly-Yunqing.Ye
* Date:  	 2016/03/30
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2016/03/30, Shelly-Yunqing.Ye create this file
* 
******************************************************************************/

#ifndef __SHARE__MEMS__H__
#define __SHARE__MEMS__H__

#include "osa.h"

#define PR_IS_SETTINGS      "./usr/app/ShareMemFile/13PR_IS_SETTINGS"
#define PR_IS_DEVICEINFO    "./usr/app/ShareMemFile/13PR_IS_DEVICEINFO"

#define PR_IS_CALLHISTORY   "./usr/app/ShareMemFile/13PR_IS_CALLHISTORY"
#define PR_IS_INDOORLIST    "./usr/app/ShareMemFile/13PR_IS_INDOORLIST"
#define PR_IS_MONITORLIST   "./usr/app/ShareMemFile/13PR_IS_MONITORLIST"
#define PR_IS_IPCHOMELIST   "./usr/app/ShareMemFile/13PR_IS_IPCHOMELIST"
#define PR_IS_IPCCOMMLIST   "./usr/app/ShareMemFile/13PR_IS_IPCCOMMLIST"
#define PR_IS_VOICEMSG   	"./usr/app/ShareMemFile/13PR_IS_VOICEMSG"
#define PR_IS_DEVICEMANAGER	"./usr/app/ShareMemFile/13PR_IS_DEVICEMANAGER"
#define PR_IS_GUALARM		"./usr/app/ShareMemFile/13PR_IS_GUALARM"



#define kPR_IS_SETTINGS_SEMID      13
#define kPR_IS_SETTINGS_MEMID     14

#define kPR_IS_SYSTEM_SETTINGS_SEMID      15
#define kPR_IS_SYSTEM_SETTINGS_MEMID      16


#define kPR_IS_CALLHISTORY_MEMID   17
#define kPR_IS_CALLHISTORY_SEMID   18


#define kPR_IS_DEVICE_INFO_SEMID      19
#define kPR_IS_DEVICE_INFO_MEMID      20

#define kPR_IS_IPC_MEMID 21
#define kPR_IS_IPC_SEMID 22

#define kPR_IS_VOICEMSG_MEMID 23
#define kPR_IS_VOICEMSG_SEMID 24

#define kPR_IS_DEVICEMANAGER_MEMID 25
#define kPR_IS_DEVICEMANAGER_SEMID 26

#define kPR_IS_GUALARM_MEMID 27
#define kPR_IS_GUALARM_SEMID 28



typedef struct DEVICE_ADDRESS_TAG
{
     /* �豸����*/
    INT32   devicetype;        //�豸���� DEVICE_INDOOR        
    CHAR  unit_num[8];         //��Ԫ��
    CHAR  room_num[8];         //�����
    CHAR  inside_num[8];       //���ں�
	
}DEVICE_ADDRESS;

typedef struct ENGINEER_SETTING_SWITCHES_TAG
{
	INT32 liftSwitch;
	INT32 secondLockSwitch;
	INT32 haSwitch;
	INT32 desSwitch;
	INT32 alSwitch;
	INT32 cctvSwitch;
	INT32 alarmSwitch;
	INT32 autoUnlockSwitch;
	INT32 callGuardUniSwitch;
	
}ENGINEER_SETTING_SWITCHES;


typedef struct ENGINEER_SETTINGS_TAG
{
	DEVICE_ADDRESS local_Address;
	ENGINEER_SETTING_SWITCHES masterSwitch;
	INT32 defaultGuardUnitAddress;
	INT32 callMode;        //0-logical address  1-physical address
	INT32 deviceMode;      //0-master   1-slave
	INT32 domesticNetPort;   //0-LAN1   1-LAN2   2-wifi
	INT32 defaultDesktop;  //0-H/A    1-DES     2-CCTV
	CHAR systemId[4];   
	CHAR alias[32];
	

    /*��ַ����*/
    INT32  ipGateway_enable;    //����IP-Gateway����
    INT32  ipGateway_connect;   //����IP-Gateway������
    INT32  ipGateway_position;   //IP-Gatewayλ�� 0-�ⲿ 1-�ڲ�
    CHAR  sipserver[16];       //sipserver IP  ���ֺ͵���ɵ��ַ���
    INT32   dhcp;                // 1=DHCP, 0=StaticIP
    CHAR  ipaddress[16];       //local IP
    CHAR  mask[16];            //mask
    CHAR  gateway[16];         //default gateway
    CHAR  dns[16];             //DNS   
    CHAR  host[256];           //����IP/������

    /*���������*/
    INT32 guardunit_type;  //0:PC�������1:�绰�����
    INT32 guardunit;

    /*��������*/
    INT32 community_enable;    //������������
    CHAR url_3rd[256 + 1]; // third party digital URL
    INT32 sos_enable;          //sos����
    INT32 lift_enable;         //���ݿ���
    INT32 alarmzone;          //��������8/23/32

    /*���뵼��*/
    INT32 basic;        //��������
    INT32 alarm;        //����ϵͳ����
    INT32 smarthome;    //���ܼҾ�����

    BYTE deviceID[6]; // 750001010101 hex
    CHAR ip_eth0[32]; // 10.0.65.0 str
    CHAR ip_eth1[32]; // 192.168.1.200 str

	/*system mode*/
	INT32 iSystemMode;     //0- Compatibility mode   1-safemode
    
}ENGINEER_SETTINGS_PARAM;


#endif




