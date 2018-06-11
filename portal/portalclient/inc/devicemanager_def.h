#ifndef DEVICEMANAGER_DEF_H
#define DEVICEMANAGER_DEF_H



#pragma pack(4)
typedef struct _DeviceManager_Tag
{
	BOOL bValid;            //是否有效数据
	char szDeviceBCDId[MAX_ID_SIZE];//bcd id
	char szDeviceStrId[16];//字符串ID
	char szDeviceIp[16]; //设备ip地址
	BOOL bExistCamera;	//是否存在副摄像头
}DEVICEMANAGER_ITEM;
#pragma pack()


#pragma pack(4)
typedef struct _DeviceManagerFileHead_tag
{
    UINT16 u16Counts; //设备数量
}DEVICEMANAGER_FILE_HEAD;
#pragma pack()




#endif

