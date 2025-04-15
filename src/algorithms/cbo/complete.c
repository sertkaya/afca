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
#include "../../utils/implication.h"
#include "complete.h"

#include "../../utils/stack.h"
#include "../../utils/argument_set.h"
#include "../../utils/priority_queue.h"
#include "../../utils/timer.h"
/*
struct attacker_score_pair {
	ARG_TYPE arg;
	int score;
};
int compare_attackers(const void *v1, const void *v2) {
	if ((((struct attacker_score_pair*) v1)-> score) > (((struct attacker_score_pair*) v2)-> score))
		return(-1);
	else if ((((struct attacker_score_pair*) v1)-> score) < (((struct attacker_score_pair*) v2)-> score))
		return(1);
	else
		return(0);
}

void *sort_attackers(AF *attacks, AF *attacked_by, ARG_TYPE least_attacked_attacker, State *state, int *c, ARG_TYPE **a) {
	struct  attacker_score_pair *pairs = calloc(attacked_by->list_sizes[least_attacked_attacker], sizeof(struct attacker_score_pair));
	assert(pairs != NULL);
	int index = 0;
	for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
		ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
		if ( state->conflicts[attacker_of_least_attacked_attacker]) {
			// state->scheduled[attacker_of_least_attacked_attacker]) {
			// this attacker is already victim of current->set, or causes a conflict, or is already scheduled
			// so skip it
			continue;
		}
		int count = 0;
		for (SIZE_TYPE j = 0; j < attacks->list_sizes[attacker_of_least_attacked_attacker]; ++j) {
			if (state->unattacked_attackers->vector[attacks->lists[attacker_of_least_attacked_attacker][j]]) {
				++count;
			}
		}
		// for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[attacker_of_least_attacked_attacker]; ++j) {
		// 	if (state->unattacked_attackers->vector[attacked_by->lists[attacker_of_least_attacked_attacker][j]]) {
		//		++count;
		//	}
		// }
		pairs[index].arg = attacker_of_least_attacked_attacker;
		pairs[index].score = count;
		// pairs[index].score =  state->unattacked_attackers->count - count + state->unattacked_attackers_count[attacker_of_least_attacked_attacker];
		printf("%d %d %d\n", attacker_of_least_attacked_attacker, pairs[index].score, attacks->list_sizes[attacker_of_least_attacked_attacker]);
		++index;
	}
	qsort(pairs, index, sizeof(struct attacker_score_pair), compare_attackers);
	ARG_TYPE *attackers = calloc(index, sizeof(ARG_TYPE));
	assert(attackers != NULL);
	for (SIZE_TYPE i = 0; i < index; ++i)
		attackers[i] = pairs[i].arg;
	*a = attackers;
	*c = index;
}


void jump_back(Stack *s, SIZE_TYPE jump) {
	Stack tmp;
	init_stack(&tmp);
	State *x;
	for (SIZE_TYPE i = 0; i < jump; ++i) {
		x = pop_ptr(s);
		push(&tmp, new_stack_element_ptr(x));
	}
	State *top = pop_ptr(s);
	while ((x = pop_ptr(&tmp)) != NULL) {
		push(s, new_stack_element_ptr(x));
	}
	push(s, new_stack_element_ptr(top));
}
 // TODO: Testing ...
bool are_together_defeatable(ARG_TYPE a1, ARG_TYPE a2, AF *af, AF *af_t, bool **conflict_matrix) {
	// check if every attacker of a1 is in conflict with every attacker of a2
	for (SIZE_TYPE i = 0; i < af_t->list_sizes[a1]; ++i) {
		ARG_TYPE attacker_a1 = af_t->lists[a1][i];
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[a2]; ++j) {
			ARG_TYPE attacker_a2 = af_t->lists[a2][j];
			if (!conflict_matrix[attacker_a1][attacker_a2])
				return(true);
		}
	}

	return(false);
}
*/

struct state {
	ARG_TYPE new_argument;
	ArrayList *set;
	bool *scheduled;
	bool *conflicts;
	// Number of unattacked attackers of an argument
	SIZE_TYPE* unattacked_attackers_count;
	// Number of unattacked and unscheduled attackers of an argument
	// SIZE_TYPE* unscheduled_conflict_free_attackers_count;
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

