/**
 * Title:	declaration of the structures and functions
 * Copyright:	Copyright (c) 2005
 * Company:	UD tech
 * @author:	djw
 * @version:	1.0  2005-03-21
 * @file:	video_image_f.h
 * @brief:	stucture and functions of manipulating image packages.
 */

#ifndef VIDEO_IMAGE_F_H_
#define VIDEO_IMAGE_F_H_

#define DEFAULT_START_SEQ 0xffffffff
#define DEFAULT_END_SEQ 0xffffffff
#define MAX_SEQ 0xffff
#define NO_COMPLETE 0
#define NORMOL_COMPLETE 1
#define ABNORMOL_COMPLETE 2
#define REVERSE_FLAG 10000

#include "osip_list.h"
#include "rtp_all.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**TODEL: maybe define in other files*/

#ifndef PLATFORM_CX92755
typedef unsigned int u_int;
#endif
/**structure of a image package*/
typedef struct video_image_p_info
{
   u_int seq;          /*seqence number of the package*/
   char* data;         /*the point to video data*/
   u_int data_len;
}VIDEO_IMAGE_PACKAGE;

/**structure of a image frame*/
/*
image frame 的结构，表示一帧数据 
其中 video_image_pkgs 是一个链表表头，链接的是  VIDEO_IMAGE_PACKAGE 数据链表
*/
typedef struct video_image_f_info
{
   u_int timestamp;         /*image timestamp */
   u_int start_seq;          /*start seqence number of the frme*/
   u_int end_seq;           
   u_int pkgs;
   u_int is_complete;        /*Is the frome complete, 0:no,1:normal complete,2:non normal*/
   osip_list_t video_image_pkgs;
}VIDEO_IMAGE_FRAME;

/**api for VIDEO_IMAGE_PACKAGE*/
int video_image_p_init(VIDEO_IMAGE_PACKAGE** image_package);

void video_image_p_free(VIDEO_IMAGE_PACKAGE* image_package);

void video_image_p_set_seq(VIDEO_IMAGE_PACKAGE* image_package, u_int seq);

u_int video_image_p_get_seq(const VIDEO_IMAGE_PACKAGE* image_package);

int video_image_p_set_data(VIDEO_IMAGE_PACKAGE* image_package, 
						   const char* data, u_int datalen);

int video_image_p_get_data(const VIDEO_IMAGE_PACKAGE* image_package, 
						   char* data, u_int* datalen);

/**api for VIDEO_IMAGE_FRAME*/
int video_image_f_init(VIDEO_IMAGE_FRAME** image_frame);

void video_image_f_free(VIDEO_IMAGE_FRAME* image_frame);

void video_image_f_set_timestamp(VIDEO_IMAGE_FRAME* image_frame, 
								 u_int timestamp);

u_int video_image_f_get_timestamp(const VIDEO_IMAGE_FRAME* image_frame);

void video_image_f_set_startseq(VIDEO_IMAGE_FRAME* image_frame, 
								u_int start_seq);

u_int video_image_f_get_startseq(const VIDEO_IMAGE_FRAME* image_frame);

void video_image_f_set_endseq(VIDEO_IMAGE_FRAME* image_frame, 
							  u_int end_seq);

u_int video_image_f_get_endseq(const VIDEO_IMAGE_FRAME* image_frame);

void video_image_f_set_pkgcount(VIDEO_IMAGE_FRAME* image_frame, 
								u_int pkgcount);

void video_image_f_addone_pkgcount(VIDEO_IMAGE_FRAME* image_frame);

u_int video_image_f_get_pkgcount(const VIDEO_IMAGE_FRAME* image_frame);

void video_image_f_set_compflag(VIDEO_IMAGE_FRAME* image_frame, 
								u_int compflag);

u_int video_image_f_get_compflag(const VIDEO_IMAGE_FRAME* image_frame);

int video_image_f_add_package(VIDEO_IMAGE_FRAME* image_frame, 
							             const RTP_HEADER* rtp_head, 
	                                                       const char* rtp_data, 
	                                                       int datalen);

#ifdef __cplusplus
}
#endif


#endif

