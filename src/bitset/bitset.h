/*
 * AFCA - argumentation framework using closed sets
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
#include <math.h>
#include <stdint.h>

// Base type for storing the bits.
#define BITSET_BASE_TYPE	uint64_t

// Number of bits in the base type.
#define BITSET_BASE_SIZE	(8*sizeof(BITSET_BASE_TYPE))

typedef unsigned int SIZE_TYPE;

typedef struct bitset BitSet;

// A BitSet is an array of BITSET_BASE_TYPE
struct bitset {
	SIZE_TYPE size;
	unsigned short base_count;
	BITSET_BASE_TYPE* elements;
};

// Create an empty bitset of the given size and return the address.
BitSet* create_bitset(SIZE_TYPE);

// Free the memory allocated for the bitset bs.
// Returns the number of bytes freed
int free_bitset(BitSet* bs);

// Print the given bitset with the given ending
void print_set(BitSet* bs, FILE *f, const char *end);

// Print the given bitset as set
void print_bitset(BitSet* bs, FILE *f);

// Caution: The below methods do not check the index k !
// Set bit at index k
 #define SET_BIT(bs,k)		( bs->elements[(k)/BITSET_BASE_SIZE] |= (1UL << ((k)%BITSET_BASE_SIZE)) )
// Reset bit at index k
#define RESET_BIT(bs,k)		( bs->elements[(k)/BITSET_BASE_SIZE] &= ~(1UL << ((k)%BITSET_BASE_SIZE)) )
// Test bit at index k
#define TEST_BIT(bs,k)		( bs->elements[(k)/BITSET_BASE_SIZE] & (1UL << ((k)%BITSET_BASE_SIZE)) )

// Caution: The methods below do not check and compare the sizes of bs1 and bs2

// Return the number of bits set to 1
// int bitset_get_length(BitSet* bs);

// Return 1 if bs1 is subset of bs2, otherwise 0.
static inline char bitset_is_subset(BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		if (bs1->elements[i] != (bs1->elements[i] & bs2->elements[i]))
			return(0);
	return(1);
}

inline char bitset_is_equal(BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		if (bs1->elements[i] != bs2->elements[i])
			return(0);
	return(1);
}

// Intersect bs1 and bs2, store the result in r
static inline void bitset_intersection(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		r->elements[i] = bs1->elements[i] & bs2->elements[i];
}

// Check if intersection bs1 and bs2 is empty
// Return 1 if yes, 0 othersie
inline char is_bitset_intersection_empty(BitSet* bs1, BitSet* bs2) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		if ((bs1->elements[i] & bs2->elements[i]) != 0UL)
			return(0);
	return(1);
}
// Unite bs1 and bs2, store the result in r
inline void bitset_union(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		r->elements[i] = bs1->elements[i] | bs2->elements[i];
}

// Negate bitset (flip the bits) bs and store the result in r.
inline void complement_bitset(BitSet* bs, BitSet* r) {
	int i;
	for (i = 0; i < bs->base_count; ++i)
		r->elements[i] = ~(bs->elements[i]);

}

// Clear all bits
inline void reset_bitset(BitSet* bs) {
	int i;

	for (i = 0; i < bs->base_count; ++i)
		bs->elements[i] = 0UL;
}

// Compute set difference bs1 \ bs2, store it in r.
inline void bitset_set_minus(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	// reset_bitset(r);
	for (i = 0; i < bs1->base_count; ++i)
		r->elements[i] = bs1->elements[i] & ~(bs2->elements[i]);
}

// Copy bs1 into bs2
inline void copy_bitset(BitSet* bs1, BitSet* bs2) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		bs2->elements[i] = bs1->elements[i];
}

// Return true if all bits are set, otherwise false
// TODO: optimize!
inline char bitset_is_fullset(BitSet* bs) {
	int i;
	for (i = 0; i < bs->size; ++i)
		if (!(TEST_BIT(bs, i)))
			return 0;
	return(1);
}

// Return true if all bits are 0, otherwise false
inline char bitset_is_emptyset(BitSet* bs) {
	int i;

	for (i = 0; i < bs->base_count; ++i)
		if (bs->elements[i] != 0UL)
			return(0);
	return(1);
}

inline SIZE_TYPE count_bits(BitSet* bs) {
    SIZE_TYPE count = 0;
    for (SIZE_TYPE i = 0; i < bs->size; ++i) {
        if (TEST_BIT(bs, i)) {
            ++count;
        }
    }
	return count;
}

// Set all bits
inline void set_bitset(BitSet* bs) {
/*
	int i;
	for (i = 0; i < bs->base_count; ++i)
		// bs->elements[i] = -1;
		bs->elements[i] = ~(0UL);
*/
	for (SIZE_TYPE i = 0; i < bs->size; ++i) {
		SET_BIT(bs, i);
	}
}

// key for hashing
inline BITSET_BASE_TYPE get_key(BitSet* bs) {
	BITSET_BASE_TYPE key = 0;
	for (int i = 0; i < bs->base_count; ++i) {
		key ^= bs->elements[i];
	}
	return key;
}

#endif /* BITSET_BITSET_H_ */
