/*****************************************************************************
 *     
 *	#         #####    #     #    #     #    #     #
 *	#           #      ##    #    #     #     #   #
 *	#           #      # #   #    #     #      # #
 *	#           #      #  #  #    #     #       #
 *	#           #      #   # #    #     #      # #
 *	#           #      #     #    #     #     #   #
 *	#######   #####    #     #     #####     #     #
 *
 *	 Copyright (c) 2001 - 2005	UDTech Corporation
 *****************************************************************************
 *
 * Author:	Liuwei
 *
 * File name:	rtp_rtcp.h 2003/08
 *	
 * Abstract:	Providing extern interface of rtcp module
 *
 *****************************************************************************
 * Revision History
 *
 * Time         Rev  by	      contents
 * -----------  ---  ------   -------------------------------------------
 * 03-Jul-2002  1.0  Liuwei   written
 * 16-Feb-2004  2.0  Shenhang updated 
 *****************************************************************************/

#ifndef _RTP_RTCP_H
#define _RTP_RTCP_H

#ifdef ALPHA_DEBUG
#include "debug.h"
#endif

#include "sys/time.h"
#include "netinet/in.h"
#define RTPTIMEOFFSET(newt,oldt) (((newt)<(oldt))?(0xffffffff-(oldt)+(newt)+1):((newt)-(oldt)))
/* return value for the receiving video packet function  */
#define RTP_RCV_END			1
#define RTP_RCV_CONTINUE	2
#define RTP_RCV_1FRAME		3
#define RTP_RCV_MP4_HEAD	4
#define RTP_RCV_1FRAME_I	5
#define RTP_RCV_1FRAME_P	6

#define RTP_VIDEO_BUF_LEN	100*1024
#define RTP_PACKET_LEN		1024 // 512
#define RTP_QUALITY			70
#define FORCE_RFC2833



/*******************************************************************************/
/*                                                                             */
/*   The following define and data structure for dealing RTP/RTCP packet       */
/*                                                                             */
/*******************************************************************************/
struct SESSION_STATES
{
	struct timeval tp;      /* the last time an RTCP pkt was transmitted */ 
	struct timeval tn;      /* the next scheduled transmission time of an RTCP pkt */ 
	u_int pmembers;       /* the estimated number of session members at time tm was last recomputed */
	u_int members;        /* the most currente estimate for the number of the session members */
	u_int senders;        /* the most currente estimate for the number of senders in the session */
	u_int rtcp_bw;        /* the target RTCP bandwidht */
	u_int avg_rtcp_size;  /* the average Compound RTCP pkt size, in octets, over all RTCP pkts sent and received by this partecipant */
	u_int we_sent;        /* flag that is true if the app has sent data since the second previous RTCP Report was transmitted */
	u_int initial;	/* the flag that is true if the app has not yet sent an RTCP pkt */
	u_int sentpacketall;
	u_int sentbyteall;
	u_int sentpacket;
	u_int sentbyte;
};

typedef struct tag_SSRC_STATES
{
	u_int receivedall;    /* all the received packet from session start*/
	u_int received;       /* the received packet from last SR/RR sent */
	u_int receivedbyteall; /* all the received rtp payload bytes from session start*/
	u_int receivedbyte;    /* the received rtp payload bytes from last SR/RR sent */	
	u_short base_seq;       /* the seq of first packet from this source */
	u_short curr_seq;	/* the current sequence of received packet */
	u_short seq_cycles;	/* shifted count of seq. number cycles */
	u_int last_srrr_seq;	/* the sequence of last SR/RR sent */
	
	u_int currrtptime;	/* rtp time of last rtp packet */
	u_int currrtptime_reach; /* reach rtp time of last rtp packet */
	u_int jitter;         /* estimated jitter */
	
	u_int brecvsr;        /* identify of receive SR packet */
	struct timeval lastsr;  /* last RTCP SR pkt reception time */
	u_int ntp_lastsr[2];  /* last RTCP SR pkt NTP reception time */
}SSRC_STATES;

