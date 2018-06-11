 /******************************************************************************
* Copyright 2010-2013 ABB Genway Co.,Ltd.
* FileName: 	 nat_maintainer.c 
* Desc:		使用iptables根据室内分机列表维护网络透传规则
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "network.h"
#include "osa/osa_debug.h"

#include "id_ip_maintainer.h"
#include "nat_maintainer.h"

//#define DBG_NAT_RULE_INDEX

//#define DEBUG_NAT_RULE
#ifdef DEBUG_NAT_RULE
#define	NAT_DBG(...) \
	do \
	{	\
		 fprintf(stdout,"\n"); \
		 fprintf(stdout, __VA_ARGS__); \
	} \
	while(0);
#else
#define NAT_DBG(...)
#endif


#define YELLOW_NET "10.0.0.0"
#define YELLOW_NET_LEN 8

#if defined(__Use__HB__miniOS__)

#define RULE_NUMBER_EACH_IS 	5

#else

#define RULE_NUMBER_EACH_IS 	4

#endif

/*
	gcList_rule_index is used to mapping rule number with list id, 
	the array index is the id, and the value of this element is the mapping nat rule number 
	the interval of element value is [1--LIST_MAX], 0 is used to indicate that there is no rule number 

	giCurrent_index is used to index the currenttly max rule number
*/
#define NAT_LIST_MAX (LIST_MAX+LIST_MAX)

char gcList_rule_index[NAT_LIST_MAX]; 	/*modified by Andy-wei.hou 2015.04.20, [0~31] for IS; [32~63] for 2ndOS*/
int giCurrent_index=0;




/******************************************************************************
* Name: 	 nat_init_prostrouting 
*
* Desc: 		init the prostrouting rule of ip-gateway, which is used to nat from inside to outside
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void nat_init_prostrouting(void){
	char shellstr[200] = { 0 };
	#if defined(__Use__HB__miniOS__)
	sprintf(shellstr,"iptables -t nat -A POSTROUTING -p tcp -d 10.0.0.0/8 -j MASQUERADE");
	do_system1(shellstr,0);
	sprintf(shellstr,"iptables -t nat -A POSTROUTING -p udp -d 10.0.0.0/8 --dport 8302 -j MASQUERADE");
	do_system1(shellstr,0);
	#else
	sprintf(shellstr,"iptables -t nat -A POSTROUTING -d 10.0.0.0/8 -j MASQUERADE");
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);
    #endif
}


void nat_show_LIST_RULE_INDEX(void){
#ifdef DBG_NAT_RULE_INDEX
	int i;
	for(i=0;i<NAT_LIST_MAX;i++){
		NAT_DBG("gcList_rule_index[%d]=%d",i,gcList_rule_index[i]);
	}
		
#endif
}



/******************************************************************************
* Name: 	 nat_adjust_list_rule_index 
*
* Desc: 		when delete a rule_number in gcList_rule_index, adjust all the affected rule_numbers
*			Accoring to iptables, when you delete a rule, all the rule number larger than this rule's rule number should minus 1
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void nat_adjust_list_rule_index(int rule_number){
	int i;
	for(i=0;i<NAT_LIST_MAX;i++){
		if(gcList_rule_index[i]>rule_number)
			gcList_rule_index[i]--;
	}


}


/******************************************************************************
* Name: 	 nat_add_rule_prerouting 
*
* Desc: 		add a new nat rule from outside to inside.
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
*		 2013.09.12, Modify this function by Andy-wei.hou. Remove the global variables changes to caller and add the protocol parameter
 ******************************************************************************/

void nat_add_rule_prerouting(/*int id,*/char *s_net,int s_net_len,int d_port, char *tag_addr,int tag_port,const char *proto){
	 char shellstr[200] = { 0 };
	 sprintf(shellstr,"iptables -t nat -A PREROUTING -i %s -s %s/%d -p %s  --dport %d -j DNAT --to %s:%d", \
	 		SystemDeviceInfo_GetCommunityPhyName(),s_net,s_net_len,proto,d_port,tag_addr,tag_port);
	 NAT_DBG(shellstr);
	 //do_system1(shellstr,0);
     system(shellstr);
/*	 
	 sprintf(shellstr,"iptables -t nat -A PREROUTING -i %s -s %s/%d -p udp  --dport %d -j DNAT --to %s:%d",  \
	 		UP_DEVICE,s_net,s_net_len,d_port,tag_addr,tag_port);
	 NAT_DBG(shellstr);
	 do_system1(shellstr,0);
	 giCurrent_index++;
	 gcList_rule_index[id] = giCurrent_index;	 
*/

}


