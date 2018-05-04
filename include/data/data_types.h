#pragma once
#include "defines.h"

typedef struct list_node_t
{
	struct list_node_t *next;
	uint8 data[];
} ListNode;

typedef struct list_t
{
	uint32 dataSize;
	ListNode *front;
	ListNode *back;
} List;

typedef struct binary_search_tree_node_t
{
	struct binary_search_tree_node_t *left;
	struct binary_search_tree_node_t *right;
	uint8 data[];
} BinarySearchTreeNode;

typedef struct binary_search_tree_t
{
	uint32 dataSize;
	BinarySearchTreeNode *root;
} BinarySearchTree;

typedef List HashMapBucket;

typedef int32(*ComparisonOp)(void*, void*);

typedef struct hash_map_t
{
	uint32 keySizeBytes;
	uint32 valueSizeBytes;
	uint32 bucketCount;
	int32 (*comparison)(void *key1, void *key2);
	HashMapBucket buckets[];
} HashMap;