	// s->unscheduled_conflict_free_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	// assert(s->unscheduled_conflict_free_attackers_count != NULL);

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

	// n->unscheduled_conflict_free_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	// assert(n->unscheduled_conflict_free_attackers_count != NULL);
	// memcpy(n->unscheduled_conflict_free_attackers_count, s->unscheduled_conflict_free_attackers_count, size * sizeof(SIZE_TYPE));

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
	// free(s->unscheduled_conflict_free_attackers_count);
	s->unattacked_attackers_count = NULL;
	// s->unscheduled_conflict_free_attackers_count = NULL;
	free_argument_set(s->unattacked_attackers);
	s->unattacked_attackers = NULL;
	free(s);
}


// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;

State *process_stack(Stack *update, State *next, AF *af, AF* af_t) {
	SIZE_TYPE a = -1;
	while ((a = pop_int(update)) != -1) {
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
				if (next->unattacked_attackers->count == 0) {
					printf("%d\n", closure_count);
					print_list(stdout, next->set, "\n");
				}
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(next->unattacked_attackers_count[victim_victim_a]);
					if (!next->scheduled[victim_victim_a] && next->unattacked_attackers_count[victim_victim_a] == 0)  {
						// check if victim_victim_a causes a conflict
						if (next->conflicts[victim_victim_a]) {
							delete_state(next);
							return(NULL);
						}
						push(update, new_stack_element_int(victim_victim_a));
						next->scheduled[victim_victim_a] = true;

						/*
						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k) {
							--next->unscheduled_conflict_free_attackers_count[af->lists[victim_victim_a][k]];
							printf("1: %d\n", next->unscheduled_conflict_free_attackers_count[af->lists[victim_victim_a][k]]);
						}
						*/

						for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k)
							next->conflicts[af->lists[victim_victim_a][k]] = true;
						for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k)
							next->conflicts[af_t->lists[victim_victim_a][k]] = true;

						/*
						for (SIZE_TYPE k = 0; k < conflicts_graph->list_sizes[victim_victim_a]; ++k) {
						 	next->conflicts[conflicts_graph->lists[victim_victim_a][k]] = true;
						}
						*/
					}
				}
			}
		}
	}
	return(next);
}

// s: the set to be closed
State *first_closure(AF *af, AF *af_t, ArrayList *s) {
	Stack *update = new_stack();
	State *next = create_state(af->size);

	++closure_count;

	// Push elements of s to the stack, mark them as scheduled
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		if (next->scheduled[s->elements[i]])
			continue;

		for (SIZE_TYPE j = 0; j < af->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af->lists[s->elements[i]][j]] = true;
		}
		for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->elements[i]]; ++j) {
			next->conflicts[af_t->lists[s->elements[i]][j]] = true;
		}

		if (next->conflicts[s->elements[i]]) {
			delete_state(next);
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
				delete_state(next);
				return(NULL);
			}

			push(update, new_stack_element_int(i));
			next->scheduled[i] = true;

			for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
				next->conflicts[af->lists[i][j]] = true;
			}
		}
	}

	memcpy(next->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	// memcpy(next->unscheduled_conflict_free_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	next = process_stack(update, next, af, af_t);
	free_stack(update);

	return(next);
}

State *incremental_closure(AF* af, AF* af_t, ARG_TYPE new_argument, State *current) {
	Stack *update = new_stack();

	current->scheduled[new_argument] = true;
	State *next = duplicate_state(current, af->size);
	next->new_argument = new_argument;

	++closure_count;
	// Push the current argument to the stack
	push(update, new_stack_element_int(new_argument));
	// next->scheduled[new_argument] = true;

	for (SIZE_TYPE j = 0; j < af->list_sizes[new_argument]; ++j)
		next->conflicts[af->lists[new_argument][j]] = true;
	for (SIZE_TYPE j = 0; j < af_t->list_sizes[new_argument]; ++j)
		next->conflicts[af_t->lists[new_argument][j]] = true;

	next = process_stack(update, next, af, af_t);
	free_stack(update);

	return(next);
}

