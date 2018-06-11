#include "PortalClient.h"

INT fnPortalClientSendMsg2IPGateway(BYTE u8OperCode, BYTE * pSrcDataBuf, UINT32 u32DataLen)
{
	BYTE aNetCmdDat[1024] = { 0 };
	SOCK_DATA_PACKET_T *pNetCmdHeader = (SOCK_DATA_PACKET_T *)aNetCmdDat;
	BYTE *pDestDataBuf = &aNetCmdDat[sizeof(SOCK_DATA_PACKET_T)]; 			/*数据段的起始地址*/

	if(u32DataLen != 0)
	{
		OSA_MemCopy(pDestDataBuf,pSrcDataBuf,u32DataLen);
	}

	pNetCmdHeader->dstId[0] = 0;
	pNetCmdHeader->srcId[0] = 0;
	pNetCmdHeader->funcCode = PROCESS_FUNC_IPGW;
	pNetCmdHeader->operCode = u8OperCode;
	MAKE_DATALEN(pNetCmdHeader->dataLen, u32DataLen);

	#ifndef PORTAL_CLIENT_RELEASE
	printf("%s\n",DEBUG_HEADER_PORTALCLIENT_SENDTO_IPGW_MSG);
	printf("\t\tDst Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dstId[5],pNetCmdHeader->dstId[4],pNetCmdHeader->dstId[3],pNetCmdHeader->dstId[2],pNetCmdHeader->dstId[1],pNetCmdHeader->dstId[0]);
	printf("\t\tSrc Addr: [0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->srcId[5],pNetCmdHeader->srcId[4],pNetCmdHeader->srcId[3],pNetCmdHeader->srcId[2],pNetCmdHeader->srcId[1],pNetCmdHeader->srcId[0]);
	printf("\t\tFuncCode: [0x%02x]\n",pNetCmdHeader->funcCode);
	printf("\t\tOperCode: [0x%02x]\n",pNetCmdHeader->operCode);
	printf("\t\tDataLen:  [0x%02x 0x%02x 0x%02x 0x%02x]\n",pNetCmdHeader->dataLen[3],pNetCmdHeader->dataLen[2],pNetCmdHeader->dataLen[1],pNetCmdHeader->dataLen[0]);
	printf("\t\tData[");
	int i;
	for(i = 0; i < u32DataLen; i++)
	{
		printf("0x%02x ",pDestDataBuf[i]);
	}
	printf("]\n\n"); 
	#endif

	s_PacketMsg stHeaderMsg = { 0 };

	stHeaderMsg.order_type = PHONE_ORDER_ENTER_LOGIC;
	stHeaderMsg.sockfd = -1;
	stHeaderMsg.datalen = sizeof(SOCK_DATA_PACKET_T) + u32DataLen;
	
	return SendMsg2IPGW((BYTE *)&stHeaderMsg,sizeof(s_PacketMsg),aNetCmdDat,sizeof(SOCK_DATA_PACKET_T) + u32DataLen);
}

