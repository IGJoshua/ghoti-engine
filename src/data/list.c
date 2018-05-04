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

void pushFront(List *l, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);

	node->next = l->front;

	l->front = node;
	if (!l->back)
		l->back = node;
}

void pushBack(List *l, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);

	node->next = 0;

	l->back->next = node;
	l->back = node;
	if (!l->front)
		l->front = node;
}

inline
void popFront(List *l)
{
	ListNode *node = l->front;

	if (l->front)
		l->front = l->front->next;
	if (!l->front)
		l->back = 0;

	free(node);
}

inline
void clearList(List *l)
{
	while (l->front)
	{
		popFront(l);
	}
}

inline
ListNode **getListIterator(List *l)
{
	return &l->front;
}

inline
void moveListIterator(ListNode ***itr)
{
	*itr = &(**itr)->next;
}

void removeListItem(List *l, ListNode **itr)
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
void insertListItem(List *l, ListNode **itr, void *data)
{
	ListNode *node = malloc(sizeof(ListNode) + l->dataSize);
	memcpy(node->data, data, l->dataSize);
	node->next = *itr;
	(*itr)->next = node;
}
