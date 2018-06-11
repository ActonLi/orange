#include "osa/osa_file.h"
#include "network.h"
#include "ipgw_maintenance.h"
#include "osa/osa_debug.h"
#include "osa/osa_mem.h"


#define __MACADDR__KEY__FILE__NAME__ "./.__mac_addr"
LOCAL int ReadMacAddrReportKey(BYTE dataBuf[], INT32 dataLen)
{
    E_FILE_OPEN_MODE eOpenMode = MODE_OPEN_EXISTING;
    FILE_HANDLE hFile = INVALID_FILE_HANDLE;
    
    if(!OSA_FileIsExist(__MACADDR__KEY__FILE__NAME__))
    {
        eOpenMode = MODE_CREATE_ALWAYS;
    }
    
    hFile = OSA_FileOpen(__MACADDR__KEY__FILE__NAME__, eOpenMode, ACCESS_READ);
    if(INVALID_FILE_HANDLE == hFile)
    {
        OSA_ERROR("Unexpect error ... ");
        return 1;
    }
    
    //¶ÁÎÄ¼þÄÚÈÝ
    DWORD dwRet = OSA_FileRead(hFile, (BYTE *)dataBuf, dataLen);
    if(0 == dwRet)
    {
        OSA_ERROR("Get %s file content is NULL", __MACADDR__KEY__FILE__NAME__);
        return 2;
    }
    
    OSA_FileClose(hFile);
    return 0;
}

LOCAL int WriteMacAddrReportKey(BYTE dataBuf[], INT32 dataLen)
{
    E_FILE_OPEN_MODE eOpenMode = MODE_CREATE_ALWAYS;
    FILE_HANDLE hFile = INVALID_FILE_HANDLE;
    
    hFile = OSA_FileOpen(__MACADDR__KEY__FILE__NAME__, eOpenMode, ACCESS_WRITE);
    if(INVALID_FILE_HANDLE == hFile)
    {
        OSA_ERROR("Unexpect error ... ");
        return 1;
    }
    
    DWORD dwRet = OSA_FileWrite(hFile, dataBuf, dataLen);
    if(0 == dwRet)
    {
        OSA_ERROR("Write %s file content failed.", __MACADDR__KEY__FILE__NAME__);
        OSA_FileClose(hFile);
        return 2;
    }
    
    OSA_FileClose(hFile);
    return 0;
}

