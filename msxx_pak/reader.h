#ifndef _READER_H
#define _READER_H

#include "args.h"
#include "common.h"
#include "pak.h"

typedef struct PAKReader {
	const Args* args;

	PAK_Header* header;

	PAK_Item** itemTable;
	uint32_t itemCount;
	uint32_t itemCount_real;

	PAK_File** fileArray;

	FILE* pak;
} PAKReader;

extern void PAKReader_init(PAKReader* __restrict preader, const Args* args); // 初始化类
// extern void PAKReader_read(PAKReader* __restrict preader, const uint32_t fileIndex);	     // 读取PAK_File中的几项数据
extern void PAKReader_read_metadata(PAKReader* __restrict preader);			     // 读取PAK_File中的元数据
extern void PAKReader_read_content(PAKReader* __restrict preader, const uint32_t fileIndex); // 读取PAK_File中的内容数据
extern void PAKReader_copy_content(PAKReader* __restrict preader, const uint32_t fileIndex); // 将PAK中指定序号的文件拷出
extern void PAKReader_clear(PAKReader* __restrict preader);				     // 释放类的内存，类本身仍需要额外释放
extern bool PAKReader_file_check(PAKReader* __restrict preader);			     // 检查文件

extern void extract(const Args* __restrict args); // 前端函数，解压
extern void list(const Args* __restrict args);	  // 前端函数，罗列文件表

#endif
