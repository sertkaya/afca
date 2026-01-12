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

#include "../utils/stack.h"
#include "../utils/timer.h"

#include "../af/af.h"
#include "node.h"
#include "closure.h"


// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

int closure_count = 0;


ArrayList* dc_co(AF* attacks, ARG_TYPE argument) {
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
	push(update, new_stack_element_int(argument));

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
		// first closure has a conflict. complete extension does not exist
		return(NULL);
	}

	// closure is self-defending, complete set found.
	if (is_set_self_defending(attacks, attacked_by, current_node->set)) {
		free_stack(&nodes);
		free_argumentation_framework(attacks);
		free_argumentation_framework(attacked_by);
		return(current_node->set);
	}

	push(&nodes, new_stack_element_ptr(current_node));

	int node_count = 0;
	while (current_node =  pop_ptr(&nodes)) {
		++node_count;
		// print_list(stdout, current_node->set,"\n");
		// find the unattacked attacker of current_node->set that has the smallest number of attackers, which are not
		// scheduled and are not conflicting with current_node->set.
		int min_attacker_count = attacks->size;
		ARG_TYPE least_attacked_attacker = -1;

		bool *attacker_processed = calloc(attacks->size, sizeof(bool));
		assert(attacker_processed != NULL);

		for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
			if (current_node->attackers[i] && !current_node->victims[i] && ! attacker_processed[i]) {
				attacker_processed[i] = true;
				if (current_node->allowed_attackers_count[i] < min_attacker_count) {
					min_attacker_count = current_node->allowed_attackers_count[i];
					least_attacked_attacker = i;
				}

			}
		}
		free(attacker_processed);


		// add unscheduled and non-conflicting attackers of least_attacked_attacker one by one and close.
		// if none of them leads to a solution, abandon that branch
		bool defendable = false;
		// printf("%d: ", least_attacked_attacker+1);
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			if (IS_IN_CONFLICT_WITH(attacker_of_least_attacked_attacker, current_node) || current_node->processed[attacker_of_least_attacked_attacker]) {
				// this attacker is already victim of current->set->set, or causes a conflict, or is already scheduled
				// so skip it
				continue;
			}
			// otherwise add it and close

			defendable = true;

			current_node->processed[attacker_of_least_attacked_attacker] = true;
			// attacker-of-least-attacked-attacker is processed,
			// decrement the allowed-attackers-counts of its victims
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[attacker_of_least_attacked_attacker]; ++j) {
				--(current_node->allowed_attackers_count[attacks->lists[attacker_of_least_attacked_attacker][j]]);
			}
			Node *child_node = duplicate_node(current_node, attacks->size);
			++child_node->depth;

			Stack *update = new_stack();
			push(update, new_stack_element_int(attacker_of_least_attacked_attacker));
			child_node = pseudo_complete(update, child_node, attacks, attacked_by);
			free_stack(update);
			// printf("%d ", attacker_of_least_attacked_attacker+1);
			// closure has a conflict. stop this branch, try with another attacker
			// of least_attacked_attacker
			if (!child_node) {
				// node "child_node" is already deleted in process_stack
				// upon noticing the conflict. not required here.
				// printf("closure has conflict\n");
				continue;
			}
			// closure is self-defending, complete set found.
			if (is_set_self_defending(attacks, attacked_by, child_node->set)) {
				free_stack(&nodes);
				free_argumentation_framework(attacks);
				free_argumentation_framework(attacked_by);
				printf("node count: %d\n", node_count);
				printf("node depth: %d\n", child_node->depth);
				printf("closure count: %d\n", closure_count);
				return(child_node->set);
			}

			push(&nodes, new_stack_element_ptr(child_node));
		}
		// printf("\n");
		// if (!defendable)
		// 	printf("set not defendable\n");
		delete_node(current_node);
		current_node = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("Closure count: %d\n", closure_count);
	printf("node count: %d\n", node_count);
	return(NULL);
}