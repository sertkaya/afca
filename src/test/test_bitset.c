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

	printf("is_subset: %d\n", bitset_is_subset(bs1, bs2));

	BitSet* r = create_bitset(size);
	bitset_intersection(bs1, bs2, r);
	printf("r: ");
	print_bitset(r);

	return(0);
}
