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


#include <string.h>
#include <stdio.h>
#include <time.h>

#include "../../af/af.h"
#include "complete.h"
#include "../connected-components/scc.h"
#include "../../utils/linked_list.h"
#include "../../utils/stack.h"
#include "../../utils/timer.h"
#include "../../af/sort.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

// s: the set to be closed
// r: the closure of s
// TODO: Caution! We assume that s and r do not contain double values!
void closure_semi_complete(AF* attacks, AF* attacked_by, List* s, List* r) {
	Stack update;
	init_stack(&update);

	// Push elements of s to the stack
	for (SIZE_TYPE i = 0; i < s->size; ++i)
		push(&update, s->elements[i]);

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i)
		if (attacked_by->list_sizes[i] == 0) {
			push(&update, i);
		}

	SIZE_TYPE* unattacked_attackers_count = calloc(attacked_by->size, sizeof(SIZE_TYPE));
	assert(unattacked_attackers_count != NULL);
	memcpy(unattacked_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));

	bool* victims_a = calloc(attacks->size, sizeof(bool));
	assert(victims_a != NULL);
	// TODO: initialize all to false? check if required

	SIZE_TYPE a = pop(&update);
	while (a != -1) {
		if (attacks->list_sizes[a] == 0) {
			// a does not attack anybody, pop and continue
			a = pop(&update);
			continue;
		}
		for (SIZE_TYPE i = 0; i < attacks->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = attacks->lists[a][i];
			if (!victims_a[victim_a]) {
				victims_a[victim_a] = true;
				for (SIZE_TYPE j = 0; j < attacks->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = attacks->lists[victim_a][j];
					--unattacked_attackers_count[victim_victim_a];
					if ((unattacked_attackers_count[victim_victim_a] == 0)) { // && (!TEST_BIT(r, victim_victim_a)))  {
						push(&update, victim_victim_a);
					}
				}
			}
		}
		a = pop(&update);
	}
	free(unattacked_attackers_count);
	free(victims_a);

	// STOP_TIMER(stop_time);
	// printf("closure_semi_complete_adj time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	/*
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
	*/
}

bool next_conflict_free_semi_complete_intent_adj(AF* attacks, AF* attacked_by, BitSet* current, BitSet* next, AF_ADJ_L *attacks_adj, AF_ADJ_L *attacked_by_adj) {
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
			closure_semi_complete_adj(attacks, attacked_by, tmp, next, attacks_adj, attacked_by_adj);

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

BitSet* dc_co_next_closure(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	List* current = list_create();
	List* current_closure = list_create();

	list_add(current, argument);
	closure_semi_complete(attacks, attacked_by, current, current_closure);

	if (!is_set_conflict_free(attacks, current)) {
		// closure has a conflict. complete extension
		// does not exist.
		printf("=== dc_co_next_closure finished ===\n");
		return(NULL);
	}
	STOP_TIMER(stop_time);
	printf("Checking conflict-free time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	START_TIMER(start_time);

	BitSet* attackers = create_bitset(attacks->size);
	BitSet* victims = create_bitset(attacks->size);
	// get attackers of current
	get_attackers(attacked_by, current, attackers);
	// get victims of current
	get_victims(attacks, current, victims);

	STOP_TIMER(stop_time);
	printf("Getting attackers and victims time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// Check if current is self-defending
	START_TIMER(start_time);
	if (bitset_is_subset(attackers, victims)) {
		// current is a complete extension containing the argument
		STOP_TIMER(stop_time);
		printf("bitset_is_subset time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
		printf("=== dc_co_next_closure finished ===\n");
		return(current);
	}
	// Not self-defending. That is, the argument is not defended.
	reset_bitset(current);

	printf("HERE 1\n");

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

	printf("HERE 2\n");
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
	printf("HERE 3\n");

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
		print_set(current, stdout, "\n");
		++concept_count;
		get_attackers(attacked_by_sorted, current, attackers);
		get_victims(attacks_sorted, current, victims);
		// Check if current is self-defending
		if (bitset_is_subset(attackers, victims)) {
			printf("Number of concepts generated: %d\n", concept_count);
			free_bitset(attackers);
			free_bitset(victims);
			printf("=== dc_co_next_closure finished ===\n");
			return(map_indices(current, mapping));
		}
	} while (next_conflict_free_semi_complete_intent_adj(attacks_sorted, attacked_by_sorted, current, current, attacks_adj, attacked_by_adj));

	printf("Number of concepts generated: %d\n", concept_count);

	free_bitset(attackers);
	free_bitset(victims);
	free_argumentation_framework(attacks_sorted);
	free_argumentation_framework(attacked_by_sorted);

	// reset_bitset(current);
	printf("=== dc_co_next_closure finished ===\n");
	return(NULL);
}

AF* extract_subgraph(AF* attacked_by, ARG_TYPE argument) {
	AF* subgraph = create_argumentation_framework(attacked_by->size);

	Stack s;
	init_stack(&s);

	bool* visited = calloc(attacked_by->size, sizeof(bool));
	assert(visited != NULL);

	SIZE_TYPE a = argument;
	while (a != -1) {
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[a]; ++i) {
			if (!visited[attacked_by->lists[a][i]]) {
				push(&s, attacked_by->lists[a][i]);
				visited[attacked_by->lists[a][i]] = true;
				add_attack(subgraph, a, i);
			}
		}
		a = pop(&s);
	}
	free(visited);
	return(subgraph);
}

List* dc_co_subgraph(AF* attacks, int argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	bool* subgraph_nodes = calloc(attacks->size, sizeof(bool));
	assert(subgraph_nodes != NULL);
	AF* subgraph = extract_subgraph(attacked_by, argument);
	STOP_TIMER(stop_time);
	printf("Extracting subgraph: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// solve DC-CO in the subgraph
	START_TIMER(start_time);
	BitSet *extension = dc_co_next_closure(subgraph, projected_argument, attacks_projection_adj, attacked_by_projection_adj);
	STOP_TIMER(stop_time);
	printf("dc_co_next_closure_adj: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	// TODO!
	// free_argumentation_framework(attacks_adj);
	// free_argumentation_framework(attacked_by_projection_adj)

	if (!extension)
		return(NULL);

	// print_set(extension,stdout,"\n");
	// close the computed extension in the whole framework
	START_TIMER(start_time);
	BitSet* back_projected_extension = project_back(extension, projection, af->size);
	STOP_TIMER(stop_time);
	printf("Projecting af back: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	BitSet *closure = create_bitset(af->size);
	// closure_semi_complete(af, attacked_by, back_projected_extension, extension);


	closure_semi_complete_adj(af, attacked_by, back_projected_extension, closure, attacks_adj, attacked_by_adj);
	return(closure);
}