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

#include "../../utils/stack.h"
#include "../../utils/priority_queue.h"
#include "../../utils/timer.h"

struct state {
	ARG_TYPE new_argument;
	ArrayList *set;
	bool *scheduled;
	bool *conflicts;
	// Number of unattacked attackers of an argument
	SIZE_TYPE* unattacked_attackers_count;
	SIZE_TYPE* attackers_not_in_current;
	bool* victims;
	bool* attackers;
};

typedef struct state State;

State *create_state_ad_bt(SIZE_TYPE size) {
	State *s = calloc(1, sizeof(State));
	assert(s != NULL);

	s->set = list_create();
	s->new_argument = 0;

	s->conflicts = calloc(size, sizeof(bool));
	assert(s->conflicts != NULL);

	s->scheduled = calloc(size, sizeof(bool));
	assert(s->scheduled != NULL);

	s->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->unattacked_attackers_count != NULL);

	s->attackers_not_in_current = calloc(size, sizeof(SIZE_TYPE));
	assert(s->attackers_not_in_current != NULL);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	s->attackers = calloc(size, sizeof(bool));
	assert(s->attackers != NULL);

	return(s);
}

State *duplicate_state_ad_bt(State *s, SIZE_TYPE size) {
	State *n = calloc(1, sizeof(State));
	assert(n != NULL);

	n->new_argument = s->new_argument;
	n->set = list_duplicate(s->set);

	n->conflicts = calloc(size, sizeof(bool));
	assert(n->conflicts != NULL);
	memcpy(n->conflicts, s->conflicts, size * sizeof(bool));

	n->scheduled = calloc(size, sizeof(bool));
	assert(n->scheduled != NULL);
	memcpy(n->scheduled, s->scheduled, size * sizeof(bool));

	n->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->unattacked_attackers_count != NULL);
	memcpy(n->unattacked_attackers_count, s->unattacked_attackers_count, size * sizeof(SIZE_TYPE));

	n->attackers_not_in_current = calloc(size, sizeof(SIZE_TYPE));
	assert(n->attackers_not_in_current != NULL);
	memcpy(n->attackers_not_in_current, s->attackers_not_in_current, size * sizeof(SIZE_TYPE));

	n->victims = calloc(size, sizeof(bool));
	assert(n->victims != NULL);
	memcpy(n->victims, s->victims, size * sizeof(bool));

	n->attackers = calloc(size, sizeof(bool));
	assert(n->attackers != NULL);
	memcpy(n->attackers, s->attackers, size * sizeof(bool));

	return(n);
}

void delete_state_ad_bt(State *s) {
	list_free(s->set);
	s->set = NULL;
	free(s->conflicts);
	s->conflicts = NULL;
	free(s->scheduled);
	s->scheduled = NULL;
	free(s->victims);
	s->victims = NULL;
	free(s->attackers);
	s->attackers = NULL;
	free(s->unattacked_attackers_count);
	s->unattacked_attackers_count = NULL;
	free(s->attackers_not_in_current);
	s->attackers_not_in_current = NULL;
	free(s);
}

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;

void extend_current(State *current, AF *af, AF *af_t) {
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		if (current->scheduled[i])
			continue;
		bool all_attackers_in_current = true;
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[i]; ++j) {
			if (!current->attackers[af_t->lists[i][j]]) {
				all_attackers_in_current = false;
				break;
			}
		}
		if (all_attackers_in_current && !current->conflicts[i]) {
			list_add(i, current->set);
			current->scheduled[i] = true;
			for (SIZE_TYPE k = 0; k < af->list_sizes[i]; ++k) {
				current->conflicts[af->lists[i][k]] = true;
				current->victims[af->lists[i][k]] = true;
			}
			for (SIZE_TYPE k = 0; k < af_t->list_sizes[i]; ++k) {
				current->conflicts[af_t->lists[i][k]] = true;
				current->attackers[af_t->lists[i][k]] = true;
			}
		}
	}
}

