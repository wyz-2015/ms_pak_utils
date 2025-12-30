#ifndef _COMPRESS_C
#define _COMPRESS_C

#include "decompress.h"

typedef Decomper Comper;

#define Comper_clear(cer) Decomper_clear((cer))
extern void Comper_init(Comper* __restrict cer, const Args* __restrict args);
extern void Comper_outFile_select(Comper* __restrict cer); // 输出文件选择？没得选
extern void Comper_raw_copy(Comper* __restrict cer);	   // 没有LZSS压缩算法，只有直接拷贝并加头
extern void compress(const Args* __restrict args);

#endif
