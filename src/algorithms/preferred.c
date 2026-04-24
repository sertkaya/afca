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
#include "preferred.h"

#include "node.h"
#include "closure.h"
#include "complete.h"

// A preferred extension is a maximal admissible set. Every preferred extension is pseudo-complete.
// A set is pseudo-complete if it is semi-complete and quasi-complete.
// A set is semi-complete if it is conflict-free and contains every argument that it defends.
// A set S is quasi-complete if it is conflict-free and contains every argument that is attacked
// by only attackers of S.
// Pseudo-complete sets form a closure system.

extern int closure_count;

bool *se_pr_camera_ready_kr26(AF* attacks) {
	AF* attacked_by = transpose_argumentation_framework(attacks);

	Stack nodes;
	init_stack(&nodes);

	// The root node
	Node *current_node = create_node(attacks->size);
	memcpy(current_node->unattacked_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	memcpy(current_node->not_attacker_of_current_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));
	memcpy(current_node->allowed_attackers_count, attacked_by->list_sizes, attacked_by->size * sizeof(SIZE_TYPE));

	Stack *update = new_stack();

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
		// first closure has a conflict. preferred extension does not exist
		printf("Closure count: %d\n", closure_count);
		return(NULL);
	}

	for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
		if (!current_node->set[i] && !current_node->processed[i] && !IS_IN_CONFLICT_WITH(i, current_node)) {
			// add this argument to the candidates
			ARG_TYPE *tmp = realloc(current_node->candidate_arguments, sizeof(ARG_TYPE) * (current_node->candidate_arguments_count + 1));
			assert(tmp != NULL);
			current_node->candidate_arguments = tmp;
			current_node->candidate_arguments[current_node->candidate_arguments_count] = i;
			++current_node->candidate_arguments_count;
		}
	}
	push(&nodes, new_stack_element_ptr(current_node));

	// the candidate arguments to add to the node before closing it
	// The set "C" in the algorithm

	int node_count_preferred = 0;
	while (current_node =  top_ptr(&nodes)) {
		if (current_node->candidate_arguments_count == 0) {
			if (is_node_self_defending(current_node, attacks)) {
				/*
				free_stack(&nodes);
				// free_argumentation_framework(attacks);
				free_argumentation_framework(attacked_by);
				printf("node count: %d\n", node_count_preferred);
				printf("node depth: %d\n", current_node->depth);
				printf("closure count: %d\n", closure_count);
				return(current_node->set);
				*/
				break;
			}
			current_node = pop_ptr(&nodes);
			delete_node(current_node);
			current_node = NULL;
		}
		else {
			++node_count_preferred;

			// ARG_TYPE candidate = current_node->candidate_arguments[--current_node->candidate_arguments_count];
			--current_node->candidate_arguments_count;
			printf("cand. count: %d\n", current_node->candidate_arguments_count);
			ARG_TYPE candidate = current_node->candidate_arguments[current_node->candidate_arguments_count];
			ARG_TYPE *tmp = realloc(current_node->candidate_arguments, sizeof(ARG_TYPE) * current_node->candidate_arguments_count);
			// assert(tmp != NULL);
			current_node->candidate_arguments = tmp;

			current_node->processed[candidate] = true;
			// candidate is processed,
			// decrement the allowed-attackers-counts of its victims
			for (SIZE_TYPE j = 0; j < attacks->list_sizes[candidate]; ++j) {
				--(current_node->allowed_attackers_count[attacks->lists[candidate][j]]);
			}
			Node *child_node = duplicate_node(current_node, attacks->size);
			++child_node->depth;
			Stack *update = new_stack();
			push(update, new_stack_element_int(candidate));
			child_node = pseudo_complete(update, child_node, attacks, attacked_by);
			free_stack(update);

			if (!child_node) {
				// closure has a conflict. stop this branch, try with another candidate
				// node "child_node" is already deleted in pseudo_complete
				// upon noticing the conflict. not required to delete here.
				continue;
			}

			if (is_node_self_defending(child_node, attacks)) {
				// printf("self-defending\n");
				for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
					if (!child_node->set[i] && !child_node->processed[i] && !IS_IN_CONFLICT_WITH(i, child_node)) {
						// add this argument to the candidates
						ARG_TYPE *tmp = realloc(child_node->candidate_arguments, sizeof(ARG_TYPE) * (child_node->candidate_arguments_count + 1));
						assert(tmp != NULL);
						child_node->candidate_arguments = tmp;
						child_node->candidate_arguments[child_node->candidate_arguments_count] = i;
						++child_node->candidate_arguments_count;
					}
				}
			}
			else {
				// child_node is not self-defending. find the unattacked attacker with the smallest
				// number of allowed attackers.
				// printf("not self-defending\n");
				int min_attacker_count = attacks->size;
				ARG_TYPE least_attacked_attacker = -1;

				for (SIZE_TYPE i = 0; i < attacks->size; ++i) {
					if (child_node->attackers[i] && !child_node->victims[i]) {
						if (child_node->allowed_attackers_count[i] < min_attacker_count) {
							min_attacker_count = child_node->allowed_attackers_count[i];
							least_attacked_attacker = i;
						}
					}
				}
				// add the allowed attackers of least_attacked_attacker to the candidates
				for (SIZE_TYPE i = 0; i < attacked_by->list_sizes[least_attacked_attacker]; ++i) {
					ARG_TYPE attacker_of_least_attacked_attacker = attacked_by->lists[least_attacked_attacker][i];
					if (!IS_IN_CONFLICT_WITH(attacker_of_least_attacked_attacker, child_node) && !child_node->processed[attacker_of_least_attacked_attacker]) {
						// attacker is allowed, add to candidates
						// candidate_arguments[attacker_of_least_attacked_attacker] = true;
						ARG_TYPE *tmp = realloc(child_node->candidate_arguments, sizeof(ARG_TYPE) * (child_node->candidate_arguments_count + 1));
						assert(tmp != NULL);
						child_node->candidate_arguments = tmp;
						child_node->candidate_arguments[child_node->candidate_arguments_count] = i;
						++child_node->candidate_arguments_count;
					}
				}
			}
			push(&nodes, new_stack_element_ptr(child_node));
		}
	}

	// free_argumentation_framework(attacks);
	free_argumentation_framework(attacked_by);
	printf("node count: %d\n", node_count_preferred);
	printf("node depth: %d\n", current_node->depth);
	printf("closure count: %d\n", closure_count);
	bool* extension = current_node->set;
	free_stack(&nodes);
	return(extension);
}

