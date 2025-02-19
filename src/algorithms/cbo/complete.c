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
#include "../../utils/timer.h"

struct state {
	ARG_TYPE index;
	ArrayList *set;
	bool *bset;
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

	s->bset = calloc(size, sizeof(bool));
	assert(s->bset != NULL);

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

	n->bset = calloc(size, sizeof(bool));
	assert(n->bset != NULL);
	memcpy(n->bset, s->bset, size * sizeof(bool));

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
	free(s->bset);
	s->conflicts = NULL;
	s->bset = NULL;
	free(s);
}

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;
// s: the set to be closed
// r: the closure of s
// r_bits: bool array representation of r
// TODO: Caution! We assume that s and r do not contain double values!

State *cbo_closure(AF *af, AF *af_t, State *s) {
	printf("=== cbo_closure starting ===\n");
	++closure_count;

	Stack update;
	init_stack(&update);

	print_list(stdout, s->set, "<--s->set\n");

	// Push elements of s to the stack, add to c and to c_bv
	for (SIZE_TYPE i = 0; i < s->set->size; ++i) {
		push(&update, new_stack_element_int(s->set->elements[i]));
		printf("++%d\n", s->set->elements[i]);
		// list_add(s->elements[i], c);
		// c_bv[s->elements[i]] = true;

		// mark victims of elements of as conflict
		for (SIZE_TYPE j = 0; j < af->list_sizes[s->set->elements[i]]; ++j) {
			s->victims[af->lists[s->set->elements[i]][j]] = true;
			s->conflicts[af->lists[s->set->elements[i]][j]] = true;
		}
		// mark attackers of elements of as conflict
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->set->elements[i]]; ++j) {
			s->conflicts[af_t->lists[s->set->elements[i]][j]] = true;
		}
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		// if (af_t->list_sizes[i] == 0 && !c_bv[i]) {
		if (af_t->list_sizes[i] == 0) {
			push(&update, new_stack_element_int(i));
			printf("**%d\n", i);
			// list_add(i, c);
			// c_bv[i] = true;
			// mark victims of i as conflict
			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				s->victims[af->lists[i][j]] = true;
				s->conflicts[af->lists[i][j]] = true;
			}
			// i has no attackers, no need to traverse the attackers list
		}
	}

	State *st = duplicate_state(s, af->size);
	print_list(stdout, st->set, ": st->set\n");
	// SIZE_TYPE* unattacked_attackers_count = calloc(af_t->size, sizeof(SIZE_TYPE));
	// assert(unattacked_attackers_count != NULL);
	memcpy(st->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));

	// bool* victims_a = calloc(af->size, sizeof(bool));
	// assert(victims_a != NULL);
	// memset(victims_a, 0, af->size * sizeof(bool));

	SIZE_TYPE a = pop_int(&update);
	while (a != -1) {
		printf("a: %d\n", a);
		if (st->conflicts[a]) {
			delete_state(st);
			printf("=== cbo_closure ending 1 ===\n");
			return(NULL);
		}
		if (st->bset[a])
			continue;
		list_add(a, st->set);
		st->bset[a] = true;
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
		printf("i: %d\n", i);
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!st->victims[victim_a]) {
				st->victims[victim_a] = true;
				// mark victim of a as conflict
				st->conflicts[victim_a] = true;
				// mark attackers of a as conflict
				for (SIZE_TYPE j = 0; j < af_t->list_sizes[a]; ++j)
					st->conflicts[af_t->lists[a][j]] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(st->unattacked_attackers_count[victim_victim_a]);
					if ((st->unattacked_attackers_count[victim_victim_a] == 0) && !st->bset[victim_victim_a]) { // && !conflicts[victim_victim_a])  {
						push(&update, new_stack_element_int(victim_victim_a));
						// list_add(victim_victim_a, c);
						// c_bv[victim_victim_a] = true;
					}
				}
			}
		}
		a = pop_int(&update);
	}
	// free(victims_a);
	printf("=== cbo_closure ending 2 ===\n");
	return(st);
}

State *incremental_closure(AF* af, AF* af_t, ARG_TYPE index, ARG_TYPE *order, State *current) {
	printf("=== incremental_closure starting ===\n");
	Stack update;
	init_stack(&update);

	++closure_count;

	State *next = duplicate_state(current, af->size);

	// Push the current argument to the stack
	push(&update, new_stack_element_int(order[index]));

	SIZE_TYPE a = pop_int(&update);
	while (a != -1) {
		if (next->conflicts[a]) {
			delete_state(next);
			printf("=== incremental_closure ending 1 ===\n");
			return(NULL);
		}
		if (next->bset[a])
			continue;
		list_add(a, next->set);
		next->bset[a] = true;
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!next->victims[victim_a]) {
				next->victims[victim_a] = true;
				// mark victim of a as conflict
				next->conflicts[victim_a] = true;
				// mark attackers of a as conflict
				for (SIZE_TYPE j = 0; j < af_t->list_sizes[a]; ++j)
					next->conflicts[af_t->lists[a][j]] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--next->unattacked_attackers_count[victim_victim_a];
					if ((next->unattacked_attackers_count[victim_victim_a] == 0) && !next->bset[victim_victim_a]) { // && !conflicts[victim_victim_a])  {
						// conflicts[victim_a] = true;
						/*
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							return(NULL);
						}
						*/
						push(&update, new_stack_element_int(victim_victim_a));
						// list_add(victim_victim_a, next->set);
						// next->set_bv[victim_victim_a] = true;
					}
				}
			}
		}
		a = pop_int(&update);
	}
	printf("=== incremental_closure ending 2 ===\n");
	return(next);
}