typedef struct tag_SSRC_SDES
{
	
#define RTCP_SDES_STR_MAX_SIZE    255
#define RTCP_SDES_STR_NAME_SIZE   23
#define RTCP_SDES_STR_EMAIL_SIZE  47
#define RTCP_SDES_STR_PHONE_SIZE  23
#define RTCP_SDES_STR_LOC_SIZE    23
#define RTCP_SDES_STR_TOOL_SIZE   23
#define RTCP_SDES_STR_NOTE_SIZE   47

	char cname[RTCP_SDES_STR_MAX_SIZE+1];
	char name[RTCP_SDES_STR_NAME_SIZE+1];
	char email[RTCP_SDES_STR_EMAIL_SIZE+1];
	char phone[RTCP_SDES_STR_PHONE_SIZE+1];
	char loc[RTCP_SDES_STR_LOC_SIZE+1];
	char tool[RTCP_SDES_STR_TOOL_SIZE+1];
	char note[RTCP_SDES_STR_NOTE_SIZE+1];
}SSRC_SDES;

struct STREAM_SOURCE
{
	u_int ssrc;                     /* network byte order (big_endian) */
	struct sockaddr_in rtp_from;     /* network byte order (big_endian) */
	struct sockaddr_in rtcp_from;    /* network byte order (big_endian) */
	SSRC_STATES ssrc_states;
	SSRC_SDES ssrc_sdes;
	struct STREAM_SOURCE *next;	
};

typedef struct tag_RTP_REDUNDANT
{
#if BYTE_ORDER == BIG_ENDIAN
  u_int end:1;
  u_int blockpt:7;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  u_int blockpt:7;
  u_int end:1;
#endif

  u_int timeoffset_h8bits:8;

#if BYTE_ORDER == BIG_ENDIAN
  u_int timeoffset_l6bits:6;
  u_int blocklength_h2bits:2;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  u_int blocklength_h2bits:2;
  u_int timeoffset_l6bits:6;
#endif

  u_int blocklength_l8bits:8;

}RTP_REDUNDANT;

/* RFC2833 header */
/* 
   
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     event     |E|R| volume    |          duration             |  
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
              Payload Format for Named Events 
*/

typedef struct tag_RTP_NAMED_EVENT
{
  u_int event:8;

#if BYTE_ORDER == BIG_ENDIAN
  u_int end:1;
  u_int reserved:1;
  u_int volume:6;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  u_int volume:6;
  u_int reserved:1;
  u_int end:1;
#endif
  u_int duration:16;
}RTP_NAMED_EVENT;

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |    modulation   |T|  volume   |          duration             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |R R R R|       frequency       |R R R R|       frequency       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |R R R R|       frequency       |R R R R|       frequency       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ......

    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |R R R R|       frequency       |R R R R|      frequency        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                
                 Payload format for tones
*/

struct RTP_TONE_FREQUENCY
{
#if BYTE_ORDER == BIG_ENDIAN
  u_int reserved1:4;
  u_int frequency1_h4bits:4;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  u_int frequency1_h4bits:4;
  u_int reserved1:4;
#endif
  u_int frequency1_l8bits:8;

#if BYTE_ORDER == BIG_ENDIAN
  u_int reserved2:4;
  u_int frequency2_h4bits:4;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  u_int frequency2_h4bits:4;
  u_int reserved2:4;
#endif
  u_int frequency2_l8bits:8;

};


/* for supporting RFC2833, record all the dtmf event */
 
typedef struct tag_TONE_INFO
{
  u_int dealstatus; /* 0,1,2,3 */
  u_int modufreq;   /* modulation frequency*/
  u_int bdivmodu;   /* 1: modulation frequency divided by three */
  u_int volume;     /* 0 ~ -63 dbm0*/
  u_int timestamp;  /* timeoffset, first*/
  u_int duration;   /* duration */
#define RTP_MAX_ADDITIVE_FREQUENCY_COUNT 4
  u_int addfreq[4]; /* additive frequency */
}TONE_INFO;


