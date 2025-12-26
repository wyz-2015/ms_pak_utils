#ifndef _PAK_H
#define _PAK_H

#include "common.h"

typedef struct PAK_Header { // [0x00, 0x10)	PAK头部，0x10 Bytes。区间表示相对于PAK文件的偏移
	char magicStr[4];   // [0x00, 0x04)	魔术字，"DATA" "MENU" "FONT" "STRD"，无'\0'结尾，0x04 Bytes
	int32_t unknown1;   // [0x04, 0x08)
	int32_t unknown2;   // [0x08, 0x0c)
	uint32_t fileCount; // [0x0c, 0x10)	包内文件数，重要数据
} PAK_Header;

/* 紧随PAK_Header后的，是一个文件偏移表。
 * 是一个uint32_t数组，设c = PAK_Header.fileCount，项数为(c + 1 + o)
 * 从0数起，[0, c)项分别是各文件的相对(于PAK文件，后不赘述)偏移
 * 第c项是整个PAK文件的长，应该对齐、补0。PAK文件自己或许也没遵守这个设定233.
 * o是个任意非负常数(不 定 积 分)，实际这些PAK包中，情况没那么简单，(c + 1)项uint32_t数后不见得就是正文数据的起点。至于这o项数据什么意义，未知。
 */
// typedef uint32_t PAK_FileOffsetTable_t;

typedef struct PAK_File {
	uint32_t relativeOffset; // 文件相对偏移
	uint32_t length;	 // 文件长
	void* content;		 // 只有这个，文件内容，是实际写入PAK文件中的。上面两项只是辅助
} PAK_File;

#endif
