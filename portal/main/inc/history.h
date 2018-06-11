/******************************************************************************
* Copyright 2010-2012 ABB Genway Co.,Ltd.
* FileName: 	 history.c 
* Desc:    
* 
* 
* Author: 	 daniel-qinghua.huang 
* Date: 	 2013/03/26
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2013/03/26, daniel-qinghua.huang create this file
* 
******************************************************************************/ 
#ifndef __HISTORY_H__
#define __HISTORY_H__

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/
//extern void ad__history(int os_no, int call_type);
//extern void sv__history(int idx, char *name, int len, char *dat);
//extern void rd__history(void);
//extern void de__history(int history_Id);
//extern int Snap_HistoryFun(void);
extern VOID InitTalkHistoryProcess(VOID);
extern VOID DeInitTalkHistoryProcess(VOID);
extern VOID DeleteTalkHistoryFun(int index);		/*used for web to delete call history */
extern INT32 dealRcvHistoryRecords(s_PacketMsg *interMsg,BYTE *dataBuf);

#endif /*end of __HISTORY_H__*/