typedef struct tag_NAMED_EVENT_INFO
{
#define RTP_NAMED_EVENT_NULL          0x0
#define RTP_NAMED_EVENT_WAITDEALING   0x1
#define RTP_NAMED_EVENT_DEALING       0x2
#define RTP_NAMED_EVENT_INTERVAL      0x3
  u_int dealstatus; /* 0,1,2,3 */
  u_int event;      /* 0~9 means dtmf '0'~'9', 10 means '*', 11 means '#' */
  u_int timestamp;  /* timestamp */
  u_int volume;     /* volume */
  u_int duration;   /* duration */
  struct tag_NAMED_EVENT_INFO *next;
}NAMED_EVENT_INFO;

struct RTP_SESSION
{
	u_int local_ssrc     /* host byte order */;
	int     rtpfd;
	int     rtcpfd;
	int     rtp4who;
	int     payloadtype;
#if 1 /* 2004112401 */	
	int     packettime;
#endif
	int     dspmsgtype;
	int     rtcpinterval;
#define	RTP_DIR_SENDRECV	0x0
#define	RTP_DIR_RECVONLY	0x1
#define	RTP_DIR_SENDONLY	0x2
#define	RTP_DIR_INACTIVE	0x3
	int	direction;

        u_int	rtptimestamp;
	u_short	tseq;

#ifdef RTP_DEBUG
	u_int  ttime;
#endif	

  u_int   dsptimestamp;
  struct timeval timedsptime;
#define RTP_SESSION_SPEC_SIZE 63        
        char    spec[RTP_SESSION_SPEC_SIZE+1];

        enum deliveries { unicast, multicast } delivery;
        enum modes { play, record } mode;

  u_int   dstaddr;       /* host byte order */
  u_short dstports[2];   /* host byte order */
  u_short srcports[2];   /* host byte order */
  struct SESSION_STATES sess_states;
  struct STREAM_SOURCE *pstream_source;
	void	*rtcptimer;
	void *video_image;
	int     remaining_data_size;
	u_int	audiochannel;
	u_int	videochannel;

  /*
   * for supporting RFC2833
   */
#define RTP_MIN_REDUNDANT_PAYLOAD 96
#define RTP_MAX_REDUNDANT_PAYLOAD 127
  u_int   redundant_pt;    /* rtp redundant payload type, range is 96~127*/
  u_int   namedevent_pt;   /* name event payload type , 96~127*/
#ifdef FORCE_RFC2833
  u_int   X_redundant_pt;  /* redundant payload type of remote */
  u_int   X_namedevent_pt; /* name event payload type of remote */
#endif
  u_int   tone_pt;         /* tone payload type ,value range is from 96~127*/
  NAMED_EVENT_INFO *tobesent_event, /* store single event wating for sending */
                   *sentevent_list,
                   *sentevent_last;
};

typedef struct RTP_SESSION rtp_session;

/* The following struct is only used by rtp module */

typedef struct tag_rtp_voice_data
{
	u_short len;			/* DSP message size in words (payload size + 3)*/
	u_short seq;        		/* sequential message number*/
  u_int pttype;     /* for supporting rfc2833, payload type maybe changed */
	u_int timestamp;		/* timestamp*/
  u_int mark;       /* marker bit */
	u_char *databuf;        /* actual data allocated by the driver */
} rtp_voice_data;

/*******************************************************************************/
/*                                                                             */
/*   The following is extern routine declaration of RTP/RTCP module.           */
/*                                                                             */
/*******************************************************************************/
#define ITU_H225_PT_PCMU   0
#define ITU_H225_PT_PCMA   8
#define ITU_H225_PT_G723_1 4
#define ITU_H225_PT_G729   18
#define ITU_H225_PT_G728   15
#define ITU_H225_PT_ADPCMA 49

////////////////////////////////////////
//added by zhchli 2006-03-08 --begin--
//note: add the G.726 codec support, using the value from GaoKe IP phone
        
