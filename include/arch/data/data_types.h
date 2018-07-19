#pragma once
#include "defines.h"

typedef struct list_node_t
{
	struct list_node_t *next;
	uint8 data[];
} ListNode;

typedef struct list_t
{
	ListNode *front;
	ListNode *back;
	uint32 dataSize;
} List;

typedef struct list_iterator_t
{
	ListNode *prev;
	ListNode *curr;
} ListIterator;

typedef struct binary_search_tree_node_t
{
	struct binary_search_tree_node_t *left;
	struct binary_search_tree_node_t *right;
	uint8 data[];
} BinarySearchTreeNode;

typedef struct binary_search_tree_t
{
	BinarySearchTreeNode *root;
	uint32 dataSize;
} BinarySearchTree;

typedef List HashMapBucket;

typedef int32(*ComparisonOp)(void*, void*);

typedef struct hash_map_t
{
	ComparisonOp comparison;
	uint32 keySizeBytes;
	uint32 valueSizeBytes;
	uint32 bucketCount;
	HashMapBucket buckets[];
} *HashMap;

typedef struct hash_map_iterator_t
{
	HashMap map;
	ListIterator itr;
	uint32 bucket;
} HashMapIterator;

typedef struct closure_data_t
{
	uint8 numArgs;
	uint8 data[];
} ClosureData;

typedef void(*HashMapFunctorFn)(void*, void*, ClosureData*);
