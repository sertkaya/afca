/*
 * test_bit_array.c
 *
 *  Created on: 20.11.2022
 *      Author: bs
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../bitset/bitset.h"

int main(int argc, char *argv[]) {

	printf("BITSET_BASE_SIZE: %d\n", BITSET_BASE_SIZE);

	BitSet *bs = create_bitset(20);

	int i;
	for (i = 0; i < bs->size; ++i) {
		SET_BIT(bs,i);
		print_bitset(bs);
	}

	return(0);
}
