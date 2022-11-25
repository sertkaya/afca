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


	BitSet* bs1 = create_bitset(size);
	BitSet* bs2 = create_bitset(size);

	SET_BIT(bs1, 0);
	SET_BIT(bs1, 3);
	SET_BIT(bs1, 11);
	SET_BIT(bs1, 19);

	SET_BIT(bs2, 0);
	SET_BIT(bs2, 3);
	SET_BIT(bs2, 10);
	SET_BIT(bs2, 15);
	SET_BIT(bs2, 19);

	printf("is_subset: %d\n", is_subset(bs1, bs2));

	BitSet* r = create_bitset(size);
	intersection(bs1, bs2, r);
	printf("r: ");
	print_bitset(r);

	return(0);
}
