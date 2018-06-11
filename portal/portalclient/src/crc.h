#ifndef _H_CRC_
#define _H_CRC_
#include "define.h"

extern void crc_calcvalue(U8 val, U8* crc);
extern void crc_calc(U8* msg, U16 length, U8* crc);
#endif
