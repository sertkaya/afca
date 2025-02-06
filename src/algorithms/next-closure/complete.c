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

static  int closure_count = 0;

// s: the set to be closed
// r: the closure of s
// r_bits: bool array representation of r
// TODO: Caution! We assume that s and r do not contain double values!
void closure_semi_complete(AF* af, AF* af_t, ArrayList* s, ArrayList* r, bool *r_bv) {
	Stack update;
	init_stack(&update);

	++closure_count;

	// empty r
	list_reset(r);
	// empty r_bv
	memset(r_bv, 0, af->size * sizeof(bool));

	// Push elements of s to the stack, add to r and to r_bv
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		// push(&update, s->elements[i]);
		push(&update, new_stack_element_int(s->elements[i]));
		list_add(s->elements[i], r);
		r_bv[s->elements[i]] = true;
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	// TODO: This is independent of s. It can be done outside the closure function.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		if (af_t->list_sizes[i] == 0 && !r_bv[i]) {
			// push(&update, i);
			push(&update, new_stack_element_int(i));
			list_add(i, r);
			r_bv[i] = true;
		}
	}

	SIZE_TYPE* unattacked_attackers_count = calloc(af_t->size, sizeof(SIZE_TYPE));
	assert(unattacked_attackers_count != NULL);
	memcpy(unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));

	bool* victims_a = calloc(af->size, sizeof(bool));
	assert(victims_a != NULL);
	memset(victims_a, 0, af->size * sizeof(bool));

	// SIZE_TYPE a = pop(&update);
	SIZE_TYPE a = pop_int(&update);
	while (a != -1) {
		if (af->list_sizes[a] == 0) {
			// a does not attack anybody, pop and continue
			// a = pop(&update);
			a = pop_int(&update);
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
						// push(&update, victim_victim_a);
						push(&update, new_stack_element_int(victim_victim_a));
						list_add(victim_victim_a, r);
						r_bv[victim_victim_a] = true;
					}
				}
			}
		}
		// a = pop(&update);
		a = pop_int(&update);
	}
	free(unattacked_attackers_count);
	free(victims_a);
}

bool next_conflict_free_semi_complete_intent(AF* attacks, AF* attacked_by, ArrayList* current, ArrayList* current_closure) {
	ArrayList* tmp = list_duplicate(current);

	bool* current_bv = calloc(attacks->size, sizeof(bool));
	assert(current_bv != NULL);

	for (SIZE_TYPE i = 0; i < tmp->size; ++i) {
		current_bv[tmp->elements[i]] = true;
	}

	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);

	for (SIZE_TYPE i = attacks->size - 1; i > 0 ; --i) {
		if (current_bv[i]) {
			current_bv[i] = false;
			// remove i from tmp
			list_remove(i, tmp);

		} else if (!check_arg_attacks_arg(attacks, i, i) &&
				   !check_arg_attacks_set(attacks, i, tmp) &&
				   !check_set_attacks_arg(attacks, tmp, i)) { // &&
				   // && (attacks->list_sizes[i] > 0)) {

			current_bv[i] = true;
			// add i to tmp
			list_add(i, tmp);

			closure_semi_complete(attacks, attacked_by, tmp, current_closure, current_closure_bv);

			bool good = true;
			// is current_closure canonical?
			for (SIZE_TYPE j = 0; j < i; ++j) {
				if (current_closure_bv[j] && !current_bv[j]) {
					good = false;
					break;
				}
			}
			if (good) {
				// is current_closure conflict-free?
				for (SIZE_TYPE j = i + 1; j < attacks->size; ++j) {
					if (current_closure_bv[j] && check_arg_attacks_set(attacks, j, current_closure)  &&  check_set_attacks_arg(attacks, tmp, j)) {
						good = false;
						break;
					}
				}
			}
			if (good) {
				list_free(tmp);
				free(current_bv);
				free(current_closure_bv);
				return(1);
			}
			current_bv[i] = false;
			// remove i from tmp
			list_remove(i, tmp);
		}
	}
	free(current_bv);
	free(current_closure_bv);
	return(0);
}