/******************************************************************************
* Name: 	 nat_modify_rule_prerouting 
*
* Desc: 		instead of old rule by new parameters
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 *		  2013.09.12, Modify this function by Andy-wei.hou. 
 ******************************************************************************/

void nat_modify_rule_prerouting(int index,char *s_net,int s_net_len, int d_port, char *tag_addr,int tag_port,const char *proto){
	 char shellstr[200] = { 0 };
	 sprintf(shellstr,"iptables -t nat -R PREROUTING %d -i %s -s %s/%d -p %s  --dport %d -j DNAT --to %s:%d", \
	 		index,SystemDeviceInfo_GetCommunityPhyName(),s_net,s_net_len,proto,d_port,tag_addr,tag_port);
	 NAT_DBG(shellstr);
	 //do_system1(shellstr,0);
	 system(shellstr);
/*	 
	 sprintf(shellstr,"iptables -t nat -R PREROUTING %d -i %s -s %s/%d -p udp  --dport %d -j DNAT --to %s:%d",  \
	 		gcList_rule_index[id]*2,UP_DEVICE,s_net,s_net_len,d_port,tag_addr,tag_port);
	 NAT_DBG(shellstr);
	 do_system1(shellstr,0);
	*/ 
}


/******************************************************************************
* Name: 	 nat_del_rule_prerouting 
*
* Desc: 		delete the specificate rule 
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 *		  2013.09.12, Modify this function by Andy-wei.hou. Remove the global variables changes to caller and add the protocol parameter
 ******************************************************************************/

void nat_del_rule_prerouting(int index){
	 char shellstr[200] = { 0 };
//	 int rule_index= gcList_rule_index[id];
	 sprintf(shellstr,"iptables -t nat -D PREROUTING %d", index);
	 NAT_DBG(shellstr);
	 //do_system1(shellstr,0);
	 system(shellstr);
/*
	 //as the rule number automally minus 1 after delete one rule
	 sprintf(shellstr,"iptables -t nat -D PREROUTING %d", gcList_rule_index[id]*2-1);
	 NAT_DBG(shellstr);
	 do_system1(shellstr,0);
	 
	 gcList_rule_index[id]=0;
	 nat_adjust_list_rule_index(rule_index);
	 giCurrent_index--;
*/
	 
}


/******************************************************************************
* Name: 	 nat_rule_maintainer 
*
* Desc: 		维护由外向内透传规则
* Param: 	 
*			operation:	操作类型 	
*			id		:	室内分机的编号(户内编-1)
*			ip_addr	:	室内分机的实际IP
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void nat_rule_maintainer(char opeaation, int id, char *ip_ddr){
	
	
	switch (opeaation){
		case (ADD): { 
				nat_add_rule_prerouting( YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"tcp");
				nat_add_rule_prerouting( YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"udp");
				nat_add_rule_prerouting( YELLOW_NET,YELLOW_NET_LEN,RTSP_LISTEN_PORT+2*id,ip_ddr,RTSP_LISTEN_PORT+2*id,"udp");
				nat_add_rule_prerouting( YELLOW_NET,YELLOW_NET_LEN,RTCP_LISTEN_PORT+2*id,ip_ddr,RTCP_LISTEN_PORT+2*id,"udp");
                
				#if defined(__Use__HB__miniOS__)
				nat_add_rule_prerouting( YELLOW_NET,YELLOW_NET_LEN,kHbAudioPort+id*5,ip_ddr,kHbAudioPort,"udp");
				#endif
                
			 	giCurrent_index++;
		 		gcList_rule_index[id] = giCurrent_index;	
				break;
			}
		case (MODIFY): {
				#if defined(__Use__HB__miniOS__)
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-4, YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"tcp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-3, YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"udp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-2, YELLOW_NET,YELLOW_NET_LEN,RTSP_LISTEN_PORT+2*id,ip_ddr,RTSP_LISTEN_PORT+2*id,"udp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-1, YELLOW_NET,YELLOW_NET_LEN,RTCP_LISTEN_PORT+2*id,ip_ddr,RTCP_LISTEN_PORT+2*id,"udp");	
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-0, YELLOW_NET,YELLOW_NET_LEN,kHbAudioPort+id*5,ip_ddr,kHbAudioPort,"udp");
				#else
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-3, YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"tcp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-2, YELLOW_NET,YELLOW_NET_LEN,NET_CMD_PORT+id,ip_ddr,NET_CMD_PORT,"udp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-1, YELLOW_NET,YELLOW_NET_LEN,RTSP_LISTEN_PORT+2*id,ip_ddr,RTSP_LISTEN_PORT+2*id,"udp");
				nat_modify_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS  , YELLOW_NET,YELLOW_NET_LEN,RTCP_LISTEN_PORT+2*id,ip_ddr,RTCP_LISTEN_PORT+2*id,"udp");		
                #endif
                break;
			}
		case (DEL): {
				//as the rule number automally minus 1 after delete one rule
				if(gcList_rule_index[id] == 0)/*index <=0 is invalid parameter */
					break;
                
				nat_del_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-(RULE_NUMBER_EACH_IS - 1));
				nat_del_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-(RULE_NUMBER_EACH_IS - 1));
				nat_del_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-(RULE_NUMBER_EACH_IS - 1));
				nat_del_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-(RULE_NUMBER_EACH_IS - 1));

				#if defined(__Use__HB__miniOS__)
				nat_del_rule_prerouting(gcList_rule_index[id]*RULE_NUMBER_EACH_IS-(RULE_NUMBER_EACH_IS - 1));
				#endif
                
				nat_adjust_list_rule_index(gcList_rule_index[id]);
				gcList_rule_index[id]=0;
				giCurrent_index--;
				break;
			}
	};
	nat_show_LIST_RULE_INDEX();
	
}