State *process_stack_ad_bt(Stack *update, State *next, AF *af, AF* af_t) {
	SIZE_TYPE a = -1;
	while ((a = pop_int(update)) != -1) {
		list_add(a, next->set);

		/*
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
			}
		}
		*/
		for (SIZE_TYPE i = 0; i < af_t->list_sizes[a]; ++i) {
			SIZE_TYPE attacker_a = af_t->lists[a][i];
			if (!next->attackers[attacker_a]) {
				next->attackers[attacker_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[attacker_a]; ++j) {
					SIZE_TYPE victim_attacker_a = af->lists[attacker_a][j];
					--(next->attackers_not_in_current[victim_attacker_a]);
					if (!next->scheduled[victim_attacker_a] && next->attackers_not_in_current[victim_attacker_a] == 0) {
						if (next->conflicts[victim_attacker_a]) {
							delete_state_ad_bt(next);
							return(NULL);
						}
						// printf("%d %d\n", victim_attacker_a, af_t->list_sizes[victim_attacker_a]);
						push(update, new_stack_element_int(victim_attacker_a));
						next->scheduled[victim_attacker_a] = true;
						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_attacker_a]; ++k) {
							next->conflicts[af->lists[victim_attacker_a][k]] = true;
							next->victims[af->lists[victim_attacker_a][k]] = true;
						}
						for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_attacker_a]; ++k) {
							next->conflicts[af_t->lists[victim_attacker_a][k]] = true;
							next->attackers[af_t->lists[victim_attacker_a][k]] = true;
						}
					}
				}
			}
		}

		/*
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (!next->scheduled[victim_victim_a] && next->unattacked_attackers_count[victim_victim_a] == 0)  {
						// check if victim_victim_a causes a conflict
						if (next->conflicts[victim_victim_a]) {
							delete_state_ad_bt(next);
							return(NULL);
						}
						push(update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k)
							next->conflicts[af->lists[victim_victim_a][k]] = true;
						for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k)
							next->conflicts[af_t->lists[victim_victim_a][k]] = true;
					}
				}
			}
		}
		*/
	}
	// printf("%d %d %d\n", next->set->size, attacker_count, unattacked_count);
	return(next);
}

// s: the set to be closed
State *first_closure_ad_bt(AF *af, AF *af_t, ArrayList *s) {
	Stack *update = new_stack();
	State *next = create_state_ad_bt(af->size);

	++closure_count;

	// Push elements of s to the stack, mark them as scheduled
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		if (next->scheduled[s->elements[i]])
			continue;

		for (SIZE_TYPE j = 0; j < af->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af->lists[s->elements[i]][j]] = true;
			next->victims[af->lists[s->elements[i]][j]] = true;
		}
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af_t->lists[s->elements[i]][j]] = true;
			next->attackers[af_t->lists[s->elements[i]][j]] = true;
		}

		if (next->conflicts[s->elements[i]]) {
			delete_state_ad_bt(next);
			return(NULL);
		}

		push(update, new_stack_element_int(s->elements[i]));
		next->scheduled[s->elements[i]] = true;
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		if (af_t->list_sizes[i] == 0) {
			if (next->scheduled[i])
				continue;

			if (next->conflicts[i]) {
				delete_state_ad_bt(next);
				return(NULL);
			}

			push(update, new_stack_element_int(i));
			next->scheduled[i] = true;

			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				next->conflicts[af->lists[i][j]] = true;
				next->victims[af->lists[i][j]] = true;
			}
		}
	}

	memcpy(next->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	memcpy(next->attackers_not_in_current, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	next = process_stack_ad_bt(update, next, af, af_t);

	/*
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		printf("==>%d\n", s->elements[i]);
		list_add(s->elements[i], next->set);
	}
	extend_current(next, af, af_t);
	*/

	free_stack(update);

	return(next);
}

State *incremental_closure_ad_bt(AF* af, AF* af_t, ARG_TYPE new_argument, State *current) {
	Stack *update = new_stack();

	current->scheduled[new_argument] = true;
	State *next = duplicate_state_ad_bt(current, af->size);
	next->new_argument = new_argument;

	++closure_count;
	// Push the current argument to the stack
	push(update, new_stack_element_int(new_argument));

	for (SIZE_TYPE j = 0; j < af->list_sizes[new_argument]; ++j) {
		next->conflicts[af->lists[new_argument][j]] = true;
		next->victims[af->lists[new_argument][j]] = true;
	}
	for (SIZE_TYPE j = 0; j < af_t->list_sizes[new_argument]; ++j) {
		next->conflicts[af_t->lists[new_argument][j]] = true;
		next->attackers[af_t->lists[new_argument][j]] = true;
	}

	next = process_stack_ad_bt(update, next, af, af_t);

	/*
	list_add(new_argument, next->set);
	extend_current(next, af, af_t);
	*/

	free_stack(update);

	return(next);
}

ArrayList* dc_ad_bt(AF* attacks, ARG_TYPE argument) {
	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);
	// TODO: experimenting
	// sort adjacency lists of attacked_by according to number of victims
	// sort_adjacency_lists(attacked_by, attacks);

	Stack states;
	init_stack(&states);

	ArrayList *tmp = list_create();
	list_add(argument, tmp);
	State *current = first_closure_ad_bt(attacks, attacked_by, tmp);
	list_free(tmp);
	if (!current) {
		// first closure has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}
	current->new_argument = argument;
	push(&states, new_stack_element_ptr(current));

	while (current =  pop_ptr(&states)) {
		// print_list(stdout, current->set,"\n");
		// find the unattacked attacker of current->set that has the smallest number of attackers, which are not
		// scheduled and are not conflicting with current->set.
		int min_attacker_count = attacks->size;
		ARG_TYPE least_attacked_attacker = -1;

		bool *attacker_processed = calloc(attacks->size, sizeof(bool));
		assert(attacker_processed != NULL);

		bool is_current_self_defending = true;
		for (SIZE_TYPE i = 0; i < current->set->size; ++i) {
			for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[current->set->elements[i]]; ++j) {
				ARG_TYPE attacker = attacked_by->lists[current->set->elements[i]][j];
				if (attacker_processed[attacker])
					continue;
				attacker_processed[attacker] = true;
				if (!current->victims[attacker]) {
					is_current_self_defending = false;
					int count = 0;
					// attacker is unattacked
					for (SIZE_TYPE k = 0; k < attacked_by->list_sizes[attacker]; ++k) {
						ARG_TYPE unattacked_attacker_attacker = attacked_by->lists[attacker][k];
						if (!current->scheduled[unattacked_attacker_attacker] &&
							!current->conflicts[unattacked_attacker_attacker]) {
							++count;
						}
					}
					if (count < min_attacker_count) {
						min_attacker_count = count;
						least_attacked_attacker = attacker;
					}
				}
			}
		}
		free(attacker_processed);
		if (is_current_self_defending) {
			printf("Closure count: %d\n", closure_count);
			return(current->set);
		}

		// add unscheduled and non-conflicting attackers of least_attacked_attacker one by one and close.
		// if none of them leads to a solution, abandon that branch
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			if (current->conflicts[attacker_of_least_attacked_attacker] ||
				current->scheduled[attacker_of_least_attacked_attacker]) {
				// this attacker is already victim of current->set, or causes a conflict, or is already scheduled
				// so skip it
				continue;
			}
			// otherwise add it and close
			State *next = incremental_closure_ad_bt(attacks, attacked_by, attacker_of_least_attacked_attacker, current);

			// closure has a conflict, try with another attacker
			// of least_attacked_attacker
			if (!next) {
				// state "next" is already deleted in process_stack_ad_bt
				// upon noticing the conflict. not required here.
				continue;
			}

			push(&states, new_stack_element_ptr(next));
		}
		delete_state_ad_bt(current);
		current = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("Closure count: %d\n", closure_count);
	return(NULL);
}

