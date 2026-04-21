/*
 * CLAS - Closure based Argumentation Solver
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

#include "../utils/stack.h"
#include "../utils/timer.h"

#include "../af/af.h"
#include "node.h"
#include "closure.h"
#include "complete.h"

// A stable extension is an admissible extension that attacks every argument outside itself.
// A stable extension is a complete extension, which is a pseudo-complete set.
// A set is pseudo-complete if it is semi-complete and quasi-complete.
// A set is semi-complete if it is conflict-free and contains every argument that it defends.
// A set S is quasi-complete if it is conflict-free and contains every argument that is attacked
// by only attackers of S.
// Pseudo-complete sets form a closure system.

extern int closure_count;

/*
// ArrayList* dc_st(AF* attacks, ARG_TYPE argument) {
bool* dc_st_2(AF* attacks, ARG_TYPE argument) {
	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	// check for a complete extension containing the argument
	bool *ce = dc_co(attacks, argument);
	if (!ce) {
		// complete extension and hence stable extension does not exist
		return(NULL);
	}

	// ce is a complete extension containing the argument

	AF *rf = attacks;
	SIZE_TYPE rf_size = attacks->size;
	do {
		// TODO: reconsider the data type for extensions. currently bool*
		SIZE_TYPE ce_size = 0;
		for (SIZE_TYPE i = 0; i < rf_size; ++i)
			if (ce[i])
				++ce_size;
		ARG_TYPE *tmp = calloc(ce_size, sizeof(ARG_TYPE));
		assert(tmp != NULL);
		SIZE_TYPE j = 0;
		for (SIZE_TYPE i = 0; i < rf_size; ++i) {
			if (ce[i]) {
				tmp[j] = i;
				++j;
			}
		}
		// TODO: improve until here

		// extract the residual framework
		rf = extract_residual_framework(rf, tmp, ce_size);
		rf_size = rf->size;
		// printf("%d %d\n", ce_size, rf_size);
		printf("ce:");
		for (SIZE_TYPE i = 0; i < rf_size; ++i) {
			if (ce[i])
				printf("%d ", i + rf->offsets[i]);
		}
		printf("\n");
		if (rf_size == 0)
			break;
		ce = dc_co(rf, 0);
		free(tmp);
		printf("-------------\n");
	} while (rf_size != 0);

	return(NULL);
}
*/

