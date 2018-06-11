#ifndef DEVICEMANAGER_INTERFACE_H
#define DEVICEMANAGER_INTERFACE_H

#include "devicemanager_def.h"
#include "shares.h"

#define DEVICEMANAGER_OS_MAX_COUNT		64
#define DEVICEMANAGER_GS_MAX_COUNT		64

#define DEVICEMANAGER_MAX_COUNT         (DEVICEMANAGER_OS_MAX_COUNT + DEVICEMANAGER_GS_MAX_COUNT)


#define DEVICEMANAGER_SHAREMEMORY_FILE  PR_IS_DEVICEMANAGER
#define DEVICEMANAGER_SHAREMEMORY_MEMID kPR_IS_DEVICEMANAGER_MEMID
#define DEVICEMANAGER_SHAREMEMORY_SEMID kPR_IS_DEVICEMANAGER_SEMID


#pragma pack(4)
typedef struct _DeviceItem_Tag
{
	char szDeviceStrId[12+1];//�豸ID
	char szDeviceIp[16]; //�豸ip��ַ
}DEVICEITEM;
#pragma pack()

#pragma pack(4)
typedef struct _DeviceManagerList_Tag{
    UINT16 u16Counts; //�豸����
	DEVICEITEM stItem[DEVICEMANAGER_OS_MAX_COUNT]; 
}DEVICEMANAGER_LIST;
#pragma pack()


extern int DeviceManager_CreateInit(void);
extern int DeviceManager_CreateDeInit(void);
extern int DeviceManager_Init(void);
extern int DeviceManager_DeInit(void);
extern int DeviceManager_DisplayAllShareMemory(void);
extern int DeviceManager_AddNewItem(int devType, char *pszDeviceBCDId, char*pszDeviceStrId, char camera);
extern int DeviceManager_AddNewItemEx(int devType, char *list, char *camera, int num);
extern int DeviceManager_AddNewItemEx2(int devType, char *list, char *camera, int num);
extern int DeviceManager_GetIPByDeviceStrId(const char* pszDeviceStrId, char *pszDeviceIP);
extern int DeviceManager_GetDeviceListByDeviceType(BYTE byDeviceType, DEVICEMANAGER_LIST *pDeviceList);
extern int DeviceManager_ClearList(int devType);


#endif

