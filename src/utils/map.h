/**
 * A simple map interface. Just a macro for bitset hash map.
 */

#ifndef MAP_H_
#define MAP_H_

#include "../bitset/bitset_hash_map.h"

typedef BitSetHashMap Map;
typedef BitSetHashMapIterator MapIterator;


/**
 * Create a map with an underlying hash map of the given size.
 * Returns the map created.
 */
#define MAP_CREATE(size)				bitset_hash_map_create(size)

/**
 * Initialize a map with an underlying hash map of the given size.
 */
#define MAP_INIT(map, size)				bitset_hash_map_init(map, size)

/**
 * Adds the pair (key,value) to the map m.
 * Returns 1 if e is successfully added, 0 otherwise.
 */
#define MAP_PUT(key, value, map)		bitset_hash_map_put(map, key, value)

/**
 * Removes the element with the given key if it is present. The map stays unchanged
 * if the key does not occur in map.
 * Returns 1 if key is removed, 0 otherwise.
 */
#define MAP_REMOVE(key, map)			bitset_hash_map_remove(key, map)

/**
 * Get the value associated with the given key.
 * Returns 1 if the key occurs, 0 otherwise
 */
#define MAP_GET(key, map)				bitset_hash_map_get(map, key)

/**
 * Free the space allocated for the given map.
 * Returns the number of freed bytes.
 */
#define MAP_FREE(map)					bitset_hash_map_free(map)

/**
 * Free the space allocated for the elements of the given map.
 * The space for the set itself is not freed. Intended for
 * maps that are not dynamically created.
 * Returns the number of freed bytes.
 */
#define MAP_RESET(s)					bitset_hash_map_reset(s)

/**
 * Create an iterator for the elements of the map.
 * It is the iterator of the underlying hash map.
 */
#define MAP_ITERATOR_CREATE(map)		bitset_hash_map_iterator_create(map)

/**
 * Create an iterator for the elements of the map.
 * It is the iterator of the underlying hash map.
 */
#define MAP_ITERATOR_INIT(map_it, map)	bitset_hash_map_iterator_init(map_it, map)

/**
 * Get the next element in the map.
 * Gets the next element in the underlying hash map.
 * Elements are not necessarily returned in the order of addition.
 */
#define MAP_ITERATOR_NEXT(map_it)		bitset_hash_map_iterator_next(map_it)

/**
 * Free the space for the given map iterator.
 */
#define MAP_ITERATOR_FREE(map_it)		bitset_hash_map_iterator_free(map_it)

#endif
