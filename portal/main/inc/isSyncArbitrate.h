/*IP-GW模式，户内多分机同步请求仲裁接口*/

#ifndef __IS_SYNC_ARBITRATE_H
#define _IS_SYNC_ARBITRATE_H
#include <network.h>
#include <osa/osa_debug.h>

extern void sync_req_flag_init();
extern void sync_req_flag_process(s_PacketMsg *interMsg, BYTE *data,UINT32 len);


#endif 
