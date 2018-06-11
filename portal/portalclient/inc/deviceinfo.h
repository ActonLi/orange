/******************************************************************************
* Copyright 2010-2017 ABB Genway Co.,Ltd.
* FileName:  Deviceinfo.h
* Desc: 设备信息数据的存储和获取
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
 * @brief 该模块主要提供系统设备信息相关的接口
 *
 *
 * @section module_description 模块功能介绍
 * 该模块提供跟设备信息相关的数据获取，Logic进程创建和维护一块共享内存，数据
 * 内容包括本地ID/本地IP/分机列表/门口机列表等数据，其他进程通过该模块接口获取
 * 数据。
 * @section module_prerequisite 模块的适用范围，适用场景，依赖条件
 * 该模块适用于室内机
 * 
 */

#ifndef ___SystemDeviceInfo__H__
#define ___SystemDeviceInfo__H__

/* @{
*/



/**
 * @brief 创建一块共享内存保存本地ID/IP / 各种设备列表
 * 
 * @section module_description 模块功能介绍
 * 这块内存应该由Logic进程创建，并写入数据，其他进程只能获取数据
 *@return 无
*/
extern void SystemDeviceInfo_Create(void);
/**
 * @brief 获取共享内存，如果没有创建则会阻塞，直到Logic进程创建了共享内存
 *
 *@return 无
*/
extern void SystemDeviceInfo_Init(void);
/**
 * @brief 设置本机BCD 码的设备ID 到sharememory
 * @param num  6个字节的BCD码
 *@return 无
*/
extern void SystemDeviceInfo_SetLocalBCDID(char *num);
/**
 * @brief  从sharememory中获取本机BCD码设备ID
 * @param 无
 *@return   BCD码的字节数组地址，有效数据6 bytes
*/

extern unsigned char* SystemDeviceInfo_GetLocalBCDID(void);


/**
 * @brief 设置 本机 字符串设备ID到Share memory
 * @param num  6个字节的BCD码
 *@return 无
*/
extern void SystemDeviceInfo_SetLocalStrID(char *num);
/**
 * @brief  获取本机字符串设备ID
 * @param 无
 *@return 字符串设备ID的字符串指针，12 字节
*/
extern char *SystemDeviceInfo_GetLocalStrID(void);

/**
 * @brief  设置本机ETH0 网卡IP
 * @param ipaddress IP地址 字符串
 *@return 无
*/

extern void SystemDeviceInfo_SetEth0IPAddress(char *ipaddress);

/**
 * @brief  获取本机ETH0 网卡IP
 * @param 无
 *@return IP地址 字符串
*/

extern char * SystemDeviceInfo_GetEth0IPAddress(void);

/**
 * @brief 设置本机ETH1 网卡IP
 * @param ipaddress IP地址 字符串
 *@return 无
*/

extern void SystemDeviceInfo_SetEth1IPAddress(char *ipaddress);

/**
 * @brief 获取本机ETH1 网卡IP
 * @param 无
 *@return   IP地址 字符串
*/

extern char * SystemDeviceInfo_GetEth1IPAddress(void);


/**
 * @brief 设置sip server ip地址
 * @param ipaddress IP地址 字符串
 *@return 无
*/

extern void SystemDeviceInfo_SetSipServerAddress(char *ipaddress);

/**
 * @brief 获取sip server ip地址
 * @param 无
 *@return   IP地址 字符串
*/

extern char * SystemDeviceInfo_GetSipServeAddress(void);

/**
 *@brief 设置手柄状态
 *@return 无
*/
extern void SystemDeviceInfo_SetHandShankStatus(int status);

/**
 * @brief 获取手柄状态
 * @param 无
 *@return 
*/
extern int SystemDeviceInfo_GetHandShankStatus(void);


/** @} */

#endif
