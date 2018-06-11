/******************************************************************************
 *
 * Author:	zhujueyou
 *
 * File name:	stack_comm.h
 *	
 * Abstract:	this is for h323 and sip stack.
 *
 ******************************************************************************
 * Revision History
 *
 * Time         		Rev  	by			contents
 * -----------  ---  ---     -------------------------------------------
 *
 *****************************************************************************/
#include "sip.h"

#ifndef _STACK_COMM_H
#define _STACK_COMM_H

typedef unsigned short		u_int16;
typedef unsigned int		u_int32;

#ifndef PLATFORM_CX92755
typedef unsigned char		u_char;
#endif
					
/* signal ==> app */
#define SIGNAL_SIG2APP_OFFHOOK					0x2001
#define SIGNAL_SIG2APP_ONHOOK						0x2002

#define SIGNAL_SIG2APP_REG_PROCEEDING			0x2003
#define SIGNAL_SIG2APP_REG_SUCCESS				0x2004
#define SIGNAL_SIG2APP_REG_FAIL					0x2005

#define SIGNAL_SIG2APP_PROCEEDING					0x2006
#define SIGNAL_SIG2APP_RINGBACK					0x2007
#define SIGNAL_SIG2APP_PEERHOOKOFF				0x2008
#define SIGNAL_SIG2APP_TALK						0x2009
#define SIGNAL_SIG2APP_CALLIN						0x200a
#define SIGNAL_SIG2APP_CALLEND					0x200b
#define SIGNAL_SIG2APP_BUSY						0x200c

#define SIGNAL_SIG2APP_HOLDING_PROCEEDING		0x200d
#define SIGNAL_SIG2APP_HOLDING_SUCCESS			0x200e
#define SIGNAL_SIG2APP_HOLDING_FAIL				0x200f

#define SIGNAL_SIG2APP_HOLDED_REQUEST			0x2010
#define SIGNAL_SIG2APP_HOLDED_SUCCESS			0x2011
#define SIGNAL_SIG2APP_HOLDED_FAIL				0x2012

#define SIGNAL_SIG2APP_UNHOLDING_PROCEEDING		0x2013
#define SIGNAL_SIG2APP_UNHOLDING_SUCCESS		0x2014
#define SIGNAL_SIG2APP_UNHOLDING_FAIL			0x2015

#define SIGNAL_SIG2APP_UNHOLDED_REQUEST			0x2016
#define SIGNAL_SIG2APP_UNHOLDED_SUCCESS			0x2017
#define SIGNAL_SIG2APP_UNHOLDED_FAIL				0x2018

#define SIGNAL_SIG2APP_TRANSING_PROCEEDING		0x2019
#define SIGNAL_SIG2APP_TRANSING_ACCEPTED		0x201a
#define SIGNAL_SIG2APP_TRANSING_REJECT			0x201b
#define SIGNAL_SIG2APP_TRANSING_START			0x201c
#define SIGNAL_SIG2APP_TRANSING_SUCCESS			0x201d
#define SIGNAL_SIG2APP_TRANSING_FAIL				0x201e

#define SIGNAL_SIG2APP_TRANSED_REQUEST			0x201f
#define SIGNAL_SIG2APP_TRANSED_SUCCESS			0x2020
#define SIGNAL_SIG2APP_TRANSED_FAIL				0x2021


#define SIGNAL_SIG2APP_CALL_WAITING				0x2022

#define SIGNAL_SIG2APP_UNREG_PROCEEDING			0x2023
#define SIGNAL_SIG2APP_UNREG_SUCCESS				0x2024
#define SIGNAL_SIG2APP_UNREG_FAIL					0x2025
#define SIGNAL_SIG2APP_DISPLAY_CHANGE				0x2026
#define SIGNAL_SIP2APP_RECV_INFO					0x2027
#define SIGNAL_SIP2APP_PSTN_TALK					0x2028
#define SIGNAL_SIG2APP_PSTN_CALLIN					0x2029

#define SIGNAL_SIG2APP_MESSAGE_IN			0x202a
#define SIGNAL_SIG2APP_MESSAGE_SUCCESS			0x202b
#define SIGNAL_SIG2APP_MESSAGE_FAILED			0x202c

