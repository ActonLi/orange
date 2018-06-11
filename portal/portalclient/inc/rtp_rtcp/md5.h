/******************************************************************************
*
* Author:	H.Yanai
*
* File name:	md5.h
*	
* Abstruct:	IPsec definitions for MD5.
*
******************************************************************************
* Revision History
*
* Modified   Rev  by	contents
* ---------  ---  ---	-------------------------------------------
* 19-Dec-97  1.0  YAN	released
*
******************************************************************************/

/* md5.h: "RSA Data Security, Inc. MD5 Message-Digest Algorithm"
        (header file for MD5C.C)

Portions of this software are Copyright 1995 by Randall Atkinson and Dan
McDonald, All Rights Reserved. All Rights under this copyright are assigned
to the U.S. Naval Research Laboratory (NRL). The NRL Coyright Notice and
License Agreement applies to this software.

        History:

        Modified at NRL for OPIE 2.0.
        Originally from RSADSI reference code.
*/
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef _NETSEC_MD5_H
#define _NETSEC_MD5_H 1

#ifndef __P
#  define __P(protos)	protos /*videophone*/
#endif

/* MD5 context. */
typedef struct {
  u_int32_t state[4];	/* state (ABCD) */
  u_int32_t count[2];	/* number of bits, modulo 2^64 (lsb first) */
  u_int8_t  buffer[64];	/* input buffer */
}MD5_CTX;

void MD5Init __P((MD5_CTX *));
void MD5Update __P((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final __P((unsigned char[16], MD5_CTX *));
#endif /* _NETSEC_MD5_H */