#define ITU_H225_PT_G726_16    23 //DEF_VOIPPEERCODECTYPE_G726_16  // 23 
#define ITU_H225_PT_G726_24    22 //DEF_VOIPPEERCODECTYPE_G726_24  // 22
#define ITU_H225_PT_G726_32     2
#define ITU_H225_PT_G726_40    21 //DEF_VOIPPEERCODECTYPE_G726_40  // 21

//added by zhchli 2006-03-08 --end--
///////////////////////////////////////


#define ITU_H225_PT_MPEG4	100   //210
#define ITU_H225_PT_H263   	34
#define ITU_H225_PT_H264	98   	

#define RTP_FOR_H323  0
#define RTP_FOR_SIP   1

#define RTP_UNICAST   0
#define RTP_MULTICAST 1

typedef struct tag_CREATE_VIDEOIMAGE_PARAMS
{
	 u_int ssrc;
	 u_int pl;/*payload*/
	 u_int payloadtype;
	 u_int videophonetype;/*door or not*/
	 
}CREATE_VIDEOIMAGE_PARAMS;


typedef struct tag_CREATE_RTP_PARAMS
{
	u_short bvoiptype;  /* 0: audio only,   1: audio & video */
#define VOIP_TYPE_DEFAULT	0
#define VOIP_TYPE_VIDEO		1
	u_int mode;       /* 0: unicast,1: multicast */
	u_int daddr;
	u_short dport;
	u_short sport;
	u_char payloadtype;
	u_char dspmsgtype;
#if 1 /* 2004112401 */	
	u_char packettime;
#endif
//	u_char dspmsgtype;
	int     rtptos;
	int     rtcptos;
	int     direction;
	u_int bindwidth;
	u_int audiochannel;
	u_int videochannel;       	
	char spec[RTP_SESSION_SPEC_SIZE+1];
  /* for supporting rfc2833 */
  u_char  redundancy;
  u_char  telephone_event;
#ifdef FORCE_RFC2833
  u_char  X_redundancy;
  u_char  X_telephone_event;
#endif
}CREATE_RTP_PARAMS;

typedef struct tag_RTP_MEDIA_DATA
{
	char    *pDataBuf;
	/*
		nLen 变量主要在 rtp 库代码中维护，因为一帧可能分多个 rtp 
		包接收，在应用层需要重新组装，因此在一个包没有接收完 (返回 
		RTP_RCV_1FRAME 表示一个完整的帧收到了)的时候，不能把 nLen 的值改变
	*/
	int	nLen;
	int	nPayloadType;
	u_short usSeq;
	u_int   unTimestamp;
	u_char  ucMark; /* end flag for video */
}RTP_MEDIA_DATA;


/* for supporting rfc2833 */
typedef struct tag_dsp2rtp_eventinfo
{
  u_int dtmfdigit; /* 0~9 means dtmf '0'~'9', 10 means '*', 11 means '#'*/
  u_int timestamp;
  u_int duration;
  u_int volume;
}dsp2rtp_eventinfo;

/*add a struct as the para of callback function,supply application with pkt info.*/
/*pkt type defination.*/

#define  PKT_SND_MPEG4     1
#define  PKT_SND_H263      2
#define  PKT_RCV_RTCP      3
#define  PKT_RCV_RFC2833	4

typedef struct tag_PACKET_INFOS
{
	u_int type;       /*packet type such as PKT_RCV_RFC2833 PKT_RCV_RTCP */
	rtp_session *rtpsess;
	u_int fraction;         /*  fraction lost since last SR/RR */
	u_char event;/*dtmf*/
}PACKET_INFOS;


