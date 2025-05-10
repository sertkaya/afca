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

#include "../../utils/stack.h"
#include "../../utils/priority_queue.h"
#include "../../utils/timer.h"

struct state {
	ArrayList *set;
	bool *processed;
};

typedef struct state State;

State *create_state_adm(SIZE_TYPE size) {
	State *s = calloc(1, sizeof(State));
	assert(s != NULL);

	s->set = list_create();

	s->processed = calloc(size, sizeof(bool));
	assert(s->processed != NULL);

	return(s);
}

State *duplicate_state_adm(State *s, SIZE_TYPE size) {
	State *n = calloc(1, sizeof(State));
	assert(n != NULL);

	n->set = list_duplicate(s->set);

	n->processed = calloc(size, sizeof(bool));
	assert(n->processed != NULL);
	memcpy(n->processed, s->processed, size * sizeof(bool));

	return(n);
}

void delete_state_adm(State *s) {
	list_free(s->set);
	s->set = NULL;
	free(s->processed);
	s->processed = NULL;
	free(s);
}

// true if l1 is a subset of l2
// l1 and l2 have to be sorted
bool check_subset_sorted_array(ARG_TYPE *l1, int l1_size, ArrayList *l2) {
	if (l1_size > l2->size)
		return(false);
	SIZE_TYPE index_l1 = 0;
	for (SIZE_TYPE i = 0; i < l2->size && index_l1 < l1_size && l1[index_l1] <= l2->elements[i]; ++i) {
		if (l2->elements[i] == l1[index_l1]) {
			++index_l1;
			// if (index_l1 == l1_size)
			// 	break;
		}
	}
	if (index_l1 == l1_size)
		return(true);
	else
		return(false);
}

static int closure_count = 0;

State *sd_complement_first_closure(AF *attacks, AF *attacked_by, ARG_TYPE searched_argument) {
	++closure_count;
	State *next = create_state_adm(attacks->size);
	bool updated = false;
	do {
		updated = false;
		for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[i]; ++j) {
				ARG_TYPE victim_i = attacks->lists[i][j];
				if (next->processed[victim_i])
					continue;
				if (!check_arg_attacks_arg_sorted(attacks, victim_i, i)) {
					if (check_subset_sorted_array(attacked_by->lists[i], attacked_by->list_sizes[i], next->set)) {
						if (victim_i == searched_argument) {
							delete_state_adm(next);
							return(NULL);
						}
						if (next->processed[victim_i]) {
							delete_state_adm(next);
							return(NULL);
						}
						list_add(victim_i, next->set);
						// TODO: optimize!
						list_sort(next->set);
						next->processed[victim_i] = true;
						updated = true;
					}
				}
			}
		}
	} while (updated);
	return(next);
}

State *sd_complement_next_closure(AF *attacks, AF *attacked_by, ARG_TYPE searched_argument, ARG_TYPE new_argument, State *current) {
	++closure_count;
	current->processed[new_argument] = true;
	State *next = duplicate_state_adm(current, attacks->size);
	list_add(new_argument, next->set);
	list_sort(next->set);
	bool updated = false;
	do {
		updated = false;
		for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[i]; ++j) {
				ARG_TYPE victim_i = attacks->lists[i][j];
				if (next->processed[victim_i])
					continue;
				if (!check_arg_attacks_arg_sorted(attacks, victim_i, i)) {
					if (check_subset_sorted_array(attacked_by->lists[i], attacked_by->list_sizes[i], next->set)) {
						if (victim_i == searched_argument) {
							delete_state_adm(next);
							return(NULL);
						}
						if (current->processed[victim_i]) {
							delete_state_adm(next);
							return(NULL);
						}
						list_add(victim_i, next->set);
						// TODO: optimize!
						list_sort(next->set);
						next->processed[victim_i] = true;
						updated = true;
					}
				}
			}
		}
	} while (updated);
	return(next);
}

ARG_TYPE find_candidate_argument(State *current, ARG_TYPE searched_argument, AF *attacks, AF *attacked_by) {
	ARG_TYPE candidate = -1;
	// self-conflict
	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		if (current->processed[i] || i == searched_argument)
			continue;
		if (check_arg_attacks_arg_sorted(attacks, i, i))
			return(i);
	}
	int  max_conflict_count = 0;
	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		if (current->processed[i] || i == searched_argument)
			continue;
		int conflict_count = 0;
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[i]; ++j) {
			ARG_TYPE victim_i = attacks->lists[i][j];
			if (!current->processed[victim_i]) {
				++conflict_count;
			}
		}
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[i]; ++j) {
			ARG_TYPE attacker_i = attacked_by->lists[i][j];
			if (!current->processed[attacker_i]) {
				++conflict_count;
			}
		}
		if (conflict_count > max_conflict_count) {
			max_conflict_count = conflict_count;
			candidate = i;
		}
	}
	return(candidate);
}

