#ifndef _ARGS_H
#define _ARGS_H

#include "common.h"
#include "deque.h"

typedef struct Args {
	const char* filePath;
	const char* dir;

	Deque* itemDeque;

	bool verbose;
	bool extractAll;
} Args;

#endif
