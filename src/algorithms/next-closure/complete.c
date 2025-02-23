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
#include "../connected-components/scc.h"

#include <string.h>

#include "../../utils/linked_list.h"
#include "../../utils/stack.h"
#include "../../af/sort.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

BitSet* peaceful_arguments;

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
bool next_conflict_free_semi_complete_intent(AF* attacks, AF* attacked_by, BitSet* current, BitSet* next, BitSet** lectically_smaller_defended) {
	BitSet* tmp = create_bitset(attacks->size);
	copy_bitset(current, tmp);

	BitSet* tmp_complement = create_bitset(attacks->size);
	complement_bitset(tmp, tmp_complement);

	// printf("tmp: ");
	// print_bitset(tmp, stdout);
	// printf(" ");
	// print_set(tmp, stdout, "\n");
	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(tmp, i)) {
			RESET_BIT(tmp, i);
		} else if (!TEST_BIT(peaceful_arguments, i) &&
				   // adding i to tmp will not make tmp self-defending.
				   // because tmp is not self-defending and i does not attack any arguments.
				   // in this case continue with the next i
			       !CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, tmp) &&
				   // !TEST_BIT(defends_lectically_smaller_arg, i) &&
				   is_bitset_intersection_empty(tmp_complement, lectically_smaller_defended[i]) &&
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
				free_bitset(tmp_complement);
				// printf("it is conflict-free\n");
				return(1);
			}
			RESET_BIT(tmp, i);
		}
	}

	free_bitset(tmp);
	free_bitset(tmp_complement);
	return(0);
}

// Returns an array of bitsets. The bitset at index i contains the lectically smaller
// arguments that are defended by i.
BitSet** get_lectically_smaller_defended_arguments(AF* attacks, AF* attacked_by) {
	int defends_lectically_smaller_count = 0;
	BitSet** defends_lectically_smaller_args = calloc(sizeof(BitSet*), attacks->size);
	assert(defends_lectically_smaller_args != NULL);
	int i;
	for (i = 0; i < attacks->size; ++i) {
		defends_lectically_smaller_args[i] = create_bitset(attacks->size);
		int j;
		bool defends = false;
		for (j = 0; j < attacks->size; ++j) {
			if (j < i && bitset_is_subset(attacked_by->graph[j], attacks->graph[i])) {
				// printf("%d defends %d\n", i + 1 , j + 1);
				SET_BIT(defends_lectically_smaller_args[i], j);
				defends = true;
			}
		}
		if (defends)
			++defends_lectically_smaller_count;
	}
	printf("Defends lectically smaller count: %d\n", defends_lectically_smaller_count);
	return(defends_lectically_smaller_args);
}


ListNode* ee_co_next_closure(AF *attacks) {
	AF* attacked_by = transpose_argumentation_framework(attacks);
	// AF* attacked_by_cp = create_argumentation_framework(attacked_by->size);

	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);

	BitSet* current = create_bitset(attacks->size);
	BitSet* next = create_bitset(attacks->size);

	SIZE_TYPE i;

	// peaceful arguments are those, that do not attack any arguments
	peaceful_arguments = create_bitset(attacks->size);

	int peaceful_args_count = 0;
	int attacks_one = 0;
	int attacks_more_than_half = 0;
	int self_defending_args_count = 0;
	int self_attacking_args_count = 0;
	int safe_args_count = 0;
	int single_attacker_count = 0;
	int multiple_attacker_count = 0;
	int defends_lect_smaller_count = 0;

	for (i = 0; i < attacks->size; ++i) {
		if (bitset_is_emptyset(attacks->graph[i])) {
			++peaceful_args_count;
			SET_BIT(peaceful_arguments, i);
		}
		if (CHECK_ARG_ATTACKS_ARG(attacks,i,i))
			++self_attacking_args_count;

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
	}


	printf("Arguments attacking 0 arguments: %d\n", peaceful_args_count);
	printf("Arguments attacking 1 argument: %d\n", attacks_one);
	printf("Arguments attacking more than %d arguments: %d\n", attacks->size / 10, attacks_more_than_half);
	printf("Arguments with zero attackers: %d\n", safe_args_count);
	printf("Arguments with a single attacker: %d\n", single_attacker_count);
	printf("Arguments with multiple attackers: %d\n", multiple_attacker_count);
	printf("Self-defending arguments: %d\n", self_defending_args_count);
	printf("Self-attacking arguments: %d\n", self_attacking_args_count);
	printf("Arguments defending a lectically smaller argument: %d\n", defends_lect_smaller_count);

	int concept_count = 0, complete_extension_count = 0;
	ListNode* extensions = NULL;

	BitSet** lectically_smaller_defended = get_lectically_smaller_defended_arguments(attacks, attacked_by);

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
	} while (next_conflict_free_semi_complete_intent(attacks, attacked_by, current, current, lectically_smaller_defended));

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


