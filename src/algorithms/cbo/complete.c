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
#include "../../utils/argument_set.h"
#include "../../utils/priority_queue.h"
#include "../../utils/timer.h"

struct state {
	ARG_TYPE new_argument;
	ArrayList *set;
	bool *scheduled;
	bool *conflicts;
	SIZE_TYPE* unattacked_attackers_count;
	ArgumentSet *unattacked_attackers;
	bool* victims;
};

typedef struct state State;

State *create_state(SIZE_TYPE size) {
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

	s->unattacked_attackers = new_argument_set(size);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	return(s);
}

State *duplicate_state(State *s, SIZE_TYPE size) {
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

	n->unattacked_attackers = duplicate_argument_set(s->unattacked_attackers);

	n->victims = calloc(size, sizeof(bool));
	assert(n->victims != NULL);
	memcpy(n->victims, s->victims, size * sizeof(bool));

	return(n);
}

void delete_state(State *s) {
	list_free(s->set);
	s->set = NULL;
	free(s->conflicts);
	s->conflicts = NULL;
	free(s->scheduled);
	s->scheduled = NULL;
	free(s->victims);
	s->victims = NULL;
	free(s->unattacked_attackers_count);
	s->unattacked_attackers_count = NULL;
	free_argument_set(s->unattacked_attackers);
	s->unattacked_attackers = NULL;
	free(s);
}

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;

// s: the set to be closed
State *first_closure(AF *af, AF *af_t, ArrayList *s) {
	++closure_count;

	Stack update;
	init_stack(&update);

	State *next = create_state(af->size);

	// Push elements of s to the stack, mark them as scheduled
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		if (next->scheduled[s->elements[i]])
			continue;

		// mark victims of elements of s as conflict
		for (SIZE_TYPE j = 0; j < af->list_sizes[s->elements[i]]; ++j) {
			// s->victims[af->lists[s->set->elements[i]][j]] = true;
			next->conflicts[af->lists[s->elements[i]][j]] = true;
		}
		// mark attackers of elements of s as conflict
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af_t->lists[s->elements[i]][j]] = true;
		}

		if (next->conflicts[s->elements[i]]) {
			delete_state(next);
			return(NULL);
		}

		push(&update, new_stack_element_int(s->elements[i]));
		next->scheduled[s->elements[i]] = true;

	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		if (af_t->list_sizes[i] == 0) {
			if (next->scheduled[i])
				continue;

			if (next->conflicts[i]) {
				delete_state(next);
				return(NULL);
			}

			push(&update, new_stack_element_int(i));
			next->scheduled[i] = true;

			// mark victims of i as conflict
			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				// s->victims[af->lists[i][j]] = true;
				next->conflicts[af->lists[i][j]] = true;
			}
			// i has no attackers, no need to traverse the attackers list
		}
	}

	memcpy(next->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));

	SIZE_TYPE a = -1;
	while ((a = pop_int(&update)) != -1) {
		list_add(a, next->set);
		// add unattacked attackers of a to next->unattacked_attackers
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[a]; ++j) {
			if (!next->victims[af_t->lists[a][j]])
				add_to_argument_set(af_t->lists[a][j], next->unattacked_attackers);
		}

		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				// remove victim_a from next->unattacked_attackers
				delete_from_argument_set(victim_a, next->unattacked_attackers);
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (next->unattacked_attackers_count[victim_victim_a] == 0 && !next->scheduled[victim_victim_a]) {
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							return(NULL);
						}

						push(&update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						// mark victims of victim_victim_a conflict
						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k) {
							// s->victims[af->lists[s->set->elements[i]][k]] = true;
							next->conflicts[af->lists[victim_victim_a][k]] = true;
						}
						// mark attackers of victim_victim_a as conflict
						for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k) {
							next->conflicts[af_t->lists[victim_victim_a][k]] = true;
						}
					}
				}
			}
		}
	}

	return(next);
}

