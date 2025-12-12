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
	bool *scheduled;
	bool *conflicts;
	// Number of unattacked attackers of an argument
	SIZE_TYPE* unattacked_attackers_count;
	// Number of attackers of an argument that do not attack this node
	SIZE_TYPE* not_attacker_of_current_count;
	bool* victims;
	bool* attackers;
};

typedef struct node Node;

Node *create_node(SIZE_TYPE size) {
	Node *s = calloc(1, sizeof(Node));
	assert(s != NULL);

	s->set = list_create();

	s->conflicts = calloc(size, sizeof(bool));
	assert(s->conflicts != NULL);

	s->scheduled = calloc(size, sizeof(bool));
	assert(s->scheduled != NULL);

	s->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->unattacked_attackers_count != NULL);

	s->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->not_attacker_of_current_count != NULL);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	s->attackers = calloc(size, sizeof(bool));
	assert(s->attackers != NULL);

	return(s);
}

Node *duplicate_node(Node *s, SIZE_TYPE size) {
	Node *n = calloc(1, sizeof(Node));
	assert(n != NULL);

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

	n->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->not_attacker_of_current_count != NULL);
	memcpy(n->not_attacker_of_current_count, s->not_attacker_of_current_count, size * sizeof(SIZE_TYPE));

	n->victims = calloc(size, sizeof(bool));
	assert(n->victims != NULL);
	memcpy(n->victims, s->victims, size * sizeof(bool));

	n->attackers = calloc(size, sizeof(bool));
	assert(n->attackers != NULL);
	memcpy(n->attackers, s->attackers, size * sizeof(bool));

	return(n);
}

void delete_node(Node *s) {
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
	free(s->not_attacker_of_current_count);
	s->not_attacker_of_current_count = NULL;
	free(s);
}

// A complete extension is an admissible extension that contains every argument that it defends.
// I suggest to use the name semi-complete extension for an extension that contains every argument that it defends.
// Semi-complete extensions form a closure system.

static int closure_count = 0;

Node *pseudo_complete(Stack *update, Node *node, AF *af, AF* af_t) {
	SIZE_TYPE a = -1;
	++closure_count;
	while ((a = pop_int(update)) != -1) {

		if (!node->scheduled[a]) {
			node->scheduled[a] = true;
			list_add(a, node->set);
			for (SIZE_TYPE k = 0; k < af->list_sizes[a]; ++k) {
				node->conflicts[af->lists[a][k]] = true;
			}
			for (SIZE_TYPE k = 0; k < af_t->list_sizes[a]; ++k) {
				node->conflicts[af_t->lists[a][k]] = true;
			}
		}
		if (node->conflicts[a]) {
			delete_node(node);
			return(NULL);
		}


		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!node->victims[victim_a]) {
				node->victims[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(node->unattacked_attackers_count[victim_victim_a]);
					if (!node->scheduled[victim_victim_a] && node->unattacked_attackers_count[victim_victim_a] == 0)  {
						// check if victim_victim_a causes a conflict
						// if (node->conflicts[victim_victim_a]) {
						// 	delete_node(node);
						// 	return(NULL);
						// }
						push(update, new_stack_element_int(victim_victim_a));

						// node->scheduled[victim_victim_a] = true;
						// for (SIZE_TYPE k = 0; k < af->list_sizes[victim_victim_a]; ++k) {
						// 	node->conflicts[af->lists[victim_victim_a][k]] = true;
						// }
						// for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k) {
						// 	node->conflicts[af_t->lists[victim_victim_a][k]] = true;
						// }
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
					if (!node->scheduled[victim_attacker_a] && node->not_attacker_of_current_count[victim_attacker_a] == 0) {
						// victim_attacker_a cannot cause a conflict in current_node. Its attackers all contained in current_node.
						// If there were was a conflict, we would return NULL above.
						// printf("%d %d\n", victim_attacker_a, af_t->list_sizes[victim_attacker_a]);
						push(update, new_stack_element_int(victim_attacker_a));

						// node->scheduled[victim_attacker_a] = true;
						// for (SIZE_TYPE k = 0; k < af->list_sizes[victim_attacker_a]; ++k) {
						// 	node->conflicts[af->lists[victim_attacker_a][k]] = true;
						// }
						// for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_attacker_a]; ++k) {
						// 	node->conflicts[af_t->lists[victim_attacker_a][k]] = true;
						// }
					}
				}
			}
		}

	}
	// printf("%d %d %d\n", next->set->size, attacker_count, unattacked_count);
	return(node);
}

