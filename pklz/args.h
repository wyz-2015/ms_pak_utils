#ifndef _ARGS_H
#define _ARGS_H

#include "common.h"

typedef struct Args {
	const char* inFilePath;
	const char* outFilePath;
	char mode; // 'z'强制压缩，'d'强制解压
	bool verbose;
	bool toStdout; // 是否输出内容到stdout
} Args;

#endif
