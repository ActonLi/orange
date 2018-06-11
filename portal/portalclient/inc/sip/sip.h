/******************************************************************************
* Copyright 2010-2011 ABB Genway Co.,Ltd.
* FileName:       sip.h
* Desc:
* 
* 
* Author:    daniel_qinghua.huang
* Date:      2010/06/06
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2010/06/06, daniel_qinghua.huang create this file
* 
******************************************************************************/

#ifndef __SIP_H__
#define __SIP_H__

//#include "stack_comm.h"
//modified by martin 20080128 

#define M_H263

#define TELPORTS        1       /* Number of support tel ports  */
#define DEF_PEER_NUM	20
#define DEF_PHNE_NUM	1
#define DEF_CTRL_NUM	1
#define DEF_PRXY_NUM	1 /* 2004121501 */
#define DEFAULT_NONE	0
#define MOD_DEFAULT_PHONE					0x00
#define MOD_VIDEO_PHONE					0x01

#define VOIPPEERENTRY		32
#define DEF_VOIPPEERDIALNUMBER 		""
#define MIN_VOIPPEERDIALNUMBER 		0
#define MAX_VOIPPEERDIALNUMBER 		32
#define DEF_VOIPPEERUSERNAME 		""
#define MIN_VOIPPEERUSERNAME 		0
#define MAX_VOIPPEERUSERNAME 		16
#define DEF_VOIPPEERPASSWORD 		""
#define MIN_VOIPPEERPASSWORD 		0
#define MAX_VOIPPEERPASSWORD 		16
#define DEF_VOIPPEERADDRESS 		""
#define MIN_VOIPPEERADDRESS 		0
#define MAX_VOIPPEERADDRESS 		64
#define DEF_VOIPPEEROUTPROXY 		""
#define MIN_VOIPPEEROUTPROXY 		0
#define MAX_VOIPPEEROUTPROXY 		12
#define DEF_VOIPPEERCODECTYPE		0
#define DEF_VOIPPEERCODECTYPE_PCMU	0
#define DEF_VOIPPEERCODECTYPE_G726_32	2
#define DEF_VOIPPEERCODECTYPE_G723	4
#define DEF_VOIPPEERCODECTYPE_G728	15
#define DEF_VOIPPEERCODECTYPE_PCMA	8
#define DEF_VOIPPEERCODECTYPE_G729	18

/*these define values are from GaoKe ip phone parameters*/
#define DEF_VOIPPEERCODECTYPE_G726_16	23
#define DEF_VOIPPEERCODECTYPE_G726_24	22
#define DEF_VOIPPEERCODECTYPE_G726_40	21
#define DEF_VOIPPEERCODECTYPE_ADPCM     49
//add by xuyan 20060306 for eyebeam video
#ifdef M_H263
#define DEF_VOIPPEERCODECTYPE_H263		34  // 20050314:zhuyunzhi
#endif
#define DEF_VOIPPEERCODECTYPE_H264		98

#define DEF_VOIPPEERCODECTYPE_G723_5P3	203
#define DEF_VOIPPEERCODECTYPE_MPEG4	210 /* videophone: video codec */
#define DEF_VOIPPEERCODECTYPE_NONE	255
#define MIN_VOIPPEERCODECTYPE			0
#define MAX_VOIPPEERCODECTYPE			255
#define DEF_VOIPPEERPHONEMODE				0
#define DEF_VOIPPEERPHONEMODE_AUDIO_ONLY	0
#define DEF_VOIPPEERPHONEMODE_TV_PHONE	1
#define DEF_VOIPPEERPACKETTIME			20
#define DEF_VOIPPEERPACKETTIME_10MS	10
#define DEF_VOIPPEERPACKETTIME_20MS	20
#define DEF_VOIPPEERPACKETTIME_30MS	30
#define DEF_VOIPPEERPACKETTIME_40MS	40
#define DEF_VOIPPEERPACKETTIME_50MS	50
#define DEF_VOIPPEERPACKETTIME_60MS	60
#define MIN_VOIPPEERPACKETTIME 		10
#define MAX_VOIPPEERPACKETTIME 		60
#define DEF_VOIPPEERRESERVE1			0
#define MIN_VOIPPEERRESERVE1 			0
#define MAX_VOIPPEERRESERVE1			65535
#define DEF_VOIPPRFILLGAP				""
#define MIN_VOIPPRFILLGAP				0
#define MAX_VOIPPRFILLGAP				110
#define DEF_VOIPPRFILLGAPLEN			110
#define MIN_VOIPPRFILLGAPLEN			0
#define MAX_VOIPPRFILLGAPLEN			65535