ArrayList* dc_co_cbo(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	struct timeval start_time, stop_time;

	// prepare the ordering:
	// first attackers of argument, then the argument, then attackers of its attackers

	// initially every argument is mapped to its index
	ARG_TYPE *order = calloc(attacks->size, sizeof(ARG_TYPE));
	assert(order != NULL);

	bool *tmp_bv = calloc(attacked_by->size, sizeof(bool));
	assert(tmp_bv != NULL);
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i)
		tmp_bv[i] = false;

	// place attackers at the beginning
	SIZE_TYPE index = 0;
	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[argument]; ++i) {
		ARG_TYPE attacker_of_argument = attacked_by->lists[argument][i];
		if (!tmp_bv[attacker_of_argument]) {
			order[index++] = attacker_of_argument;
			tmp_bv[attacker_of_argument] = true;
		}
	}
	// next the victims
	for (SIZE_TYPE i = 0; i < attacks->list_sizes[argument]; ++i) {
		ARG_TYPE victim_of_argument = attacks->lists[argument][i];
		if (!tmp_bv[victim_of_argument]) {
			order[index++] = victim_of_argument;
			tmp_bv[victim_of_argument] = true;
		}
	}

	// now place the argument
	// first note the index of the argument
	ARG_TYPE argument_index = index;
	order[index++] = argument;
	tmp_bv[argument] = true;

	// as next move attackers of attackers of argument to the right of the argument
	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[argument]; ++i) {
		ARG_TYPE attacker_of_argument = attacked_by->lists[argument][i];
		for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[attacker_of_argument]; ++j) {
			ARG_TYPE attacker_of_attacker_of_argument = attacked_by->lists[attacker_of_argument][j];
			if (!tmp_bv[attacker_of_attacker_of_argument]) {
				order[index++] = attacker_of_attacker_of_argument;
				tmp_bv[attacker_of_attacker_of_argument] = true;
			}
		}
	}

	// now the rest
	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		if (!tmp_bv[i]) {
			order[index++] = i;
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
	State *current = create_state(attacks->size);
	list_add(argument, current->set);
	current->index = argument_index;
	State *next = cbo_closure(attacks, attacked_by, current);
	next->index = argument_index;

	delete_state(current);
	push(&states, new_stack_element_ptr(next));

	while (current =  pop_ptr(&states)) {
		for (SIZE_TYPE i = current->index + 1; i < attacks->size; ++i) {
			if (current->conflicts[order[i]])
				continue;

			next = incremental_closure(attacks, attacked_by, i, order, current);

			// if closure is conflict-free and self-defending then found
			if (!next) {
				// printf("closure has conflict\n");
				continue;
			}
			if (is_set_self_defending(attacks, attacked_by, next->set)) {
					return(next->set);
			}
			// printf("closure not self-defending\n");

			memset(tmp_bv, 0, attacks->size * sizeof(bool));
			for (SIZE_TYPE j = 0; j < next->set->size; ++j)
				tmp_bv[next->set->elements[j]] = true;

			// TODO: if canonical ...
			bool canonical = true;
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (next->bset[order[j]] && !tmp_bv[order[j]]) {
					canonical = false;
					// printf("closure not canonical\n");
					break;
				}
			}

			if (canonical) {
				State *new = duplicate_state(next, attacks->size);
				push(&states, new_stack_element_ptr(new));
			}
		}
		delete_state(current);
		current = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

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

	State *current = create_state(subgraph->af->size);
	list_add(subgraph->mapping_to_subgraph[argument], current->set);
	State *next = cbo_closure(subgraph->af, subgraph_t, current);

	if (!next) {
		// closure in the subgraph has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	print_list(stdout, next->set, "=== next->set ===\n");

	// closure is conflict-free. check if it is self-defending
	ArrayList *extension = NULL;
	printf("here\n");
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
	delete_state(current);
	current = create_state(attacks->size);
	// map indices of the computed extension back
	printf("Extension size: %d\n", extension->size);
	for (SIZE_TYPE i = 0; i < extension->size; ++i) {
		list_add(subgraph->mapping_from_subgraph[extension->elements[i]], current->set);
	}
	delete_state(next);
	next = cbo_closure(attacks, attacked_by, current);

	/*
	ArrayList* closure = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	cbo_closure(attacks, attacked_by, mapped_extension, closure);
	*/

	printf("Closure count: %d\n", closure_count);
	return(next->set);
}