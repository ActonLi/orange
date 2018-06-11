/**
 * Title:	declaration of the structures and functions
 * Copyright:	Copyright (c) 2005
 * Company:	UD tech
 * @author:	djw
 * @version:	1.0  2005-03-21
 * @file:	video_image_b.h
 * @brief:	stuctures and functions of resolving disorder.
 */

#ifndef VIDEO_IMAGE_B_H_
#define VIDEO_IMAGE_B_H_

#include "video_image_f.h"
#include "rtp_all.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef  int (*video_is_beyond_buffers_size_t) (const osip_list_t* image_frames, 
												const RTP_HEADER* rtp_head, 
												const void* limit);
typedef void (*video_process_extend_buffers_t) (osip_list_t* image_frames, 
												const RTP_HEADER* rtp_head, 
												const void* limit);
typedef int (*video_process_rtp_package_t) (osip_list_t* image_frames, 
											const RTP_HEADER* rtp_head,
											const char* rtp_data, 
											int datalen);
typedef int (*video_is_image_over_t) (const VIDEO_IMAGE_FRAME* one_frame);

typedef struct video_images_buffer_before_decode
{
   video_is_beyond_buffers_size_t  judge_buffers;
   video_process_extend_buffers_t  process_buffers;
   video_process_rtp_package_t   process_rtp_pkgs;
   video_is_image_over_t  judge_frame;
   u_int ssrc;         /* synchronization source */
   u_int payload;         /* payload type */
   void* limit;
   osip_list_t video_image_frames;
}VIDEO_IMAGES_BUFFER;

/**api for VIDEO_IMAGES_BUFFER*/
int video_image_b_init(VIDEO_IMAGES_BUFFER** image_buffer);

void video_image_b_free(VIDEO_IMAGES_BUFFER* image_buffer);

void video_image_b_set_ssrc(VIDEO_IMAGES_BUFFER* image_buffer, u_int ssrc);

u_int video_image_b_get_ssrc(const VIDEO_IMAGES_BUFFER* image_buffer);

void video_image_b_set_payload(VIDEO_IMAGES_BUFFER* image_buffer, u_int pl);

u_int video_image_b_get_payload(const VIDEO_IMAGES_BUFFER* image_buffer);

#ifdef __cplusplus
}
#endif

#endif