/* cancel bye µÄÓ¦´ð*/
#define SIGNAL_SIG2APP_CANCEL_SUCCESS               0x202e
#define SIGNAL_SIG2APP_BYE_SUCCESS                  0x2030
#define SIGNAL_SIG2APP_ACK_403                      0x2031
#define SIGNAL_SIG2APP_ACK_481                      0x2033
/* sip ==> signal */
#define SIGNAL_SIP2SIG_REG_PROCEEDING			SIGNAL_SIG2APP_REG_PROCEEDING
#define SIGNAL_SIP2SIG_REG_SUCCESS				SIGNAL_SIG2APP_REG_SUCCESS
#define SIGNAL_SIP2SIG_REG_FAIL					SIGNAL_SIG2APP_REG_FAIL
#define SIGNAL_SIP2SIG_UNREG_PROCEEDING			SIGNAL_SIG2APP_UNREG_PROCEEDING
#define SIGNAL_SIP2SIG_UNREG_SUCCESS				SIGNAL_SIG2APP_UNREG_SUCCESS
#define SIGNAL_SIP2SIG_UNREG_FAIL					SIGNAL_SIG2APP_UNREG_FAIL

#define SIGNAL_SIP2SIG_PROCEEDING				SIGNAL_SIG2APP_PROCEEDING
#define	SIGNAL_SIP2SIG_RINGBACK					SIGNAL_SIG2APP_RINGBACK
#define	SIGNAL_SIP2SIG_PEERHOOKOFF				SIGNAL_SIG2APP_PEERHOOKOFF
#define	SIGNAL_SIP2SIG_TALK					SIGNAL_SIG2APP_TALK
#define	SIGNAL_SIP2SIG_CALLIN					SIGNAL_SIG2APP_CALLIN
#define	SIGNAL_SIP2SIG_CALLEND					SIGNAL_SIG2APP_CALLEND

#define SIGNAL_SIP2SIG_HOLDING_PROCEEDING			SIGNAL_SIG2APP_HOLDING_PROCEEDING
#define	SIGNAL_SIP2SIG_HOLDING_SUCCESS				SIGNAL_SIG2APP_HOLDING_SUCCESS
#define SIGNAL_SIP2SIG_HOLDING_FAIL				SIGNAL_SIG2APP_HOLDING_FAIL

#define SIGNAL_SIP2SIG_HOLDED_REQUEST				SIGNAL_SIG2APP_HOLDED_REQUEST
#define SIGNAL_SIP2SIG_HOLDED_SUCCESS				SIGNAL_SIG2APP_HOLDED_SUCCESS
#define	SIGNAL_SIP2SIG_HOLDED_FAIL				SIGNAL_SIG2APP_HOLDED_FAIL

#define SIGNAL_SIP2SIG_UNHOLDING_PROCEEDING			SIGNAL_SIG2APP_UNHOLDING_PROCEEDING
#define SIGNAL_SIP2SIG_UNHOLDING_SUCCESS			SIGNAL_SIG2APP_UNHOLDING_SUCCESS
#define SIGNAL_SIP2SIG_UNHOLDING_FAIL				SIGNAL_SIG2APP_UNHOLDING_FAIL

#define SIGNAL_SIP2SIG_UNHOLDED_REQUEST				SIGNAL_SIG2APP_UNHOLDED_REQUEST
#define SIGNAL_SIP2SIG_UNHOLDED_SUCCESS				SIGNAL_SIG2APP_UNHOLDED_SUCCESS
#define SIGNAL_SIP2SIG_UNHOLDED_FAIL				SIGNAL_SIG2APP_UNHOLDED_FAIL

#define SIGNAL_SIP2SIG_TRANSING_PROCEEDING			SIGNAL_SIG2APP_TRANSING_PROCEEDING
#define SIGNAL_SIP2SIG_TRANSING_ACCEPTED			SIGNAL_SIG2APP_TRANSING_ACCEPTED
#define SIGNAL_SIP2SIG_TRANSING_REJECT				SIGNAL_SIG2APP_TRANSING_REJECT
#define SIGNAL_SIP2SIG_TRANSING_START				SIGNAL_SIG2APP_TRANSING_START
#define SIGNAL_SIP2SIG_TRANSING_SUCCESS				SIGNAL_SIG2APP_TRANSING_SUCCESS
#define SIGNAL_SIP2SIG_TRANSING_FAIL				SIGNAL_SIG2APP_TRANSING_FAIL

