#include "deque.h"

void Deque_init(Deque* restrict d)
{
	d->end = NULL;
	d->head = NULL;
	d->__len__ = 0;
}

void Deque_append(Deque* restrict d, const char* value)
{
	Deque_Node* newNode = (Deque_Node*)malloc(sizeof(Deque_Node));
	if (not newNode) {
		error(ENOMEM, ENOMEM, "%s：为newNode(%p)malloc失败", __func__, newNode);
	}

	newNode->value = value;

	if (d->__len__ == 0) {
		d->head = newNode;
	} else {
		d->end->next = newNode;
	}
	newNode->prev = d->end;
	d->end = newNode;
	d->__len__ += 1;
}

void Deque_pop(Deque* restrict d)
{
	Deque_Node* lastNode = d->end;
	if (not lastNode) {
		error(EPERM, EPERM, "%s：deque(%p)为空，无法pop", __func__, d);
	}

	if (lastNode->prev) {
		lastNode->prev->next = NULL;
	}
	d->end = lastNode->prev;
	d->__len__ -= 1;

	free(lastNode);
}

void Deque_clear(Deque* restrict d)
{
	while (d->__len__ > 0) {
		Deque_pop(d);
	}

	d->head = NULL;
}
