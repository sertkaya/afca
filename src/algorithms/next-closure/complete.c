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

// bool next_conflict_free_semi_complete_intent_opt(AF* attacks,  ListNode** victims, int *attacker_counts, BitSet* current, BitSet* next) {
// bool next_conflict_free_semi_complete_intent_opt(AF* attacks,  ListNode** victims, AF* attacked_by, BitSet* current, BitSet* next) {
bool next_conflict_free_semi_complete_intent(AF* attacks, AF* attacked_by, BitSet* current, BitSet* next) {
	BitSet* tmp = create_bitset(attacks->size);
	copy_bitset(current, tmp);

	// make a copy of attacker_counts
	// int *attacker_counts_cp = calloc(attacks->size, sizeof(int));
	// assert(attacker_counts_cp != NULL);
	// memcpy(attacker_counts_cp, attacker_counts, sizeof(int) * attacks->size);


	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(tmp, i)) {
			RESET_BIT(tmp, i);
		} else if (!CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, tmp) &&
				   !check_set_attacks_arg(attacks, tmp, i)) {
			SET_BIT(tmp, i);
			// closure_semi_complete_opt(tmp, victims, attacker_counts_cp,  next);
			// closure_semi_complete_opt(tmp, victims, attacked_by,  next);
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
				return(1);
			}
			RESET_BIT(tmp, i);
		}
	}

	free_bitset(tmp);
	// free(attacker_counts_cp);
	return(0);
}

ListNode* ee_co_next_closure(AF *attacks) {
	AF* attacked_by = transpose_argumentation_framework(attacks);
	AF* attacked_by_cp = create_argumentation_framework(attacked_by->size);

	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);

	BitSet* current = create_bitset(attacks->size);
	BitSet* next = create_bitset(attacks->size);

	// int* attacker_counts = calloc(attacks->size, sizeof(int));
	// assert(attacker_counts != NULL);
	ListNode** victims_list = calloc(attacks->size, sizeof(ListNode*));
	assert(victims_list != NULL);

	SIZE_TYPE i;
	long j;
	for (i = 0; i < attacks->size; ++i) {
		for (j = 0; j < attacks->size; ++j) {
			if (CHECK_ARG_ATTACKS_ARG(attacks, i, j)) {
				// ++attacker_counts[j];
				victims_list[i] = insert_list_node((void*) j, victims_list[i]);
			}
		}
	}

	int concept_count = 0, complete_extension_count = 0;
	ListNode* extensions = NULL;

	// closure of the empty set
	closure_semi_complete(attacks, attacked_by, current, current);
	// closure_semi_complete_opt(current, victims_list, attacker_counts, current);
	// closure_semi_complete(attacks, current, current);
	do {
		++concept_count;
		// print_set(current, stdout, "\n");
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
		for (int i = 0; i < attacked_by->size; i++) {
			copy_bitset(attacked_by->graph[i], attacked_by_cp->graph[i]);
		}
	} while (next_conflict_free_semi_complete_intent(attacks, attacked_by_cp, current, current));
	// } while (next_conflict_free_semi_complete_intent_opt(attacks, victims_list, attacker_counts, current, current));
	// } while (next_conflict_free_semi_complete_intent(attacks, current, current));

	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of complete extensions: %d\n", complete_extension_count);

	free_bitset(current);
	free_bitset(next);
	free_bitset(attackers);
	free_bitset(victims);
	/*
	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		free_list(victims_list[i], (void (*)(void *)) free_list_node);
	}
	*/
	free(victims_list);
	free_argumentation_framework(attacked_by);
	return(extensions);
}

/*
// TODO: Optimize
void closure_semi_complete_naive(AF* attacks, BitSet* s, BitSet* r) {
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
*/

// s: BitSet to be closed
// victims: Array of linked lists. Index is argument, value is linked list of its victims
// attacker_counts: Array of integers. Index is argument, value is number of its attackers
// r: Output BitSet. Closure of s.
// void closure_semi_complete_opt(BitSet* s, ListNode** victims, int *attacker_counts, BitSet* r) {
/*
void closure_semi_complete_opt(BitSet* s, ListNode** victims, AF *attacked_by, BitSet* r) {
	copy_bitset(s, r);

	Stack args_to_process;
	init_stack(&args_to_process);

	long i;
	for (i = 0; i < s->size; ++i) {
		if (TEST_BIT(s, i))
			push(&args_to_process, (void*) i);
	}

	long arg = (long) pop(&args_to_process);

	// printf("===\n");
	while(arg != 0) {
		// printf("arg:%ld\n", arg +1);
		ListNode* vp = victims[arg];
		while (vp) {
			// printf("vp->c:%ld\n", (long) vp->c + 1);
			ListNode* vvp = victims[(long) vp->c];
			while (vvp) {
				// printf("vvp->c:%ld\n", (long) vvp->c + 1);
				// if (((--attacker_counts[(long) vvp->c]) == 0) && (!TEST_BIT(r, (long) vvp->c))) {
				RESET_BIT(attacked_by->graph[(long) vvp->c], (long) vp->c);
				// if (((--attacker_counts[(long) vvp->c]) == 0) && (!TEST_BIT(r, (long) vvp->c))) {
				if ((!TEST_BIT(r, (long) vvp->c)) && bitset_is_emptyset(attacked_by->graph[(long) vvp->c])) {
					SET_BIT(r, (long) vvp->c);
					push(&args_to_process, (void*) vvp->c);
				}
				vvp = vvp->next;
			}
			vp = vp->next;
		}
		arg = (long) pop(&args_to_process);
	}
	free_stack(&args_to_process);
}
*/

/*
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
*/

