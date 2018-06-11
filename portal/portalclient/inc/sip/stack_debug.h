#ifndef __STACK__DEBUG_H__
#define __STACK__DEBUG_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define CALLIN_FIRST_DISPLAY_FW                  //fuwei 090728
#define FOR_GERMANY_AMEND                        //daniel 110317
#define FIRST_SIP_AFTER_RTP                      //daniel 110402
#define FOR_MUTIPLE_CALL                         //daniel 120222

//add by lewis
//#define CHECK_RTP_SSRC          //check ssrc for rtp package  
//#define SUPPORT_MUTICAST        //recevice muticast from sip port

#define DEBUG_H323_INFO   0
#undef DEBUG_H323_INFO

#define DEBUG_SIP_SIPMW_INTF   0 
#define DEBUG_VIDEO_NETWORK_PAYLOAD   0

//#define UDTECH_USE_IDEA

#define  ENABLE_RTP_DEBUG  0
#define  ENABLE_PROC_DEBUG 0
#define  ENABLE_ERR_DEBUG  0

#define  ENABLE_H323_DEBUG  0
#define  ENABLE_H323_ERR   0
#define  ENABLE_NAT_DEBUG  0
#define  ENABLE_SIP_DEBUG  0
#define  ENABLE_SDP_DEBUG  0
#define  ENABLE_FUNC_DEBUG  0  /* function debug , for temporary debug code  */
#define  ENABLE_H323_INTF   0
#define  ENABLE_SIP_INTF   0
#define  ENABLE_RTCP_ERR   0
#define  ENABLE_H263_DEBUG   0
#define  ENABLE_ABB_DEBUG   0
#define  ENABLE_HQH_DEBUG   0
#define  ENABLE_LWX_DEBUG   0
//#define  SUPPORT_9INCH

#if ENABLE_RTP_DEBUG
#define DEBUG_RTP(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_RTP(Fmt...) do { } while (0)
#endif


#if ENABLE_PROC_DEBUG
#define DEBUG_PROC(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_PROC(Fmt...) do { } while (0)
#endif

#if ENABLE_ERR_DEBUG
#define DEBUG_ERR(Fmt...) do { printf("ERR ***%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_ERR(Fmt...) do { } while (0)
#endif

#if ENABLE_H323_DEBUG
#define H323_INFO(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  H323_INFO(Fmt...) do { } while (0)
#endif


#if ENABLE_H323_ERR
#define H323_ERR(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  H323_ERR(Fmt...) do { } while (0)
#endif



#if ENABLE_NAT_DEBUG
#define DEBUG_NAT(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_NAT(Fmt...) do { } while (0)
#endif


#if ENABLE_SIP_DEBUG
#define DEBUG_SIP(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_SIP(Fmt...) do { } while (0)
#endif


#if ENABLE_SDP_DEBUG
#define DEBUG_SDP(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_SDP(Fmt...) do { } while (0)
#endif



#if ENABLE_FUNC_DEBUG
#define DEBUG_FUNC(Fmt...) do { printf("++++%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_FUNC(Fmt...) do { } while (0)
#endif

#if ENABLE_H323_INTF
#define DEBUG_H323_INTF(Fmt...) do { printf("--  %s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_H323_INTF(Fmt...) do { } while (0)
#endif


#if ENABLE_SIP_INTF
#define DEBUG_SIP_INTF(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_SIP_INTF(Fmt...) do { } while (0)
#endif

#if ENABLE_RTCP_ERR
#define DEBUG_RTCP_ERR(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_RTCP_ERR(Fmt...) do { } while (0)
#endif


#if ENABLE_H263_DEBUG
#define DEBUG_H263(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_H263(Fmt...) do { } while (0)
#endif

#if ENABLE_ABB_DEBUG
#define DEBUG_ABB(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_ABB(Fmt...) do { } while (0)
#endif

#if ENABLE_HQH_DEBUG
#define DEBUG_HQH(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_HQH(Fmt...) do { } while (0)
#endif

#if ENABLE_LWX_DEBUG
#define DEBUG_LWX(Fmt...) do { printf("--%s() %d:", __FUNCTION__,__LINE__); printf Fmt;}while (0)
#else   
#define  DEBUG_LWX(Fmt...) do { } while (0)
#endif


#endif

