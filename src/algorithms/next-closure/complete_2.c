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
#include "../../utils/linked_list.h"
#include "../../utils/stack.h"
#include "../../utils/timer.h"
#include "../../af/sort.h"

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

// s: the set to be closed
// r: the closure of s
// r_bits: bool array representation of r
// TODO: Caution! We assume that s and r do not contain double values!
void closure_semi_complete(AF* af, AF* af_t, ArrayList* s, ArrayList* r, bool *r_bv) {
	Stack update;
	init_stack(&update);

	// empty r
	list_reset(r);
	// empty r_bv
	memset(r_bv, 0, af->size * sizeof(bool));
	/*
	for (SIZE_TYPE i = 0; i < attacks->size; ++i)
		r_bv[i] = false;
		*/

	// Push elements of s to the stack, add to r and to r_bv
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		push(&update, s->elements[i]);
		list_add(s->elements[i], r);
		r_bv[s->elements[i]] = true;
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	// TODO: This is independent of s. It can be done outside the closure function.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i)
		if (af_t->list_sizes[i] == 0) {
			push(&update, i);
			list_add(i, r);
			r_bv[i] = true;
		}

	SIZE_TYPE* unattacked_attackers_count = calloc(af_t->size, sizeof(SIZE_TYPE));
	assert(unattacked_attackers_count != NULL);
	memcpy(unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));

	bool* victims_a = calloc(af->size, sizeof(bool));
	assert(victims_a != NULL);
	memset(victims_a, 0, af->size * sizeof(bool));
	// TODO: initialize all to false? check if required

	SIZE_TYPE a = pop(&update);
	while (a != -1) {
		if (af->list_sizes[a] == 0) {
			// a does not attack anybody, pop and continue
			a = pop(&update);
			continue;
		}
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!victims_a[victim_a]) {
				victims_a[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--unattacked_attackers_count[victim_victim_a];
					if ((unattacked_attackers_count[victim_victim_a] == 0) && !r_bv[victim_victim_a])  {
						push(&update, victim_victim_a);
						list_add(victim_victim_a, r);
						r_bv[victim_victim_a] = true;
					}
				}
			}
		}
		a = pop(&update);
	}
	free(unattacked_attackers_count);
	free(victims_a);
}

bool next_conflict_free_semi_complete_intent(AF* attacks, AF* attacked_by, ArrayList* current, ArrayList* current_closure) {
	bool* current_bv = calloc(attacks->size, sizeof(bool));
	assert(current_bv!=NULL);
	// copy_bitset(current, tmp);
	for (SIZE_TYPE i = 0; i < attacks->size; ++i)
		current_bv[current->elements[i]] = true;

	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);

	for (int i = attacks->size - 1; i >= 0; --i) {
		// if (TEST_BIT(tmp, i)) {
		if (current_bv[i]) {
			// RESET_BIT(tmp, i);
			current_bv[i] = false;
			// remove i from current
			list_remove(i, current);

		} else if (!check_arg_attacks_arg(attacks, i, i) &&
				   !check_arg_attacks_set(attacks, i, current) &&
				   !check_set_attacks_arg(attacks, current, i)) {

			// SET_BIT(tmp, i);
			current_bv[i] = true;
			// TODO: add i to current
			list_add(i, current);

			closure_semi_complete(attacks, attacked_by, current, current_closure, current_closure_bv);

			bool good = true;
			// is next canonical?
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (current_closure_bv[j] && !current_bv[j]) {
					good = false;
					break;
				}
			}
			if (good) {
				// is next conflict-free?
				for (SIZE_TYPE j = i + 1; j < attacks->size; ++j) {
					if (current_closure_bv[j] && check_arg_attacks_set(attacks, j, current_closure)) { // &&  check_set_attacks_arg(attacks, tmp, j)) {
						good = false;
						break;
					}
				}
			}
			if (good) {
				free(current_bv);
				free(current_closure_bv);
				// printf("it is conflict-free\n");
				return(1);
			}
			// RESET_BIT(tmp, i);
			current_bv[i] = false;
			// remove i from current
			list_remove(i, current);
		}
	}
	free(current_bv);
	free(current_closure_bv);
	return(0);
}

