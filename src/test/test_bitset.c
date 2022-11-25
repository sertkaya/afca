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

	int size = 20;
	BitSet* bs = create_bitset(size);

	int i;
	for (i = 0; i < size; ++i) {
		SET_BIT(bs,i);
		print_bitset(bs);
		printf("\n");
	}

	return(0);
}
