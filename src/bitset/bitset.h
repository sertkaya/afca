/*
 * AFCA - argumentation framework using closed sets
 *
 * Copyright (C) Baris Sertkaya (sertkaya@fb2.fra-uas.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BITSET_BITSET_H_
#define BITSET_BITSET_H_

#include <stdio.h>
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
	BITSET_BASE_TYPE* elements;
};

// Create an empty bitset of the given size and return the address.
BitSet* create_bitset(int size);

// Free the memory allocated for the bitset bs.
void free_bitset(BitSet* bs);

// Print the given bitset
void print_bitset(BitSet* bs, FILE *f);

// Caution: The below methods do not check the index k !
// Set bit at index k
 #define SET_BIT(bs,k)     ( bs->elements[(k)/BITSET_BASE_SIZE] |= (1UL << ((k)%BITSET_BASE_SIZE)) )
// Reset bit at index k
#define RESET_BIT(bs,k)   ( bs->elements[(k)/BITSET_BASE_SIZE] &= ~(1UL << ((k)%BITSET_BASE_SIZE)) )
// Test bit at index k
#define TEST_BIT(bs,k)    ( bs->elements[(k)/BITSET_BASE_SIZE] & (1UL << ((k)%BITSET_BASE_SIZE)) )

// Return 1 if bs1 is subset of bs2, otherwise 0.
// Caution: The methods below do not check and compare the sizes of bs1 and bs2
char bitset_is_subset(BitSet* bs1, BitSet* bs2);

char bitset_is_equal(BitSet* bs1, BitSet* bs2);

// Intersect bs1 and bs2, store the result in r
void bitset_intersection(BitSet* bs1, BitSet* bs2, BitSet* r);

// Negate bitset (flip the bits) bs and store the result in r.
void negate_bitset(BitSet* bs, BitSet* r);

// Compute set difference bs1 \ bs2, store it in r.
void bitset_set_minus(BitSet* bs1, BitSet* bs2, BitSet* r);

// Return true if all bits are set, otherwise false
char bitset_is_fullset(BitSet* bs);

// Return true if all bits are 0, otherwise false
char bitset_is_emptyset(BitSet* bs);

// Copy bs1 into bs2
void copy_bitset(BitSet* bs1, BitSet* bs2);

// Clear all bits
void reset_bitset(BitSet* bs);
#endif /* BITSET_BITSET_H_ */
