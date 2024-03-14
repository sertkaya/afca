#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "bitset_hash_map.h"
#include "../hashing/utils.h"
// #include "utils.h"


inline BitSetHashMap* bitset_hash_map_create(unsigned int size) {

	BitSetHashMap* hash_map = (BitSetHashMap*) malloc(sizeof(BitSetHashMap));
	assert(hash_map != NULL);

	if (size < 16)
		size = 16;
	else
		size = roundup_pow2(size);

	// allocate space for the buckets
	hash_map->buckets = (BitSetHashMapElement***) calloc(size, sizeof(BitSetHashMapElement**));
	assert(hash_map->buckets != NULL);

	// allocate space for the chain sizes
	hash_map->chain_sizes = (unsigned*) calloc(size, sizeof(unsigned int));
	assert(hash_map->buckets != NULL);

	hash_map->bucket_count = size;

	hash_map->tail = NULL;

	hash_map->element_count = 0;

	return hash_map;
}

void bitset_hash_map_init(BitSetHashMap* hash_map, unsigned int size) {

	// allocate space for the buckets
	hash_map->buckets = (BitSetHashMapElement***) calloc(size, sizeof(BitSetHashMapElement**));
	assert(hash_map->buckets != NULL);

	// allocate space for the chain sizes
	hash_map->chain_sizes = (unsigned*) calloc(size, sizeof(unsigned int));
	assert(hash_map->buckets != NULL);

	hash_map->bucket_count = size;

	hash_map->tail = NULL;

	hash_map->element_count = 0;
}


int bitset_hash_map_free(BitSetHashMap* hash_map) {
	int freed_bytes = 0;

	int i;
	for (i = 0; i < hash_map->bucket_count; ++i) {
		if (hash_map->buckets[i] != NULL) {
			int j;
			for (j = 0; j < hash_map->chain_sizes[i]; ++j) {
				// note that we only free the node.
				// the space allocated for  node->value is not freed!
				free(hash_map->buckets[i][j]);
				freed_bytes += sizeof(BitSetHashMapElement);
			}

			free(hash_map->buckets[i]);
			freed_bytes += hash_map->chain_sizes[i] * sizeof(BitSetHashMapElement*);
		}
	}
	free(hash_map->buckets);
	freed_bytes += hash_map->bucket_count * sizeof(BitSetHashMapElement***);

	free(hash_map->chain_sizes);
	freed_bytes += hash_map->bucket_count * sizeof(unsigned int);

	free(hash_map);
	freed_bytes += sizeof(BitSetHashMap);

	return freed_bytes;
}

int bitset_hash_map_reset(BitSetHashMap* hash_map) {
	int freed_bytes = 0;

	int i;
	for (i = 0; i < hash_map->bucket_count; ++i) {
		if (hash_map->buckets[i] != NULL) {
			int j;
			for (j = 0; j < hash_map->chain_sizes[i]; ++j) {
				// note that we only free the node.
				// the space allocated for  node->value is not freed!
				free(hash_map->buckets[i][j]);
				freed_bytes += sizeof(BitSetHashMapElement);
			}

			free(hash_map->buckets[i]);
			freed_bytes += hash_map->chain_sizes[i] * sizeof(BitSetHashMapElement*);
		}
	}
	free(hash_map->buckets);
	freed_bytes += hash_map->bucket_count * sizeof(BitSetHashMapElement***);

	free(hash_map->chain_sizes);
	freed_bytes += hash_map->bucket_count * sizeof(unsigned int);

	hash_map->bucket_count = 0;
	hash_map->element_count = 0;

	return freed_bytes;
}

extern inline int bitset_hash_map_put(BitSetHashMap* hash_map, BitSet* bs, void* value);

extern inline void* bitset_hash_map_get(BitSetHashMap* hash_map, BitSet* bs);

void bitset_hash_map_iterator_init(BitSetHashMapIterator* iterator, BitSetHashMap* map) {
	iterator->hash_map = map;
	BitSetHashMapElement* tmp = (BitSetHashMapElement*) calloc(1, sizeof(BitSetHashMapElement));
	assert(tmp != NULL);
	tmp->previous = map->tail;
	iterator->current_element = tmp;
}

extern inline void* bitset_hash_map_iterator_next(BitSetHashMapIterator* iterator);
