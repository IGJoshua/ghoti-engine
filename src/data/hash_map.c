#include "data/hash_map.h"

#include "data/data_types.h"
#include "data/list.h"

#include <malloc.h>
#include <string.h>

typedef struct hash_map_storage_t
{
	uint64 hash;
	uint8 data[];
} HashMapStorage;

HashMap createHashMap(uint32 keySize, uint32 valueSize, uint32 bucketCount, ComparisonOp comparison)
{
	uint32 mapSize = sizeof(struct hash_map_t) + (sizeof(HashMapBucket) * bucketCount);
	HashMap map = malloc(mapSize);

	map->keySizeBytes = keySize;
	map->valueSizeBytes = valueSize;
	map->bucketCount = bucketCount;
	map->comparison = comparison;

	for (uint32 i = 0; i < map->bucketCount; ++i)
	{
		map->buckets[i] = createList(sizeof(HashMapStorage) + map->keySizeBytes + map->valueSizeBytes);
	}

	return map;
}

void deleteHashMap(HashMap *map)
{
	hashMapClear(*map);
	free(*map);
	*map = 0;
}

internal
uint64 hash(void *bytes, uint32 numBytes)
{
	uint64 hash = 5381;
	int32 c;

	for (uint32 i = 0; i < numBytes; ++i)
	{
		c = *((int32 *)bytes++);
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

void hashMapPush(HashMap map, void *key, void *value)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	HashMapStorage *storage = malloc(sizeof(HashMapStorage) + map->keySizeBytes + map->valueSizeBytes);
	storage->hash = keyHash;
	memcpy(storage->data, key, map->keySizeBytes);
	memcpy(storage->data + map->keySizeBytes, value, map->valueSizeBytes);

	listPushFront(&map->buckets[bucketIndex], storage);

	free(storage);
}

void hashMapInsert(HashMap map, void *key, void *value)
{
	hashMapDeleteKey(map, key);
	hashMapPush(map, key, value);
}

void *hashMapGetKey(HashMap map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = listGetIterator(&map->buckets[bucketIndex]);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			return ((HashMapStorage *)(*itr)->data)->data + map->keySizeBytes;
		}
	}

	return 0;
}

void hashMapPopKey(HashMap map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = listGetIterator(&map->buckets[bucketIndex]);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			listRemove(&map->buckets[bucketIndex], itr);
			break;
		}
	}
}

void hashMapDeleteKey(HashMap map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = listGetIterator(&map->buckets[bucketIndex]);
		 !listIteratorAtEnd(itr);)
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			listRemove(&map->buckets[bucketIndex], itr);
		}
		else
		{
			listMoveIterator(&itr);
		}
	}
}

void hashMapClear(HashMap map)
{
	for (uint32 i = 0; i < map->bucketCount; ++i)
	{
		listClear(&map->buckets[i]);
	}
}
