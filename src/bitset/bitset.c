/*
 * bitarray.c
 *
 *  Created on: 20.11.2022
 *      Author: bs
 */

#include "bitset.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

BitSet* create_bitset(int size) {

	int base_count = (size / BITSET_BASE_SIZE);
	if (size % BITSET_BASE_SIZE)
		++base_count;

	BitSet* bs = (BitSet*) calloc(1,  sizeof(BitSet));
	assert(bs != NULL);

	bs->base_count = base_count;
	bs->size = size;
	bs->a = (BITSET_BASE_TYPE*) calloc(base_count,  BITSET_BASE_SIZE / 8);
	assert(bs->a != NULL);

	return(bs);
}

void print_bitset(BitSet* bs) {
	int i;
	for (i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			printf("%d", 1);
		else
			printf("%d", 0);
}

char is_subset(BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		if (bs1->a[i] != (bs1->a[i] & bs2->a[i]))
			return(0);
	return(1);
}

void intersection(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		r->a[i] = bs1->a[i] & bs2->a[i];
}

