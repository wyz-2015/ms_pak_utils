#ifndef _READER_H
#define _READER_H

#include "args.h"
#include "common.h"
#include "pak.h"

typedef struct PAKReader {
	PAK_Header* header;

	uint32_t* fileOffsetTable;
	uint32_t fileOffsetTableCount;

	PAK_File** fileArray; // 针对文件内容，便于操作的结构体的数组

	FILE* pak; // pak文件指针本身
	// char* inFileDir;
	// char* inFileName;
	// char* outDir;

	const Args* args;
} PAKReader;

extern void PAKReader_init(PAKReader* __restrict preader, const Args* args);
extern void PAKReader_clear(PAKReader* __restrict preader);
extern void PAKReader_read_content(PAKReader* __restrict preader, const uint32_t fileIndex); // 读取文件内容。只有要进行提取操作时才读取。建立目录还是必要的。
extern void PAKReader_copy_content(PAKReader* __restrict preader, const uint32_t fileIndex, const char* outFilePath);

extern void extract(Args* __restrict args);
extern void list(Args* __restrict args);

#endif
