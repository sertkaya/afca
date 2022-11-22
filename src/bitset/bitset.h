/*
 * bitarray.h
 *
 *  Created on: 20.11.2022
 *      Author: bs
 */

#ifndef BITSET_BITSET_H_
#define BITSET_BITSET_H_

#include <stdint.h>

// Base type for storing the bits.
#define BITSET_BASE_TYPE	unsigned long int
// Number of bytes in the base type.
#define BITSET_BASE_SIZE	(8*sizeof(BITSET_BASE_TYPE))

typedef struct bitset BitSet;

struct bitset {
	// The number of bits in the bitset.
	int size;
	// Array for storing the bits.
	BITSET_BASE_TYPE* array;
};

// Create an empty bitset of the given size and return the address.
BitSet* create_bitset(int size);

// Print the given bitset
void print_bitset(BitSet *bs);

// Caution: The below methods do not check the index k !
// Set bit at index k
#define SET_BIT(bs,k)     ( bs->array[(k)/BITSET_BASE_SIZE] |= (1ULL << ((k)%BITSET_BASE_SIZE)) )
// Reset bit at index k
#define RESET_BIT(bs,k)   ( b->arrays[(k)/BITSET_BASE_SIZE] &= ~(1ULL << ((k)%BITSET_BASE_SIZE)) )
// Test bit at index k
#define TEST_BIT(bs,k)    ( bs->array[(k)/BITSET_BASE_SIZE] & (1ULL << ((k)%BITSET_BASE_SIZE)) )

#endif /* BITSET_BITSET_H_ */
