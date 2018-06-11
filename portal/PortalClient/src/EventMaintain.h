#ifndef __EVENT_MAINTAIN_H_H__
#define __EVENT_MAINTAIN_H_H__


/**
 * @defgroup EventListTag Event Maintain
 * @ingroup EventListTag
 *
 *
 * @section module_description 模块功能介绍
 * 维护 sessionID-MasterID的映射表
 * 维护未发送成功事件的链表
 *
 * @{
 */


#include "osa/osa.h"
#include "osa/osa_mem.h"


typedef struct tagEventSessionMasterIDMap
{
	BYTE pu8SessionID[32];	
	BYTE pu8MasterID[64];    //每个事件都有UUID，Ring -Answer-监视 时间的uuid 将作为MsterID， 其他事件belong to 该Master ID
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

