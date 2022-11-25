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
#define BITSET_BASE_TYPE	uint64_t
// Number of bytes in the base type.
#define BITSET_BASE_SIZE	(8*sizeof(BITSET_BASE_TYPE))

typedef struct bitset BitSet;

// A BitSet is an array of BITSET_BASE_TYPE
struct bitset {
	int base_count;
	int size;
	BITSET_BASE_TYPE* a;
};

// Create an empty bitset of the given size and return the address.
BitSet* create_bitset(int size);

// Print the given bitset
void print_bitset(BitSet* bs);

// Caution: The below methods do not check the index k !
// Set bit at index k
 #define SET_BIT(bs,k)     ( bs->a[(k)/BITSET_BASE_SIZE] |= (1UL << ((k)%BITSET_BASE_SIZE)) )
// Reset bit at index k
#define RESET_BIT(bs,k)   ( bs->a[(k)/BITSET_BASE_SIZE] &= ~(1UL << ((k)%BITSET_BASE_SIZE)) )
// Test bit at index k
#define TEST_BIT(bs,k)    ( bs->a[(k)/BITSET_BASE_SIZE] & (1UL << ((k)%BITSET_BASE_SIZE)) )

// Return 1 if bs1 is subset of bs2, otherwise 0.
char is_subset(BitSet* bs1, BitSet* bs2);

#endif /* BITSET_BITSET_H_ */
