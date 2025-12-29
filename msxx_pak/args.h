#ifndef _ARGS_H
#define _ARGS_H

#include "common.h"
#include "deque.h"

typedef struct Args {
	char mode;
	const char* filePath;
	const char* dir;

	Deque* itemDeque;

	bool verbose;
	bool extractAll;

	const char* fileListPath;
} Args;

#endif
