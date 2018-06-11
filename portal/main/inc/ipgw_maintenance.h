#ifndef __IPGW_MAINTENANCE_H__
#define __IPGW_MAINTENANCE_H__
#include "network.h"

extern BOOLEAN ipgwSendLocalMacAddr(SOCK_DATA_PACKET_T *cmdPacket, BYTE *dataBuf, INT32 dataLen);
extern BOOLEAN ipgwConnectedDeviceSearch(SOCK_DATA_PACKET_T *cmdPacket, BYTE *dataBuf, INT32 dataLen);

#endif
