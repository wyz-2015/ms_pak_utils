#ifndef _PAK_H
#define _PAK_H

#include "common.h"

// 文件头，[0x00, 0x04)
typedef struct PAK_Header {
	uint32_t unknown0;
	uint32_t unknown1;
	uint32_t unknown2;
	uint32_t fileCount; // 整个PAK包中一共几个文件
} PAK_Header;

// 紧随文件头后，一条4 * 4 Byte。fileCount条如下条目，组成pak文件目录
typedef struct PAK_Item {
	// 以下两条为猜测，
	// 且暂且以“子目录”“子目录中的文件”来称呼猜测的这种关系。
	uint32_t subID;	 // 在子目录中的文件ID
	uint32_t subDir; // 存在于何子目录

	// 以下两条是比较实际的，大概率对的
	uint32_t relativeOffset; // 于pak文件中的相对偏移
	uint32_t fileLength;	 // 文件长度。MSXX的pak包是比较严谨的，不像MS7、3D可能有“说话不算话”的情况。
				 // 大部分情况下，relativeOffset(n - 1) + fileLength(n - 1) = relativeOffset(n)
} PAK_Item;

typedef struct PAK_File { // 用于处理包中文件的结构体，不是指pak文件本身
	void* content;	  // 文件内容，真正写入pak文件中的。
	uint32_t fileLength;
	uint32_t bufferSize;
	uint32_t subID;		 // 在子目录中的文件ID
	uint32_t subDir;	 // 存在于何子目录
	uint32_t relativeOffset; // 相对偏移
} PAK_File;

#endif
