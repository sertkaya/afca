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

#include <iso646.h>

#include "../../utils/stack.h"
#include "../../utils/timer.h"

struct state {
	ARG_TYPE index;
	ArrayList *set;
	bool *scheduled;
	bool *conflicts;
	SIZE_TYPE* unattacked_attackers_count;
	bool* victims;
};

typedef struct state State;

State *create_state(SIZE_TYPE size) {
	State *s = calloc(1, sizeof(State));
	assert(s != NULL);

	s->set = list_create();
	s->index = 0;

	s->conflicts = calloc(size, sizeof(bool));
	assert(s->conflicts != NULL);

	s->scheduled = calloc(size, sizeof(bool));
	assert(s->scheduled != NULL);

	s->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->unattacked_attackers_count != NULL);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	return(s);
}

State *duplicate_state(State *s, SIZE_TYPE size) {
	State *n = calloc(1, sizeof(State));
	assert(n != NULL);

	n->index = s->index;
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

		// mark victims of elements of as conflict
		for (SIZE_TYPE j = 0; j < af->list_sizes[s->elements[i]]; ++j) {
			// s->victims[af->lists[s->set->elements[i]][j]] = true;
			next->conflicts[af->lists[s->elements[i]][j]] = true;
		}
		// mark attackers of elements of as conflict
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af_t->lists[s->elements[i]][j]] = true;
		}

		if (next->conflicts[s->elements[i]]) {
			delete_state(next);
			printf("conflict 1\n");
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
				printf("conflict 2\n");
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

		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (next->unattacked_attackers_count[victim_victim_a] == 0 && !next->scheduled[victim_victim_a]) {
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							printf("conflict 3\n");
							return(NULL);
						}

						push(&update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						// mark victims of victim_victim_a conflict
						for (SIZE_TYPE j = 0; j < af->list_sizes[victim_victim_a]; ++j) {
							// s->victims[af->lists[s->set->elements[i]][j]] = true;
							next->conflicts[af->lists[victim_victim_a][j]] = true;
						}
						// mark attackers of elements of as conflict
						for (SIZE_TYPE j = 0; j < af_t->list_sizes[victim_victim_a]; ++j) {
							next->conflicts[af_t->lists[victim_victim_a][j]] = true;
						}
					}
				}
			}
		}
	}

	return(next);
}

