#ifndef _DECOMPRESS_H
#define _DECOMPRESS_H

#include "args.h"
#include "common.h"
#include "pklz.h"

typedef struct Decomper {
	const Args* args;

	PKLZ_Header* header;

	uint32_t fileSize;	  // 整个文件大小
	uint32_t decompSize;	  // 读头所得解压后大小
	uint32_t decompSize_real; // 实际解压后大小
	uint8_t type;		  // 文件类型，'\x00' '\x02'

	FILE* inFile;
	FILE* outFile;
} Decomper;

// 检验：魔术字为"PK"(带'\0')，且dec->type为0、2其中之一
#define Decomper_inFile_check(dec) (strcmp((dec)->header->magicStr, "PK") == 0 and strchr("\x02\x00", (dec)->header->type))
extern void Decomper_init(Decomper* __restrict dec, const Args* __restrict args);
extern void Decomper_clear(Decomper* __restrict dec);
extern void Decomper_outFile_select(Decomper* __restrict dec); // 决定输出文件
extern void Decomper_decompress(Decomper* __restrict dec);     // 处理0x02型，有压缩数据
extern void Decomper_raw_copy(Decomper* __restrict dec);       // 未压缩类型，直接拷个爽~
extern void decompress(const Args* __restrict args);	       // “前端”函数

#endif
