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
 * File name:	rtp_all.h 2003/08
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

#ifndef _RTP_ALL_H
#define _RTP_ALL_H

#if defined(WIN32)   /* WIN32 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <windows.h>
#include <process.h>
void bzero(void *s, size_t n);

#else

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h> /*viedophone*/
#include <sys/fcntl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "rtp_rtcp.h"

#endif
typedef unsigned short		u_int16;
typedef unsigned int		u_int32;
typedef unsigned char		u_int8;
/*
   Current protocol version.
*/
#define RTP_VERSION    2
#define RTP_SEQ_MOD (1<<16)
extern RTP_CALLBACK g_rtp_callback;

/* 用于 RTCP PT 字段  */
typedef enum
{
    RTCP_TYPE_SR   = 200,/* 发送报告  */
    RTCP_TYPE_RR   = 201,/* 接收报告  */
    RTCP_TYPE_SDES = 202,/* 源描述项  */
    RTCP_TYPE_BYE  = 203,/* 退出会议  */
    RTCP_TYPE_APP  = 204/* 面向应用的功能  */
} rtcp_type_t;

typedef enum 
{
       RTCP_SDES_END   = 0,
       RTCP_SDES_CNAME = 1,
       RTCP_SDES_NAME  = 2,
       RTCP_SDES_EMAIL = 3,
       RTCP_SDES_PHONE = 4,
       RTCP_SDES_LOC   = 5,
       RTCP_SDES_TOOL  = 6,
       RTCP_SDES_NOTE  = 7,
       RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_t;


/*
      RFC  1889  5.1 
      The RTP header has the following format:    
 0                   1                   2                   3    
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
|                           timestamp                           |   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
|           synchronization source (SSRC) identifier            |   
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+   
|            contributing source (CSRC) identifiers             |   
|                             ....                              |   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


5.3.1 RTP Header Extension

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
|      defined by profile       |           length              |   
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
|                        header extension                       |   
|                             ....                              |

*/
/* RTP 头部数据结构  */
typedef struct tag_RTP_HEADER
{
#if BYTE_ORDER == BIG_ENDIAN
	u_int ver:2;        /* protocol version */ 
	u_int pad:1;        /* padding flag */
	u_int ext:1;        /* header extension flag */
	u_int cc:4;         /* CSRC count */
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int cc:4;   
	u_int ext:1;
	u_int pad:1;
	u_int ver:2;
#endif
#if BYTE_ORDER == BIG_ENDIAN	
	u_int mark:1;       /* marker bit 如果是当前帧的结尾则设置为1 否则为0 */
	u_int pt:7;         /* payload type 负载类型 参见 RFC 3551 */
#endif    
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int pt:7;
	u_int mark:1;
#endif
	u_int seq:16;       /* sequence number */
	u_int time;         /* timestamp */
	u_int ssrc;         /* synchronization source */
	u_int csrc[1];      /* optional CSRC list */
}RTP_HEADER;


/* 

    RFC  2190  RTP Payload Format for H.263 Video Streams:
    section 5:
    每个 RTP 数据包只承载1个 H.263 视频数据包. H.263 payload Header
    出现在视频流数据之前，有三种模式的数据包

    模式 A:
    
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |F|P|SBIT |EBIT | SRC |I|U|S|A|R      |DBQ| TRB |    TR         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   

*/

typedef struct tag_RTP_H263_HEADER
{
#if BYTE_ORDER == BIG_ENDIAN
	unsigned int	F:1;  /* 标志位，表示有效载荷头的模式，具有以下几个值：0 ― 模式 A；1 ― 模式 
                          B 或模式 C，取决于 P 位。   */
	unsigned int	P:1;  /*  P 指定可选的 PB 帧模式。  */
	unsigned int	SBIT:3;  /* 起始位，指定最重要位（在第一个数据字节中忽略）的编号。  */
	unsigned int	EBIT:3;	/* 结束位，指定最不重要位（在最后一个数据字节中忽略）的编号。  */
	unsigned int	SRC:3;  /* 源格式 (H.263 标准中 PTYPE 的位6、7和8压缩比特流)表示当前图片解析度 */
	unsigned int	I:1; /* Picture coding type "0" is   intra-coded, "1" is inter-coded. 1 表示i 帧 */
	unsigned int	U:1;
	unsigned int	S:1;
	unsigned int	A:1;
	unsigned int	R:4;
	unsigned int	DBQ:2;
	unsigned int	TRB:3;
	unsigned int	TR:8;
#else
	unsigned int   TR:8;
	unsigned int   TRB:3;
	unsigned int   DBQ:2;
	unsigned int   R:4;
	unsigned int   A:1;
	unsigned int   S:1;
	unsigned int   U:1;
	unsigned int   I:1;
	unsigned int   SRC:3;
	unsigned int   EBIT:3;
	unsigned int   SBIT:3;
	unsigned int   P:1;
	unsigned int   F:1;	
#endif
}RTP_H263_HEADER;

//add by katy start
typedef struct tag_RTP_H264_HEADER
{
#if BYTE_ORDER == BIG_ENDIAN
	unsigned char	F:1;
	unsigned char	NRI:2;
	unsigned char	Type:5;
#else
	unsigned char	Type:5;
	unsigned char	NRI:2;
	unsigned char	F:1;
#endif
}RTP_H264_HEADER;
//add by katy end
/*
    RTCP common header word
*/
typedef struct tag_RTCP_COMMON_HEADER
{
#if BYTE_ORDER == BIG_ENDIAN
       u_int ver:2;         /* protocol version */
       u_int pad:1;         /* padding flag */
       u_int count:5;       /* varies by packet type */
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
       u_int count:5;       /* varies by packet type */
       u_int pad:1;         /* padding flag */
       u_int ver:2;         /* protocol version */
#endif       
       u_int pt:8;          /* RTCP packet type */
       u_int length:16;     /* pkt len in words, w/o this word */
} RTCP_COMMON_HEADER;


/*
    Mask for version, padding bit and packet type pair, and ntolh24
*/
#if BYTE_ORDER == BIG_ENDIAN
#define RTCP_VALID_MASK ( 0xc000 | 0x2000 | 0xfe ) /* ver | pad | pt */
#define RTCP_VALID_VALUE ( (RTP_VERSION << 14) | RTCP_SR )
#define ntohl24(x) (x)
#define htonl24(x) (x)
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define RTCP_VALID_MASK ( 0xfe00 | 0x20 | 0xc0 ) /* pt | pad | ver |*/
#define RTCP_VALID_VALUE ( ( RTCP_SR << 8 ) | (RTP_VERSION << 6) )
#define ntohl24(x) (((x&0xff) << 16) | (x&0xff00) | ((x&0xff0000)>>16))
#define htonl24(x) (((x&0xff) << 16) | (x&0xff00) | ((x&0xff0000)>>16))
#endif


/*
    RTCP Reception report block
*/
typedef struct tag_RTCP_RR
{
       u_int ssrc;             /* data source being reported */
       u_int fraction:8;       /* fraction lost since last SR/RR */
       int lost:24;              /* cumul. no. pkts lost (signed!) */
       u_int last_seq;         /* extended last seq. no. received */
       u_int jitter;           /* interarrival jitter */
       u_int lsr;          /* last SR packet from this source */
       u_int dlsr;             /* delay since last SR packet */
} RTCP_RR;


/*
    RTCP SDES item
*/
typedef struct tag_RTCP_SDES_ITEM
{
       u_char type;              /* type of item (rtcp_sdes_type_t) */
       u_char length;            /* length of item (in octets) */
       char data[1];             /* text, not null-terminated */
} RTCP_SDES_ITEM;


/*
    One RTCP packet
*/
typedef struct tag_RTCP_PKT
{
       RTCP_COMMON_HEADER common;     /* common header */
       union 
       {
           /* sender report (SR) */
           struct rtcp_sr
	   {
               u_int ssrc;     /* sender generating this report */
               u_int ntp_sec;  /* NTP timestamp */
               u_int ntp_frac;
               u_int rtp_ts;   /* RTP timestamp */
               u_int psent;    /* packets sent */
               u_int osent;    /* octets sent */
               RTCP_RR rr[1];    /* variable-length list */
           } sr;

           /* reception report (RR) */
           struct 
	   {
               u_int ssrc;     /* receiver generating this report */
               RTCP_RR rr[1];    /* variable-length list */
           } rr;

           /* source description (SDES) */
           struct rtcp_sdes 
	   {
               u_int ssrc;      /* first SSRC/CSRC */
               RTCP_SDES_ITEM item[1]; /* list of SDES items */
           } sdes;

           /* BYE */
           struct 
	   {
               u_int ssrc[1];   /* list of sources */
               /* can't express trailing text for reason */
           } bye;
       } r;
}RTCP_PKT;

typedef struct rtcp_sdes RTCP_SDES;

/*******************************************************************************/
/*                                                                             */
/*   The following define and data structure for control rtp session.          */
/*                                                                             */
/*******************************************************************************/


/* Data structure of saving result of parse rtcp packet  */

#include "rtp_rtcp.h"
#include "rtp_log.h"

typedef struct tag_combi_rtcp_info
{
	int type;	/* RTCP packet type SR,RR,SDES,BYE,APP*/
	int offset;     /* the rtcp packet offset in buff */
	int len;	/* the length of this rtcp packet */
}COMBI_RTCP_INFO;

/* Some definition  */

#define RTP_PACKET_MAX_SIZE  (1024*24)
//#define RTP_PACKET_MAX_SIZE 1460
#define RTCP_PACKET_MAX_SIZE 1460
#define RTCP_COMBINATION_MAX_COUNT 32

/* videophone */
#define RTP_VIDEO_HEADER	-1
#define RTP_VIDEO_PACKET_NORMAL	0
#define RTP_VIDEO_PACKET_END	1

/* Declaration of global variable of rtp-rtcp module */

/* Declaration of internal routine */

unsigned long rtp_ramdom32(int type,int seed1,int seed2);
extern int rtp_hdr_val_chk(struct RTP_SESSION *psess,unsigned char* pkt,int len,int *pheaderlen,int *pdatalen);

int rtp_ssrc_check(struct RTP_SESSION *psess,unsigned char* pktbuf,struct sockaddr_in* pfrom,struct STREAM_SOURCE **ppstream);
int rtp_deal_ssrc_collision_or_loop(struct RTP_SESSION *psess);
int rtp_record_packet_info(struct RTP_SESSION *psess,struct STREAM_SOURCE *pstream,unsigned char *pktbuf,int datalen);
unsigned long rtp_get_pkt_reach_rtptime(struct RTP_SESSION *psess);

int rtcp_pkt_val_chk(struct RTP_SESSION *psess,unsigned char* pkt,int len);
int rtcp_pkt_ssrc_check(struct RTP_SESSION *psess,unsigned char* pkt,struct sockaddr_in* pfrom);
int rtcp_pkt_parse(struct RTP_SESSION *psess,unsigned char* pktbuf);
int rtcp_pkt_sr_parse(struct RTP_SESSION *psess,unsigned char *pbuf);
int rtcp_pkt_rr_parse(struct RTP_SESSION *psess,unsigned char *pbuf);
int rtcp_pkt_sdes_parse(struct RTP_SESSION *psess,unsigned char *pbuf);
int rtcp_pkt_bye_parse(struct RTP_SESSION *psess,unsigned char *pbuf);
int rtcp_send_bye_or_emptyrr_pkt(struct RTP_SESSION* psess,int bbyepkt);

int rtcp_send_bye_pkt(struct RTP_SESSION* psess);
int rtcp_send_pkt(struct RTP_SESSION* psession);
int rtcp_get_curr_ntp_rtp_time(struct RTP_SESSION* psess,unsigned long *phntpt,unsigned long *plntpt,unsigned long *prtpt);
int rtprtcp_get_curr_time(struct timeval *ptv);

struct STREAM_SOURCE* rtp_stream_source_chain_add(struct RTP_SESSION *psess);
int rtp_stream_source_chain_del(struct RTP_SESSION *psess,struct STREAM_SOURCE* oldstream);
int rtp_stream_source_chain_get_len(struct RTP_SESSION *psess);
struct STREAM_SOURCE* rtp_stream_source_match_ssrc(struct RTP_SESSION *psess,unsigned long ssrc);
unsigned long rtp_stream_source_calc_sum_recv(struct RTP_SESSION *psess);
int rtp_stream_source_calc_recv_source(struct RTP_SESSION *psess);
int rtp_stream_source_calc_rr(struct RTP_SESSION *psess,RTCP_RR *prr);

#endif 
