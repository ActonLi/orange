/******************************************************************************
* Copyright 2010-2012 ABB Genway Co.,Ltd.
* FileName: 	 
* Desc:
* YellowRiver IP-Gateway, �������ڶ����ַֻ�ID_IP�б�ά��
*�б�Ļ�ȡ�Ǵ�SIPע����Ϣ��display_name��contact_IP����ȡ
* 
* Author: 	Andy Hou
* Date: 	 	2013.03.25
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2013.03.25, Andy Hou create this file
*         
* 
******************************************************************************/


#ifndef __ID_IP_MAINTAINER_H__
#define __ID_IP_MAINTAINER_H__


#include <netinet/in.h>
#include <arpa/inet.h>
#include <network.h>



#define DOWN_DEVICE INTERFACE_INTERNAL
#define UP_DEVICE	"eth0"
#define LIST_MAX 	32		





extern void start_id_ip_maintainer(VOID);
extern int  get_ID_IP_list(char * pData_buf);
extern int get_list_num(void);


extern void inside_alarm_deal_net_msg(s_PacketMsg *interMsg, BYTE *data,UINT32 len);
extern int transfer_packet_to_inner_network(char* IPAddress,int port,char* pPacket,int iPacket_len);





#endif
