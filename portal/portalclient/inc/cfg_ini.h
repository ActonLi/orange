/******************************************************************************
* Copyright 2010-2015 ABB Genway Co.,Ltd.
* FileName: 	 etccfg.h 
* Desc:   cfg配置文件读写头文件
* * Author: 	 Tom-hongtao.gao
* Date: 	 2015/03/04

* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/04, Tom-hongtao.gao create this file
* 
******************************************************************************/
#ifndef __ECTCFG_H__
#define __ETCCFG_H__


/*-------------------------------- Includes ----------------------------------*/
  




/*-------------------- Global Definitions -------------------*/



/*-------------------- Global Typedefs ---------------------*/



/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/
int GetBoolValueFromEtcFile (const char* pEtcFile, const char* pSection,
                                      const char* pKey, int* iValue);

int GetIntValueFromEtcFile (const char* pEtcFile, const char* pSection,
                                    const char* pKey, int* iValue);

int GetValueFromEtcFile( const char* pEtcFile, const char* pSection, 
                                 const char* pKey, char* pValue, int iLen);
                                   
int SetValueToEtcFile (const char* pEtcFile, const char* pSection,
                              const char* pKey, char* pValue);
int SetIntValueToEtcFile(const char* pEtcFile, const char* pSection,
                               const char* pKey, int iValue);
int GetFloatValueFromEtcFile (const char* pEtcFile, const char* pSection, const char* pKey, float* fValue);
int SetFloatValueToEtcFile(const char* pEtcFile, const char* pSection, const char* pKey, float fValue);


/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  初始化ini文件内容保存的句柄内存\n
            操作成功返回0\n
            否则，exit退出进程
            此函数必须在加载或创建ini文件之前调用，否则之后的ini文件读和写都会异常

    @param[in] count    需要cache到内存中的ini文件数量
    @return             0
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
int IniGlobalInit(int count);



/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  保存制定的ini数据句柄内容到制定的ini文件，写入磁盘\n

    @param[in] pEtcFile    ini文件的绝对路径
    @return                无
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
int SaveIniData(const char *pEtcFile);


/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  加载ini文件内容内存句柄\n

    @param[in] pEtcFile    ini文件的绝对路径
    @return                无
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
void LoadIniData(const char *pEtcFile);

#endif
