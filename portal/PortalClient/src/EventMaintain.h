#ifndef __EVENT_MAINTAIN_H_H__
#define __EVENT_MAINTAIN_H_H__


/**
 * @defgroup EventListTag Event Maintain
 * @ingroup EventListTag
 *
 *
 * @section module_description ģ�鹦�ܽ���
 * ά�� sessionID-MasterID��ӳ���
 * ά��δ���ͳɹ��¼�������
 *
 * @{
 */


#include "osa/osa.h"
#include "osa/osa_mem.h"


typedef struct tagEventSessionMasterIDMap
{
	BYTE pu8SessionID[32];	
	BYTE pu8MasterID[64];    //ÿ���¼�����UUID��Ring -Answer-���� ʱ���uuid ����ΪMsterID�� �����¼�belong to ��Master ID
}ST_EVENT_SESSION_MASTERID_MAP;

typedef struct tagEventSessionMasterIDMapList
{
	ST_EVENT_SESSION_MASTERID_MAP  node;
    struct tagEventSessionMasterIDMapList * prev;
	struct tagEventSessionMasterIDMapList * next;
}EVENT_SESSION_MASTERID_MAP_LIST;	

EVENT_SESSION_MASTERID_MAP_LIST * fnAddNewSessionMasterIDMapNode(BYTE * pu8SessionID, BYTE * pu8MasterID);
EVENT_SESSION_MASTERID_MAP_LIST * fnUpdateNewSessionMasterIDMapNode(BYTE * pu8SessionID, BYTE * pu8MasterID);
EVENT_SESSION_MASTERID_MAP_LIST * fnFindSessionMasterIDMapNode(BYTE * pu8SessionID);
VOID fnDeleteSessionMasterIDMapNode(BYTE * pu8SessionID);

#endif

