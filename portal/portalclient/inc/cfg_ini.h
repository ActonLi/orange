/******************************************************************************
* Copyright 2010-2015 ABB Genway Co.,Ltd.
* FileName: 	 etccfg.h 
* Desc:   cfg�����ļ���дͷ�ļ�
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
    @brief  ��ʼ��ini�ļ����ݱ���ľ���ڴ�\n
            �����ɹ�����0\n
            ����exit�˳�����
            �˺��������ڼ��ػ򴴽�ini�ļ�֮ǰ���ã�����֮���ini�ļ�����д�����쳣

    @param[in] count    ��Ҫcache���ڴ��е�ini�ļ�����
    @return             0
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
int IniGlobalInit(int count);



/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  �����ƶ���ini���ݾ�����ݵ��ƶ���ini�ļ���д�����\n

    @param[in] pEtcFile    ini�ļ��ľ���·��
    @return                ��
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
int SaveIniData(const char *pEtcFile);


/////////////////////////////////////////////////////////////////////////////////
/*!
    @brief  ����ini�ļ������ڴ���\n

    @param[in] pEtcFile    ini�ļ��ľ���·��
    @return                ��
    
    @author  liqun.wei@cn.abb.com
    @version 1.0
    @date    2015-10-21
*/
/////////////////////////////////////////////////////////////////////////////////
void LoadIniData(const char *pEtcFile);

#endif
