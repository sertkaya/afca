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

#include "../af/af.h"
#include "complete.h"

#include "../utils/stack.h"
#include "../utils/priority_queue.h"
#include "../utils/timer.h"

struct node {
	ArrayList *set;
	bool *processed;
	// Number of unattacked attackers of an argument
	SIZE_TYPE* unattacked_attackers_count;
	// Number of attackers of an argument that do not attack this node
	SIZE_TYPE* not_attacker_of_current_count;
	// Number of allowed attackers of an argument
	SIZE_TYPE* allowed_attackers_count;
	bool* victims;
	bool* attackers;
	// depth of this node
	int depth;
};

typedef struct node Node;

Node *create_node(SIZE_TYPE size) {
	Node *s = calloc(1, sizeof(Node));
	assert(s != NULL);

	s->set = list_create();

	s->processed = calloc(size, sizeof(bool));
	assert(s->processed != NULL);

	s->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->unattacked_attackers_count != NULL);

	s->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->not_attacker_of_current_count != NULL);

	s->allowed_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->allowed_attackers_count != NULL);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	s->attackers = calloc(size, sizeof(bool));
	assert(s->attackers != NULL);

	s->depth = 0;

	return(s);
}

Node *duplicate_node(Node *s, SIZE_TYPE size) {
	Node *n = calloc(1, sizeof(Node));
	assert(n != NULL);

	n->set = list_duplicate(s->set);

	n->processed = calloc(size, sizeof(bool));
	assert(n->processed != NULL);
	memcpy(n->processed, s->processed, size * sizeof(bool));

	n->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->unattacked_attackers_count != NULL);
	memcpy(n->unattacked_attackers_count, s->unattacked_attackers_count, size * sizeof(SIZE_TYPE));

	n->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->not_attacker_of_current_count != NULL);
	memcpy(n->not_attacker_of_current_count, s->not_attacker_of_current_count, size * sizeof(SIZE_TYPE));

	n->allowed_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->allowed_attackers_count != NULL);
	memcpy(n->allowed_attackers_count, s->allowed_attackers_count, size * sizeof(SIZE_TYPE));

	n->victims = calloc(size, sizeof(bool));
	assert(n->victims != NULL);
	memcpy(n->victims, s->victims, size * sizeof(bool));

	n->attackers = calloc(size, sizeof(bool));
	assert(n->attackers != NULL);
	memcpy(n->attackers, s->attackers, size * sizeof(bool));

	n->depth = s->depth;

	return(n);
}

void delete_node(Node *s) {
	list_free(s->set);
	s->set = NULL;
	free(s->processed);
	s->processed = NULL;
	free(s->victims);
	s->victims = NULL;
	free(s->attackers);
	s->attackers = NULL;
	free(s->unattacked_attackers_count);
	s->unattacked_attackers_count = NULL;
	free(s->not_attacker_of_current_count);
	s->not_attacker_of_current_count = NULL;
	free(s->allowed_attackers_count);
	s->allowed_attackers_count = NULL;
	free(s);
}

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;

// is arg in conflict with node->set
#define IS_IN_CONFLICT_WITH(arg,node) (node->victims[arg]||node->attackers[arg])

Node *pseudo_complete(Stack *update, Node *node, AF *af, AF* af_t) {
	SIZE_TYPE a = -1;
	++closure_count;
	while ((a = pop_int(update)) != -1) {

		if (IS_IN_CONFLICT_WITH(a, node)) {
			delete_node(node);
			return(NULL);
		}

		node->processed[a] = true;
		list_add(a, node->set);

		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!node->victims[victim_a]) {
				node->victims[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(node->unattacked_attackers_count[victim_victim_a]);
					if (!node->attackers[victim_a]  && !node->processed[victim_a]) {
						--(node->allowed_attackers_count[victim_victim_a]);
					}
					if (!node->processed[victim_victim_a] && node->unattacked_attackers_count[victim_victim_a] == 0)  {
						// victim_victim_a is defended, push to the stack
						push(update, new_stack_element_int(victim_victim_a));
						node->processed[victim_victim_a] = true;
					}
				}
			}
		}

		for (SIZE_TYPE i = 0; i < af_t->list_sizes[a]; ++i) {
			SIZE_TYPE attacker_a = af_t->lists[a][i];
			if (!node->attackers[attacker_a]) {
				node->attackers[attacker_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[attacker_a]; ++j) {
					SIZE_TYPE victim_attacker_a = af->lists[attacker_a][j];
					--(node->not_attacker_of_current_count[victim_attacker_a]);
					if (!node->victims[attacker_a] && !node->processed[attacker_a]) {
						--(node->allowed_attackers_count[victim_attacker_a]);
					}
					if (!node->processed[victim_attacker_a] && node->not_attacker_of_current_count[victim_attacker_a] == 0) {
						// attackers of victim_attacker_a are all attackers of node->set, push it to the stack
						push(update, new_stack_element_int(victim_attacker_a));
						node->processed[victim_attacker_a] = true;
					}
				}
			}
		}

	}
	return(node);
}

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
		if (attacked_by->list_sizes[i] == 0) {
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
		print_list(stdout, current_node->set,"\n");
		// find the unattacked attacker of current_node->set that has the smallest number of attackers, which are not
		// scheduled and are not conflicting with current_node->set.
		int min_attacker_count = attacks->size;
		ARG_TYPE least_attacked_attacker = -1;

		bool *attacker_processed = calloc(attacks->size, sizeof(bool));
		assert(attacker_processed != NULL);

		for (SIZE_TYPE i = 0; i < current_node->set->size; ++i) {
			for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[current_node->set->elements[i]]; ++j) {
				ARG_TYPE attacker = attacked_by->lists[current_node->set->elements[i]][j];
				if (attacker_processed[attacker])
					continue;
				attacker_processed[attacker] = true;
				if (!current_node->victims[attacker]) {
					// attacker is unattacked
					if (current_node->allowed_attackers_count[attacker] < min_attacker_count) {
						min_attacker_count = current_node->allowed_attackers_count[attacker];
						least_attacked_attacker = attacker;
					}
				}
			}
		}
		free(attacker_processed);


		// add unscheduled and non-conflicting attackers of least_attacked_attacker one by one and close.
		// if none of them leads to a solution, abandon that branch
		bool defendable = false;
		printf("%d: ", least_attacked_attacker+1);
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			if (IS_IN_CONFLICT_WITH(attacker_of_least_attacked_attacker, current_node) || current_node->processed[attacker_of_least_attacked_attacker]) {
				// this attacker is already victim of current->set->set, or causes a conflict, or is already scheduled
				// so skip it
				continue;
			}
			// otherwise add it and close

			defendable = true;

			if (current_node->allowed_attackers_count[attacker_of_least_attacked_attacker] == 1) {
				printf("\n==>%d\n", attacker_of_least_attacked_attacker);
			}
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
			printf("%d ", attacker_of_least_attacked_attacker+1);
			// closure has a conflict. stop this branch, try with another attacker
			// of least_attacked_attacker
			if (!child_node) {
				// node "child_node" is already deleted in process_stack
				// upon noticing the conflict. not required here.
				printf("closure has conflict\n");
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
		printf("\n");
		if (!defendable)
			printf("set not defendable\n");
		delete_node(current_node);
		current_node = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	printf("Closure count: %d\n", closure_count);
	printf("node count: %d\n", node_count);
	return(NULL);
}