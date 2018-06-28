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

	ASSERT(node != 0);

	memcpy(node->data, data, l->dataSize);

	node->next = l->front;

	l->front = node;
	if (!l->back)
	{
		l->back = node;
	}
}

void listPushBack(List *l, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);

	ASSERT(node);

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

void listPopFront(List *l)
{
	ListNode *node = l->front;

	if (l->front)
	{
		l->front = l->front->next;
	}
	if (!l->front)
	{
		l->back = 0;
	}

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

ListIterator listGetIterator(List *l)
{
	ListIterator itr = {};
	itr.curr = l->front;
	itr.prev = 0;
	return itr;
}

inline
void listMoveIterator(ListIterator *itr)
{
	if (itr->curr)
	{
		itr->prev = itr->curr;
		itr->curr = itr->curr->next;
	}
}

inline
int32 listIteratorAtEnd(ListIterator itr)
{
	return !itr.curr;
}

void listRemove(List *l, ListIterator *itr)
{
	// Save node to delete after
	ListNode *temp = itr->curr;

	// Redirect the previous node to point at the next one
	if (itr->prev)
	{
		itr->prev->next = itr->curr->next;
		itr->curr = itr->prev->next;
	}
	else
	{
		l->front = itr->curr->next;
		itr->curr = l->front;
	}

	// Ensure that front and back are up to date
	if (temp == l->back)
	{
		l->back = itr->prev;
	}

	// Delete the node
	free(temp);
}

inline
void listInsert(List *l, ListIterator *itr, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);

	ASSERT(node != 0);

	memcpy(node->data, data, l->dataSize);
	node->next = itr->curr;
	itr->curr = node;
	if (itr->prev)
	{
		itr->prev->next = node;
	}
	else
	{
		l->front = node;
		l->back = node;
	}
}

inline
uint32 listGetSize(List *l)
{
	uint32 size = 0;

	for (ListIterator itr = listGetIterator(l);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		size++;
	}

	return size;
}