#define SIGNAL_SIP2SIG_TRANSED_REQUEST				SIGNAL_SIG2APP_TRANSED_REQUEST
#define SIGNAL_SIP2SIG_TRANSED_SUCCESS				SIGNAL_SIG2APP_TRANSED_SUCCESS
#define SIGNAL_SIP2SIG_TRANSED_FAIL				SIGNAL_SIG2APP_TRANSED_FAIL

#define SIGNAL_SIP2SIG_CALL_WAITING				SIGNAL_SIG2APP_CALL_WAITING
#define SIGNAL_SIP2SIG_DISPLAY_CHANGE				SIGNAL_SIG2APP_DISPLAY_CHANGE
#define SIGNAL_SIP2SIG_RECV_INFO					SIGNAL_SIP2APP_RECV_INFO

#define SIGNAL_SIP2SIG_MESSAGE_IN				SIGNAL_SIG2APP_MESSAGE_IN
#define SIGNAL_SIP2SIG_MESSAGE_SUCCESS				SIGNAL_SIG2APP_MESSAGE_SUCCESS
#define SIGNAL_SIP2SIG_MESSAGE_FAILED				SIGNAL_SIG2APP_MESSAGE_FAILED

#define SIGNAL_SIP2SIG_CANCEL_SUCCESS              SIGNAL_SIG2APP_CANCEL_SUCCESS               
#define SIGNAL_SIP2SIG_BYE_SUCCESS                 SIGNAL_SIG2APP_BYE_SUCCESS 
#define SIGNAL_SIP2SIG_ACK_403                     SIGNAL_SIG2APP_ACK_403
#define SIGNAL_SIP2SIG_ACK_481                     SIGNAL_SIG2APP_ACK_481

/* sip ==> signal, cause code */
								/* call fail cause */
#define VCL_CAUSE_CC_ERROR              			0x0001
#define VCL_CAUSE_PEER_HOOKON           			0x0002
#define VCL_CAUSE_CALLED_BUSY           			0x0003
#define VCL_CAUSE_CALLING_TIMEOUT       			0x0004
#define VCL_CAUSE_CALLED_TIMEOUT        			0x0005
#define VCL_CAUSE_CALLED_REJECT         			0x0006
#define VCL_CAUSE_CALLING_CANCEL        			0x0007
#define VCL_CAUSE_SIPSERVER_ERROR       			0x0008
#define VCL_CAUSE_HOLD_ERROR            			0x0009
#define VCL_CAUSE_UNHOLD_ERROR          			0x000a
#define VCL_CAUSE_TRANS_SUCCESS         			0x000b
#define VCL_CAUSE_LINK_CHANGED          			0x000c
#define VCL_CAUSE_NOT_FOUND             			0x000d
#define VCL_CAUSE_SELF_HOOKON           			0x000e
#define VCL_CAUSE_CALLED_REJECT1         			0x000f

								/* rejecting to transfer cause */
#define VCL_CAUSE_TRANSRJCT_OTHER       			0x0001
#define VCL_CAUSE_TRANSRJCT_MSGERR      			0x0002
#define VCL_CAUSE_TRANSRJCT_REDERR      			0x0003
#define VCL_CAUSE_TRANSRJCT_TIMEOUT     			0x0004
#define VCL_CAUSE_TRANSRJCT_SRVERR      			0x0005
								/* holding fail cause */
#define VCL_CAUSE_HOLDING_FAIL          			0x0001
#define VCL_CAUSE_HOLDING_MSGERR        			0x0002
#define VCL_CAUSE_HOLDING_REDERR        			0x0003
#define VCL_CAUSE_HOLDING_TIMEOUT       			0x0004
#define VCL_CAUSE_HOLDING_SRVERR        			0x0005
#define VCL_CAUSE_HOLDING_LCLERR        			0x0006
								/* holded fail cause */
#define VCL_CAUSE_HOLDED_FAIL           			0x0001
#define VCL_CAUSE_HOLDED_MSGERR         			0x0002
#define VCL_CAUSE_HOLDED_TIMEOUT        			0x0003
#define VCL_CAUSE_HOLDED_SRVERR         			0x0004
								/* unholding fail cause */
