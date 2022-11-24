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

BitSet create_bitset(int size) {

	int base_count = (size / BITSET_BASE_SIZE);
	if (size % BITSET_BASE_SIZE)
		++base_count;

	BitSet bs = (BitSet) calloc(base_count,  BITSET_BASE_SIZE / 8);
	assert(bs != NULL);

	return(bs);
}

void print_bitset(BitSet bs, int size) {
	int i;
	for (i = 0; i < size; ++i)
		if (TEST_BIT(bs, i))
			printf("%d", 1);
		else
			printf("%d", 0);
}

// TODO
char is_subset(BitSet bs1, BitSet bs2, int size) {
	int i;

	return(0);
}
