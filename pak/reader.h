#ifndef _READER_H
#define _READER_H

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
	char* outDir;
} PAKReader;

#endif
