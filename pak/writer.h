#ifndef _WRITER_H
#define _WRITER_H

#include "args.h"
#include "common.h"
#include "pak.h"
#include "reader.h"

typedef PAKReader PAKWriter;

/*
extern void PAKReader_init(PAKReader* __restrict preader, const char* inFilePath);
extern void PAKReader_clear(PAKReader* __restrict preader);
extern void PAKReader_read_content(PAKReader* __restrict preader, const uint32_t fileIndex); // 读取文件内容。只有要进行提取操作时才读取。建立目录还是必要的。
extern void PAKReader_copy_content(PAKReader* __restrict preader, const uint32_t fileIndex, const char* outFilePath);

extern void extract(Args* __restrict args);
extern void list(Args* __restrict args);
*/

extern void PAKWriter_init(PAKWriter* __restrict pwriter, const Args* __restrict args); // 类的初始化
extern void PAKWriter_read(PAKWriter* __restrict pwriter);			    // 读取待写入文件的内容
extern void PAKWriter_clac_offset(PAKWriter* __restrict pwriter);			    // 计算文件偏移表
extern void PAKWriter_build_pakFile(PAKWriter* __restrict pwriter);		    // 构建PAK文件
// 清理类，释放内存
#define PAKWriter_clear(pwriter) PAKReader_clear((pwriter))

extern void archive(const Args* __restrict args);

#define BLOCK_SIZE (4 * sizeof(uint32_t))

#endif
