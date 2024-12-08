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

#include "../../af/af.h"
#include "complete.h"

#include <string.h>

#include "../../utils/linked_list.h"
#include "../../utils/stack.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

BitSet* defends_lectically_smaller_arg;

void closure_semi_complete(AF* attacks, AF* attacked_by, BitSet* s, BitSet* r) {
	SIZE_TYPE i;
	copy_bitset(s, r);
	bool closure_modified;

	BitSet* victims_r = create_bitset(attacks->size);
	for (i = 0; i < attacks->size; ++i) {
		if (TEST_BIT(r, i))
			bitset_union(victims_r, attacks->graph[i], victims_r);
	}
	do {
		closure_modified = false;
		for (i = 0; i < attacks->size; i++) {
			if (!TEST_BIT(r, i) && bitset_is_subset(attacked_by->graph[i], victims_r)) {
				SET_BIT(r, i);
				bitset_union(victims_r, attacks->graph[i], victims_r);
				closure_modified = true;
			}
		}
	} while (closure_modified);
	free_bitset(victims_r);
}

// Peaceful arguments have to be the rightmost bits of the arguments set
bool next_conflict_free_semi_complete_intent(AF* attacks, AF* attacked_by, BitSet* peaceful_arguments, BitSet* current, BitSet* next) {
	BitSet* tmp = create_bitset(attacks->size);
	copy_bitset(current, tmp);

	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(tmp, i)) {
			RESET_BIT(tmp, i);
		} else if (!TEST_BIT(peaceful_arguments, i) &&
				   // adding i to tmp will not make tmp self-defending.
				   // because tmp is not self-defending and i does not attack any arguments.
				   // in this case continue with the next i
			       !CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, tmp) &&
				   !TEST_BIT(defends_lectically_smaller_arg, i) &&
				   !check_set_attacks_arg(attacks, tmp, i)) {


			SET_BIT(tmp, i);
			closure_semi_complete(attacks, attacked_by, tmp, next);


			bool good = true;
			// is next canonical?
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (TEST_BIT(next, j) && !TEST_BIT(tmp, j)) {
					good = false;
					break;
				}
			}
			if (good) {
				// is next conflict-free?
				for (SIZE_TYPE j = i + 1; j < attacks->size; ++j) {
					if (TEST_BIT(next, j) && CHECK_ARG_ATTACKS_SET(attacks, j, next) &&  check_set_attacks_arg(attacks, tmp, j)) {
						good = false;
						break;
					}
				}
			}
			if (good) {
				free_bitset(tmp);
				// printf("it is conflict-free\n");
				return(1);
			}
			RESET_BIT(tmp, i);
		}
	}

	free_bitset(tmp);
	return(0);
}

ListNode* ee_co_next_closure(AF *attacks) {
	AF* attacked_by = transpose_argumentation_framework(attacks);
	// AF* attacked_by_cp = create_argumentation_framework(attacked_by->size);

	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);

	BitSet* current = create_bitset(attacks->size);
	BitSet* next = create_bitset(attacks->size);

	defends_lectically_smaller_arg = create_bitset(attacks->size);

	SIZE_TYPE i;

	// peaceful arguments are those, that do not attack any arguments
	BitSet* peaceful_arguments = create_bitset(attacks->size);
	int peaceful_args_count = 0;
	int attacks_one = 0;
	int attacks_more_than_half = 0;
	int self_defending_args_count = 0;
	int safe_args_count = 0;
	int single_attacker_count = 0;
	int multiple_attacker_count = 0;
	int defends_lect_smaller_count = 0;
	for (i = 0; i < attacks->size; ++i) {
		if (bitset_is_emptyset(attacks->graph[i])) {
			++peaceful_args_count;
			SET_BIT(peaceful_arguments, i);
		}
		if (count_bits(attacks->graph[i]) == 1)
			++attacks_one;
		if (count_bits(attacks->graph[i]) > (attacks->size / 10))
			++attacks_more_than_half;

		if (bitset_is_subset(attacked_by->graph[i], attacks->graph[i])) {
			++self_defending_args_count;
		}
		//`if (bitset_is_emptyset(attacked_by->graph[i])) {
		if (count_bits(attacked_by->graph[i]) == 0) {
			++safe_args_count;
		}
		if (count_bits(attacked_by->graph[i]) == 1)
			++single_attacker_count;
		if (count_bits(attacked_by->graph[i]) > 1)
			++multiple_attacker_count;
		SIZE_TYPE j;
		for (j = 0; j < attacks->size; ++j) {
			if (j < i && bitset_is_subset(attacked_by->graph[j], attacks->graph[i])) {
				SET_BIT(defends_lectically_smaller_arg, i);
			}
		}
		++defends_lect_smaller_count;
	}
	fflush(stdout);

	printf("Arguments attacking 0 arguments: %d\n", peaceful_args_count);
	printf("Arguments attacking 1 argument: %d\n", attacks_one);
	printf("Arguments attacking more than %d arguments: %d\n", attacks->size / 10, attacks_more_than_half);
	printf("Arguments with zero attackers: %d\n", safe_args_count);
	printf("Arguments with a single attacker: %d\n", single_attacker_count);
	printf("Arguments with multiple attackers: %d\n", multiple_attacker_count);
	printf("Self-defending arguments: %d\n", self_defending_args_count);
	printf("Arguments defending a lectically smaller argument: %d\n", defends_lect_smaller_count);

	int concept_count = 0, complete_extension_count = 0;
	ListNode* extensions = NULL;

	// closure of the empty set
	closure_semi_complete(attacks, attacked_by, current, current);
	do {
		++concept_count;
		// print_set(current, stdout, "\n");
		// print_bitset(current, stdout);
		// printf("\n");
		get_attackers(attacked_by, current, attackers);
		get_victims(attacks, current, victims);
		// Check if current is self-defending
		if (bitset_is_subset(attackers, victims)) {
			BitSet *co_ext = create_bitset(attacks->size);
			++complete_extension_count;
			copy_bitset(current, co_ext);
			extensions = insert_list_node(co_ext, extensions);
		}
	} while (next_conflict_free_semi_complete_intent(attacks, attacked_by, peaceful_arguments, current, current));

	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of complete extensions: %d\n", complete_extension_count);

	free_bitset(current);
	free_bitset(next);
	free_bitset(attackers);
	free_bitset(victims);
	free_bitset(peaceful_arguments);

	free_argumentation_framework(attacked_by);
	return(extensions);
}