#define MAX_CODEC_ENTRIES 32


#ifdef CALL_HOLD
typedef enum {
    e_Video_VGA = 1,
    e_Video_SMALL    /*新来电小窗口视频*/
}SDP_VIDEO_TYPE;

#endif

typedef struct tag_SIP_SESSION
{
	int sip_fd;
} SIP_SESSION;

typedef struct tag_CHANNEL_PARAMS
{
	unsigned long ulLineNum;
	unsigned short bMediaType;
	unsigned long ulRemoteAddr;
	unsigned short usRemotePort;
	unsigned short usLocalPort;
	unsigned char ucPayloadType; 
   // unsigned char ucVideoFormatType; /*对端支持的分辨率格式 详见 video_resolution_t*/
	unsigned char ucPacketTime; 
	char spec[64];
	int res;
  /* for supporting rfc2833 */
  unsigned char  redundancy;
  unsigned char  telephone_event;
#ifdef FORCE_RFC2833
  unsigned char  X_redundancy;
  unsigned char  X_telephone_event;
#endif	

}CHANNEL_PARAMS;




typedef struct SDPinfo 
{
    int resList[16];
	int resListLen;
	int mapList[16];
	int  mapListLen;
	char mutipleAddr[16];
}SDP_INFO;


#if 0
//#ifdef FOR_SDP_RESOLUTION
typedef enum
{  
  eCIF = 0,		  // 352x288
  eVGA = 1,         // 352x288
  eSVGA = 2,         // 640x480
  e720P = 3,        // 1280x720
  e1080P = 4        // 1920x1080
}video_resolution_t;

#endif


typedef struct tag_SIP_DIALPEER
{
	char		szPeerDialNumber[32];
	char		szPeerUserName[16];
	char		szPeerPassword[16];
	char		szPeerAddress[64];
	unsigned char	ucPeerCodecType;
	unsigned char	ucPeerVideoCodecType;
	unsigned char	ucPeerPhoneMode;
	unsigned char	ucPeerPacketTime;
#ifdef UDTECH_SUPPORT_RADIO			
	radio_info_t ci;
#endif	

} SIP_DIALPEER;

typedef struct tag_SIP_LOCAL
{
	int		sess_id;
	char		voipPhoneNumber[32];
	char		voipPhoneUserName[16];
	char		voipPhonePassword[16];
	unsigned long	voipPhoneAccess;
	char		voipPhoneDomain[64];
	char		voipPhoneRegistrar[32];
	unsigned char	voipPhonePrivacy;
	unsigned char	voipPhoneNumDisp;
} SIP_LOCAL;


typedef struct tag_SIP_MESSAGE {
        char    localipaddr[32];
		char    localdomain[32];
        char    localnumber[32];
        char    localusername[16];
        char    localpasswd[16];
        char    peeripaddr[32];
        char    peernumber[32];
        char    peerusername[16];
        char    peerpasswd[16];
        char    msgbody[512+1];
} SIP_MESSAGE;

typedef struct vsipd_voipPhoneTable 
{
	char				voipPhoneNumber[32];
	char				voipPhoneUserName[16];
	char				voipPhonePassword[16];
	unsigned long		voipPhoneAccess;
	char				voipPhoneDomain[64];
	char				voipPhoneRegistrar[32];
	unsigned char		voipPhonePrivacy;
	unsigned char		voipPhoneNumDisp;
	unsigned char		voipPhoneRelayPstn;
	/*
	unsigned char		voipPhoneReserve1;
	*/
	unsigned char		voipPhonePrivacySpec;
	char				voipphFillgap[106];
	unsigned short	voipphFillgaplen;
} VSIPD_VOIPPHONETABLE, *VSIPD_VOIPPHONETABLE_Ptr;



