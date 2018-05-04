#pragma once
#include "defines.h"

#include "data_types.h"

HashMap *createHashMap(uint32 keySize, uint32 valueSize, uint32 bucketCount, ComparisonOp comparison);

void pushKeyValue(HashMap *map, void *key, void *value);
void insertKeyValue(HashMap *map, void *key, void *value);
void *getKey(HashMap *map, void *key);
void popKey(HashMap *map, void *key);
void deleteKey(HashMap *map, void *key);