bool* dc_st(AF* attacks, ARG_TYPE argument) {
	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	Stack nodes;
	init_stack(&nodes);

	// The root node
	Node *current_node = create_node(attacks->size);
	memcpy(current_node->unattacked_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	memcpy(current_node->not_attacker_of_current_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	memcpy(current_node->allowed_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));

	Stack *update = new_stack();
	// Push the given argument to the stack
	if (argument < attacks->size) {
		push(update, new_stack_element_int(argument));
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i) {
		if ((attacked_by->list_sizes[i] == 0) && (i != argument)) {
			push(update, new_stack_element_int(i));
		}
	}
	// First closure.
	current_node = pseudo_complete(update, current_node, attacks, attacked_by);
	free_stack(update);

	if (!current_node) {
		// first closure has a conflict. stable extension does not exist
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	push(&nodes, new_stack_element_ptr(current_node));

	// the candidate arguments to add to the node before closing it
	// The set "C" in the algorithm
	ARG_TYPE *candidate_arguments = calloc(attacks->size, sizeof(ARG_TYPE));
	assert(candidate_arguments != NULL);

	int node_count_stable = 0;
	while (current_node =  pop_ptr(&nodes)) {
		++node_count_stable;
		// print the current_node->set
		/*
		for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
			if (current_node->set[i])
				printf("%d ", i+1);
		}
		printf("\n");
		*/

		SIZE_TYPE candidate_count = 0;
		if (is_node_self_defending(current_node, attacks)) {
			if (node_attacks_everything_outside(current_node, attacks)) {
				// printf("stable\n");
				// set is self-defending and attacks everything outside it:
				// stable extension found, return
				free_stack(&nodes);
				free(candidate_arguments);
				// free_argumentation_framework(attacks);
				free_argumentation_framework(attacked_by);
				printf("Closure count: %d\n", closure_count);
				printf("node count: %d\n", node_count_stable);
				return(current_node->set);
			}
			// printf("self-defending\n");
			// self-defending but does not attack everything outside it:
			// find an unattacked argument that is not contained in the set and
			// has the minimum number of allowed attackers
			SIZE_TYPE min_candidate_count = attacks->size;
			ARG_TYPE candidate_argument = -1;
			for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
				if (!current_node->set[i] && !current_node->victims[i]) {
					// the argument with the minimum number of allowed attackers
					if (current_node->allowed_attackers_count[i] <= min_candidate_count) {
						min_candidate_count = current_node->allowed_attackers_count[i];
						candidate_argument = i;
					}
				}
			}
			// add this argument to the candidates
			candidate_arguments[candidate_count] = candidate_argument;
			++candidate_count;
			// add its allowed attackers to the candidates
			for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[candidate_argument]; ++i) {
				if (!current_node->processed[attacked_by->lists[candidate_argument][i]] && !IS_IN_CONFLICT_WITH(attacked_by->lists[candidate_argument][i], current_node)) {
					candidate_arguments[candidate_count] = attacked_by->lists[candidate_argument][i];
					++candidate_count;
				}
			}
		}
		else {
			// current_node is not self-defending. find the unattacked attacker with the smallest
			// number of allowed attackers.
			// printf("not self-defending\n");
			int min_attacker_count = attacks->size;
			ARG_TYPE least_attacked_attacker = -1;

			for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
				if (current_node->attackers[i] && !current_node->victims[i]) {
					if (current_node->allowed_attackers_count[i] < min_attacker_count) {
						min_attacker_count = current_node->allowed_attackers_count[i];
						least_attacked_attacker = i;
					}

				}
			}
			// add the allowed attackers of least_attacked_attacker to the candidates
			for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
				ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
				if (!IS_IN_CONFLICT_WITH(attacker_of_least_attacked_attacker, current_node) && !current_node->processed[attacker_of_least_attacked_attacker]) {
					// attacker is allowed, add to candidates
					// candidate_arguments[attacker_of_least_attacked_attacker] = true;
					candidate_arguments[candidate_count] = attacker_of_least_attacked_attacker;
					++candidate_count;
				}
			}
		}

		// now add the candidates one by one and close
		// if none of them leads to a solution, abandon that branch
		// printf("%d\n", candidate_count);
		for (SIZE_TYPE i = 0; i < candidate_count; ++i) {
			current_node->processed[candidate_arguments[i]] = true;
			// candidate is processed,
			// decrement the allowed-attackers-counts of its victims
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[candidate_arguments[i]]; ++j) {
				--(current_node->allowed_attackers_count[attacks->lists[candidate_arguments[i]][j]]);
			}
			Node *child_node = duplicate_node(current_node, attacks->size);
			++child_node->depth;

			Stack *update = new_stack();
			push(update, new_stack_element_int(candidate_arguments[i]));
			child_node = pseudo_complete(update, child_node, attacks, attacked_by);
			free_stack(update);

			// closure has a conflict. stop this branch, try with another candidate
			if (!child_node) {
				// node "child_node" is already deleted in process_stack
				// upon noticing the conflict. not required to delete here.
				continue;
			}
			if (is_node_self_defending(child_node, attacks) && node_attacks_everything_outside(child_node, attacks)) {
				// closure is self-defending and attacks everything outside it
				// stable extension is found
				free_stack(&nodes);
				free(candidate_arguments);
				// free_argumentation_framework(attacks);
				free_argumentation_framework(attacked_by);
				printf("node count: %d\n", node_count_stable);
				printf("node depth: %d\n", child_node->depth);
				printf("closure count: %d\n", closure_count);
				return(child_node->set);
			}
			push(&nodes, new_stack_element_ptr(child_node));
		}
		delete_node(current_node);
		current_node = NULL;
	}

	// free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("Closure count: %d\n", closure_count);
	printf("node count: %d\n", node_count_stable);
	return(NULL);
}


bool* se_st(AF* attacks) {
	bool* res = dc_st(attacks, attacks->size);
	return res;
}