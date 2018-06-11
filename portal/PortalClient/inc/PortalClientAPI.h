#ifndef PORTAL_CLIENT_API_H_H__
#define PORTAL_CLIENT_API_H_H__

/**
 * @defgroup PortalClientTag PortalClient 
 * @ingroup PortalClientTag
 *
 *
 * @section module_description ģ�鹦�ܽ���
 *ʵ��Դ��λ��function/ipgw/PortalClient\n \n
 * 
 *
 */



/**
 * @ingroup PortalClientTag
 * @brief ��������: PortalClient Module ��ʼ������\n
 *������:ipgw \n
 *���ó���:  ipgw��������ʱ����\n
 *
 * @return �ޡ�
 * @note
 * 
*/
int PortalClientModuleInit(void);


/**
 * @ingroup PortalClientTag
 * @brief ��������: ��ȡPortal Client ��domain�ַ�������"ipgw123..."\n
 * ������:ipgw \n
 *
 * @return �ޡ�
 * @note  Ҫ��PortalClient ģ���ʼ���� (PortalClientModuleInit ����ִ�����)�����ܻ����ʵ��domain\n
 * 
*/
char * GetDeviceDomain(VOID);


#endif

