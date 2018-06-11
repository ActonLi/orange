/******************************************************************************
* Copyright 2011-2012 ABB Genway Co.,Ltd.
*
* FileName:   SmartHomeAPI.h
*
* Desc:       EIB模块与MAIN函数之间的接口

* 
* Author:     Shelly
*
* Date:       2012/02/13
* 
* -----------------------------------------------------------------
* Histroy: v1.0  2012/02/13, Shelly create this file
* 
******************************************************************************/
#ifndef _EIBAPI_H_H_
#define _EIBAPI_H_H_

/*----------------------------------Includes------------------------*/


/*--------------------------Typedef--------------------------*/

/*----------------------------------Global Funcs------------------------*/

extern VOID   InitSmartHomeModule(VOID);

extern INT32  IsSearchIPSSucessful(VOID);
extern INT32  IsConnectToIPSSucessful(VOID);


#endif