#define VCL_CAUSE_UNHOLDING_FAIL        			0x0001
#define VCL_CAUSE_UNHOLDING_MSGERR      			0x0002
#define VCL_CAUSE_UNHOLDING_REDERR      			0x0003
#define VCL_CAUSE_UNHOLDING_TIMEOUT     			0x0004
#define VCL_CAUSE_UNHOLDING_SRVERR      			0x0005
#define VCL_CAUSE_UNHOLDING_LCLERR      			0x0006
								/* unholed fail cause */
#define VCL_CAUSE_UNHOLDED_FAIL         			0x0001
#define VCL_CAUSE_UNHOLDED_MSGERR       			0x0002
#define VCL_CAUSE_UNHOLDED_TIMEOUT      			0x0003
#define VCL_CAUSE_UNHOLDED_SRVERR       			0x0004
								/* rejecting to transfer cause */
#define VCL_CAUSE_TRANSRJCT_OTHER      				0x0001
#define VCL_CAUSE_TRANSRJCT_MSGERR      			0x0002
#define VCL_CAUSE_TRANSRJCT_REDERR      			0x0003
#define VCL_CAUSE_TRANSRJCT_TIMEOUT     			0x0004
#define VCL_CAUSE_TRANSRJCT_SRVERR      			0x0005
								/* transfer fail cause */
#define VCL_CAUSE_TRANSING_FAIL         			0x0001
#define VCL_CAUSE_TRANSING_BUSY         			0x0002
#define VCL_CAUSE_TRANSING_TIMEOUT1     			0x0003
#define VCL_CAUSE_TRANSING_TIMEOUT2     			0x0004
#define VCL_CAUSE_TRANSING_REJECT       			0x0005
#define VCL_CAUSE_TRANSING_SRVERR      				0x0006
#define VCL_CAUSE_TRANSING_NOTFOUND			        0x0007
								/* transfered fail cause */
#define VCL_CAUSE_TRANSED_FAIL          			0x0001
#define VCL_CAUSE_TRANSED_BUSY          			0x0002
#define VCL_CAUSE_TRANSED_TIMEOUT1      			0x0003
#define VCL_CAUSE_TRANSED_TIMEOUT2      			0x0004
#define VCL_CAUSE_TRANSED_REJECT        			0x0005
#define VCL_CAUSE_TRANSED_SRVERR        			0x0006
								/* register fail cause */
#define VCL_CAUSE_REG_TIMEROUT          			0x0001
#define VCL_CAUSE_REG_AUTH_ERR          			0x0002
#define VCL_CAUSE_REG_ERR               			0x0003
#define VCL_CAUSE_REG_LINKCHANGED       			0x0004
#define VCL_CAUSE_VCLCALLID_ERROR       			0x0081

                              /* ringback  cause */
#define VCL_CAUSE_RINGBACK_SERVER                   0x0001
#define VCL_CAUSE_RINGBACK_PEER                     0x0002

typedef struct {
	char type[64];
	char	phonenum[32];
	char body[1024];
	unsigned short int rdlen; /*radio ctrl cmd data length.*/	
	unsigned short int rddata[35]; /*radio ctrl cmd data,rddata[0]-start,rddata[rdlen+1]-ctrl,rddata[rdlen+1]-checksum.*/	
} info_msg_t;

typedef struct tag_signal_msg_head
{
	int		msgtype;
	u_int32		chan_id;
	int		sess_id;
	int		value;
	info_msg_t infodata;
}SIGNAL_MSG_HEADER;

#define PHONE_DTMF_RFC2833	0
#define PHONE_DTMF_INFO		1
#define PHONE_DTMF_INBAND		2

typedef struct {
  char    dtmf[2];   /* DTMF event  '0'-'9','*','#','A'-'D' + '\0' */
  int dtmfmode;
} second_dial_t;

#ifdef UDTECH_SUPPORT_RADIO
#define RADIO_CALLINFO_MAX 32
typedef struct {
	unsigned int flag;
	char callinfo[RADIO_CALLINFO_MAX]; 
} radio_info_t;
#endif
typedef struct SDPinfo 
{
    int resList[16];
	int resListLen;
	int mapList[16];
	int  mapListLen;
	char mutipleAddr[16];
}SDP_INFO;