/*****************************************************************************
  RTP 模块的回调函数指针结构, RTP 模块需要 timer_XXX 函数指针作为支持函数,
  通过 process_pkt 函数指针，外部模块也可以获取 RTP 模块中的一些信息。

  
    创建 定时器函数指针   
  	void * (*timer_create) (void (*func)(), void *arg);

  	删除定时器函数指针
	void (* timer_delete) (void *timer);

	启动定时器函数指针
	void (* timer_start) (void *timer, unsigned int interval);

	停止定时器函数指针
	void (* timer_stop) (void *timer);
	

	处理数据包函数指针，RTP_RTCP 模块可能会在多个地方调用此函数，例如，
	  RTCP 模块收到 RR (接受者报告)的时候会调用此函数通知外部模块丢包的信息，
	  当 RTP 模块要发送数据包到网络的时候，也调用此接口通知外部模块有数据包
    发送出去
	  当 RTP 模块接收到 DTMF 码信息的时候，把 DTMF 码信息通知给调用模块;
	  外部模块可以根据 PACKET_INFOS 结构中的 type 字段判断是什么信息。
	  例如 PKT_RCV_RFC2833 就是表示收到了 DTMF 码信息

	  此函数的功能会根据具体的需求来定制。
	  
	int (* process_pkt) (PACKET_INFOS *pinfo);
*****************************************************************************/
typedef struct tag_RTP_CALLBACK	
{
	void * (*timer_create) (void (*func)(), void *arg);
	void (* timer_delete) (void *timer);
	void (* timer_start) (void *timer, unsigned int interval);
	void (* timer_stop) (void *timer);
	int (* process_pkt) (PACKET_INFOS *pinfo);/*for tw,if drop frame return >=0,or else return -1 */
#ifdef FOR_MUTIPLE_CALL
    char* (* getOffhookDevice) (void);
#endif
}RTP_CALLBACK;

/* SP_VideoImage_create  和  SP_VideoImage_delete 函数值针对富士通平台提供的 */
void * SP_VideoImage_create(CREATE_VIDEOIMAGE_PARAMS *pvi_params);
void  SP_VideoImage_delete(void* video_image);

struct RTP_SESSION * SP_RTP_create(CREATE_RTP_PARAMS *prtp_params);
long SP_RTP_delete (struct RTP_SESSION *psession);
RTP_MEDIA_DATA * SP_RTP_receiveRtpPacket (struct RTP_SESSION *psession);
long SP_RTP_receiveRtcpPacket(struct RTP_SESSION *psession);
long SP_RTP_sendRtpPacket (struct RTP_SESSION *psession, RTP_MEDIA_DATA *pPacket);
long SP_RTP_send_mpeg4_frame (struct RTP_SESSION *psession, RTP_MEDIA_DATA *pPacket);
void SP_RTP_setCallbackFunc (RTP_CALLBACK *pCallback);
int SP_RTP_receive_mpeg4_packet(struct RTP_SESSION *psession, char *video_buf, int *buf_len);
int SP_RTP_receive_h263_packet(struct RTP_SESSION *psession, char *video_buf, int *buf_len,u_int *timestamp,u_int *seq);
long SP_RTP_send_h263_frame(struct RTP_SESSION* psession, RTP_MEDIA_DATA *pPacket);
long SP_RTP_send_h263_packet(struct RTP_SESSION* psession, RTP_MEDIA_DATA *pPacket);
int SP_RTP_receive_h264_packet(struct RTP_SESSION *psession, char *video_buf, int *buf_len,u_int *timestamp,u_int *seq);
long SP_RTP_send_h264_frame(struct RTP_SESSION* psession, RTP_MEDIA_DATA *pPacket);
/* rfc2833 supporting */
int
rtp_rfc2833_set_payload_type(struct RTP_SESSION* psess,
                             u_int redundant_pt, u_int event_pt,
#ifdef FORCE_RFC2833
                             u_int X_redundant_pt, u_int X_event_pt,
#endif
                             u_int tone_pt, u_int rate);
void rtp_rfc2833_receivepacket(struct RTP_SESSION* psess,char *buff,
                                    int headerlen,int payloadlen,int dspfd);
int rtp_rfc2833_sendpacket(struct RTP_SESSION* psess,
                           u_int timestamp,u_int event,u_int stop);
int rtp_rfc2833_addevent2audiodata(struct RTP_SESSION* psess,char *buff,
                                   u_int datatimestamp,u_int payloadtype);

#endif

