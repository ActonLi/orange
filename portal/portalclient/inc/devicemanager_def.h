#ifndef DEVICEMANAGER_DEF_H
#define DEVICEMANAGER_DEF_H



#pragma pack(4)
typedef struct _DeviceManager_Tag
{
	BOOL bValid;            //�Ƿ���Ч����
	char szDeviceBCDId[MAX_ID_SIZE];//bcd id
	char szDeviceStrId[16];//�ַ���ID
	char szDeviceIp[16]; //�豸ip��ַ
	BOOL bExistCamera;	//�Ƿ���ڸ�����ͷ
}DEVICEMANAGER_ITEM;
#pragma pack()


#pragma pack(4)
typedef struct _DeviceManagerFileHead_tag
{
    UINT16 u16Counts; //�豸����
}DEVICEMANAGER_FILE_HEAD;
#pragma pack()




#endif