ArrayList* dc_co_next_closure(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	struct timeval start_time, stop_time;
	printf("argument: %d\n", argument);

	ArrayList* current = list_create();
	ArrayList* current_closure = list_create();
	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);

	list_add(argument, current);

	/*
	printf("attacks:");
	print_argumentation_framework(attacks);
	printf("attacked_by:");
	print_argumentation_framework(attacked_by);
	*/
	printf("current:");
	print_list(stdout, current, "\n");

	closure_semi_complete(attacks, attacked_by, current, current_closure, current_closure_bv);

	printf("current_closure:");
	print_list(stdout, current_closure, "\n");


	// if (!is_set_consistent(attacks, current_closure)) {
	if (!is_set_consistent(attacks, current_closure)) {
		// closure has a conflict. complete extension
		// does not exist.
		printf("=== dc_co_next_closure finished 1===\n");
		return(NULL);
	}

	START_TIMER(start_time);
	// Check if closure is self-defending
	START_TIMER(start_time);
	if (is_set_self_defending(attacks, attacked_by, current_closure)) {
		// closure is a complete extension containing the argument
		STOP_TIMER(stop_time);
		printf("is_set_self_defending time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
		printf("=== dc_co_next_closure finished 2===\n");
		return(current_closure);
	}
	// Not self-defending. That is, the argument is not defended.
	// reset_bitset(current);
	list_reset(current_closure);

	/*
	printf("HERE 1\n");

	// Sort in descending order of victims
	AF* attacks_sorted = create_argumentation_framework(attacks->size);
	int *mapping = sort_af(attacks, attacks_sorted, VICTIM_COUNT, SORT_DESCENDING);

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

	// Now move the argument to the very left bit ...
	swap_arguments(attacks_sorted, 0, argument_index);
	swap_arguments(attacked_by_sorted, 0, argument_index);
	int tmp = mapping[0];
	mapping[0] = mapping[argument_index];
	mapping[argument_index] = tmp;
	// and set the very left bit
	SET_BIT(current, 0);

*/

	int concept_count = 0;
	/*
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
	} while (next_conflict_free_semi_complete_intent(attacks_sorted, attacked_by_sorted, current, current, attacks_adj, attacked_by_adj));
	*/

	while (next_conflict_free_semi_complete_intent(attacks, attacked_by, current, current_closure)) {
		++concept_count;
		if (is_set_self_defending(attacks, attacked_by, current_closure)) {
			free(current_closure_bv);
			free_argumentation_framework(attacks);
			free_argumentation_framework(attacked_by);
			return(current_closure);
		}
	}

	printf("Number of concepts generated: %d\n", concept_count);

	free(current_closure_bv);
	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("=== dc_co_next_closure finished 3===\n");
	return(NULL);
}


ArrayList* dc_co_subgraph(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
	for (SIZE_TYPE i = 0; i < subgraph->af->size; ++i)
		printf("%d %d %d\n", i, subgraph->mapping_from_subgraph[i], subgraph->mapping_to_subgraph[i]);
	AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	STOP_TIMER(stop_time);
	printf("Extracting and transposing subgraph: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// solve DC-CO in the subgraph
	START_TIMER(start_time);
	ArrayList *extension = dc_co_next_closure(subgraph_t, subgraph->mapping_to_subgraph[argument], subgraph->af);
	STOP_TIMER(stop_time);
	printf("dc_co_next_closure_adj: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	// TODO!
	// free_argumentation_framework(attacks_adj);
	// free_argumentation_framework(attacked_by_projection_adj)

	if (!extension)
		return(NULL);

	// print_set(extension,stdout,"\n");
	// close the computed extension in the whole framework
	/*
	START_TIMER(start_time);
	BitSet* back_projected_extension = project_back(extension, projection, af->size);
	STOP_TIMER(stop_time);
	printf("Projecting af back: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	BitSet *closure = create_bitset(af->size);
	// closure_semi_complete(af, attacked_by, back_projected_extension, extension);
	*/

	// closure_semi_complete_adj(af, attacked_by, back_projected_extension, closure, attacks_adj, attacked_by_adj);
	ArrayList* closure = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	closure_semi_complete(attacks, attacked_by, extension, closure, closure_bv);

	return(closure);
}