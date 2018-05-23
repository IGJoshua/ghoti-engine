#pragma once
#include "defines.h"

#include "data_types.h"

#define LIST_ITERATOR_GET_ELEMENT(type, itr) ((type *)(*(itr))->data)

List createList(uint32 dataSize);

void listPushFront(List *l, void *data);
void listPushBack(List *l, void *data);

void listPopFront(List *l);

void listClear(List *l);

ListNode **listGetIterator(List *l);
void listMoveIterator(ListNode ***itr);
int32 listIteratorAtEnd(ListNode **itr);

void listRemove(List *l, ListNode **itr);
void listInsert(List *l, ListNode **itr, void *data);