/*****************************************************************************
vsipd_voipControlTable  结构用于 SIP 模块初始化的时候传递初始化参数使用
*****************************************************************************/
typedef struct vsipd_voipControlTable {
	unsigned char		voipProtocol;/* 目前此变量无用 */
	unsigned char		voipRtpTos; /* 目前此变量无用 */
	unsigned char		voipRtcpTos;/* 目前此变量无用 */
	unsigned char		voipDefPacketTime; /*  */
	unsigned char       voipDtmfMode;	/* DTMF 码的传递方式 PHONE_DTMF_RFC2833/PHONE_DTMF_INFO/PHONE_DTMF_INBAND  */
	unsigned char	    voipReserve1;	/* 目前次变量无用 */
	
	unsigned short	    voipRtpPort;	/* RTP 端口号 */
	unsigned short	    voipRtcpInterval;/* 此变量没有使用 */
	
	/*unsigned char		voipDefCodecType;*/	
    unsigned char        voipDefCodecType[MAX_CODEC_ENTRIES];
#if defined(FOR_SDP_RESOLUTION_IS) || defined(FOR_SDP_RESOLUTION_OS)
	unsigned char		voipDefVideoResolution; /* 默认视频分辨率为VAG,20140509:huangqinghua:add */
#endif	

//modify by xuyan 20060306 for eyebeam video
#ifdef M_H263
	unsigned char		voipDefVideoCodecType; // 20050314:zhuyunzhi added video codec support
	unsigned char        voipDefVideoPacketTime; // // 20050315:zhuyunzhi:add
	char                 voipctFillgap[242 - MAX_CODEC_ENTRIES];
#else
	char                 voipctFillgap[244 - MAX_CODEC_ENTRIES];
#endif
	unsigned short	    voipctFillgaplen;
} VSIPD_VOIPCONTROLTABLE, *VSIPD_VOIPCONTROLTABLE_Ptr;

/*****************************************************************************
vsipd_sipControlTable  结构用于 SIP 模块初始化的时候传递初始化参数使用
*****************************************************************************/
typedef struct vsipd_sipControlTable {
	unsigned short 	sipLocalPort;
	unsigned char		sipTos;
	unsigned char		sipNatTraversal;
	unsigned char		sipLocalIpAddr[4];
	char				sipIfName[8];
	unsigned short 	sipCallTimer;
	unsigned char		sip1xxTimer;
	unsigned char		sip18xTimer;
	unsigned char		sipT1Timer;
	unsigned char		sipT2Timer;
	unsigned char		sipT4Timer;
	unsigned char		sipUAHeader;
	unsigned short 	sipSessionTimer;
	unsigned char		sipSessionRefresher;
	unsigned char		sipReserve2;
	unsigned char		useSessionTimer;
	unsigned char 	sipReserve3;
	unsigned short 	sipReserve4;
	char				sipctlFillgap[222];
	unsigned short	sipctlFillgaplen;

	unsigned char       sip_upnp_use;   //added by zhouliyuan for upnp
	unsigned char       sip_stun_use;   //added by zhchli for stun
	
} VSIPD_SIPCONTROLTABLE, *VSIPD_SIPCONTROLTABLE_Ptr;

typedef struct vsipd_sipProxyTable {
	char				sipProxyAddress[64];
	unsigned short	sipProxyPort;
	unsigned short	sipProxyReserve1;
	char				sipProxyUserName[32];
	unsigned char		sipProxyUserPhone;
	unsigned char		sipProxyUseTelUrl;
	unsigned short	sipProxyReserve2;
	char				sipProxyDomain[64];
	char				sipProxyLoginName[16];
	char				sipProxyPassword[16];
	unsigned long		sipProxyRegExpire;
	unsigned char		sipProxySpecPrivacy;
	unsigned char		sipProxySpecEMOpen;
	unsigned char		sipProxySpec100Rel;
	unsigned char		sipProxyReserve4;
	char				sipProxyDisplayName[32];
	char				siprxyFillgap[14];
	unsigned short	siprxyFillgaplen;
} VSIPD_SIPPROXYTABLE, *VSIPD_SIPPROXYTABLE_Ptr;