BOOLEAN ipgwSendLocalMacAddr(SOCK_DATA_PACKET_T *cmdPacket, BYTE *dataBuf, INT32 dataLen)
{
    BYTE macAddr[10] = { 0 };
	BYTE strMac[30];
    SOCK_DATA_PACKET_T sendCmdPacket = 
    { 
        { DT_PC_GUARD_UNIT, 0x00, 0x01, 0x00, 0x00, 0x00 }, 
        { 0 }, 
        cmdPacket->funcCode, 
        OPER_SEND_MAC_ADDR, 
        { 0 } 
    };
    INT32 len = MAX_ID_SIZE;
    INT32 ackDataLen = 0;
    BYTE *acdDataBuf = NULL;

	read_mac_addr(strMac,macAddr,SystemDeviceInfo_GetCommunityPhyName());
   	OSA_DBG_MSG("GetLocalMac: %s",strMac);

    BYTE targetDevType = dataBuf[4];
    
    INT16 startUnitNo = (dataBuf[5] << 8) | dataBuf[6];
    INT16 endUnitNo   = (dataBuf[11] << 8) | dataBuf[12];
    
    INT16 startRoomNo = (dataBuf[7] << 8) | dataBuf[8];
    INT16 endRoomNo   = (dataBuf[13] << 8) | dataBuf[14];

    
    get_local_ID(sendCmdPacket.srcId);



    BYTE  localDevType = sendCmdPacket.srcId[0];
    INT16 localUnitNo = (sendCmdPacket.srcId[1] << 8) + sendCmdPacket.srcId[2];
    INT16 localRoomNo = (sendCmdPacket.srcId[3] << 8) + sendCmdPacket.srcId[4];
    BYTE  localUniqueId[4] = { 0 };

    INT32 res = ReadMacAddrReportKey(localUniqueId, sizeof(localUniqueId));
    OSA_DBG_MSGXX("targetDevType(0x%02x), unitNo(0x%04x, 0x%04x), roomNo(0x%04x, 0x%04x)", targetDevType, startUnitNo, endUnitNo, startRoomNo, endRoomNo);
    OSA_DBG_MSGXX("localDevType(0x%02x), localUnitNo(0x%04x), localRoomNo(0x%04x)", localDevType, localUnitNo, localRoomNo);
    BOOL bFlag = FALSE;

    if(localDevType == DT_INDOOR_STATION)
    {
        
        bFlag = (localDevType == targetDevType) && (localUnitNo >= startUnitNo && localUnitNo <= endUnitNo) && (localRoomNo >= startRoomNo && localRoomNo <= endRoomNo);
    }
    else
    {
        bFlag = (localDevType == targetDevType) && (localUnitNo >= startUnitNo && localUnitNo <= endUnitNo);
    }
    
    BYTE xxStartDev[MAX_ID_SIZE] = { 0 };
    BYTE xxEndDev[MAX_ID_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if(memcmp(xxStartDev, dataBuf + 4, MAX_ID_SIZE) == 0 && memcmp(xxEndDev, dataBuf + 10, MAX_ID_SIZE) == 0)
        bFlag = TRUE;
    OSA_DBG_MSGXX("bFlag(%d), res(%d), localUniqueId(%x, %x, %x, %x)", bFlag, res, localUniqueId[0], localUniqueId[1], localUniqueId[2], localUniqueId[3]);
    if(bFlag && (memcmp(localUniqueId, dataBuf, 4) != 0 || res > 0))
    {
        OSA_DBG_MSGXX("device range matched.NEED report local MAC ADDRESS.");
        OSA_MemCopy(macAddr + 6, dataBuf, 4);
        MAKE_DATALEN(sendCmdPacket.dataLen, sizeof(macAddr));
        if(UDPSocketSendMsg2Srv(&sendCmdPacket, macAddr, sizeof(macAddr)) == FALSE)
        {
            OSA_ERROR("SocketSendMsg2Srv FAILED.\n");
        }
        else
        {
            res = WriteMacAddrReportKey(dataBuf, 4);
            if(res > 0)
            {
                OSA_ERROR("WriteMacAddrReportKey FAILED.");
            }
        }
    }
    else
    {
        OSA_DBG_MSGXX("device range NOT matched.");
    }
    
    return TRUE;
}



BOOLEAN ipgwConnectedDeviceSearch(SOCK_DATA_PACKET_T *cmdPacket, BYTE *dataBuf, INT32 dataLen)
{
    BYTE localCurDeviceId[MAX_ID_SIZE] = { 0 };
    INT32 len = sizeof(localCurDeviceId);

	get_local_ID(localCurDeviceId);

    if(localCurDeviceId[0] != dataBuf[0])
    {
        OSA_DBG_MSG("Device Type Not Matched.(%02x, %02x).\n", localCurDeviceId[0], dataBuf[0]);
        return FALSE;
    }
    
    OSA_DBG_MSG("localCurDeviceId(0x%02x, 0x%02x). (%d, %d)\n", localCurDeviceId[1], localCurDeviceId[2], localCurDeviceId[1], localCurDeviceId[2]);

    INT32 startNo = (dataBuf[1] << 8) | dataBuf[2];
    INT32 endNo   = (dataBuf[3] << 8) | dataBuf[4];
    //INT32 curNo   = (BCD2VAL(localCurDeviceId[1]) << 8) | BCD2VAL(localCurDeviceId[2]);
    INT32 curNo   = (BCD2VAL(localCurDeviceId[1]) * 100) + BCD2VAL(localCurDeviceId[2]);

    if(curNo > endNo || curNo < startNo)
    {
        OSA_DBG_MSG("Device Number is Out of Range. (0x%04x, 0x%04x, 0x%04x), (%d, %d, %d)\n", startNo, endNo, curNo, startNo, endNo, curNo);
        return FALSE;
    }
    
    BYTE macAddr[6] = { 0 };
	BYTE strMac[30];
    SOCK_DATA_PACKET_T sendCmdPacket = 
    { 
        { DT_PC_GUARD_UNIT, 0x00, 0x01, 0x00, 0x00, 0x00 }, 
        { 0 }, 
        FUNC_SYS_CONFIGURATION, 
        OPER_NET_CONNECTED_DEVICE_SEARCH_ACK, 
        { 0 } 
    };
	
    BYTE *ackData = NULL;
    INT32 ackDataLen = 0;
	read_mac_addr(strMac,macAddr,SystemDeviceInfo_GetCommunityPhyName());

    OSA_MemCopy(sendCmdPacket.srcId, localCurDeviceId, MAX_ID_SIZE);
    MAKE_DATALEN(sendCmdPacket.dataLen, sizeof(macAddr));
    DisplayNetCmdPacket(&sendCmdPacket, macAddr, sizeof(macAddr));
    if(SocketSendMsg2Srv(&sendCmdPacket, macAddr, sizeof(macAddr), &ackData, &ackDataLen, TIMEOUT_4_NET_DATA) == FALSE)
    {
        OSA_ERROR("SocketSendMsg2Srv FAILED.\n");
    }

    SAFE_DELETE_MEM(ackData);
    return TRUE;
}



