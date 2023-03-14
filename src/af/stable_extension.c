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
#include <inttypes.h>

#include "../fca/context.h"
#include "../bitset/bitset.h"

// Compute the next conflict-free closure coming after "current" and store it in "next"
char next_cf_closure(Context* not_attacks, Context* attacks, BitSet* current, BitSet* next) {
	int i,j;
	BitSet* tmp = create_bitset(current->size);

	for (i = not_attacks->size - 1; i >= 0; --i) {
		// printf("i: %d\n",i);
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			char flag = 0;

			// check if argument i attacks the set current
			bitset_intersection(current, attacks->a[i], tmp);
			if (!bitset_is_emptyset(tmp))
				continue;

			// check if current attacks i
			for (j = 0; j < current->size; ++j) {
				if (TEST_BIT(current,j) && TEST_BIT(attacks->a[j], i)) {
					flag = 1;
					break;
				}
			}
			if (flag)
				continue;

			reset_bitset(tmp);
			SET_BIT(current, i);

			// compute next
			double_prime_attr_obj(not_attacks, current, next);
			RESET_BIT(current, i);
			// TODO: optimize!
			bitset_set_minus(next, current, tmp);
			flag = 0;
			for (j = 0; j < i; ++j)
				// check if next \ current contains a bit larger than i
				if (TEST_BIT(tmp, j)) {
					flag = 1;
					break;
				}
			if (!flag)
				return(1);
		}
	}
	return(0);
}

void all_stable_extensions(Context* attacks, FILE *outfile) {
	Context* not_attacks = negate_context(attacks);

	BitSet* bs = create_bitset(attacks->size);
	BitSet* ni = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	int closure_count = 0, stable_extension_count = 0;

	while (1) {
		if (!next_cf_closure(not_attacks, attacks, bs, ni))
			break;
		++closure_count;
		// printf("*");
		// print_bitset(ni);
		// printf("\n");
		prime_obj_attr(not_attacks, ni, tmp);
		if (bitset_is_equal(ni, tmp)) {
			++stable_extension_count;
			// printf(" ");
			print_bitset(ni, outfile);
			fprintf(outfile, "\n");
		}
		copy_bitset(ni, bs);
	}
	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);
}