bool next_conflict_free_semi_complete_intent_2(AF* attacks, AF* attacked_by, BitSet* current, BitSet* next) {
	BitSet* tmp = create_bitset(attacks->size);
	copy_bitset(current, tmp);

	BitSet* tmp_complement = create_bitset(attacks->size);
	complement_bitset(tmp, tmp_complement);

	for (int i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(tmp, i)) {
			RESET_BIT(tmp, i);
		} else if ( !CHECK_ARG_ATTACKS_ARG(attacks, i, i) &&
				   !CHECK_ARG_ATTACKS_SET(attacks, i, tmp) &&
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
				free_bitset(tmp_complement);
				// printf("it is conflict-free\n");
				return(1);
			}
			RESET_BIT(tmp, i);
		}
	}

	free_bitset(tmp);
	free_bitset(tmp_complement);
	return(0);
}

BitSet* dc_co_next_closure_2(AF* attacks, int argument) {
	BitSet* current = create_bitset(attacks->size);
	AF* attacked_by = transpose_argumentation_framework(attacks);

	SET_BIT(current, argument);
	closure_semi_complete(attacks, attacked_by, current, current);
	if (!is_set_conflict_free(attacks, current)) {
		// closure has a conflict. complete extension
		// does not exist.
		return(NULL);
	}

	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);
	// get attackers of current
	get_attackers(attacked_by, current, attackers);
	// get victims of current
	get_victims(attacks, current, victims);

	// Check if current is self-defending
	if (bitset_is_subset(attackers, victims)) {
		// current is a complete extension containing the argument
		return(current);
	}
	// Not self-defending. That is, the argument is not defended.
	reset_bitset(current);

	// Sort in descending order of victims
	AF* attacks_sorted = create_argumentation_framework(attacks->size);
	int *mapping = sort_af(attacks, attacks_sorted, VICTIM_COUNT, SORT_DESCENDING);

	/*
	printf("attacks:\n");
	print_argumentation_framework(attacks);
	printf("attacks_sorted:\n");
	print_argumentation_framework(attacks_sorted);
	printf("mapping:\n");
	for (int i = 0; i < attacks_sorted->size; ++i)
		printf("%d %d\n", i+1, mapping[i]+1);
		*/

	// Search the argument in the mapping
	int argument_index;
	for (int i = 0; i < attacks_sorted->size; ++i)
		if (mapping[i] == argument)
			// The argument is at index i after sorting
			argument_index = i;

	// Move defenders of the argument to the right-end.
	AF* attacked_by_sorted = transpose_argumentation_framework(attacks_sorted);
	int next_index_to_use = attacks_sorted->size - 1;
	for (int i = attacks_sorted->size - 1; i >= 0; --i) {
		int tmp;
		// check if a defender of the (mapped) argument
		if (bitset_is_subset(attacked_by_sorted->graph[argument_index], attacks_sorted->graph[i])) {
			// index i of attacks_sorted is a defender of the argument, swap index_to_use and i
			// do the swap in the frameworks
			swap_arguments(attacks_sorted, next_index_to_use, i);
			swap_arguments(attacked_by_sorted, next_index_to_use, i);
			// do the swap in the mapping
			tmp = mapping[next_index_to_use];
			mapping[next_index_to_use] = mapping[i];
			mapping[i] = tmp;

			// We do not need to check if the argument itself is also moved. This cannot be the case
			// since otherwise the set current would be self-defending.
			--next_index_to_use;
		}
	}

	/*
	printf("mapping:\n");
	for (int i = 0; i < attacks_sorted->size; ++i)
		printf("%d %d\n", i+1, mapping[i]+1);
		*/

	// Now move the argument to the very left bit ...
	swap_arguments(attacks_sorted, 0, argument_index);
	swap_arguments(attacked_by_sorted, 0, argument_index);
	int tmp = mapping[0];
	mapping[0] = mapping[argument_index];
	mapping[argument_index] = tmp;
	// and set the very left bit
	SET_BIT(current, 0);

	/*
	printf("attacks_sorted:\n");
	print_argumentation_framework(attacks_sorted);
	printf("mapping:\n");
	for (int i = 0; i < attacks_sorted->size; ++i)
		printf("%d %d\n", i+1, mapping[i]+1);
		*/

	int concept_count = 0;
	do {
		// print_bitset(current, stdout);
		// printf("\n");
		// print_set(current, stdout, "\n");
		++concept_count;
		get_attackers(attacked_by_sorted, current, attackers);
		get_victims(attacks_sorted, current, victims);
		// Check if current is self-defending
		if (bitset_is_subset(attackers, victims)) {
			printf("Number of concepts generated: %d\n", concept_count);
			free_bitset(attackers);
			free_bitset(victims);
			return(map_indices(current, mapping));
		}
	} while (next_conflict_free_semi_complete_intent_2(attacks_sorted, attacked_by_sorted, current, current));

	printf("Number of concepts generated: %d\n", concept_count);

	free_bitset(attackers);
	free_bitset(victims);
	free_argumentation_framework(attacks_sorted);
	free_argumentation_framework(attacked_by_sorted);

	// reset_bitset(current);
	return(NULL);

}


