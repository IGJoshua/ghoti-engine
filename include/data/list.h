#pragma once
#include "defines.h"

#include "data_types.h"

List createList(uint32 dataSize);

void pushFront(List *l, void *data);
void pushBack(List *l, void *data);

void popFront(List *l);

void clearList(List *l);

ListNode **getListIterator(List *l);
void moveListIterator(ListNode ***itr);

void removeListItem(List *l, ListNode **itr);
void insertListItem(List *l, ListNode **itr, void *data);
