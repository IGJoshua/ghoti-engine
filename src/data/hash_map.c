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

HashMap *createHashMap(uint32 keySize, uint32 valueSize, uint32 bucketCount, ComparisonOp comparison)
{
	HashMap *map = malloc(sizeof(HashMap) + sizeof(HashMapBucket) * bucketCount);

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

void pushKeyValue(HashMap *map, void *key, void *value)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	HashMapStorage *storage = malloc(sizeof(HashMapStorage) + map->keySizeBytes + map->valueSizeBytes);
	storage->hash = keyHash;
	memcpy(storage->data, key, map->keySizeBytes);
	memcpy(storage->data + map->keySizeBytes, value, map->valueSizeBytes);

	pushFront(&map->buckets[bucketIndex], storage);

	free(storage);
}

void insertKeyValue(HashMap *map, void *key, void *value)
{
	deleteKey(map, key);
	pushKeyValue(map, key, value);
}

void *getKey(HashMap *map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = getListIterator(&map->buckets[bucketIndex]);
		 itr != 0;
		 moveListIterator(&itr))
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			return ((HashMapStorage *)(*itr)->data)->data + map->keySizeBytes;
		}
	}

	return 0;
}

void popKey(HashMap *map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = getListIterator(&map->buckets[bucketIndex]);
		 itr != 0;
		 moveListIterator(&itr))
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			removeListItem(&map->buckets[bucketIndex], itr);
			break;
		}
	}
}

void deleteKey(HashMap *map, void *key)
{
	uint64 keyHash = hash(key, map->keySizeBytes);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListNode **itr = getListIterator(&map->buckets[bucketIndex]);
		 itr != 0;)
	{
		if (map->comparison(((HashMapStorage *)(*itr)->data)->data, key))
		{
			removeListItem(&map->buckets[bucketIndex], itr);
		}
		else
		{
			moveListIterator(&itr);
		}
	}
}
