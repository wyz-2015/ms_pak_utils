#ifndef _PKLZ_H
#define _PKLZ_H

#include "common.h"

typedef struct PKLZ_Header { // [0x00, 0x10)	PKLZ文件头部
	char magicStr[3];    // [0x00, 0x03)	"PK\0"
	uint8_t type;	     // 0x03		0x00未压缩、0x02压缩
	uint32_t decompSize; // [0x04, 0x08)	解压后大小
	uint64_t zero;	     // [0x08, 0x10)	0
} PKLZ_Header;

#endif