BitSet* dc_co_subgraph_next_closure(AF* af, int argument) {
	BitSet *subgraph = create_bitset(af->size);
	BitSet *arguments = create_bitset(af->size);
	set_bitset(arguments);

	// extract nodes of the subgraph induced by argument
	backward_dfs(af, argument, arguments, subgraph);

	// the projected framework induced by argument
	PAF *projection = project_argumentation_framework(af, subgraph);
	printf("Projected framework size: %d\n", projection->af->size);

	// find argument in the projected framework
	SIZE_TYPE i, projected_argument;
	for (i = 0; i < projection->af->size; ++i) {
		if (projection->parent_mapping[i] == argument) {
			projected_argument = i;
			break;
		}
	}
	if (i == projection->af->size)
		printf("THIS SHOULD NOT HAPPEN!\n\n");
	// solve DC-CO for the projected framework
	BitSet *extension = dc_co_next_closure_2(projection->af, projected_argument);

	if (!extension)
		return(NULL);

	// print_set(extension,stdout,"\n");
	// close the computed extension in the whole framework
	BitSet* back_projected_extension = project_back(extension, projection);
	AF *attacked_by = transpose_argumentation_framework(af);
	BitSet *closure = create_bitset(af->size);
	// closure_semi_complete(af, attacked_by, back_projected_extension, extension);
	closure_semi_complete(af, attacked_by, back_projected_extension, closure);
	return(closure);
}

// Assuming arguments are sorted in descending order of victim count: -s 1 -d 1
// af is the sorted framework. argument is the mapped argument
BitSet* dc_co_next_closure(AF* af, int argument) {
	BitSet* current = create_bitset(af->size);
	if (CHECK_ARG_ATTACKS_ARG(af, argument, argument))
		return(current);

	AF* attacked_by = transpose_argumentation_framework(af);
	closure_semi_complete(af, attacked_by, current, current);
	if (check_set_attacks_arg(af, current, argument)) { // || CHECK_ARG_ATTACKS_SET(af, argument, current)) {
		reset_bitset(current);
		return(current);
	}

	// swap the argument with the left-most argument (the argument at index 0).
	// TODO: think about the case where argument is peaceful (has no victims).
	// TODO: does the algorithm still work for this case? the assumption, the peaceful
	// TODO: arguments are the right-most ones will not be true any more.
	// display the af
	// for (int i = 0; i < af->size; ++i) {
	// 	print_set(af->graph[i], stdout, "\n");
	// }
	// printf("1 <-> %d\n", argument + 1);

	BitSet* tmp = create_bitset(af->size);
	copy_bitset(af->graph[0], tmp);
	copy_bitset(af->graph[argument], af->graph[0]);
	copy_bitset(tmp, af->graph[argument]);
	int i,j;
	for (i = 0; i < af->size; ++i) {
		bool bit_0 = TEST_BIT(af->graph[i], 0);
		bool bit_arg = TEST_BIT(af->graph[i], argument);
		if (bit_0)
			SET_BIT(af->graph[i], argument);
		else
			RESET_BIT(af->graph[i], argument);
		if (bit_arg)
			SET_BIT(af->graph[i], 0);
		else
			RESET_BIT(af->graph[i], 0);
	}

	// display the af
	// for (i = 0; i < af->size; ++i) {
	// 	print_set(af->graph[i], stdout, "\n");
	// atta}

	attacked_by = transpose_argumentation_framework(af);

	// peaceful arguments are those, that do not attack any arguments
	peaceful_arguments = create_bitset(af->size);
	for (i = 0; i < af->size; ++i) {
		if (bitset_is_emptyset(af->graph[i])) {
			SET_BIT(peaceful_arguments, i);
		}
	}

	BitSet* attackers = create_bitset(af->size);
	BitSet* victims = create_bitset(af->size);

	BitSet** lectically_smaller_defended = get_lectically_smaller_defended_arguments(af, attacked_by);

	BitSet* next = create_bitset(af->size);
	// set bit at index 0 (this is the desired argument)
	SET_BIT(current, 0);

	closure_semi_complete(af, attacked_by, current, current);

	int concept_count = 0;
	do {
		++concept_count;
		// print_set(current, stdout, "\n");
		// print_bitset(current, stdout);
		// printf("\n");
		get_attackers(attacked_by, current, attackers);
		get_victims(af, current, victims);
		// Check if current is self-defending
		if (bitset_is_subset(attackers, victims)) {

			bool bit_0 = TEST_BIT(current, 0);
			bool bit_arg = TEST_BIT(current, argument);
			if (bit_0)
				SET_BIT(current, argument);
			else
				RESET_BIT(current, argument);
			if (bit_arg)
				SET_BIT(current, 0);
			else
				RESET_BIT(current, 0);

			printf("Number of concepts generated: %d\n", concept_count);
			free_bitset(next);
			free_bitset(attackers);
			free_bitset(victims);
			free_bitset(peaceful_arguments);
			free_argumentation_framework(attacked_by);
			return(current);
		}
	} while (next_conflict_free_semi_complete_intent(af, attacked_by, current, current, lectically_smaller_defended));

	printf("Number of concepts generated: %d\n", concept_count);

	free_bitset(next);
	free_bitset(attackers);
	free_bitset(victims);
	free_bitset(peaceful_arguments);
	free_argumentation_framework(attacked_by);

	reset_bitset(current);
	return(current);
}
