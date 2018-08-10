#include "data/hash_map.h"
#include "data/data_types.h"
#include "data/list.h"

#include "core/log.h"

#ifndef _WIN32
#include <alloca.h>
#endif
#include <malloc.h>
#include <string.h>

typedef struct hash_map_storage_t
{
	uint64 hash;
	uint8 data[];
} HashMapStorage;

HashMap createHashMap(
	uint32 keySize,
	uint32 valueSize,
	uint32 bucketCount,
	ComparisonOp comparison)
{
	ASSERT(comparison);

	uint32 mapSize =
		sizeof(struct hash_map_t)
		+ (sizeof(HashMapBucket) * bucketCount);
	HashMap map = malloc(mapSize);

	ASSERT(map != 0);

	map->keySizeBytes = keySize;
	map->valueSizeBytes = valueSize;
	map->bucketCount = bucketCount;
	map->comparison = comparison;

	for (uint32 i = 0; i < map->bucketCount; ++i)
	{
		map->buckets[i] = createList(
			sizeof(HashMapStorage)
			+ map->keySizeBytes
			+ map->valueSizeBytes);
	}

	return map;
}

void freeHashMap(HashMap *map)
{
	hashMapClear(*map);
	free(*map);
	*map = 0;
}

internal
uint64 hash(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
	{
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

void hashMapPush(HashMap map, void *key, void *value)
{
	uint64 keyHash = hash(key);
	uint32 bucketIndex = keyHash % map->bucketCount;

	HashMapStorage *storage = alloca(
		sizeof(HashMapStorage)
		+ map->keySizeBytes
		+ map->valueSizeBytes);

	ASSERT(storage != 0);

	storage->hash = keyHash;
	memcpy(storage->data, key, map->keySizeBytes);
	memcpy(storage->data + map->keySizeBytes, value, map->valueSizeBytes);

	listPushFront(&map->buckets[bucketIndex], storage);
}

void hashMapInsert(HashMap map, void *key, void *value)
{
	hashMapDelete(map, key);
	hashMapPush(map, key, value);
}

void *hashMapGetData(HashMap map, void *key)
{
	uint64 keyHash = hash(key);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListIterator itr = listGetIterator(&map->buckets[bucketIndex]);
			!listIteratorAtEnd(itr);
			listMoveIterator(&itr))
	{
		if (!map->comparison(
				LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr)->data,
				key))
		{
			return LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr)->data
				+ map->keySizeBytes;
		}
	}

	return NULL;
}

void hashMapPopKey(HashMap map, void *key)
{
	uint64 keyHash = hash(key);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListIterator itr = listGetIterator(&map->buckets[bucketIndex]);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		if (!map->comparison(LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr)->data, key))
		{
			listRemove(&map->buckets[bucketIndex], &itr);
			break;
		}
	}
}

void hashMapDelete(HashMap map, void *key)
{
	uint64 keyHash = hash(key);
	uint32 bucketIndex = keyHash % map->bucketCount;

	for (ListIterator itr = listGetIterator(&map->buckets[bucketIndex]);
		 !listIteratorAtEnd(itr);)
	{
		if (!map->comparison(LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr)->data, key))
		{
			listRemove(&map->buckets[bucketIndex], &itr);
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

HashMapIterator hashMapGetIterator(HashMap map)
{
	HashMapIterator ret = {};

	ret.map = map;
	ret.bucket = 0;
	ret.itr = listGetIterator(&map->buckets[ret.bucket]);
	while (listIteratorAtEnd(ret.itr))
	{
		// move the iterator if the iterator is at a valid bucket index
		if (++ret.bucket < ret.map->bucketCount)
		{
			ret.itr = listGetIterator(&ret.map->buckets[ret.bucket]);
		}
		// break the loop if the iterator is past the end of the hashmap
		else
		{
			break;
		}
	}

	return ret;
}

void hashMapMoveIterator(HashMapIterator *itr)
{
	// Return if the iterator is already at the end
	if (itr->bucket >= itr->map->bucketCount)
	{
		return;
	}

	// If the iterator can move in the current bucket
	if (!listIteratorAtEnd(itr->itr))
	{
		// Move the iterator
		listMoveIterator(&itr->itr);
	}

	/*
	 * If the iterator has to go to the next bucket,
	 * find the next non-empty bucket
	 */
	while (listIteratorAtEnd(itr->itr))
	{
		// move the iterator if the iterator is at a valid bucket index
		if (++itr->bucket < itr->map->bucketCount)
		{
			itr->itr = listGetIterator(&itr->map->buckets[itr->bucket]);
		}
		// break the loop if the iterator is past the end of the hashmap
		else
		{
			break;
		}
	}
}

inline
int32 hashMapIteratorAtEnd(HashMapIterator itr)
{
	return itr.bucket >= itr.map->bucketCount;
}

inline
void *hashMapIteratorGetKey(HashMapIterator itr)
{
	return LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr.itr)->data;
}

inline
void *hashMapIteratorGetValue(HashMapIterator itr)
{
	return LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr.itr)->data
		+ itr.map->keySizeBytes;
}

void hashMapDeleteAtIterator(HashMapIterator *itr)
{
	ASSERT(false && "Hash map delete iterator not yet implemented.");

	listRemove(&itr->map->buckets[itr->bucket], &itr->itr);

	/*
	 * If the iterator has to go to the next bucket,
	 * find the next non-empty bucket
	 */
	while (listIteratorAtEnd(itr->itr))
	{
		// move the iterator if the iterator is at a valid bucket index
		if (++itr->bucket < itr->map->bucketCount)
		{
			itr->itr = listGetIterator(&itr->map->buckets[itr->bucket]);
		}
		// break the loop if the iterator is past the end of the hashmap
		else
		{
			break;
		}
	}
}

void hashMapFMap(HashMap map, HashMapFunctorFn fn, ClosureData *data)
{
	for (uint32 bucket = 0; bucket < map->bucketCount; ++bucket)
	{
		for (ListIterator itr = listGetIterator(&map->buckets[bucket]);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			HashMapStorage *element =
				LIST_ITERATOR_GET_ELEMENT(HashMapStorage, itr);
			fn(element->data, element->data + map->keySizeBytes, data);
		}
	}
}

uint32 hashMapCount(HashMap map)
{
	uint32 count = 0;
	for (HashMapIterator itr = hashMapGetIterator(map);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		++count;
	}
	return count;
}
