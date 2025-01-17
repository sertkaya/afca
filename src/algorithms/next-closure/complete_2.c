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
	ArrayList* tmp = list_duplicate(current);

	assert(current_bv!=NULL);
	// copy_bitset(current, tmp);
	for (SIZE_TYPE i = 0; i < tmp->size; ++i) {
		current_bv[tmp->elements[i]] = true;
	}

	bool* current_closure_bv = calloc(attacks->size, sizeof(bool));
	assert(current_closure_bv!=NULL);

	for (int i = attacks->size - 1; i >= 0; --i) {
		// if (TEST_BIT(tmp, i)) {
		if (current_bv[i]) {
			// RESET_BIT(tmp, i);
			current_bv[i] = false;
			// remove i from tmp
			list_remove(i, tmp);

		} else if (!check_arg_attacks_arg(attacks, i, i) &&
				   !check_arg_attacks_set(attacks, i, tmp) &&
				   !check_set_attacks_arg(attacks, tmp, i)) {

			// SET_BIT(tmp, i);
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
				/*
				if (!is_set_consistent(attacks, current_closure)) {
					good = false;
					break;
				}
				*/
			}
			if (good) {
				list_free(tmp);
				free(current_bv);
				free(current_closure_bv);
				// printf("it is conflict-free\n");
				return(1);
			}
			// RESET_BIT(tmp, i);
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

	list_add(argument, current);

	START_TIMER(start_time);
	closure_semi_complete(attacks, attacked_by, current, current_closure, current_closure_bv);
	STOP_TIMER(stop_time);
	printf("closure time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	if (!is_set_conflict_free(attacks, current_closure)) {
		// closure has a conflict. complete extension does not exist.
		print_argumentation_framework(attacks);
		print_list(stdout, current_closure, "\n");
		printf("=== dc_co_next_closure finished 1===\n");
		return(NULL);
	}

	// Check if closure is self-defending
	START_TIMER(start_time);
	if (is_set_self_defending(attacks, attacked_by, current_closure)) {
		// closure is a complete extension containing the argument
		STOP_TIMER(stop_time);
		printf("is_set_self_defending time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
		printf("=== dc_co_next_closure finished 2===\n");
		return(current_closure);
	}
	// Not self-defending. That is, the given argument is not defended.
	list_reset(current);
	list_reset(current_closure);

	// Sort in descending order of victims
	// ...

	// Move defenders of the argument to the right-end.
	// ...

	// Now move the argument to the very left bit ...
	printf("Argument: %d\n", argument);
	swap_arguments(attacks, 0, argument);

	// recompute the attacked_by framework
	free_argumentation_framework(attacked_by);
	attacked_by = transpose_argumentation_framework(attacks);

	// add argument 0 to current
	list_add(0, current);

	int concept_count = 0;

	while (next_conflict_free_semi_complete_intent(attacks, attacked_by, current, current_closure)) {
		printf("current:");
		print_list(stdout, current, "\n");
		++concept_count;
		if (is_set_self_defending(attacks, attacked_by, current_closure)) {
			free(current_closure_bv);
			free_argumentation_framework(attacks);
			free_argumentation_framework(attacked_by);
			printf("=== dc_co_next_closure finished 3===\n");
			return(current_closure);
		}
		list_copy(current_closure, current);
	}

	printf("Number of concepts generated: %d\n", concept_count);

	free(current_closure_bv);
	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("=== dc_co_next_closure finished 4===\n");
	return(NULL);
}


ArrayList* dc_co_subgraph(AF* attacks, ARG_TYPE argument) {

	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	// extract the subgraph induced by the argument
	START_TIMER(start_time);
	Subgraph* subgraph = extract_subgraph_backwards(attacked_by, argument);
	printf("Subgraph size:%d\n", subgraph->af->size);
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

	// map indices of the computed extension back
	// ...
	ArrayList *mapped_extension = list_create();
	for (SIZE_TYPE i = 0; i < extension->size; ++i)
		list_add(subgraph->mapping_from_subgraph[extension->elements[i]], mapped_extension);

	// now close the mapped extension in the whole framework
	ArrayList* closure = list_create();
	bool* closure_bv = calloc(attacks->size, sizeof(bool));
	assert(closure_bv != NULL);
	closure_semi_complete(attacks, attacked_by, mapped_extension, closure, closure_bv);

	return(closure);
}