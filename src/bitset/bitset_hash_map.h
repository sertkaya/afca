#ifndef BITSET_HASH_MAP_H
#define BITSET_HASH_MAP_H

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "bitset.h"

typedef struct bitset_hash_map BitSetHashMap;
typedef struct bitset_hash_map_element BitSetHashMapElement;
typedef struct bitset_hash_map_iterator BitSetHashMapIterator;

struct bitset_hash_map_element {
	BitSet* key;
	void* value;
	BitSetHashMapElement* previous;
};

struct bitset_hash_map {
	BitSetHashMapElement*** buckets;		// the buckets
	unsigned int bucket_count;		// the number of buckets
	unsigned int* chain_sizes;		// sizes of the chains
	unsigned int element_count;	// the number of elements
	BitSetHashMapElement* tail;			// the last node of the hash.
									// we maintain a backward linked list.
};

/**
 * Iterator for hash map.
 */
struct bitset_hash_map_iterator {
	BitSetHashMap* hash_map;
	BitSetHashMapElement* current_element;// of the current element
};

/**
 * Create a hash map with the given number of buckets.
 */
BitSetHashMap* bitset_hash_map_create(unsigned int size);

/**
 * Initialize a hash map with the given number of buckets.
 */
void bitset_hash_map_init(BitSetHashMap* hash_map, unsigned int size);

/**
 * Free the space allocated for the given hash map.
 */
int bitset_hash_map_free(BitSetHashMap* hash_map);

/**
 * Free the space allocated for the elements of a given hash map.
 * Does not free the space allocated for the hash map itself.
 */
int bitset_hash_map_reset(BitSetHashMap* hash_map);


/**
 * Insert a key value pair to the hash map. If the key already exists
 * the existing value is overwritten.
 * Returns 1 if a new key value pair is inserted, 0 otherwise.
 */
inline int bitset_hash_map_put(BitSetHashMap* hash_map, BitSet* bs, void* value) {
	printf("in hash put\n");
	fflush(stdout);
	int hash_value = get_key(bs) & (hash_map->bucket_count - 1);
	BitSetHashMapElement** bucket = hash_map->buckets[hash_value];
	int chain_size = hash_map->chain_sizes[hash_value];

	for (unsigned int i = 0; i < chain_size; ++i) {
		if (bitset_is_equal(bucket[i]->key, bs)) {
			bucket[i]->value = value;
			return 0;
		}
    }

	BitSetHashMapElement** tmp = realloc(bucket, (chain_size + 1) * sizeof(BitSetHashMapElement*));
	printf("still in hash put\n");
	assert(tmp != NULL);
	bucket = hash_map->buckets[hash_value] = tmp;

	bucket[chain_size] = malloc(sizeof(BitSetHashMapElement));
	assert(bucket[chain_size] != NULL);
	bucket[chain_size]->key = bs;
	bucket[chain_size]->value = value;
	bucket[chain_size]->previous = hash_map->tail;

	hash_map->tail = bucket[chain_size];

	++hash_map->chain_sizes[hash_value];

	++hash_map->element_count;

	return 1;
}


/**
 * Returns the value for the given key, it it exists, NULL if it does not exist.
 */
inline void* bitset_hash_map_get(BitSetHashMap* hash_map, BitSet* bs) {
	int bucket_index = get_key(bs) & (hash_map->bucket_count - 1);
	printf("bucket index: %d; ", bucket_index);
	BitSetHashMapElement** bucket = hash_map->buckets[bucket_index];
	int chain_size = hash_map->chain_sizes[bucket_index];
	printf("chain size = %d\n", chain_size);

	for (unsigned int i = 0; i < chain_size; ++i) {
		if (bitset_is_equal(bs, bucket[i]->key)) {
			return bucket[i]->value;
        }
    }

	return NULL;
}
/**
 * Reset a given hash map iterator.
 */
void bitset_hash_map_iterator_init(BitSetHashMapIterator* iterator, BitSetHashMap* map);

/**
 * Get the next element.
 * Returns NULL if there is no next element.
 */
inline void* bitset_hash_map_iterator_next(BitSetHashMapIterator* iterator) {
	void* next = iterator->current_element->previous;
	iterator->current_element = iterator->current_element->previous;
	if (!next) return NULL;
	return ((BitSetHashMapElement*) next)->value;
}


/**
 * Returns the last node in the hash table or NULL if it is empty.
 * Note that we iterate the nodes in hash table in the backward order
 * of insertion, since the implementation is easier and the order of
 * iteration is not relevant for our purposes.
 */
#define BITSET_HASH_MAP_LAST_ELEMENT(hash_map)			hash_map->tail

/**
 * Returns the node that comes before the current node, or NULL if
 * there is none.
 */
#define BITSET_HASH_MAP_PREVIOUS_ELEMENT(current_element)	current_element->previous

#endif
