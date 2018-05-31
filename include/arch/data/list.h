#pragma once
#include "defines.h"

#include "data_types.h"

#define LIST_ITERATOR_GET_ELEMENT(type, itr) ((type *)(*(itr))->data)

List createList(uint32 dataSize);

void listPushFront(List *l, void *data);
void listPushBack(List *l, void *data);

void listPopFront(List *l);
void listClear(List *l);

ListIterator listGetIterator(List *l);
void listMoveIterator(ListIterator *itr);
int32 listIteratorAtEnd(ListIterator itr);

void listRemove(List *l, ListIterator itr);
void listInsert(List *l, ListIterator itr, void *data);