/******************************************************************************
* Name: 	 nat_init_rule 
*
* Desc: 		初始化网络透传规则
*			1.	打开ip_forward
*			2.	清空所有透传规则	
*			3.	设置收到的多播数据报的TTL值为64(供mrouted转发使用)
*			4.	设置由内向外透传规则
*
* Param: 	 
* Return: 	
* Global: 	 
* Note: 	 
* Author: 	 Andy Hou
* -------------------------------------
* Log: 	 2013.03.25, Create this function by Andy Hou
 ******************************************************************************/

void nat_init_rule(void){
	char shellstr[200] = { 0 };
	int i=0;
	for(i=0; i < NAT_LIST_MAX; i++ ){
		gcList_rule_index[i] = 0;
	}
	giCurrent_index = 0;
	//enable ip_forward
	sprintf(shellstr,"echo 1 > /proc/sys/net/ipv4/ip_forward");
	//do_system1(shellstr,0);
    system(shellstr);
	NAT_DBG(shellstr);

	//IP conntrack timeout will be reset once a IP packet that matches the iptables received ,so it should not be that long(30s) .
	sprintf(shellstr,"echo 2 > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout");
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);

		
	//clear all the nat rules
	sprintf(shellstr,"iptables -t nat -F");
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);

	//clear all the nat rules
	sprintf(shellstr,"iptables -t mangle -F");
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);

	//clear all the nat rules
	sprintf(shellstr,"iptables -t filter -F");
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);
#if 1


	//set multicast ttl to 3
	sprintf(shellstr,"iptables -t mangle -A PREROUTING  -i %s -s 10.0.0.0/8 -d 238.0.0.0/8 -m pkttype  --pkt-type multicast -j TTL --ttl-set 3",SystemDeviceInfo_GetCommunityPhyName());
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);	

	//in order to avoid 239.0.0.1 go through the local network if the multicast packets sender set the ttl of 239.0.0.0/8 to more than 1  --andy 2013.06.27
	sprintf(shellstr,"iptables -t mangle -A PREROUTING	-i %s -s 10.0.0.0/8 -d 239.0.0.0/8 -m pkttype  --pkt-type multicast -j TTL --ttl-set 1",SystemDeviceInfo_GetCommunityPhyName());
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);	

	//set multicast ttl to 3
	sprintf(shellstr,"iptables -t mangle -A PREROUTING	-i %s -s 10.0.0.0/8 -d 237.0.0.0/8 -m pkttype  --pkt-type multicast -j TTL --ttl-set 3",SystemDeviceInfo_GetCommunityPhyName());
	//do_system1(shellstr,0);
	system(shellstr);
	NAT_DBG(shellstr);	
#endif


	nat_init_prostrouting();
	
	
}



