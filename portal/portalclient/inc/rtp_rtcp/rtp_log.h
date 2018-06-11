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
 * File name:	rtp_log.h 2003/08
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

#ifndef	_RTP_LOG_H_
#define	_RTP_LOG_H_

#include "rtp_rtcp.h"

#ifdef WIN32
void rtp_log(int id, long logc, char *format, ...);
void rtp_DEBUG_SIP(char *format, ...);
void rtp_dmp(char *str, char *data, int len);
	#define LOGC_ERROR 100
	#define LOGC_INFO  0
#else
	//#define	errno	get_errno()
#endif


#define RTP_CREATESESSION			0xc000
#define RTP_DELETESESSION			0xc200
#define RTP_RECEIVEPACKET			0xc300
#define RTP_SENDPACKET				0xc400
#define RTP_OPENUDPSOCKET			0xc500
#define RTP_HDRVALCHK				0xc600				
#define RTP_SSRCCHECK				0xc700
#define RTP_DEALSSRCCOLLISIONORLOOP		0xc800
#define RTP_RECORDPACKETINFO			0xc900

#define RTP_STREAMSOURCECHAINADD		0xca00
#define RTP_STREAMSOURCECHAINDEL		0xcb00
#define RTP_STREAMSOURCECHAINGETLEN		0xcc00
#define RTP_STREAMSOURCECHAINMATCHSSRC		0xcd00
#define RTP_STREAMSOURCECALUSUMRECV             0xcf00

#define RTCP_RECEIVEPACKET			0xd000
#define RTCP_TIMER				0xd100
#define RTCP_PKTVALCHK				0xd200
#define RTCP_PKTSSRCCHECK			0xd300
#define RTCP_PKTPARSE				0xd400
#define RTCP_PKTSRPARSE				0xd500
#define RTCP_PKTRRPARSE				0xd600
#define RTCP_PKTSDESPARSE			0xd700
#define RTCP_PKTBYEPARSE			0xd800

#define RTCP_SENDBYEPKT				0xd900
#define RTCP_SENDPKT				0xda00             

#define RTP_RFC2833SETPAYLOADTYPE       0xdb00
#define RTP_RFC2833STIMER               0xdc00
#define RTP_RFC2833RECEIVEPACKET        0xde00
#define RTP_RFC2833SENDPACKET           0xdf00
#define RTP_RFC2833ADDEVENT2AUDIODATA   0xe000

#endif	/* _RTP_LOG_H_ */
