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
#include "../../utils/linked_list.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

// TODO: Optimize
void closure_semi_complete(AF* attacks, BitSet* s, BitSet* r) {
	SIZE_TYPE i;
	copy_bitset(s, r);
	bool closure_modified;
	do {
		closure_modified = false;
		for (i = 0; i < attacks->size; i++) {
			if (!TEST_BIT(r, i) && check_set_defends_arg(attacks, r, i)) {
				SET_BIT(r, i);
				closure_modified = true;
			}
		}
	} while (closure_modified);
}

/*
BitSet* next_conflict_free_semi_complete_intent(AF* attacks, BitSet* previous) {
	BitSet* current = create_bitset(attacks->size);
	copy_bitset(previous, current);
	BitSet* next = create_bitset(attacks->size);

	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i)) {
			RESET_BIT(current, i);
		} else if (!CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, current) &&
				   !check_set_attacks_arg(attacks, current, i)) {
			SET_BIT(current, i);
			closure_semi_complete(attacks, current, next);

			bool good = true;
			// is next canonical?
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (TEST_BIT(next, j) && !TEST_BIT(current, j)) {
					good = false;
					break;
				}
			}
			if (good) {
				// is next conflict-free?
				for (SIZE_TYPE j = i + 1; j < attacks->size; ++j) {
					if (TEST_BIT(next, j) && CHECK_ARG_ATTACKS_SET(attacks, j, next) &&  check_set_attacks_arg(attacks, current, j)) {
						good = false;
						break;
					}
				}
			}
			if (good) {
				free_bitset(current);
				return next;
			}
			RESET_BIT(current, i);
		}
	}

	free_bitset(current);
	free_bitset(next);
	return(NULL);
}
*/

bool next_conflict_free_semi_complete_intent(AF* attacks, BitSet* current, BitSet* next) {
	BitSet* tmp = create_bitset(attacks->size);
	copy_bitset(current, tmp);

	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(tmp, i)) {
			RESET_BIT(tmp, i);
		} else if (!CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, tmp) &&
				   !check_set_attacks_arg(attacks, tmp, i)) {
			SET_BIT(tmp, i);
			closure_semi_complete(attacks, tmp, next);

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
	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);

	BitSet* current = create_bitset(attacks->size);
	BitSet* next = create_bitset(attacks->size);


	int concept_count = 0, complete_extension_count = 0;
	ListNode* extensions = NULL;

	// closure of the empty set
	closure_semi_complete(attacks, current, current);
	do {
		++concept_count;
		print_bitset(current, stdout);
		printf("\n");
		// printf("current: ");
		get_attackers(attacked_by, current, attackers);
		get_victims(attacks, current, victims);
		// Check if next is admissible
		if (bitset_is_subset(attackers, victims)) {
			// print_set(current, stdout, "\n");
			// printf("attackers: ");
			// print_set(attackers, stdout, "\n");
			// printf("victims: ");
			// print_set(victims, stdout, "\n");
			BitSet *co_ext = create_bitset(attacks->size);
			++complete_extension_count;
			copy_bitset(current, co_ext);
			extensions = insert_list_node(co_ext, extensions);
		}
	} while (next_conflict_free_semi_complete_intent(attacks, current, current));

	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of complete extensions: %d\n", complete_extension_count);

	free_bitset(current);
	free_bitset(next);

	return(extensions);
}