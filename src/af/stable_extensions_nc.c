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
char next_closure(Context* not_attacks, Context* attacks, BitSet* current, BitSet* next) {
	int i,j;
	BitSet* tmp = create_bitset(current->size);

	for (i = not_attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			// check if i attacks i
			if (TEST_BIT(attacks->a[i], i))
					continue;

			// check if argument i attacks the set current
			bitset_intersection(current, attacks->a[i], tmp);
			if (!bitset_is_emptyset(tmp))
				continue;

			// check if current attacks i
			char flag = 0;
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
			down_up_arrow(not_attacks, current, next);
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

void all_stable_extensions_nc(Context* attacks, FILE *outfile) {
	Context* not_attacks = negate_context(attacks);

	// reducible_objects(not_attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* nc = create_bitset(attacks->size);
	BitSet* nc_up = create_bitset(attacks->size);

	int closure_count = 0, stable_extension_count = 0;

	while (1) {
		if (!next_closure(not_attacks, attacks, tmp, nc))
			break;
		++closure_count;
		// printf("*");
		// print_bitset(ni, stdout);
		// printf("\n");

		up_arrow(not_attacks, nc, nc_up);
		// ni is closed but has a conflict
		// if (!bitset_is_subset(ni, tmp)) {
		// 	printf("*");
		// 	print_bitset(ni, stdout);
		// 	printf("\n");
		// }
		if (bitset_is_equal(nc, nc_up)) {
			++stable_extension_count;
			print_bitset(nc, outfile);
			fprintf(outfile, "\n");
		}
		copy_bitset(nc, tmp);
	}
	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(nc);
	free_bitset(nc_up);

	free_context(attacks);
	free_context(not_attacks);
}

void one_stable_extension_nc(Context* attacks, FILE *outfile) {
	Context* not_attacks = negate_context(attacks);

	reducible_objects(not_attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* nc = create_bitset(attacks->size);
	BitSet* nc_up = create_bitset(attacks->size);

	int closure_count = 0;

	while (1) {
		if (!next_closure(not_attacks, attacks, tmp, nc))
			break;
		++closure_count;
		// printf("*");
		// print_bitset(ni, stdout);
		// printf("\n");

		up_arrow(not_attacks, nc, nc_up);
		// ni is closed but has a conflict
		// if (!bitset_is_subset(ni, tmp)) {
		// 	printf("*");
		// 	print_bitset(ni, stdout);
		// 	printf("\n");
		// }
		if (bitset_is_equal(nc, nc_up)) {
			print_bitset(nc, outfile);
			fprintf(outfile, "\n");
			break;
		}
		copy_bitset(nc, tmp);
	}
	printf("Number of closures generated: %d\n", closure_count);

	free_bitset(tmp);
	free_bitset(nc);
	free_bitset(nc_up);

	free_context(attacks);
	free_context(not_attacks);
}
