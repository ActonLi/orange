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

#ifndef _CFG_H_H_
#define _CFG_H_H_
#include "stdio.h"

typedef enum tagCFG_FILE_ACCESS_MODE
{
	CFG_ACCESS_READ = 0,  				//Ö»¶Á
   	CFG_ACCESS_WRITE,					//Ö»Ð´
    CFG_ACCESS_READ_WARITE,				//¶ÁÐ´
    CFG_ACCESS_APPEND_WRITE,			//×·¼ÓÖ»Ð´
    CFG_ACCESS_APPEND_READ_WRITE,		//×·¼Ó¶ÁÐ´
}CFG_FILE_ACCESS_MODE;

extern FILE * Cfg_Bin_Open(const char * ptrFileName, CFG_FILE_ACCESS_MODE eAccessMode);
extern int Cfg_Bin_Close(FILE * pFile);
extern int Cfg_Bin_Write(FILE * pFile,  int iLen,  void *ptrData);
extern int Cfg_Bin_Read(FILE * pFile,  int iOffset,  int iLen,  void *ptrData);
extern int Cfg_DelFile(const char *ptrFileName);

#endif

