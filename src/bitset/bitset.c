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
	BitSet *bs = (BitSet*) calloc(1, sizeof(BitSet));
	assert(bs != NULL);

	int base_count = (size / BITSET_BASE_SIZE);
	if (size % BITSET_BASE_SIZE)
		++base_count;

	bs->size = size;
	bs->array = (BITSET_BASE_TYPE*) calloc(base_count,  BITSET_BASE_SIZE / 8);
	assert(bs->array != NULL);

	return(bs);
}

void print_bitset(BitSet *bs) {
	int i;
	for (i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			printf("%d", 1);
		else
			printf("%d", 0);
	printf("\n");
}
