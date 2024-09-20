/*
 * AFCA - argumentation framework using closed sets
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

#include "../af/af.h"
#include "../bitset/bitset.h"
#include "../../utils/list.h"

// Compute the next conflict-free closure coming after "current" and store it in "next"
char next_conflict_free_closure(AF* not_attacks, AF* attacks, BitSet* current, BitSet* next) {
	BitSet* tmp = create_bitset(attacks->size);

	for (int i = not_attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			// check if i attacks i
			if (CHECK_ARG_ATTACKS_ARG(attacks,i, i))
				continue;

			// check if argument i attacks the set current
			if (CHECK_ARG_ATTACKS_SET(attacks, i, current))
				continue;

			// check if current attacks i
			if (check_set_attacks_arg(attacks, current, i))
				continue;

			// TODO: Compare what is more efficient:
			// The three checks above or adding "i" to "current" and checking
			// whether the result is conflict-free

			reset_bitset(tmp);
			SET_BIT(current, i);

			// compute next
			down_up_arrow(not_attacks, current, next);
			RESET_BIT(current, i);

			// TODO: optimize!
			bitset_set_minus(next, current, tmp);
			char canonicity_test_passed = 1;
			for (SIZE_TYPE j = 0; j < i; ++j)
				// check if next \ current contains a bit larger than i
				if (TEST_BIT(tmp, j)) {
					canonicity_test_passed = 0;
					break;
				}
			if (canonicity_test_passed) {
				free_bitset(tmp);
				return(1);
			}
		}
	}
	free_bitset(tmp);
	return(0);
}

void ee_st_next_closure(AF *attacks, List* result) {

	AF *not_attacks = complement_argumentation_framework(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* c = create_bitset(attacks->size);
	// up-arrow of c
	BitSet* c_up = create_bitset(attacks->size);

	int concept_count = 0, stable_extension_count = 0;
	while (1) {
		if (!next_conflict_free_closure(not_attacks, attacks, tmp, c))
			break;
		++concept_count;
		// up-arrow of c
		up_arrow(not_attacks, c, c_up);

		if (bitset_is_equal(c, c_up)) {
			BitSet *st_ext = create_bitset(attacks->size);
			++stable_extension_count;
			copy_bitset(c, st_ext);
			list_add(st_ext, result);
			// print_set(c, stdout, "\n");
		}
		copy_bitset(c, tmp);
	}
	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(c);
	free_bitset(c_up);

	free_argumentation_framework(not_attacks);

	return;
}

void se_st_next_closure(AF* attacks, BitSet* result) {
	AF* not_attacks = complement_argumentation_framework(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* c = create_bitset(attacks->size);
	BitSet* c_up = create_bitset(attacks->size);

	int concept_count = 0;

	while (1) {
		if (!next_conflict_free_closure(not_attacks, attacks, tmp, c))
			break;
		++concept_count;

		up_arrow(not_attacks, c, c_up);

		if (bitset_is_equal(c, c_up)) {
			copy_bitset(c, result);
			break;
		}
		copy_bitset(c, tmp);
	}
	printf("Number of concepts generated: %d\n", concept_count);
	free_bitset(tmp);
	free_bitset(c);
	free_bitset(c_up);
	free_argumentation_framework(not_attacks);
	return;
}

void dc_st_next_closure(AF* attacks, int argument, BitSet* result) {
	if (CHECK_ARG_ATTACKS_ARG(attacks, argument, argument))
		return;

	int i;
	int *mapping = (int *) calloc(attacks->size, sizeof(int));
	assert(mapping != NULL);
	// first fill the mapping with the usual indices
	for (i = 0; i < attacks->size; ++i) {
		mapping[i] = i;
	}
	// move the conflicting arguments to the beginning
	int p = 0, temp = 0, index_argument = argument;
	// the mask will be used later for terminating the next-closure loop
	BitSet* mask = create_bitset(attacks->size);
	for (i = 0; i < attacks->size; ++i) {
		if ((CHECK_ARG_ATTACKS_ARG(attacks, i, argument) || CHECK_ARG_ATTACKS_ARG(attacks, argument, i))) {
			printf("i: %d\n", i);
			temp = mapping[p];
			mapping[p] = mapping[i];
			mapping[i] = temp;
			SET_BIT(mask, p);
			++p;
			// remember the index of the argument
			if (mapping[i] == argument)
				index_argument = i;
		}
	}
	// move argument to right after the conflicting ones
	temp = mapping[p];
	mapping[p] = argument;
	mapping[index_argument] = temp;

	// display the mapping
	// for (i = 0; i < attacks->size; ++i)
	// 	printf("%d ", mapping[i] + 1);
	// printf("\n");

	AF *attacks_mapped = create_argumentation_framework(attacks->size);
	// fill in the new af according to the mapping
	int j;
	for (i = 0; i < attacks->size; ++i) {
		for (j = 0; j < attacks->size; ++j) {
			if (TEST_BIT(attacks->graph[mapping[i]], mapping[j]))
				SET_BIT(attacks_mapped->graph[i], j);
		}
	}

	// complement of the mapped attacks relation
	AF* not_attacks_mapped = complement_argumentation_framework(attacks_mapped);

	// print_argumentation_framework(not_attacks_mapped);

	BitSet* tmp = create_bitset(attacks->size);
	// set the argument bit. this is the starting point
	SET_BIT(tmp, argument);
	BitSet* c = create_bitset(attacks->size);
	BitSet* c_up = create_bitset(attacks->size);


	int concept_count = 0;

	while (is_bitset_intersection_empty(tmp, mask)) {
		if (!next_conflict_free_closure(not_attacks_mapped, attacks_mapped, tmp, c))
			break;
		++concept_count;

		up_arrow(not_attacks_mapped, c, c_up);

		if (bitset_is_equal(c, c_up)) {
			// TODO:
			// indices of result need to be mapped back to original indices
			copy_bitset(c, result);
			break;
		}
		copy_bitset(c, tmp);
	}
	printf("Number of concepts generated: %d\n", concept_count);
	free_bitset(tmp);
	free_bitset(c);
	free_bitset(c_up);
	free_argumentation_framework(attacks_mapped);
	free_argumentation_framework(not_attacks_mapped);
}
