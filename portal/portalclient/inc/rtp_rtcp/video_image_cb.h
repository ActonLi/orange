/**
 * Title:	define callback api functions
 * Copyright:	Copyright (c) 2005
 * Company:	UD tech
 * @author:	djw
 * @version:	1.0  2005-03-21
 * @file:	video_image_cb.h
 * @brief:	
 */

#ifndef VIDEO_IMAGE_CB_H_
#define VIDEO_IMAGE_CB_H_

#define TIME_SIZE 3
#define LIST_SIZE 2

#include "video_image_b.h"

#ifdef __cplusplus
extern "C"
{
#endif

int video_is_beyond_buffers_size(const osip_list_t* image_frames, 
								 const RTP_HEADER* rtp_head, 
								 const void* limit);

void video_process_extend_buffers(osip_list_t* image_frames, 
								  const RTP_HEADER* rtp_head, 
								  const void* limit);

int video_is_judge_buffers_count(const osip_list_t* image_frames, 
								 const RTP_HEADER* rtp_head, 
								 const void* limit);

void video_process_extend_count_buffers(osip_list_t* image_frames, 
								        const RTP_HEADER* rtp_head, 
								        const void* limit);

int video_process_rtp_package(osip_list_t* image_frames, 
	                                                     const RTP_HEADER* rtp_head, 
	                                                     const char* rtp_data, 
	                                                     int datalen);

int video_process_263_rtp_package(osip_list_t* image_frames, 
	                                                              const RTP_HEADER* rtp_head, 
	                                                              const char* rtp_data, 
	                                                              int datalen);
	                                                              
int video_is_image_over(const VIDEO_IMAGE_FRAME* one_frame);

#ifdef __cplusplus
}
#endif

#endif
