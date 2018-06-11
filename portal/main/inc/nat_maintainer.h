/******************************************************************************
* Copyright 2010-2012 ABB Genway Co.,Ltd.
* FileName: 	 nat_mainter.h
* Desc:
* YellowRiver IP-Gateway, �������ڷֻ��б�ά��͸������
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


#ifndef __NAT_MAINTAINER_H__
#define __NAT_MAINTAINER_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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

enum listOperTag{
	NEW = 1,
	MODIFY,
	DEL,
	ADD,
	INFO
};



extern void nat_init_rule(void);
extern void nat_rule_maintainer(char opeaation, int id, char *ip_ddr);




#endif