State *incremental_closure(AF* af, AF* af_t, ARG_TYPE new_argument, State *current) {
	Stack update;
	init_stack(&update);

	++closure_count;

	State *next = duplicate_state(current, af->size);
	next->new_argument = new_argument;

	// Push the current argument to the stack
	push(&update, new_stack_element_int(new_argument));
	next->scheduled[new_argument] = true;
	// mark victims conflict
	for (SIZE_TYPE j = 0; j < af->list_sizes[new_argument]; ++j) {
		// next->victims[af->lists[victim_victim_a][j]] = true;
		next->conflicts[af->lists[new_argument][j]] = true;
	}
	// mark attackers as conflict
	for (SIZE_TYPE j = 0; j < af_t->list_sizes[new_argument]; ++j) {
		next->conflicts[af_t->lists[new_argument][j]] = true;
	}

	SIZE_TYPE a = -1;
	while ((a = pop_int(&update)) != -1) {
		list_add(a, next->set);
		// add unattacked attackers of a to next->unattacked_attackers
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[a]; ++j) {
			if (!next->victims[af_t->lists[a][j]])
				add_to_argument_set(af_t->lists[a][j], next->unattacked_attackers);
		}

		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				// remove victim_a from next->unattacked_attackers
				delete_from_argument_set(victim_a, next->unattacked_attackers);
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (next->unattacked_attackers_count[victim_victim_a] == 0 && !next->scheduled[victim_victim_a])  {
						// check if victim_victim_a causes a conflict
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							return(NULL);
						}
						push(&update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						// mark victims of victim_victim_a conflict
						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k) {
							// next->victims[af->lists[victim_victim_a][k]] = true;
							next->conflicts[af->lists[victim_victim_a][k]] = true;
						}
						// mark attackers of elements of as conflict
						for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k) {
							next->conflicts[af_t->lists[victim_victim_a][k]] = true;
						}
					}
				}
			}
		}
	}
	return(next);
}

ArrayList* dc_co_cbo(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	struct timeval start_time, stop_time;

	QueueNode *states = NULL;

	ArrayList *tmp = list_create();
	list_add(argument, tmp);
	State *current = first_closure(attacks, attacked_by, tmp);
	current->new_argument = argument;

	int min_attacker_count = attacks->size;
	/*
	// find the minimum number of attackers of an unattacked attacker
	ListNode *tmp_node = current->unattacked_attackers->list;
	ARG_TYPE unattacked_attacker;
	// ARG_TYPE least_attacked_attacker = -1;
	// ARG_TYPE least_attacked_attacker = tmp_node->e->n;
	while (tmp_node) {
		unattacked_attacker = tmp_node->e->n;
		if (current->unattacked_attackers_count[unattacked_attacker] < min_attacker_count) {
			min_attacker_count = current->unattacked_attackers_count[unattacked_attacker];
			// least_attacked_attacker = unattacked_attacker;
		}
		tmp_node = tmp_node->next;
	}
	states = enqueue_ptr(current, states, min_attacker_count);
	*/
	states = enqueue_ptr(current, states, current->unattacked_attackers->count);

	while (current =  dequeue_ptr(&states)) {
		// find the argument that does not cause a conflict, is not yet scheduled and has the smallest number of unattacked attackers
		// add unattacked attackers of that argument in the loop. if none of them leads to a solution, abandon that branch
		ListNode *tmp_node = current->unattacked_attackers->list;
		// if tmp_node is  NULL, then current->set does not have any unattacked attackers
		// that is, current->set is self-defending. return it.
		if (tmp_node == NULL)
			return(current->set);
		min_attacker_count = attacks->size;
		// ARG_TYPE least_attacked_attacker = -1;
		ARG_TYPE least_attacked_attacker = tmp_node->e->n;
		// otherwise iterate over the unattacked_attackers to find the least_attacked_argument
		while (tmp_node) {
			ARG_TYPE unattacked_attacker = tmp_node->e->n;
			if (current->unattacked_attackers_count[unattacked_attacker] < min_attacker_count) {
				min_attacker_count = current->unattacked_attackers_count[unattacked_attacker];
				least_attacked_attacker = unattacked_attacker;
			}
			tmp_node = tmp_node->next;
		}
		// now unattacked attackers of least_attacked_attacker: add them one by one and close.
		// if none of them leads to a solution, abandon that branch
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			if ( current->conflicts[attacker_of_least_attacked_attacker] ||
				current->scheduled[attacker_of_least_attacked_attacker]) {
				// this attacker is already victim of current->set, or causes a conflict, or is already scheduled
				// so skip it
				continue;
			}
			// otherwise add it and close
			State *next = incremental_closure(attacks, attacked_by, attacker_of_least_attacked_attacker, current);

			// if closure has a conflict then abandon that branch
			if (!next) {
				continue;
			}

			/*
			// find the minimum number of attackers of an unattacked attacker
			ListNode *tmp_node = next->unattacked_attackers->list;
			min_attacker_count = attacks->size;
			while (tmp_node) {
				ARG_TYPE unattacked_attacker = tmp_node->e->n;
				if (next->unattacked_attackers_count[unattacked_attacker] < min_attacker_count) {
					min_attacker_count = next->unattacked_attackers_count[unattacked_attacker];
				}
				tmp_node = tmp_node->next;
			}
			states = enqueue_ptr(next, states, min_attacker_count);
			*/
			states = enqueue_ptr(next, states, next->unattacked_attackers->count);
		}
		delete_state(current);
		current = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	return(NULL);
}