/*****************************************************************************
vsipd_callControlTable  结构用于 SIP 模块初始化的时候传递初始化参数使用
*****************************************************************************/
typedef struct vsipd_callControlTable
{
    unsigned char useCallTransfer; 
    unsigned char useCallHold;
    unsigned char useCallWait;
    unsigned char callHoldMode;
#if 1 /*zhhuang add for hisi 2006/09/11*/
    unsigned char callwait_audioprompt;
    unsigned char callwait_videoprompt;
    unsigned char directforward;
    char drf_num[64];
    char drf_addr[64];
    unsigned char busyforward;
    char byf_num[64];
    char byf_addr[64];
    unsigned char noanswerforward;
    char noans_num[64];
    char noans_addr[64];
#endif
    unsigned char ccReserve1;
    unsigned char ccReserve2;
    unsigned char ccReserve3;
    unsigned char ccReserve4;
    char   ccFillgap[246];
    unsigned short ccFillgaplen;
}VSIPD_CALLCONTROLTABLE, *VSIPD_CALLCONTROLTABLE_Ptr;

typedef struct tag_PARAMS_SIP_INIT
{
	struct vsipd_voipControlTable* pVoipCtrlTbl;
	struct vsipd_sipControlTable* pSipCtrlTbl;
	struct vsipd_callControlTable* pCallCtrltbl;
	unsigned char* pPhoneType;
	int* pTelPortNum;
}PARAMS_SIP_INIT;


/*****************************************************************************
为了使用 SIP 模块，本结构中定义的函数指针需要应用程序模块实现，通过 SIP 模块
提供的 SIP_setCallbackFunc 函数接口把这些函数指针赋值给 SIP 模块。

函数包括:
    发送消息给应用程序的函数;
    用来取得本地配置信息的函数;
    用来创建 RTP session 的函数;
    用来关闭 RTP session 的函数;
    定时器函数，包括定时器的创建、删除、启动、停止.
*****************************************************************************/
typedef struct tag_SIP_CALLBACK
{
	int (* SIP_setDeviceState) (void *pmsg);/* 实际是发送消息给应用程序的函数, 函数名称不准确 */
	int (* SIP_getDeviceState) (int nPhone); /* 没有使用??!!  */
	void *(* SIP_getDevice)(char * szPhoneNum);	/* 用来取得本地信息, 返回指针为 SIP_LOCAL  结构指针 */
	unsigned long (* SIP_createChannel) (void * pParam);/* 用来创建 RTP session 的函数指针 */
	void (* SIP_closeChannel) (unsigned long ulChannelID); /* 用来创建 RTP session 的函数指针 */
	void * (*timer_create) (void (*func)(void *), void *arg);/* 以下为定时器函数 */
	void (* timer_delete) (void *timer);
	void (* timer_start) (void *timer, unsigned int interval);
	void (* timer_stop) (void *timer);
} SIP_CALLBACK;
	
extern void SIP_setCallbackFunc (SIP_CALLBACK *pCallback);
extern void SIP_init(PARAMS_SIP_INIT* pParams);
extern SIP_SESSION *SIP_open(unsigned short sipLocalListenPort);
extern int SIP_close(SIP_SESSION *pSess);
extern int SIP_receive(SIP_SESSION *pSess, char *pPacket,int packetlen);
#ifndef MODE_OLDSIP
extern int SIP_beginRing(int line_num, const char *muticast_ip);
#endif
extern int SIP_beginCaller (void *pPeer, void *pPhone,SDP_INFO* sdpinfo);
extern int SIP_beginCallee (int nLineID);
extern int SIP_endCall (int nLineID);
extern int SIP_hold(int sess_id);
extern int SIP_unhold(int sess_id);
extern int SIP_transfer(int sess_id, char * trans_num,char *tran_address);
extern void SIP_register(struct vsipd_sipProxyTable* pSipProxyTbl);
extern void SIP_unregister(struct vsipd_sipProxyTable* pSipProxyTbl);
extern int SIP_message(int sess_id,void*messageinfo);

extern int SIP_addAudioCodec(unsigned char codectype, unsigned char payload, char *a_rtpmap,
								char *a_bitrate);
extern int SIP_busy(int sess_id);
extern int SIP_notfound(int sess_id);
extern int SIP_senddtmf_rfc2833(int sess_id, unsigned int *event);
extern int SIP_info(int sess_id, void *data);
#endif
