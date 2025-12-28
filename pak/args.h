#ifndef _ARGS_H
#define _ARGS_H

#include "common.h"
#include "deque.h"

typedef struct Args {
	char mode;
	const char* filePath;
	const char* dir;
	const char* prefix;

	Deque* itemDeque;

	bool verbose;
	bool extractAll;

	const char* magicStr;
} Args;

#endif
