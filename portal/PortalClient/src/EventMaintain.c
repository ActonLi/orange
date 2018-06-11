#include "osa/osa.h"
#include "osa/osa_mem.h"
#include "osa/osa_debug.h"
#include "osa/osa_thr.h"
#include "osa/osa_mutex.h"
#include "osa/osa_sharemem.h"
#include "osa/osa_time.h"
#include "osa/osa_dir.h"
#include "osa/osa_file.h"
#include "PortalClient.h"

LOCAL EVENT_SESSION_MASTERID_MAP_LIST * s_EventSessionMasterIDMapList_Head = NULL;
LOCAL EVENT_SESSION_MASTERID_MAP_LIST * s_EventSessionMasterIDMapList_End = NULL;

LOCAL VOID DisplayAllEventSessionMasterIDMapList(VOID)
{
	OSA_DBG_MSG("\n\n%s****************All Session MasterID Map Store*************************",DEBUG_HEADER_PORTALCLIENT);
	EVENT_SESSION_MASTERID_MAP_LIST * cur = s_EventSessionMasterIDMapList_Head;
	INT i = 0;
	  
	while ( cur )
	{
		printf("[SessionID:");
		for(i = 0; i < 32; i++)
		{
			printf("0x%02x ",cur->node.pu8SessionID[i]);
		}
		printf("]\n");

		printf("[MasterID:");
		for(i = 0; i < 64; i++)
		{
			printf("0x%02x ",cur->node.pu8MasterID[i]);
		}
		printf("]\n\n");
		
		cur = cur->next;
	}
	OSA_DBG_MSG("*****************************************************************\n\n");

}

EVENT_SESSION_MASTERID_MAP_LIST * fnAddNewSessionMasterIDMapNode(BYTE * pu8SessionID, BYTE * pu8MasterID)
{
	if(fnFindSessionMasterIDMapNode(pu8SessionID) != NULL)
	{
		printf("\n%sSessionID is already in List\n",DEBUG_HEADER_PORTALCLIENT);
		return NULL;
	}
	printf("\n%sAdd New Session MasterID Map in List\n",DEBUG_HEADER_PORTALCLIENT);
	
    EVENT_SESSION_MASTERID_MAP_LIST * pNewNode = NULL;
        
    pNewNode = (EVENT_SESSION_MASTERID_MAP_LIST *)OSA_MemMalloc(sizeof(EVENT_SESSION_MASTERID_MAP_LIST));
    NOT_MEM_PRINT(pNewNode);     
    if ( pNewNode == NULL )
    {
        return NULL;
    }
	OSA_MemCopy(pNewNode->node.pu8SessionID,pu8SessionID,sizeof(pNewNode->node.pu8SessionID));
	OSA_MemCopy(pNewNode->node.pu8MasterID,pu8MasterID,sizeof(pNewNode->node.pu8MasterID));
	
	pNewNode->prev = NULL;
	pNewNode->next = NULL;

	if( s_EventSessionMasterIDMapList_Head == NULL )
 	{
    	s_EventSessionMasterIDMapList_Head = pNewNode;
     	s_EventSessionMasterIDMapList_End = pNewNode;
	}
	else
 	{
     	pNewNode->prev = s_EventSessionMasterIDMapList_End;
        s_EventSessionMasterIDMapList_End->next = pNewNode;
        s_EventSessionMasterIDMapList_End = pNewNode;
 	}

	DisplayAllEventSessionMasterIDMapList();
  	return pNewNode ;
}


EVENT_SESSION_MASTERID_MAP_LIST * fnUpdateNewSessionMasterIDMapNode(BYTE * pu8SessionID, BYTE * pu8MasterID)
{
	EVENT_SESSION_MASTERID_MAP_LIST * pNewNode = NULL;
        
    pNewNode = fnFindSessionMasterIDMapNode(pu8SessionID);

	if(pNewNode == NULL)
	{
		printf("\n%sUpdate SessionID Master Map, But SessionID is not in List\n",DEBUG_HEADER_PORTALCLIENT);
		return NULL;
	}
	printf("\n%sUpdate Session MasterID Map in List\n",DEBUG_HEADER_PORTALCLIENT);
	
	OSA_MemCopy(pNewNode->node.pu8MasterID,pu8MasterID,sizeof(pNewNode->node.pu8MasterID));
	DisplayAllEventSessionMasterIDMapList();
  	return pNewNode ;
}


EVENT_SESSION_MASTERID_MAP_LIST * fnFindSessionMasterIDMapNode(BYTE * pu8SessionID)
{
  	EVENT_SESSION_MASTERID_MAP_LIST * cur = s_EventSessionMasterIDMapList_Head;
	INT i = 0;
	  
	while ( cur )
	{
		for( i = 0; i < 32; i++ )
		{
			if (pu8SessionID[i] != cur->node.pu8SessionID[i] )
			{
				cur = cur->next;
				break;
			}
		}

		if(i == 32)
		{
			return cur;
		}
	}
			
	return NULL;	  
}


VOID fnDeleteSessionMasterIDMapNode(BYTE * pu8SessionID)
{	  
	EVENT_SESSION_MASTERID_MAP_LIST * cur = s_EventSessionMasterIDMapList_Head;
	INT i = 0;
		  
	while ( cur )
	{
		for( i = 0; i < 32; i++ )
		{
			if (pu8SessionID[i] != cur->node.pu8SessionID[i] )
			{
				cur = cur->next;
				break;
			}
		}

		if(i == 32)  //找到要删除的结点
		{
			if( (cur->prev == NULL) && (cur->next == NULL) )	//only one device
			{
				s_EventSessionMasterIDMapList_Head = NULL;
				s_EventSessionMasterIDMapList_End = NULL;
			}
			else if(cur->prev == NULL)				//head
			{
				s_EventSessionMasterIDMapList_Head = cur->next;
				cur->next->prev = NULL;
			}
			else if(cur->next == NULL) 				//end
			{
				s_EventSessionMasterIDMapList_End = cur->prev;
				cur->prev->next = NULL;
			}
			else							  		//middle
			{
				cur->prev->next = cur->next;
				cur->next->prev = cur->prev;
			}
				  
			SAFE_DELETE_MEM(cur);
			cur = NULL;
			return;
		}
	}
	DisplayAllEventSessionMasterIDMapList();
}