ArrayList* dc_adm_cbo(ARG_TYPE searched_argument, AF* attacks,  AF *attacked_by) {
	struct timeval start_time, stop_time;

	Stack states;
	init_stack(&states);

	State *current = sd_complement_first_closure(attacks, attacked_by, searched_argument);
	// closure of the empty set contains the searched argument.
	// admissible extension with the searched argument does not exist.
	if (!current)
		return(NULL);

	// if complement of the closure is conflict-free, then
	// admissible extension is found. Return it.
	ArrayList *complement = array_list_complement_sorted(current->set, attacks->size);
	if (is_set_conflict_free(attacks, complement)) {
		return(complement);
	}

	push(&states, new_stack_element_ptr(current));

	while (current =  pop_ptr(&states)) {
		print_list(stdout, current->set, "\n");
		ARG_TYPE new_argument;
		while ((new_argument = find_candidate_argument(current, searched_argument, attacks, attacked_by)) != -1) {
		// for (ARG_TYPE new_argument = 0; new_argument < attacks->size; ++new_argument) {
		// 	if (current->processed[new_argument] || new_argument == searched_argument) {
		// 		continue;
		// 	}
			// TODO: find the "best" unprocessed argument to add!

			State *next = sd_complement_next_closure(attacks, attacked_by, searched_argument, new_argument, current);

			if (next == NULL) {
				continue;
			}

			// if complement of the closure is conflict-free, then
			// admissible extension is found. Return it.
			ArrayList *complement = array_list_complement_sorted(next->set, attacks->size);
			if (is_set_conflict_free(attacks, complement)) {
				return(complement);
			}

			// otherwise push next to the stack
			push(&states, new_stack_element_ptr(next));
		}
		delete_state_adm(current);
		current = NULL;
	}
	free_argumentation_framework(attacks);

	return(NULL);
}

ArrayList* dc_adm_subgraph_cbo(AF* attacks, ARG_TYPE argument) {
	struct timeval start_time, stop_time;

	printf("Argument: %d\n", argument);
	AF* attacked_by = transpose_argumentation_framework(attacks);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
	AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	// print_argumentation_framework(subgraph_t);
	// sort the adjacency lists of subgraph->af and subgraph_t
	for (SIZE_TYPE i = 0; i < subgraph->af->size; ++i) {
		qsort(subgraph->af->lists[i], subgraph->af->list_sizes[i], sizeof(ARG_TYPE), compare_argument_ids);
		qsort(subgraph_t->lists[i], subgraph_t->list_sizes[i], sizeof(ARG_TYPE), compare_argument_ids);
	}
	STOP_TIMER(stop_time);
	printf("Extracting subgraph, transposing and sorting took: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	/*
	START_TIMER(start_time);
	ListNode *implications_subgraph = create_implications_from_af(subgraph->af, subgraph_t);
	ListNode *tmp = implications_subgraph;
	while (tmp) {
		print_implication(tmp->e->p);
		tmp = tmp->next;
	}
	STOP_TIMER(stop_time);
	printf("Creating implications took: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	*/

	// Solve DC-AD in the subgraph:
	START_TIMER(start_time);
	// ArrayList *extension_subgraph = dc_adm_cbo(subgraph->mapping_to_subgraph[argument], subgraph->af, subgraph_t, implications_subgraph);
	ArrayList *extension_subgraph = dc_adm_cbo(subgraph->mapping_to_subgraph[argument], subgraph->af, subgraph_t);
	STOP_TIMER(stop_time);
	printf("dc_adm_cbo: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	if (!extension_subgraph) {
		// Extension does not exist in the subgraph.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	// extension_subgraph is an admissible extension (in the subgraph) containing the argument.
	// Then it is also an admissible extension of the whole framework (TODO: Check this!!!)
	// Map indices of this extension back to the whole framework and return
	ArrayList *extension = list_create();
	for (SIZE_TYPE i = 0; i < extension_subgraph->size; ++i)
		list_add(subgraph->mapping_from_subgraph[extension_subgraph->elements[i]], extension);

	printf("Closure count: %d\n", closure_count);
	return(extension);
}