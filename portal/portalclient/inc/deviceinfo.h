/******************************************************************************
* Copyright 2010-2017 ABB Genway Co.,Ltd.
* FileName:  Deviceinfo.h
* Desc: �豸��Ϣ���ݵĴ洢�ͻ�ȡ
*
*
* Author: Lewis
* Date: 2017-06-19
* Notes:
*
* -----------------------------------------------------------------
* Histroy: 2017-06-19 first version
*
******************************************************************************/

/**
 * @addtogroup module_name Device_Info_API
 * @brief ��ģ����Ҫ�ṩϵͳ�豸��Ϣ��صĽӿ�
 *
 *
 * @section module_description ģ�鹦�ܽ���
 * ��ģ���ṩ���豸��Ϣ��ص����ݻ�ȡ��Logic���̴�����ά��һ�鹲���ڴ棬����
 * ���ݰ�������ID/����IP/�ֻ��б�/�ſڻ��б�����ݣ���������ͨ����ģ��ӿڻ�ȡ
 * ���ݡ�
 * @section module_prerequisite ģ������÷�Χ�����ó�������������
 * ��ģ�����������ڻ�
 * 
 */

#ifndef ___SystemDeviceInfo__H__
#define ___SystemDeviceInfo__H__

/* @{
*/



/**
 * @brief ����һ�鹲���ڴ汣�汾��ID/IP / �����豸�б�
 * 
 * @section module_description ģ�鹦�ܽ���
 * ����ڴ�Ӧ����Logic���̴�������д�����ݣ���������ֻ�ܻ�ȡ����
 *@return ��
*/
extern void SystemDeviceInfo_Create(void);
/**
 * @brief ��ȡ�����ڴ棬���û�д������������ֱ��Logic���̴����˹����ڴ�
 *
 *@return ��
*/
extern void SystemDeviceInfo_Init(void);
/**
 * @brief ���ñ���BCD ����豸ID ��sharememory
 * @param num  6���ֽڵ�BCD��
 *@return ��
*/
extern void SystemDeviceInfo_SetLocalBCDID(char *num);
/**
 * @brief  ��sharememory�л�ȡ����BCD���豸ID
 * @param ��
 *@return   BCD����ֽ������ַ����Ч����6 bytes
*/

extern unsigned char* SystemDeviceInfo_GetLocalBCDID(void);


/**
 * @brief ���� ���� �ַ����豸ID��Share memory
 * @param num  6���ֽڵ�BCD��
 *@return ��
*/
extern void SystemDeviceInfo_SetLocalStrID(char *num);
/**
 * @brief  ��ȡ�����ַ����豸ID
 * @param ��
 *@return �ַ����豸ID���ַ���ָ�룬12 �ֽ�
*/
extern char *SystemDeviceInfo_GetLocalStrID(void);

/**
 * @brief  ���ñ���ETH0 ����IP
 * @param ipaddress IP��ַ �ַ���
 *@return ��
*/

extern void SystemDeviceInfo_SetEth0IPAddress(char *ipaddress);

/**
 * @brief  ��ȡ����ETH0 ����IP
 * @param ��
 *@return IP��ַ �ַ���
*/

extern char * SystemDeviceInfo_GetEth0IPAddress(void);

/**
 * @brief ���ñ���ETH1 ����IP
 * @param ipaddress IP��ַ �ַ���
 *@return ��
*/

extern void SystemDeviceInfo_SetEth1IPAddress(char *ipaddress);

/**
 * @brief ��ȡ����ETH1 ����IP
 * @param ��
 *@return   IP��ַ �ַ���
*/

extern char * SystemDeviceInfo_GetEth1IPAddress(void);


/**
 * @brief ����sip server ip��ַ
 * @param ipaddress IP��ַ �ַ���
 *@return ��
*/

extern void SystemDeviceInfo_SetSipServerAddress(char *ipaddress);

/**
 * @brief ��ȡsip server ip��ַ
 * @param ��
 *@return   IP��ַ �ַ���
*/

extern char * SystemDeviceInfo_GetSipServeAddress(void);

/**
 *@brief �����ֱ�״̬
 *@return ��
*/
extern void SystemDeviceInfo_SetHandShankStatus(int status);

/**
 * @brief ��ȡ�ֱ�״̬
 * @param ��
 *@return 
*/
extern int SystemDeviceInfo_GetHandShankStatus(void);


/** @} */

#endif
