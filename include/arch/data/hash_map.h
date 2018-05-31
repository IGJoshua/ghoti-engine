#pragma once
#include "defines.h"

#include "data_types.h"

HashMap createHashMap(
	uint32 keySize,
	uint32 valueSize,
	uint32 bucketCount,
	ComparisonOp comparison);
void freeHashMap(HashMap *map);

void hashMapPush(HashMap map, void *key, void *value);
void hashMapInsert(HashMap map, void *key, void *value);
void *hashMapGetKey(HashMap map, void *key);
void hashMapPopKey(HashMap map, void *key);
void hashMapDeleteKey(HashMap map, void *key);
void hashMapClear(HashMap map);