ArrayList* dc_co_cbo(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	struct timeval start_time, stop_time;

	Stack states;
	init_stack(&states);

	ArrayList *tmp = list_create();
	list_add(argument, tmp);
	State *current = first_closure(attacks, attacked_by, tmp);
	current->new_argument = argument;
	push(&states, new_stack_element_ptr(current));

	while (current =  pop_ptr(&states)) {
		// print_list(stdout, current->set,"\n");
		// find the argument that does not cause a conflict, is not yet scheduled and has the smallest number of unattacked attackers
		// add unattacked attackers of that argument in the loop. if none of them leads to a solution, abandon that branch
		int min_attacker_count = attacks->size;
		ARG_TYPE least_attacked_attacker = -1;
		ListNode *tmp_node = current->unattacked_attackers->list;
		// if tmp_node is  NULL, then current->set does not have any unattacked attackers
		// that is, current->set is self-defending. return it.
		if (tmp_node == NULL)
			return(current->set);
		// otherwise iterate over the unattacked_attackers to find the least_attacked_argument

		while (tmp_node) {
			ARG_TYPE unattacked_attacker = tmp_node->e->n;
			int count = 0;
			for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[unattacked_attacker]; ++i) {
				ARG_TYPE unattacked_attacker_attacker = attacked_by->lists[unattacked_attacker][i];
				if (!current->scheduled[unattacked_attacker_attacker] &&
					!current->conflicts[unattacked_attacker_attacker]) {
					++count;
				}
			}
			if (count < min_attacker_count) {
				min_attacker_count = count;
				least_attacked_attacker = unattacked_attacker;
			}

			tmp_node = tmp_node->next;
		}

		// now unattacked attackers of least_attacked_attacker: add them one by one and close.
		// if none of them leads to a solution, abandon that branch
		int size;
		// ARG_TYPE *attackers;
		// sort_attackers(attacks, attacked_by, least_attacked_attacker, current, &size, &attackers);
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
		// for (SIZE_TYPE i = 0; i < size; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			// ARG_TYPE attacker_of_least_attacked_attacker = attackers[i];
			// printf("==>%d\n", attacker_of_least_attacked_attacker);
			if (current->conflicts[attacker_of_least_attacked_attacker] ||
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
			// if next->unattacked_attackers->list is  NULL, then next->set does not have any unattacked attackers
			// that is, next->set is self-defending. return it.
			if (next->unattacked_attackers->list == NULL) {
				return(current->set);
			}
			*/

			push(&states, new_stack_element_ptr(next));
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
	// TODO: experimenting
	// sort adjacency lists of attacked_by according to number of victims
	// sort_adjacency_lists(attacks, attacked_by);

	printf("Argument: %d\n", argument);

	// extract the subgraph induced by the argument
	// START_TIMER(start_time);
	// Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	// printf("Subgraph size:%d\n", subgraph->af->size);
	// AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	// STOP_TIMER(stop_time);

	START_TIMER(start_time);
	// AF *subgraph_conflicts_af = create_conflicts_graph(subgraph->af, subgraph_t);
	// AF *conflicts_af = create_conflicts_graph(attacks, attacked_by);
	// print_conflicts_matrix(subgraph_conflicts_matrix, subgraph->af);
	// STOP_TIMER(stop_time);
	// printf("Creating conflicht graph: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// solve DC-CO in the subgraph
	ArrayList *current = list_create();
	// list_add(subgraph->mapping_to_subgraph[argument], current);
	list_add(argument, current);
	// State *next = first_closure(subgraph->af, subgraph_t, subgraph_conflicts_af, current);
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
		// extension = dc_co_cbo(subgraph->af, subgraph->mapping_to_subgraph[argument], subgraph_t, subgraph_conflicts_af);
		extension = dc_co_cbo(attacks, argument, attacked_by);
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
	AF *conflicts_af = create_conflicts_graph(attacks, attacked_by);
	next = first_closure(attacks, attacked_by, conflicts_af, current);

	printf("Closure count: %d\n", closure_count);
	return(next->set);
	*/

	printf("Closure count: %d\n", closure_count);
	return(extension);
}