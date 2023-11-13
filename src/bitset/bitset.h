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
#include "../af/af.h"

// Base type for storing the bits.
#define BITSET_BASE_TYPE	uint64_t

// Number of bits in the base type.
#define BITSET_BASE_SIZE	(8*sizeof(BITSET_BASE_TYPE))

typedef struct bitset BitSet;

// A BitSet is an array of BITSET_BASE_TYPE
struct bitset {
	BITSET_BASE_TYPE* elements;
};

// Create an empty bitset of the given size and return the address.
BitSet* create_bitset(AF *af);

// Free the memory allocated for the bitset bs.
// Returns the number of bytes freed
int free_bitset(AF *af, BitSet* bs);

// Print the given bitset
void print_bitset(AF *af, BitSet* bs, FILE *f);

// Caution: The below methods do not check the index k !
// Set bit at index k
 #define SET_BIT(bs,k)     ( bs->elements[(k)/BITSET_BASE_SIZE] |= (1UL << ((k)%BITSET_BASE_SIZE)) )
// Reset bit at index k
#define RESET_BIT(bs,k)   ( bs->elements[(k)/BITSET_BASE_SIZE] &= ~(1UL << ((k)%BITSET_BASE_SIZE)) )
// Test bit at index k
#define TEST_BIT(bs,k)    ( bs->elements[(k)/BITSET_BASE_SIZE] & (1UL << ((k)%BITSET_BASE_SIZE)) )

// Caution: The methods below do not check and compare the sizes of bs1 and bs2

// Return 1 if bs1 is subset of bs2, otherwise 0.
char bitset_is_subset(AF *af, BitSet* bs1, BitSet* bs2);

char bitset_is_equal(AF *af, BitSet* bs1, BitSet* bs2);

// Intersect bs1 and bs2, store the result in r
void bitset_intersection(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r);

// Unite bs1 and bs2, store the result in r
void bitset_union(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r);

// Negate bitset (flip the bits) bs and store the result in r.
void complement_bitset(AF *af, BitSet* bs, BitSet* r);

// Compute set difference bs1 \ bs2, store it in r.
void bitset_set_minus(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r);

// Return true if all bits are set, otherwise false
char bitset_is_fullset(AF *af, BitSet* bs);

// Return true if all bits are 0, otherwise false
char bitset_is_emptyset(AF *af, BitSet* bs);

// Copy bs1 into bs2
void copy_bitset(AF *af, BitSet* bs1, BitSet* bs2);

// Clear all bits
void reset_bitset(AF *af, BitSet* bs);

// Set all bits
void set_bitset(AF *af, BitSet* bs);

// Return the number of bits set to 1
// int bitset_get_length(BitSet* bs);

#endif /* BITSET_BITSET_H_ */