ArrayList* dc_co_next_closure(AF* attacks, ARG_TYPE argument, AF* attacked_by) {
	struct timeval start_time, stop_time;

	ArrayList* current = list_create();
	ArrayList* current_closure = list_create();
	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);


	// Not self-defending. That is, the given argument is not defended.

	// Sort in descending order of victims
	// TODO
	// ...

	// Move defenders of the argument to the right-end.
	// TODO
	// ...

	// Now move the argument to the very left bit ...
	printf("Argument: %d\n", argument);
	swap_arguments(attacks, 0, argument);

	// recompute the attacked_by framework
	// free_argumentation_framework(attacked_by);
	// attacked_by = transpose_argumentation_framework(attacks);
	// or instead do swap in the attacked_by framework
	swap_arguments(attacked_by, 0, argument);

	list_reset(current);
	list_reset(current_closure);
	// add argument 0 to current
	list_add(0, current);
	closure_semi_complete(attacks, attacked_by, current, current_closure, current_closure_bv);
	// list_add(0, current_closure);

	int concept_count = 0;
	do {
		// print_list(stdout, current, "<- current\n");
		// print_list(stdout, current_closure, "<- current_closure\n");
		list_copy(current_closure, current);
		++concept_count;
		if (is_set_self_defending(attacks, attacked_by, current_closure)) {
			free(current_closure_bv);
			free_argumentation_framework(attacks);
			free_argumentation_framework(attacked_by);
			// swap back 0 and argument in the current_closure
			for (SIZE_TYPE i = 0; i < current_closure->size; ++i) {
				if (current_closure->elements[i] == 0)
					current_closure->elements[i] = argument;
				else if (current_closure->elements[i] == argument)
					current_closure->elements[i] = 0;
			}
			printf("=== dc_co_next_closure finished 3===\n");
			printf("Number of concepts generated: %d\n", concept_count);
			return(current_closure);
		}
	} while (next_conflict_free_semi_complete_intent(attacks, attacked_by, current, current_closure));

	printf("Number of concepts generated: %d\n", concept_count);

	free(current_closure_bv);
	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("=== dc_co_next_closure finished 4===\n");
	return(NULL);
}

ArrayList* dc_co_subgraph_nc(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacks, attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
	AF* subgraph_t = transpose_argumentation_framework(subgraph->af);
	STOP_TIMER(stop_time);
	printf("Extracting and transposing subgraph: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	// solve DC-CO in the subgraph
	ArrayList* current = list_create();
	list_add(subgraph->mapping_to_subgraph[argument], current);
	ArrayList* current_closure = list_create();
	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);
	closure_semi_complete(subgraph->af, subgraph_t, current, current_closure, current_closure_bv);
	if (!is_set_conflict_free(subgraph->af, current_closure)) {
		// closure in the subgraph has a conflict. complete extension does not exist.
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	// closure is conflict-free. check if it is self-defending
	ArrayList *extension = NULL;
	if (is_set_self_defending(subgraph->af, subgraph_t, current_closure)) {
		// closure is a complete extension (in the subgraph) containing the argument
		extension = current_closure;
	}
	else {
		// search for a solution by enumerating
		START_TIMER(start_time);
		extension = dc_co_next_closure(subgraph->af, subgraph->mapping_to_subgraph[argument], subgraph_t);
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

	// map indices of the computed extension back
	ArrayList *mapped_extension = list_create();
	for (SIZE_TYPE i = 0; i < extension->size; ++i) {
		list_add(subgraph->mapping_from_subgraph[extension->elements[i]], mapped_extension);
	}

	// now close the mapped extension in the whole framework
	ArrayList* closure = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	closure_semi_complete(attacks, attacked_by, mapped_extension, closure, closure_bv);

	printf("Closure count: %d\n", closure_count);
	return(closure);
}