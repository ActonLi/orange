/* Copyright 2010-2017 ABB Genway Co.,Ltd.
* FileName: 	 process_interface.h 
* Desc:  逻辑进程间接口命令头文件
* * Author: 	 Lewis-weixin.liu
* Date: 	 2017/08/26

* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2017/02/26, Lewis-weixin.liu create this file
* 
******************************************************************************/
#ifndef __PROCESS_INTERFACE_H__
#define __PROCESS_INTERFACE_H__


enum Process_FuncCode
{
	PROCESS_FUNC_LOGIC =  0x15,
	PROCESS_FUNC_IPGW =  0x16,
};




//logic进程
enum _tag_logic_opercode
{
	PROCESS_OPER_LOGIC_REQ_SYSTEM_STATUS          = 0x01,
	PROCESS_OPER_LOGIC_CLOSE_CURRENT_FUNC         = 0x02, 
	PROCESS_OPER_LOGIC_UPDATE_CONTACT_DB		  = 0x03, 
};


//ipgw进程
enum _tag_ipgw_opercode
{
	PROCESS_OPER_IPGW_IP_CHANGED = 0x01,
};


#endif
