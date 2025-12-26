#ifndef _DEQUE_H
#define _DEQUE_H

#include "common.h"

/*
typedef union Value {
	const char* filePath;
	const uint32_t fileIndex;
} Value;
*/

typedef struct Deque_Node {
	const char* value;
	struct Deque_Node* prev;
	struct Deque_Node* next;
} Deque_Node;

typedef struct Deque {
	uint32_t __len__;
	// uint32_t __type__;
	Deque_Node* head;
	Deque_Node* end;

} Deque;

/*
typedef enum Deque_Type {
	NUM,
	STR
} Deque_Type;
*/

extern void Deque_init(Deque* __restrict d);
extern void Deque_append(Deque* __restrict d, const char* value);
extern void Deque_pop(Deque* __restrict d);
extern void Deque_clear(Deque* __restrict d);

#endif