// careful: not only called for the very "first" closure.
// s: the set to be closed
Node *first_closure(AF *af, AF *af_t, ArrayList *s) {
	Stack *update = new_stack();
	Node *root_node = create_node(af->size);

	++closure_count;

	// Push elements of s to the stack, mark them as scheduled
	for (SIZE_TYPE i = 0; i < s->size; ++i) {
		// This check is obsolete. Will never be true.
		// if (root_node->scheduled[s->elements[i]])
		// 	continue;

		// for (SIZE_TYPE j = 0; j < af->list_sizes[s->elements[i]]; ++j) {
		// 	root_node->conflicts[af->lists[s->elements[i]][j]] = true;
		// }
		// for (SIZE_TYPE j = 0; j < af_t->list_sizes[s->elements[i]]; ++j) {
		// 	root_node->conflicts[af_t->lists[s->elements[i]][j]] = true;
		// }

		// Will hold iff the single element of s attacks itself
		// Do this check instead in dc-co
		if (root_node->conflicts[s->elements[i]]) {
			delete_node(root_node);
			return(NULL);
		}

		push(update, new_stack_element_int(s->elements[i]));
		// root_node->scheduled[s->elements[i]] = true;
	}

	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
		if (af_t->list_sizes[i] == 0) {
			if (root_node->scheduled[i])
				continue;

			if (root_node->conflicts[i]) {
				delete_node(root_node);
				return(NULL);
			}

			push(update, new_stack_element_int(i));

			// root_node->scheduled[i] = true;
			// for (SIZE_TYPE j = 0; j < af->list_sizes[i]; ++j) {
			// 	root_node->conflicts[af->lists[i][j]] = true;
			// }
		}
	}

	memcpy(root_node->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	memcpy(root_node->not_attacker_of_current_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
	root_node = pseudo_complete(update, root_node, af, af_t);
	free_stack(update);

	return(root_node);
}

/*
Node *incremental_closure(AF* af, AF* af_t, ARG_TYPE new_argument, Node *current_node) {
	Stack *update = new_stack();

	// current_node->scheduled[new_argument] = true;
	Node *child_node = duplicate_node(current_node, af->size);

	++closure_count;
	// Push the new argument to the stack
	push(update, new_stack_element_int(new_argument));

	// for (SIZE_TYPE j = 0; j < af->list_sizes[new_argument]; ++j) {
	// 	child_node->conflicts[af->lists[new_argument][j]] = true;
	// }
	// for (SIZE_TYPE j = 0; j < af_t->list_sizes[new_argument]; ++j) {
	// 	child_node->conflicts[af_t->lists[new_argument][j]] = true;
	// }

	child_node = pseudo_complete(update, child_node, af, af_t);

	free_stack(update);

	return(child_node);
}
*/

ArrayList* dc_co(AF* attacks, ARG_TYPE argument) {
	struct timeval start_time, stop_time;

	AF* attacked_by = transpose_argumentation_framework(attacks);

	Stack nodes;
	init_stack(&nodes);

	// ArrayList *tmp = list_create();
	// list_add(argument, tmp);
	Stack *update = new_stack();
	// Push the given argument to the stack
	push(update, new_stack_element_int(argument));
	// Push the unattacked arguments to the stack. They are defended by every set.
	for (SIZE_TYPE i = 0; i < attacked_by->size; ++i) {
		if (attacked_by->list_sizes[i] == 0) {
			push(update, new_stack_element_int(i));
		}
	}
	// The root node
	Node *current_node = create_node(attacks->size);
	memcpy(current_node->unattacked_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	memcpy(current_node->not_attacker_of_current_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	// First closure.
	current_node = pseudo_complete(update, current_node, attacks, attacked_by);
	free_stack(update);

	// Node *current_node = first_closure(attacks, attacked_by, tmp);
	// list_free(tmp);

	if (!current_node) {
		// first closure has a conflict. complete extension does not exist.
		// printf("Closure count: %d\n", closure_count);
		return(NULL);
	}
	push(&nodes, new_stack_element_ptr(current_node));

	while (current_node =  pop_ptr(&nodes)) {
		// print_list(stdout, current_node->set,"\n");
		// find the unattacked attacker of current_node->set that has the smallest number of attackers, which are not
		// scheduled and are not conflicting with current_node->set.
		int min_attacker_count = attacks->size;
		ARG_TYPE least_attacked_attacker = -1;

		bool *attacker_processed = calloc(attacks->size, sizeof(bool));
		assert(attacker_processed != NULL);

		bool is_current_self_defending = true;
		for (SIZE_TYPE i = 0; i < current_node->set->size; ++i) {
			for (SIZE_TYPE j = 0; j < attacked_by->list_sizes[current_node->set->elements[i]]; ++j) {
				ARG_TYPE attacker = attacked_by->lists[current_node->set->elements[i]][j];
				if (attacker_processed[attacker])
					continue;
				attacker_processed[attacker] = true;
				if (!current_node->victims[attacker]) {
					is_current_self_defending = false;
					int count = 0;
					// attacker is unattacked
					for (SIZE_TYPE k = 0; k < attacked_by->list_sizes[attacker]; ++k) {
						ARG_TYPE unattacked_attacker_attacker = attacked_by->lists[attacker][k];
						if (!current_node->scheduled[unattacked_attacker_attacker] &&
							!current_node->conflicts[unattacked_attacker_attacker]) {
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
			// current_node->set is self-defending, not necessarily complete.
			// close it and return.
			// TODO: Check why this closure is needed. current_node->set should actually be closed
			// Node *result = first_closure(attacks, attacked_by, current_node->set);
			// printf("Closure count: %d\n", closure_count);
			// return(result->set);
			return(current_node->set);
		}

		// add unscheduled and non-conflicting attackers of least_attacked_attacker one by one and close.
		// if none of them leads to a solution, abandon that branch
		for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
			ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
			if (current_node->conflicts[attacker_of_least_attacked_attacker] ||
				current_node->scheduled[attacker_of_least_attacked_attacker]) {
				// this attacker is already victim of current->set->set, or causes a conflict, or is already scheduled
				// so skip it
				continue;
			}
			// otherwise add it and close
			// Node *child_node = incremental_closure(attacks, attacked_by, attacker_of_least_attacked_attacker, current_node);
			Node *child_node = duplicate_node(current_node, attacks->size);
			Stack *update = new_stack();
			push(update, new_stack_element_int(attacker_of_least_attacked_attacker));
			child_node = pseudo_complete(update, child_node, attacks, attacked_by);
			free_stack(update);

			// closure has a conflict, try with another attacker
			// of least_attacked_attacker
			if (!child_node) {
				// node "child_node" is already deleted in process_stack
				// upon noticing the conflict. not required here.
				continue;
			}

			push(&nodes, new_stack_element_ptr(child_node));
		}
		delete_node(current_node);
		current_node = NULL;
	}

	free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);

	// printf("Closure count: %d\n", closure_count);
	return(NULL);
}