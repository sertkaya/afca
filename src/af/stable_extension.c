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
	BitSet* current_up = create_bitset(current->size);
	BitSet* current_down = create_bitset(current->size);

	for (i = not_attacks->size - 1; i >= 0; --i) {
		// printf("i: %d\n",i);
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			BitSet* tmp = create_bitset(current->size);
			bitset_intersection(current, attacks->a[i], tmp);
			// check if argument i attacks the set current
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

			/*
			BitSet *current_down = create_bitset(current->size);
			prime_attr_obj(not_attacks, current, current_down);
			if (!TEST_BIT(current_down,i))
				// i is not a candidate bit
				continue;

			prime_obj_attr(not_attacks, current, current_up);
			if (!TEST_BIT(current_up,i))
				// i is not a candidate bit
				continue;
				*/

			reset_bitset(current_down);
			SET_BIT(current, i);

			// TODO: Check whether this case is already handled by
			// the above two conditions
			prime_attr_obj(not_attacks, current, current_down);
			if (!bitset_is_subset(current, current_down)) {
				// current attacks itself
				RESET_BIT(current, i);
				continue;
			}

			double_prime_attr_obj(not_attacks, current, next);
			RESET_BIT(current, i);
			// TODO: optimize!
			bitset_set_minus(next, current, current_down);
			flag = 0;
			for (j = 0; j < i; ++j)
				if (TEST_BIT(current_down, j)) {
					flag = 1;
					break;
				}
			if (!flag)
				return(1);
		}
	}
	return(0);
}
void all_stable_extensions(Context* attacks) {
	Context* not_attacks = negate_context(attacks);

	BitSet* bs = create_bitset(attacks->size);
	BitSet* ni = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	int closure_count = 0, stable_extension_count = 0;

	printf("Stable Extensions:\n");
	while (1) {
		++closure_count;
		if (!next_cf_closure(not_attacks, attacks, bs, ni))
			break;
		// printf("*");
		// print_bitset(ni);
		// printf("\n");
		prime_obj_attr(not_attacks, ni, tmp);
		if (bitset_is_equal(ni, tmp)) {
			++stable_extension_count;
			printf(" ");
			print_bitset(ni);
			printf("\n");
		}
		copy_bitset(ni, bs);
	}
	printf("Number of closures: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);
}

/*
void all_stable_extensions(Context* attacks) {
	BitSet* current = create_bitset(attacks->size);
	BitSet* next = create_bitset(attacks->size);
	BitSet* candidate_bits = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	Context* not_attacks = negate_context(attacks);

	while (1) {
		// Compute the candidate arguments
		int i;
		for (i = 0; i < candidate_bits->size; ++i)
			SET_BIT(candidate_bits, i);
		for (i = 0; i < not_attacks->size; ++i)
			if (TEST_BIT(current, i))
				bitset_intersection(candidate_bits, not_attacks->a[i], candidate_bits);
		for (i = 0; i < not_attacks->size;  ++i) {
			if (TEST_BIT(candidate_bits,i) && !bitset_is_subset(current, not_attacks->a[i]))
				RESET_BIT(candidate_bits,i);
		}

		print_bitset(current);
		printf("\n");
		for (i = not_attacks->size - 1; TEST_BIT(candidate_bits, i) && i >= 0; --i) {
			if (TEST_BIT(current, i))
				RESET_BIT(current, i);
			else {
				SET_BIT(current, i);
				// ...
				// intersect the primes, subtract current

				double_prime_attr_obj(not_attacks, current, next);
				RESET_BIT(current, i);
				// TODO: optimize!
				bitset_set_minus(next, current, tmp);
				char flag = 0;
				int j;
				for (j = 0; j < i; ++j)
					if (TEST_BIT(tmp, j)) {
						flag = 1;
						break;
					}
				if (!flag)
					return;
			}
		}

		prime_obj_attr(not_attacks, next, tmp);
		if (bitset_is_equal(next, tmp)) {
			printf(" ");
			print_bitset(next);
			printf("\n");
		}
		copy_bitset(next, current);
	}
}
*/
