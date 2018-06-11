/*****************************************************************************
/      
/	#         #####    #     #    #     #    #     #
/	#           #      ##    #    #     #     #   #
/	#           #      # #   #    #     #      # #
/	#           #      #  #  #    #     #       #
/	#           #      #   # #    #     #      # #
/	#           #      #     #    #     #     #   #
/	#######   #####    #     #     #####     #     #
/
/	 Copyright (c) 2001 - 2005	UDTech Corporation
*****************************************************************************
/
/ Author:	shen
/
/ File name:	product.h 2003/08
/	
/ Abstruct:	VideoPhone product definitions file header
/
*****************************************************************************/
#ifndef _PRODUCT_H_
#define _PRODUCT_H_

#define LINUX
#define FORCE_RFC2833
#define RTP				/* RTP				*/
#define RTP_RESOLUTION_TIME 125		/* 125us = 8KHz sampling rate	*/

/*added by shenhang for pdk*/
#define	LITTLE_ENDIAN	1234	/* LSB first: i386, vax */
#define	BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net */
//#define BYTE_ORDER	BIG_ENDIAN//LITTLE_ENDIAN
#define BYTE_ORDER	LITTLE_ENDIAN

#ifndef	min
#define	min(a,b) (((a)<(b))?(a):(b))
#define	max(a,b) (((a)>(b))?(a):(b))
#endif

#define BLKNUM	5
#define MAXITEMNUM	15


#ifdef MEMORY_CHECK /* 2004123100 */
#include "mem.h"

#define malloc(size)			my_malloc(size, __FILE__, __LINE__)
#define realloc(ptr, size) 		my_realloc(ptr, size, __FILE__, __LINE__)
#define free(p)				my_free(p, __FILE__, __LINE__)
#endif

#endif  /* _PRODUCT_H_ */