bool* se_pr(AF* af) {
	struct timeval start_time, stop_time;

	// Array of AFs. The original AF and the residiual AFs generated in the loop are stored here
	// for later mapping the indices to the original indices.
	AF **afs = calloc(1, sizeof(AF*));
	assert(afs != NULL);
	SIZE_TYPE af_count = 1;
	afs[af_count - 1] = af;

	// the result
	bool *pe = calloc(af->size, sizeof(bool));
	assert(pe != NULL);

	while (afs[af_count - 1]->size > 0) {
		AF *sc = extract_source_component(afs[af_count - 1]);
		// printf("source component size:%d\n", sc->size);
		// print_argumentation_framework(sc);
		// printf("****\n");

		bool *ce = NULL;

		// look for a non-empty complete extension
		/*
		for (SIZE_TYPE i = 0; i < sc->size; ++i) {
			ce = dc_co(sc, i);
			if (ce != NULL) {
				break;
			}
		}
		*/
		ce = ne_co(sc);

		if (ce == NULL) {
			// bitvector for arguments in the source component
			bool *sc_v = calloc(afs[af_count - 1]->size, sizeof(bool));
			for (SIZE_TYPE i = 0; i < sc->size; ++i) {
				sc_v[sc->mapping[i]] = true;
			}
			// bitvector for victims of the source component
			bool *v = calloc(afs[af_count-1]->size, sizeof(bool));
			assert(v != NULL);
			// mark the victims of the source component that are outside the source component
			for (SIZE_TYPE i = 0; i < sc->size; ++i) {
				for (SIZE_TYPE j = 0; j < afs[af_count-1]->list_sizes[sc->mapping[i]]; ++j)
					// if the victim is not in the source component, mark it.
					if (!sc_v[afs[af_count-1]->lists[sc->mapping[i]][j]])
						v[afs[af_count-1]->lists[sc->mapping[i]][j]] = true;
			}
			// add loops to the marked victims
			for (SIZE_TYPE i = 0; i < afs[af_count-1]->size; ++i) {
				if (v[i]) {
					add_attack(afs[af_count - 1], i, i);
				}
			}
			free(v);
			free(sc_v);

			bool *remove = calloc(afs[af_count - 1]->size, sizeof(bool));
			assert(remove != NULL);
			for (SIZE_TYPE i = 0; i < sc->size; ++i) {
				remove[sc->mapping[i]] = true;
			}
			++af_count;
			AF **tmp = realloc(afs, (af_count) * sizeof(AF*));
			assert(tmp != NULL);
			afs = tmp;
			afs[af_count - 1] = extract_residual_framework(afs[af_count - 2], remove, sc->size);
			// printf("residual size: %d\n", afs[af_count - 1]->size);
			free(remove);
			// TODO: update P
		}
		else {
			Stack *update = new_stack();
			// Push the arguments in ce to the stack.
			for (SIZE_TYPE i = 0; i < sc->size; ++i)
				if (ce[i]) {
					push(update, new_stack_element_int(sc->mapping[i]));
				}

			// Push the unattacked arguments to the stack. They are defended by every set.
			AF *af_t = transpose_argumentation_framework(afs[af_count - 1]);
			for (SIZE_TYPE i = 0; i < af_t->size; ++i) {
				if (af_t->list_sizes[i] == 0) {
					push(update, new_stack_element_int(i));
				}
			}
			// Compute closure in afs[af_count], not in the sc
			Node *n = create_node(afs[af_count - 1]->size);
			memcpy(n->unattacked_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			memcpy(n->not_attacker_of_current_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			memcpy(n->allowed_attackers_count, af_t->list_sizes, af_t->size * sizeof(SIZE_TYPE));
			n = pseudo_complete(update, n, afs[af_count - 1], af_t);
			free_stack(update);
			// prepare arguments to be removed
			SIZE_TYPE size_remove = 0;
			bool *remove = calloc(afs[af_count - 1]->size, sizeof(bool));
			assert(remove != NULL);
			for (SIZE_TYPE i = 0; i < afs[af_count - 1]->size; ++i) {
				// arguments in the closure will be removed from afs[af_count]
				if (n->set[i]) {
					++size_remove;
					remove[i] = true;
					SIZE_TYPE back_mapped = i;
					for (int j = af_count - 1; j > 0; --j) {
						back_mapped = afs[j]->mapping[back_mapped];
					}
					// printf("%d ", back_mapped + 1);
					pe[back_mapped] = true;
				}
				// also the victims will be removed
				if (n->victims[i]) {
					++size_remove;
					remove[i] = true;
				}
			}

			delete_node(n);
			++af_count;
			AF **tmp = realloc(afs, af_count  * sizeof(AF*));
			assert(tmp != NULL);
			afs = tmp;
			afs[af_count - 1] = extract_residual_framework(afs[af_count - 2], remove, size_remove);
			// printf("residual size: %d\n", afs[af_count - 1]->size);
			free(remove);
		}
		free_argumentation_framework(sc);
	}
	// printf("number of residual frameworks: %d\n", af_count);
	// free the generated afs.
	// the original one will be freed later in main.
	for (SIZE_TYPE i = 1; i < af_count; ++i)
		free_argumentation_framework(afs[i]);

	return(pe);
}