struct pair {
	ARG_TYPE arg;
	SIZE_TYPE victim_count;
};

int compare_arguments(const void *v1, const void *v2) {
	if ((((struct pair*) v1)-> victim_count) > (((struct pair*) v2)-> victim_count))
		return(1);
	else if ((((struct pair*) v2)-> victim_count) > (((struct pair*) v1)-> victim_count))
		return(-1);
	else
		return(0);
}

void sort_adjacency_lists(AF *af, AF *af_t) {
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		struct pair *pairs = calloc(af_t->list_sizes[i], sizeof(struct pair));
		assert(pairs != NULL);
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[i]; ++j) {
			pairs[j].arg = af_t->lists[i][j];
			pairs[j].victim_count = af->list_sizes[af_t->lists[i][j]];
		}
		qsort(pairs, af_t->list_sizes[i], sizeof(struct pair), compare_arguments);
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[i]; ++j) {
			af_t->lists[i][j] = pairs[j].arg;
		}
		free(pairs);
	}
}


ArrayList* dc_co_subgraph_cbo(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);
	// TODO: experimenting
	// sort adjacency lists of attacked_by according to number of victims
	sort_adjacency_lists(attacks, attacked_by);

	printf("Argument: %d\n", argument);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	// Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	// printf("Subgraph size:%d\n", subgraph->af->size);
	// AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	STOP_TIMER(stop_time);

	// solve DC-CO in the subgraph
	ArrayList *current = list_create();
	// list_add(subgraph->mapping_to_subgraph[argument], current);
	list_add(argument, current);
	// State *next = first_closure(subgraph->af, subgraph_t, current);
	State *next = first_closure(attacks, attacked_by, current);

	if (!next) {
		// closure in the subgraph has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	// closure is conflict-free. check if it is self-defending
	ArrayList *extension = NULL;
	// if (is_set_self_defending(subgraph->af, subgraph_t, next->set)) {
	if (is_set_self_defending(attacks, attacked_by, next->set)) {
		// closure is a complete extension (in the subgraph) containing the argument
		extension = next->set;
	}
	else {
		// search for a solution by enumerating
		START_TIMER(start_time);
		// extension = dc_co_cbo(subgraph->af, subgraph->mapping_to_subgraph[argument], subgraph_t);
		extension = dc_co_cbo(attacks, argument, attacked_by);
		STOP_TIMER(stop_time);
		printf("dc_co_next_closure_adj: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	}

	// TODO!
	// free_argumentation_framework(attacks_adj);
	// free_argumentation_framework(attacked_by_projection_adj)

	if (!extension) {
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	/*
	// now close the mapped extension in the whole framework
	list_free(current);
	current = list_create();
	// map indices of the computed extension back
	printf("Extension size: %d\n", extension->size);
	for (SIZE_TYPE i = 0; i < extension->size; ++i) {
		list_add(subgraph->mapping_from_subgraph[extension->elements[i]], current);
	}
	delete_state(next);
	next = first_closure(attacks, attacked_by, current);

	printf("Closure count: %d\n", closure_count);
	return(next->set);
	*/
	printf("Closure count: %d\n", closure_count);
	return(extension);
}