State *incremental_closure(AF* af, AF* af_t, ARG_TYPE index, State *current, ARG_TYPE *order, ARG_TYPE *position) {
	Stack update;
	init_stack(&update);

	++closure_count;

	State *next = duplicate_state(current, af->size);

	/*
	printf("incr.:");
	for (SIZE_TYPE i = 0; i < af->size; ++i)
		printf("%d ", next->unattacked_attackers_count[i]);
	printf("\n");
	*/

	// Push the current argument to the stack
	push(&update, new_stack_element_int(order[index]));
	next->scheduled[order[index]] = true;

	SIZE_TYPE a = -1;
	while ((a = pop_int(&update)) != -1) {
		list_add(a, next->set);

		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				// printf("victim_a:%d\n", victim_a);
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					// printf("victim_victim_a:%d\n", victim_victim_a);
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (next->unattacked_attackers_count[victim_victim_a] == 0 && !next->scheduled[victim_victim_a])  {
						// check if victim_victim_a causes a conflict
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							return(NULL);
						}
						// TODO: here canonicity test
						// check if victim_victim_a breaks canonicity
						if (!current->scheduled[victim_victim_a] && position[victim_victim_a] < index) {
							delete_state(next);
							return(NULL);
						}
						push(&update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						// mark victims of victim_victim_a conflict
						for (SIZE_TYPE j = 0; j < af->list_sizes[victim_victim_a]; ++j) {
							// s->victims[af->lists[s->set->elements[i]][j]] = true;
							next->conflicts[af->lists[victim_victim_a][j]] = true;
						}
						// mark attackers of elements of as conflict
						for (SIZE_TYPE j = 0; j < af_t->list_sizes[victim_victim_a]; ++j) {
							next->conflicts[af_t->lists[victim_victim_a][j]] = true;
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

	// prepare the ordering:
	// first attackers of argument, then the argument, then attackers of its attackers

	// initially every argument is mapped to its index
	ARG_TYPE *order = calloc(attacks->size, sizeof(ARG_TYPE));
	assert(order != NULL);
	// reverse map: argument->position
	ARG_TYPE *position = calloc(attacks->size, sizeof(ARG_TYPE));
	assert(position != NULL);

	bool *tmp_bv = calloc(attacked_by->size, sizeof(bool));
	assert(tmp_bv != NULL);
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i)
		tmp_bv[i] = false;

	// place attackers at the beginning
	SIZE_TYPE index = 0;
	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[argument]; ++i) {
		ARG_TYPE attacker_of_argument = attacked_by->lists[argument][i];
		if (!tmp_bv[attacker_of_argument]) {
			// order[index++] = attacker_of_argument;
			order[index] = attacker_of_argument;
			position[attacker_of_argument] = index;
			++index;
			tmp_bv[attacker_of_argument] = true;
		}
	}
	// next the victims
	for (SIZE_TYPE i = 0; i < attacks->list_sizes[argument]; ++i) {
		ARG_TYPE victim_of_argument = attacks->lists[argument][i];
		if (!tmp_bv[victim_of_argument]) {
			// order[index++] = victim_of_argument;
			order[index] = victim_of_argument;
			position[victim_of_argument] = index;
			++index;
			tmp_bv[victim_of_argument] = true;
		}
	}

	// now place the argument
	// first note the index of the argument
	ARG_TYPE argument_index = index;
	// order[index++] = argument;
	order[index] = argument;
	position[argument] = index;
	++index;
	tmp_bv[argument] = true;

	// as next move attackers of attackers of argument to the right of the argument
	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[argument]; ++i) {
		ARG_TYPE attacker_of_argument = attacked_by->lists[argument][i];
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[attacker_of_argument]; ++j) {
			ARG_TYPE attacker_of_attacker_of_argument = attacked_by->lists[attacker_of_argument][j];
			if (!tmp_bv[attacker_of_attacker_of_argument]) {
				// order[index++] = attacker_of_attacker_of_argument;
				order[index] = attacker_of_attacker_of_argument;
				position[attacker_of_attacker_of_argument] = index;
				++index;
				tmp_bv[attacker_of_attacker_of_argument] = true;
			}
		}
	}

	// now the rest
	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		if (!tmp_bv[i]) {
			// order[index++] = i;
			order[index] = i;
			position[i] = index;
			++index;
			tmp_bv[i] = true;
		}
	}

	// Sort ?
	// TODO
	// ...

	Stack states;
	init_stack(&states);

	/*
	ArrayList* closure = list_create();
	// ArrayList *tmp = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	bool* conflicts = calloc(attacks->size, sizeof(bool));
	assert(conflicts != NULL);
	// closure of the empty set
	// cbo_closure(attacks, attacked_by, tmp, closure);
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i)
		if (attacked_by->list_sizes[i] == 0) {
			list_add(i, closure);
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[i]; ++j) {
				conflicts[attacks->lists[i][j]] = true;
			}
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[i]; ++j) {
				conflicts[attacks->lists[i][j]] = true;
			}
		}

	// State *current = create_state(attacks->size, argument_index, closure, conflicts);
	// list_add(argument, tmp);
	bool is_closure_conflict_free = incremental_closure(attacks, attacked_by, argument, current);
	// if closure is conflict-free and self-defending then found
	if (is_closure_conflict_free && is_set_self_defending(attacks, attacked_by, closure)) {
		return(closure);
	}
	*/
	ArrayList *tmp = list_create();
	list_add(argument, tmp);
	State *current = first_closure(attacks, attacked_by, tmp);
	current->index = argument_index;
	push(&states, new_stack_element_ptr(current));

	while (current =  pop_ptr(&states)) {
		for (SIZE_TYPE i = current->index + 1; i < attacks->size; ++i) {
			if (current->conflicts[order[i]] || current->scheduled[order[i]])
				continue;

			State *next = incremental_closure(attacks, attacked_by, i, current, order, position);

			// if closure has a conflict or is not canonical abandon that branch
			if (!next) {
				// delete_state(next);
				// next = NULL;
				continue;
			}
			// if closure is self-defending, then found
			if (is_set_self_defending(attacks, attacked_by, next->set)) {
					print_list(stdout, next->set, "==> dc_co_cbo 1\n");
					return(next->set);
			}

			/*
			// if not self-defending
			memset(tmp_bv, 0, attacks->size * sizeof(bool));
			for (SIZE_TYPE j = 0; j < next->set->size; ++j)
				tmp_bv[next->set->elements[j]] = true;

			// TODO: move the canonicity test to incremental_closure.
			// It should return NULL if next->set has a conflict or it is not canonical.
			// Then the rest of the for loop would just be pushing next to the stack.


			// TODO: if canonical ...
			bool canonical = true;
			for (SIZE_TYPE j = 0; j < i; ++j) {
				// if (next->bset[order[j]] && !tmp_bv[order[j]]) {
				if (!tmp_bv[order[j]]) {
					canonical = false;
					// printf("closure not canonical\n");
					break;
				}
			}

			if (canonical) {
				State *new = duplicate_state(next, attacks->size);
				push(&states, new_stack_element_ptr(new));
			}
			*/
			// State *new = duplicate_state(next, attacks->size);
			// push(&states, new_stack_element_ptr(new));
			push(&states, new_stack_element_ptr(next));
		}
		delete_state(current);
		current = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("dc_co_cbo 2\n");
	return(NULL);
}

ArrayList* dc_co_subgraph_cbo(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	printf("Argument: %d\n", argument);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
	AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	STOP_TIMER(stop_time);

	// solve DC-CO in the subgraph
	/*
	ArrayList* current = list_create();
	list_add(subgraph->mapping_to_subgraph[argument], current);
	ArrayList* current_closure = list_create();
	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);
	bool* conflicts = calloc(attacks->size, sizeof(bool));
	assert(conflicts!=NULL);
	cbo_closure(subgraph->af, subgraph_t, current, current_closure);
	if (!is_set_conflict_free(subgraph->af, current_closure)) {
		// closure in the subgraph has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}
	*/

	ArrayList *current = list_create();
	list_add(subgraph->mapping_to_subgraph[argument], current);
	State *next = first_closure(subgraph->af, subgraph_t, current);

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
		extension = dc_co_cbo(subgraph->af, subgraph->mapping_to_subgraph[argument], subgraph_t);
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

	/*
	ArrayList* closure = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	cbo_closure(attacks, attacked_by, mapped_extension, closure);
	*/

	printf("Closure count: %d\n", closure_count);
	return(next->set);
}