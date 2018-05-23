#include "data/data_types.h"
#include "data/list.h"

#include <malloc.h>
#include <string.h>

inline
List createList(uint32 dataSize)
{
	List ret;

	ret.dataSize = dataSize;
	ret.front = 0;
	ret.back = 0;

	return ret;
}

void listPushFront(List *l, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);

	node->next = l->front;

	l->front = node;
	if (!l->back)
		l->back = node;
}

void listPushBack(List *l, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);

	node->next = 0;

	if (l->back)
	{
		l->back->next = node;
		l->back = node;
	}
	else
	{
		l->front = node;
		l->back = node;
	}
}

inline
void listPopFront(List *l)
{
	ListNode *node = l->front;

	if (l->front)
		l->front = l->front->next;
	if (!l->front)
		l->back = 0;

	free(node);
}

inline
void listClear(List *l)
{
	while (l->front)
	{
		listPopFront(l);
	}
}

inline
ListNode **listGetIterator(List *l)
{
	return &l->front;
}

inline
void listMoveIterator(ListNode ***itr)
{
	*itr = &(**itr)->next;
}

inline
int32 listIteratorAtEnd(ListNode **itr)
{
	return (*itr) == 0;
}

void listRemove(List *l, ListNode **itr)
{
	// Save node to delete after
	ListNode *temp = *itr;

	// Redirect the previous node to point at the next one
	*itr = (*itr)->next;

	// Ensure that front and back are up to date
	if (temp == l->back)
		l->back = 0;

	if (!l->front)
		l->back = 0;

	// Delete the node
	free(temp);
}

inline
void listInsert(List *l, ListNode **itr, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);
	node->next = *itr;
	(*itr)->next = node;
}
