/**
 * Title:	declaration of API of resolving dis-order.
 * Copyright:	Copyright (c) 2005
 * Company:	UD tech
 * @author:	djw
 * @version:	1.0  2005-03-21
 * @file:	video_image_api.h
 * @brief:	
 */

#ifndef VIDEO_IMAGE_API_H_
#define VIDEO_IMAGE_API_H_

#include "video_image_b.h"

#ifdef __cplusplus
extern "C"
{
#endif

VIDEO_IMAGES_BUFFER* video_create_buffer(u_int ssrc, u_int pl);

void video_destroy_buffer(VIDEO_IMAGES_BUFFER* video_image_buf);

void video_set_fun_judge_buf(VIDEO_IMAGES_BUFFER* video_image_buf, 
							 video_is_beyond_buffers_size_t func);

void video_set_fun_process_buf(VIDEO_IMAGES_BUFFER* video_image_buf, 
							   video_process_extend_buffers_t func);

void video_set_fun_process_rtp(VIDEO_IMAGES_BUFFER* video_image_buf, 
							   video_process_rtp_package_t func);

void video_set_fun_judge_frame(VIDEO_IMAGES_BUFFER* video_image_buf,
							   video_is_image_over_t func);

void video_set_limit(VIDEO_IMAGES_BUFFER* video_image_buf, void* limit);

int video_process_one_rtp(VIDEO_IMAGES_BUFFER* video_image_buf, 
						  const RTP_HEADER* rtp_head,
						  const char* rtp_data, 
						  int datalen);

int video_get_one_frame(VIDEO_IMAGES_BUFFER* video_image_buf, 
						char* video_buf, int* buf_len, 
						u_int* is_complete);

int video_get_one_frame_list(VIDEO_IMAGES_BUFFER* video_image_buf, 
						VIDEO_IMAGE_FRAME** frame_buf,
						u_int* is_complete);

#ifdef __cplusplus
}
#endif


#endif
