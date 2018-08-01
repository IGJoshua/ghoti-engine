#pragma once
#include "defines.h"

#include "data_types.h"

#define HASH_SEED 10007

HashMap createHashMap(
	uint32 keySize,
	uint32 valueSize,
	uint32 bucketCount,
	ComparisonOp comparison);
void freeHashMap(HashMap *map);

void hashMapPush(HashMap map, void *key, void *value);
void hashMapInsert(HashMap map, void *key, void *value);
void *hashMapGetData(HashMap map, void *key);
void hashMapPopKey(HashMap map, void *key);
void hashMapDeleteKey(HashMap map, void *key);
void hashMapClear(HashMap map);

HashMapIterator hashMapGetIterator(HashMap map);
void hashMapMoveIterator(HashMapIterator *itr);
int32 hashMapIteratorAtEnd(HashMapIterator itr);
void *hashMapIteratorGetKey(HashMapIterator itr);
void *hashMapIteratorGetValue(HashMapIterator itr);
void hashMapDeleteAtIterator(HashMapIterator *itr);

void hashMapFMap(HashMap map, HashMapFunctorFn fn, ClosureData *data);
