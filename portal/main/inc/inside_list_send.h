#ifndef __INSIDE_LIST_SEND_H
#define __INSIDE_LIST_SEND_H

//operation code for send_list_process
typedef enum listSend_tag{
	eSEND_IS_LIST = 1,
	eSEND_2ndOS_LIST= 2,
	eSEND_IPA_LIST= 3,
}LIST_SEND_OPE_TYPE;


extern VOID InitSendListProcess(VOID);
extern VOID DeInitSendListProcess(VOID);
extern void set_list_changed(LIST_SEND_OPE_TYPE type);



#endif 