typedef struct tag_signal_msg
{
	int		msgtype;
	/* used in the set of session messages */
	u_int16		pflcallid;
	u_int16		vclcallid;
	u_int32		chan_id;

	union
	{
		struct
		{
			/* ITSP ID */
			u_int32 itspType;                       /* ITSP type (ID) */
			/* SIP Server Address Info */
			char 	sipServerAddr[64]; 		/* SIP Server Address */
			u_int32 sipServerUDPPort;               /* SIP Server listening port */
			char    sipServerDomanName[64];         /* SIP Server domain name */
			/* User Authentication Info to SIP Server */
			char    userID[64];                     /* local user ID allocated by ITSP */
			char    userPasswd[32];                 /* local user passwd allocated by ITSP */
			char    userPhoneNum[32];               /* local user phone number allocated by ITSP */
			/* SIP Proxy Info */
			char    sipProxyAddr[64];               /* 20040803 , SIP Proxy Address: IP addr or DN */
			u_int32 sipProxyPort;                   /* 20040803 , SIP Server listening port */
			/* 20040510 start */

			/*Other Info for ITSP*/
			char    sipUrl[128];                    /* SIP URL */
			u_int32 useTelUrl;                      /* whether use Tel Url */
			u_int32 useRegist;                      /* whether register */
			u_int32 sipServerRegExpire;             /* sip server register expire */
			u_int32 sipExt100Rel;                   /* whether use PRACK */
			u_int32 earlyMedial;                    /* whether use UPDATE */
			char    regQValue[8];                   /* register priority */
			u_int32 regRetry;                       /* retry period (s) when register fail */
			u_int32 regRefresh;			/* re-register time scale (%)*/
			u_int32 regExpireFirst;                 /* before REGISTER, whther delete REGISTER */
			u_int32 AoRDisName ;               	/* in address of register (AoR), whether set display-name */
			char    AoRUser[64];                    /* user part of AoR  SIP-URI */
			char    AoRHost[64];                    /* host part of AoR SIP-URI */
			u_int32 AoRParam;                       /* in AoR), whether set  "user=phone" */
			u_int32 contDisName;                    /* whether set display-name of Contact Header */
			char    contUser[64];                   /* user part of SIP-URI of  Contact Header */
			char    contHost[64];                   /* 20040803 */
			u_int32 contParam;                      /* whether set "user=phone" of Contact Header */
			u_int32 regURI;                         /* Request-URI of register */
			char    sipProxyPreferredId[96];        /* 20040803 , sipProxy2Table */

			/* 20040510 end */
		}registerInfo;
		/* session part */
		struct{
			char	phonenum[32];		/* peer phone number */
			char	userName[16];		/* peer user name */
			char	userPasswd[16];		/* peer user passwd */
			u_int16 codecType;		/* codec type */
			u_int16 videoCodecType;  /* video codec type */
			u_int16	packetTime;		/* interval of packet */
			char	dstAddress[64];		/* peer ip */
			char	calltype;		/* SIGNAL_IP_OUT, SIGNAL_PSTN_OUT */
			u_int16 callflag;
			char localnum[32];
#ifdef UDTECH_SUPPORT_RADIO			
			radio_info_t ci;
#endif
        char   srcAddress[16];  /*previous device caller ip address*/
		SDP_INFO sdpinfo;

		}callInfo;
		struct{
			u_int32	srcCallID;		/* peer callid */
			char	phonenum[32];		/* 3rd one's phone number */
			char	userName[16];		/* 3rd one's user name */
			char	userPasswd[16];		/* 3rd one's user passwd */
			char	dstAddress[64];		/* 3rd one's ip addr */
		}trans;
		struct{
                        char localipaddr[32];
                        char localnum[32];
                        char localusername[16];
                        char localpasswd[16];
                        char peeripaddr[32];
                        char peernum[32];
                        char peerusername[16];
                        char peerpasswd[16];
                        char msgbody[512+1];
                }im;
		u_int32	causetype;
	      struct
	       {
			u_int32 causeType;
	       }cause;

	       struct 
	       {
			u_int32 value1;
	       }val;		
		second_dial_t second_dial;
		info_msg_t info;
		/* channel part */
		/* device part */
	}message;
}SIGNAL_MSG;

#endif




