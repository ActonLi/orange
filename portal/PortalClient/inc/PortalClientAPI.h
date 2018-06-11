#ifndef PORTAL_CLIENT_API_H_H__
#define PORTAL_CLIENT_API_H_H__

/**
 * @defgroup PortalClientTag PortalClient 
 * @ingroup PortalClientTag
 *
 *
 * @section module_description 模块功能介绍
 *实现源码位于function/ipgw/PortalClient\n \n
 * 
 *
 */



/**
 * @ingroup PortalClientTag
 * @brief 函数功能: PortalClient Module 初始化函数\n
 *调用者:ipgw \n
 *调用场景:  ipgw进程启动时调用\n
 *
 * @return 无。
 * @note
 * 
*/
int PortalClientModuleInit(void);


/**
 * @ingroup PortalClientTag
 * @brief 函数功能: 获取Portal Client 的domain字符串，如"ipgw123..."\n
 * 调用者:ipgw \n
 *
 * @return 无。
 * @note  要等PortalClient 模块初始化后 (PortalClientModuleInit 函数执行完成)，才能获得真实的domain\n
 * 
*/
char * GetDeviceDomain(VOID);


#endif