ArrayList* dc_ad_subgraph_bt(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);
	// TODO: experimenting
	// sort adjacency lists of attacked_by according to number of victims
	// sort_adjacency_lists(attacks, attacked_by);

	// printf("Argument: %d\n", argument);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
	AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	STOP_TIMER(stop_time);

	// START_TIMER(start_time);
	// AF *subgraph_conflicts_af = create_conflicts_graph(subgraph->af, subgraph_t);
	// AF *conflicts_af = create_conflicts_graph(attacks, attacked_by);
	// print_conflicts_matrix(subgraph_conflicts_matrix, subgraph->af);
	// STOP_TIMER(stop_time);
	// printf("Creating conflicht graph: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// solve DC-CO in the subgraph
	ArrayList *current = list_create();
	list_add(subgraph->mapping_to_subgraph[argument], current);
	State *next = first_closure_ad_bt(subgraph->af, subgraph_t, current);

	if (!next) {
		// closure in the subgraph has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	// closure is conflict-free. check if it is self-defending
	ArrayList *extension = NULL;
	if (is_set_self_defending(subgraph->af, subgraph_t, next->set)) {
		// closure is a complete extension (in the subgraph) containing the argument
		extension = next->set;
	}
	else {
		// search for a solution by enumerating
		START_TIMER(start_time);
		extension = dc_ad_bt(subgraph->af, subgraph->mapping_to_subgraph[argument]);
		STOP_TIMER(stop_time);
		printf("dc_co_cbo: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	}

	// TODO!
	// free_argumentation_framework(attacks_adj);
	// free_argumentation_framework(attacked_by_projection_adj)

	if (!extension) {
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	// now close the mapped extension in the whole framework
	list_free(current);
	current = list_create();
	// map indices of the computed extension back
	printf("Extension size: %d\n", extension->size);
	for (SIZE_TYPE i = 0; i < extension->size; ++i) {
		list_add(subgraph->mapping_from_subgraph[extension->elements[i]], current);
	}
	delete_state_ad_bt(next);
	AF *conflicts_af = create_conflicts_graph(attacks, attacked_by);
	next = first_closure_ad_bt(attacks, attacked_by, current);

	printf("Closure count: %d\n", closure_count);
	return(next->set);
}